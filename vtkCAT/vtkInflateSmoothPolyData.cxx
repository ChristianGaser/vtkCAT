/*=========================================================================

  Program:  Visualization Toolkit
  Module:   $RCSfile: vtkInflateSmoothPolyData.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

    This software is distributed WITHOUT ANY WARRANTY; without even
    the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
    PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

/* Most of the code is used from Surefit (inflate_surface.cxx)  */

#include "vtkInflateSmoothPolyData.h"

#include <float.h>

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkFloatArray.h"
#include "vtkFieldData.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkPolyDataNormals.h"
#include "vtkPolyDataReader.h"
#include "vtkPolyDataWriter.h"
#include "vtkSmoothPolyDataFilter.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTriangleFilter.h"
#include "vtkTransform.h"
#include "vtkCleanPolyData.h"

#include "vtkCaretTgtSmoothPolyDataFilter.h"
#include "vtkEllipsoidProjectPolyDataFilter.h"
#include "vtkPolyDataDistortion.h"
#include "vtkPolyDataCurvature.h"
#include "vtkUtilities.h"

vtkCxxRevisionMacro(vtkInflateSmoothPolyData, "$Revision: 1.00 $");
vtkStandardNewMacro(vtkInflateSmoothPolyData);

// The following code defines methods for the vtkInflateSmoothPolyData class
//

