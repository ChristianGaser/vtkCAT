/*LICENSE_START*/
/*
 *  Copyright 1995-2002 Washington University School of Medicine
 *
 *  http://brainmap.wustl.edu
 *
 *  This file is part of CARET.
 *
 *  CARET is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  CARET is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CARET; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*LICENSE_END*/

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkPolyDataDistortion.cxx,v $
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include <float.h>
#include <vector>

#include "vtkFieldData.h"
#include "vtkFloatArray.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkTriangle.h"
#include "vtkCellArray.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
               
#include "vtkPolyDataDistortion.h"
#include "vtkPointTopologyHelper.h"

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
vtkPolyDataDistortion* vtkPolyDataDistortion::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkPolyDataDistortion");
  if(ret)
    {
    return (vtkPolyDataDistortion*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkPolyDataDistortion;
}

//------------------------------------------------------------------------------
void vtkPolyDataDistortion::Execute()
{

   vtkDebugMacro(<<"Generating polygon distortion");

   vtkPolyData *input = this->GetInput();
   vtkPolyData *output = this->GetOutput();

   if (this->DistortionNumeratorPolyData == NULL) {
      vtkErrorMacro(<<"DistortionNumberatorPolyData has not been set!");
      return;
   }
   if (this->DistortionDenominatorPolyData == NULL) {
      vtkErrorMacro(<<"DistortionDenominatorPolyData has not been set!");
      return;
   }
   
   if (this->DistortionDenominatorPolyData->GetNumberOfStrips() > 0) {
      vtkErrorMacro(<<"Strips not supported for generating distortion!");
      return;
   }
   
   const int numPolys = input->GetNumberOfPolys();
   const int numPoints = input->GetNumberOfPoints();
   if ( (numPoints < 1) || (numPolys < 1) ) {
      vtkErrorMacro(<<"No data to generate distortion for!");
      return;
   }

   if (this->DistortionNumeratorPolyData->GetNumberOfPoints() != numPoints) {
      vtkErrorMacro(<< "Number of points in DistortionNumeratorPolyData "
                    << "is different than the input poly data object!");
      return;
   }
   if (this->DistortionDenominatorPolyData->GetNumberOfPoints() != numPoints) {
      vtkErrorMacro(<< "Number of points in DistortionDenominatorPolyData "
                    << "is different than the input poly data object!");
      return;
   }
   
   // input points and polygons 
   vtkPoints *inPts = input->GetPoints();
   //vtkPointData* pointData = input->GetPointData();
      
   double* pointArealDistortion         = new double[numPoints];
   double* pointMaximumArealDistortion  = new double[numPoints];
   int*   pointArealDistortionCount    = new int[numPoints];
   double* pointInverseArealDistortion  = new double[numPoints];
   int*   pointInverseArealDistortionCount = new int[numPoints];

   double* pointLinearDistortion        = new double[numPoints];
   double* pointMaximumLinearDistortion = new double[numPoints];
   int*   pointLinearDistortionCount   = new int[numPoints];
   
   vtkPoints *numeratorPoints =
              this->DistortionNumeratorPolyData->GetPoints();   
   vtkPoints *denominatorPoints =
              this->DistortionDenominatorPolyData->GetPoints();   
   vtkCellArray *numeratorPolys = 
              this->DistortionDenominatorPolyData->GetPolys();
            
   int i = 0;  
   for (i = 0; i < numPoints; i++) {
      pointArealDistortion[i]   = 0.0;
      pointMaximumArealDistortion[i]   = 0.0;
      pointArealDistortionCount[i] = 0;
      pointInverseArealDistortion[i]   = 0.0;
      pointInverseArealDistortionCount[i] = 0;
      
      pointLinearDistortion[i] = 0.0;
      pointMaximumLinearDistortion[i] = 0.0;
      pointLinearDistortionCount[i] = 0;
      double xyz[3];
      inPts->GetPoint(i, xyz);
   }
   
   //
   // Areal Distortion Computations
   //
   //printf("Number of polygons is: %d\n", numPolys);
   int cellId = 0;
   vtkIdType npts;
   vtkIdType* pts;
   for (numeratorPolys->InitTraversal(); numeratorPolys->GetNextCell(npts,pts);
        cellId++) {
      if (npts != 3) {
         vtkErrorMacro(<<"Polygon %d is not a triangle\n");
         return;
      }
      
      double nxyz0[3], nxyz1[3], nxyz2[3];
      numeratorPoints->GetPoint(pts[0], nxyz0);
      numeratorPoints->GetPoint(pts[1], nxyz1);
      numeratorPoints->GetPoint(pts[2], nxyz2);
      const double numeratorArea = vtkTriangle::TriangleArea(nxyz0, nxyz1, nxyz2);
      
      double dxyz0[3], dxyz1[3], dxyz2[3];
      denominatorPoints->GetPoint(pts[0], dxyz0);
      denominatorPoints->GetPoint(pts[1], dxyz1);
      denominatorPoints->GetPoint(pts[2], dxyz2);
      const double denominatorArea = vtkTriangle::TriangleArea(dxyz0, dxyz1, dxyz2);
      
      if (denominatorArea > 0.0) {
         const double arealDistortion = numeratorArea / denominatorArea;
         for (int j = 0; j < 3; j++) {
            pointArealDistortion[pts[j]] += arealDistortion;
            if (arealDistortion > pointMaximumArealDistortion[pts[j]]) {
               pointMaximumArealDistortion[pts[j]] = arealDistortion;
            }
            pointArealDistortionCount[pts[j]]++;
         }
      }
      if (numeratorArea > 0.0) {
         const double inverseArealDistortion = denominatorArea / numeratorArea;
         for (int j = 0; j < 3; j++) {
            pointInverseArealDistortion[pts[j]] += inverseArealDistortion;
            pointInverseArealDistortionCount[pts[j]]++;
         }
      }
   }
      
   //
   // Linear Distortion Computations
   //
   vtkPointTopologyHelperNew* topology = new vtkPointTopologyHelperNew(input);
   for (i = 0; i < numPoints; i++) {
      int* neighbors;
      if (topology->GetNeighborsForPoint(i, neighbors, 
                                         pointLinearDistortionCount[i]) == 0) {
         for (int j = 0; j < pointLinearDistortionCount[i]; j++) {
            const int neighbor = neighbors[j];
            double nxyz1[3], nxyz2[3];
            numeratorPoints->GetPoint(i, nxyz1);
            numeratorPoints->GetPoint(neighbor, nxyz2);
            double dxyz1[3], dxyz2[3];
            denominatorPoints->GetPoint(i, dxyz1);
            denominatorPoints->GetPoint(neighbor, dxyz2);
            // linear distortion along edges shared by points
            const double denominatorDistance = sqrt(
                             vtkMath::Distance2BetweenPoints(dxyz1, dxyz2));
            if (denominatorDistance > 0.0) {
               const double numeratorDistance = sqrt(
                             vtkMath::Distance2BetweenPoints(nxyz1, nxyz2));
               const double linearDistortion = numeratorDistance / denominatorDistance;
               pointLinearDistortion[i] += linearDistortion;
               //pointLinearDistortionCount[i]++;
               if (linearDistortion > pointMaximumLinearDistortion[i]) {
                  pointMaximumLinearDistortion[i] = linearDistortion;
               }
            }
         }
      }
   }
        
   double maxRawArealDistortion = -FLT_MAX;
   double minRawArealDistortion =  FLT_MAX;
   for (i = 0; i < numPoints; i++) {
      if (pointArealDistortionCount[i] > 0) {
         pointArealDistortion[i] /= (double)pointArealDistortionCount[i];
      }
      if (pointInverseArealDistortionCount[i] > 0) {
         pointInverseArealDistortion[i] /= (double)pointInverseArealDistortionCount[i];
      }
      if (pointArealDistortion[i] < minRawArealDistortion) {
         minRawArealDistortion = pointArealDistortion[i];
      } 
      if (pointArealDistortion[i] > maxRawArealDistortion) {
         maxRawArealDistortion = pointArealDistortion[i];
      } 
      
      if (pointLinearDistortionCount[i] > 0) {
         pointLinearDistortion[i] /= (double)pointLinearDistortionCount[i];
      }
      //cout << "Point " << i << ": " << pointArealDistortion[i] << endl;
   }
   delete topology;
   topology = NULL;
   
   //cout << "Distortion (min, max): " << minRawArealDistortion << ", "
   //     << maxRawArealDistortion << endl;
   // "Reference Copy" the geometric and topological data.
   output->CopyStructure(input);

   // "Reference Copy" the cells' attribute data.
   output->GetCellData()->PassData(input->GetCellData());
   
   // Point the output to the input    
   // NEED TO COPY ALL EXCEPT POINT FIELD DATA SINCE WE WILL BE MODIFYING IT.
   
   // "Reference Copy" points' attribute.
   output->GetPointData()->PassData(input->GetPointData());

   // field data is used to store attributes such as distortion 
   vtkFloatArray* arealDistortionArray = NULL;
   vtkFloatArray* maximumArealDistortionArray = NULL;
   vtkFloatArray* inverseArealDistortionArray = NULL;
   vtkFloatArray* linearDistortionArray = NULL;
   vtkFloatArray* maximumLinearDistortionArray = NULL;
 
      vtkPointData* pd = input->GetPointData();
      arealDistortionArray = (vtkFloatArray*)pd->GetArray(
                              VTK_FIELD_DATA_AREAL_DISTORTION);
      maximumArealDistortionArray = (vtkFloatArray*)pd->GetArray(
                                     VTK_FIELD_DATA_MAXIMUM_AREAL_DISTORTION);
      inverseArealDistortionArray = (vtkFloatArray*)pd->GetArray(
                              VTK_FIELD_DATA_INVERSE_AREAL_DISTORTION);
      linearDistortionArray = (vtkFloatArray*)pd->GetArray(
                              VTK_FIELD_DATA_LINEAR_DISTORTION);
      maximumLinearDistortionArray = (vtkFloatArray*)pd->GetArray(
                                     VTK_FIELD_DATA_MAXIMUM_LINEAR_DISTORTION);
   
   int arealDistortionIsNew = 0;
   if (arealDistortionArray == NULL) {
      arealDistortionIsNew = 1;
      arealDistortionArray = vtkFloatArray::New();
      arealDistortionArray->SetNumberOfTuples(numPoints);
      arealDistortionArray->SetNumberOfComponents(1);
   }
   
   int maximumArealDistortionIsNew = 0;
   if (maximumArealDistortionArray == NULL) {
      maximumArealDistortionIsNew = 1;
      maximumArealDistortionArray = vtkFloatArray::New();
      maximumArealDistortionArray->SetNumberOfTuples(numPoints);
      maximumArealDistortionArray->SetNumberOfComponents(1);
   }
   
   int inverseArealDistortionIsNew = 0;
   if (inverseArealDistortionArray == NULL) {
      inverseArealDistortionIsNew = 1;
      inverseArealDistortionArray = vtkFloatArray::New();
      inverseArealDistortionArray->SetNumberOfTuples(numPoints);
      inverseArealDistortionArray->SetNumberOfComponents(1);
   }
   
   int linearDistortionIsNew = 0;
   if (linearDistortionArray == NULL) {
      linearDistortionIsNew = 1;
      linearDistortionArray = vtkFloatArray::New();
      linearDistortionArray->SetNumberOfTuples(numPoints);
      linearDistortionArray->SetNumberOfComponents(1);
   }
   
   int maximumLinearDistortionIsNew = 0;
   if (maximumLinearDistortionArray == NULL) {
      maximumLinearDistortionIsNew = 1;
      maximumLinearDistortionArray = vtkFloatArray::New();
      maximumLinearDistortionArray->SetNumberOfTuples(numPoints);
      maximumLinearDistortionArray->SetNumberOfComponents(1);
   }
   
   for (i = 0; i < numPoints; i++) {
      arealDistortionArray->SetValue(i, pointArealDistortion[i]);
      inverseArealDistortionArray->SetValue(i, pointInverseArealDistortion[i]);
      maximumArealDistortionArray->SetValue(i, pointMaximumArealDistortion[i]);
      linearDistortionArray->SetValue(i, pointLinearDistortion[i]);
      maximumLinearDistortionArray->SetValue(i, pointMaximumLinearDistortion[i]);
   }
   
   arealDistortionArray->SetName(VTK_FIELD_DATA_AREAL_DISTORTION);
   output->GetPointData()->AddArray(arealDistortionArray);
   inverseArealDistortionArray->SetName(VTK_FIELD_DATA_INVERSE_AREAL_DISTORTION);
   output->GetPointData()->AddArray(inverseArealDistortionArray);
   maximumArealDistortionArray->SetName(VTK_FIELD_DATA_MAXIMUM_AREAL_DISTORTION);
   output->GetPointData()->AddArray(maximumArealDistortionArray);
   linearDistortionArray->SetName(VTK_FIELD_DATA_LINEAR_DISTORTION);
   output->GetPointData()->AddArray(linearDistortionArray);
   maximumLinearDistortionArray->SetName(VTK_FIELD_DATA_MAXIMUM_LINEAR_DISTORTION);
   output->GetPointData()->AddArray(maximumLinearDistortionArray);
   
   delete pointArealDistortionCount;
   delete pointArealDistortion;
   delete pointInverseArealDistortionCount;
   delete pointInverseArealDistortion;
   delete pointLinearDistortionCount;
   delete pointLinearDistortion;
   delete pointMaximumArealDistortion;
}

//------------------------------------------------------------------------------
// Constructor
vtkPolyDataDistortion::vtkPolyDataDistortion()
{
   // nothing to initialize at this time
}

//------------------------------------------------------------------------------
void vtkPolyDataDistortion::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolyDataToPolyDataFilter::PrintSelf(os,indent);
}