// Construct object with number of iterations 20; relaxation factor .01;
// feature edge smoothing turned off; feature 
// angle 45 degrees; edge angle 15 degrees; and boundary smoothing turned 
// on. Error scalars and vectors are not generated (by default). The 
// convergence criterion is 0.0 of the bounding box diagonal.
vtkInflateSmoothPolyData::vtkInflateSmoothPolyData()
{
  this->RegularSmoothingStrength = 1.0;
  this->RegularSmoothingIterations = 50;
  this->CompressStretchThreshold = 1.0;
  this->FingerSmoothingIterations = 100;
  this->FingerSmoothingStrength = 1.0;
  this->InflationFactor = 1.0;
  this->NumberOfSmoothingCycles = 3;
  this->GenerateAverageCompressedStretched = 0;

}

   
int vtkInflateSmoothPolyData::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkPolyData *input = vtkPolyData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkDebugMacro(
   << "\n\tNumber of Smoothing Cycles: " << this->NumberOfSmoothingCycles << endl
   << "\tRegular Smoothing Strength: " << this->RegularSmoothingStrength << endl
   << "\tRegular Smoothing Iterations: " << this->RegularSmoothingIterations << endl
   << "\tInflation Factor: " << this->InflationFactor << endl
   << "\tFinger Smoothing Threshold: " << this->CompressStretchThreshold << endl
   << "\tFinger Smoothing Strength: " << this->FingerSmoothingStrength << endl
   << "\tFinger Smoothing Iterations: " << this->FingerSmoothingIterations << endl
   << "\tAverage compressed/stretched Values " << (this->GenerateAverageCompressedStretched ? "On\n" : "Off\n"));

  if ( this->NumberOfSmoothingCycles <= 0 || this->RegularSmoothingStrength == 0.0) 
   { //don't do anything! pass data through
   output->CopyStructure(input);
   output->GetPointData()->PassData(input->GetPointData());
   output->GetCellData()->PassData(input->GetCellData());
   return 1;
   }

  // get center of input surface
  double center[3];
  vtkUtilities::GetPointsCenter(input, center);
     
  // Step 2: Translate AUX coord origin to center of gravity  
  vtkTransform* inputTransform = vtkTransform::New();
  inputTransform->Translate(-center[0], -center[1], -center[2]);
   
  // replace triangle strips with triangles
  vtkTriangleFilter *inputTriangleFilter = vtkTriangleFilter::New();
  inputTriangleFilter->SetInput(input);
   
  vtkTransformPolyDataFilter* inputTransformPoly = vtkTransformPolyDataFilter::New();
  inputTransformPoly->SetInput(inputTriangleFilter->GetOutput());
  inputTransformPoly->SetTransform(inputTransform);
  inputTransformPoly->Update();
  inputTransform->Delete();
  inputTriangleFilter->Delete();
   
  double inputSurfaceArea = vtkUtilities::GetSurfaceArea(inputTransformPoly->GetOutput());
  
  vtkPolyData* inputSurface = inputTransformPoly->GetOutput();
        
  // Step 4: Determine extents in X, Y, and Z
  double refBounds[6];
  vtkPolyData* refInput = inputTransformPoly->GetOutput();
  refInput->GetBounds(refBounds);
   
  const int numberOfPoints = refInput->GetNumberOfPoints();

  const double minX = floor(refBounds[0]);
  const double maxX = ceil(refBounds[1]);
  const double diffX = maxX - minX;
  const double minY = floor(refBounds[2]);
  const double maxY = ceil(refBounds[3]);
  const double diffY = maxY - minY;
  const double minZ = floor(refBounds[4]);
  const double maxZ = ceil(refBounds[5]);
  const double diffZ = maxZ - minZ;
   
  double* averageCompressedStretched = new double[numberOfPoints];
  double* maximumLinearDistortion    = new double[numberOfPoints];
  double* averageArealCompression    = new double[numberOfPoints];
  double* compressedStretched        = new double[numberOfPoints];

  // Step 5: Calculate surface area of fiducial surface
  const double refSurfaceArea = vtkUtilities::GetSurfaceArea(refInput);
  
  double surfaceAreaRatio = 0.0;
  
  // Step 6: Main Smoothing Cycle
  // Note: No smoothing takes place in the "+1" cycle, just metric calculation
  int* needSmoothing = new int[numberOfPoints];
  vtkCaretTgtSmoothPolyDataFilter* polySmooth1 = NULL;
  vtkCaretTgtSmoothPolyDataFilter* polySmooth2 = NULL;
  vtkEllipsoidProjectPolyDataFilter* ellipse = NULL;
  vtkPolyDataDistortion* polyDistort = NULL;

  for (int cycles = 0; cycles < (NumberOfSmoothingCycles + 1); cycles++) {
    
   // I have tried to only initialize the parameters outside the loop but
   // updating in the pipeline is not working
   polySmooth1 = vtkCaretTgtSmoothPolyDataFilter::New();
   polySmooth2 = vtkCaretTgtSmoothPolyDataFilter::New();
   ellipse = vtkEllipsoidProjectPolyDataFilter::New();
   polyDistort = vtkPolyDataDistortion::New();
   
   cout << "Inflate/Smooth Fingers: Cycle " << cycles << " --------------" << endl;
   // Step 6a: Apply Smoothing to AUX coord
   //  Caret Menu: Operate->Smoothing->Smoothing...
   //cout << "Inflate/Smooth Fingers: Normal Smoothing" << endl;
   
   if (cycles < NumberOfSmoothingCycles) {
     // Step 6a: Apply Smoothing to AUX coord
     //  Caret Menu: Operate->Smoothing->Smoothing...      	
     polySmooth1->SetInput(inputSurface);
     polySmooth1->SetNumberOfIterations(RegularSmoothingIterations);
     polySmooth1->SetRelaxationFactor(RegularSmoothingStrength);
     polySmooth1->Update();
      
     // Step 6b: Incrementally Inflate AUX Surface by Ellipsoidal Projection
     ellipse->SetEllipsoidDimensions(diffX, diffY, diffZ);
     ellipse->SetInflationFactor(InflationFactor);
     ellipse->SetInput(polySmooth1->GetOutput());
     ellipse->Update();
     inputSurface = ellipse->GetOutput();
   }
   
   // Step 6c: Calculate surface area of AUX Surface
   inputSurfaceArea = vtkUtilities::GetSurfaceArea(inputSurface);
   surfaceAreaRatio = inputSurfaceArea / refSurfaceArea;
   
   // calculate areal and linear distortion
   polyDistort->SetInput(inputSurface);
   polyDistort->SetDistortionNumeratorPolyData(inputSurface);
   polyDistort->SetDistortionDenominatorPolyData(refInput);
   polyDistort->Update();
   
   inputSurface = polyDistort->GetOutput();
   
   vtkPointData* pointData = inputSurface->GetPointData();
   if (pointData == NULL) {
     cerr << "No POINT DATA from vtkPolyDataDistortion" << endl;
     exit(1);
   }
   vtkFloatArray* inverseArealDistortion = (vtkFloatArray*)
     pointData->GetArray(VTK_FIELD_DATA_INVERSE_AREAL_DISTORTION);
   if (inverseArealDistortion == NULL) {
     cerr << "No INVERSE_AREAL_DISTORTION data available" << endl;
     exit(1);
   }
   vtkFloatArray* maximumLinearDistortionField = (vtkFloatArray*)
     pointData->GetArray(VTK_FIELD_DATA_MAXIMUM_LINEAR_DISTORTION);
   if (maximumLinearDistortionField == NULL) {
     cerr << "No MAXIMUM_LINEAR_DISTORTION data available" << endl;
     exit(1);
   }

   // Step 6d: Calculate compress/stretched value for each node
   for (int i = 0; i < numberOfPoints; i++) {
     maximumLinearDistortion[i] = maximumLinearDistortionField->GetValue(i);
     averageArealCompression[i] = inverseArealDistortion->GetValue(i);
     compressedStretched[i] = maximumLinearDistortion[i]
               * averageArealCompression[i]
               * surfaceAreaRatio; 
     averageCompressedStretched[i] = compressedStretched[i];
   }
   inverseArealDistortion->Delete();

   // make "averageCompressedStretched" an average with its neighbors
   if (vtkUtilities::AverageFloatDataWithNeighbors(polyDistort->GetOutput(),
                    averageCompressedStretched) != 0) {
     cerr << "Unable to average compressedStretched metric" << endl;
   }

   // Step 6e: Flag highly compressed/stretched nodes for targeted smoothing
   double maxDistortion = -FLT_MAX;
   double minDistortion =  FLT_MAX;
   int numDistortionAboveThreshold = 0;
   
   for (int j = 0; j < numberOfPoints; j++) {
     if (averageCompressedStretched[j] > CompressStretchThreshold) {
      needSmoothing[j] = 1;
      numDistortionAboveThreshold++;
     }
     else {
      needSmoothing[j] = 0;
     }
     if (averageCompressedStretched[j] > maxDistortion) {
      maxDistortion = averageCompressedStretched[j];
     }
     if (averageCompressedStretched[j] < minDistortion) {
      minDistortion = averageCompressedStretched[j];
     }
   }
   
   cout << "Distortion Info:" << endl;
   cout << "  " << numDistortionAboveThreshold << " of " << numberOfPoints
      << " points are above "
      << "threshold = " << CompressStretchThreshold << endl;
   cout << "  minimum distortion: " << minDistortion << endl;  
   cout << "  maximum distortion: " << maxDistortion << endl;  
   
   if (cycles < NumberOfSmoothingCycles) {
     // Step 6f: Targeted smoothing
     polySmooth2->SetInput(inputSurface);
     polySmooth2->SetNumberOfIterations(FingerSmoothingIterations);
     polySmooth2->SetRelaxationFactor(FingerSmoothingStrength);
     polySmooth2->TargetedSmoothingOn();
     polySmooth2->SetTargetedPoints(needSmoothing, numberOfPoints);     
     polySmooth2->Update();
     inputSurface = polySmooth2->GetOutput();
   }   
   
   // delete temporary parameters
   polySmooth1->Delete();
   ellipse->Delete();
   if (cycles < NumberOfSmoothingCycles)
		  polyDistort->Delete();
  }
    
  // Update output.
  //
  output->GetPointData()->PassData(inputSurface->GetPointData());
  output->GetCellData()->PassData(inputSurface->GetCellData());
  output->SetPoints(inputSurface->GetPoints());
  output->SetVerts(inputSurface->GetVerts());
  output->SetLines(inputSurface->GetLines());
  output->SetPolys(inputSurface->GetPolys());
  output->SetStrips(inputSurface->GetStrips());

  if ( this->GenerateAverageCompressedStretched )
   {
   int idx = output->GetPointData()->AddArray((vtkFloatArray*)averageCompressedStretched);
   output->GetPointData()->SetActiveAttribute(idx, vtkDataSetAttributes::SCALARS);
   }

  delete[] needSmoothing;  
  delete[] maximumLinearDistortion;
  delete[] averageCompressedStretched;  
  delete[] averageArealCompression;
  delete[] compressedStretched;
  
  polySmooth2->Delete();
  polyDistort->Delete();
  inputTransformPoly->Delete();

  return 1;
}


void vtkInflateSmoothPolyData::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Number of Smoothing Cycles: " << this->NumberOfSmoothingCycles << "\n";
  os << indent << "Regular Smoothing Strength: " << this->RegularSmoothingStrength << "\n";
  os << indent << "Regular Smoothing Iterations: " << this->RegularSmoothingIterations << "\n";
  os << indent << "Inflation Factor: " << this->InflationFactor << "\n";
  os << indent << "Finger Smoothing Threshold: " << this->CompressStretchThreshold << "\n";
  os << indent << "Finger Smoothing Strength: " << this->FingerSmoothingStrength << "\n";
  os << indent << "Finger Smoothing Iterations: " << this->FingerSmoothingIterations << "\n";
  os << indent << "Generate average compressed/stretched Vector: " << (this->GenerateAverageCompressedStretched ? "On\n" : "Off\n");
}