/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkInflateSmoothPolyData.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkInflateSmoothPolyData - inflate surface and smooth fingers 
// .SECTION Description
// vtkInflateSmoothPolyData will perform the following to the input surface for 
// the specified number of cycles:
//        1) Smooth the entire surface.
//        2) Inflate the surface so that it is similar in shape 
//           to an ellipsoid
//        3) Calculate error metrics on the surface.
//        4) Perform targeted smoothing on nodes in the surface
//           having high distortion.
//
// .SECTION Caveats
// 
// .SECTION See Also
// vtkSmoothPolyDataFilter

#ifndef __vtkInflateSmoothPolyData_h
#define __vtkInflateSmoothPolyData_h

#include "vtkPolyDataAlgorithm.h"

class vtkSmoothPoints;

class VTK_GRAPHICS_EXPORT vtkInflateSmoothPolyData : public vtkPolyDataAlgorithm
{
public:
  vtkTypeRevisionMacro(vtkInflateSmoothPolyData,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct object with number of normal smoothing iterations 50; inflation 
  // factor 1.0; compressed stretch threshold 1.0; normal smoothing strength
  // 1.0; finger smoothing iterations 100; finger smoothing strenght 1.0;
  // and number of smooting cycles 3.
  static vtkInflateSmoothPolyData *New();

  // Description:
  // Specify the number of iterations for regular smoothing
  vtkSetClampMacro(RegularSmoothingIterations,int,0,VTK_LARGE_INTEGER);
  vtkGetMacro(RegularSmoothingIterations,int);

  // Description:
  // Specify the strength of regular smoothing
  vtkSetClampMacro(RegularSmoothingStrength,double,0,1.0);
  vtkGetMacro(RegularSmoothingStrength,double);

  // Description:
  // Specify the number of iterations for finger smoothing,
  vtkSetClampMacro(FingerSmoothingIterations,int,0,VTK_LARGE_INTEGER);
  vtkGetMacro(FingerSmoothingIterations,int);

  // Description:
  // Specify the strength of finger smoothing
  vtkSetClampMacro(CompressStretchThreshold,double,0,100.0);
  vtkGetMacro(CompressStretchThreshold,double);

  // Description:
  // Specify the finger compress/stretch threshold
  vtkSetClampMacro(FingerSmoothingStrength,double,0,1000.0);
  vtkGetMacro(FingerSmoothingStrength,double);

  // Description:
  // Specify the number of smoothing cycles
  vtkSetClampMacro(NumberOfSmoothingCycles,int,0,VTK_LARGE_INTEGER);
  vtkGetMacro(NumberOfSmoothingCycles,int);

  // Description:
  // Specify the inflation factor
  vtkSetClampMacro(InflationFactor,double,0,100.0);
  vtkGetMacro(InflationFactor,double);

  // Description:
  // Turn on/off the generation of average compressed/stretched values.
  vtkSetMacro(GenerateAverageCompressedStretched,int);
  vtkGetMacro(GenerateAverageCompressedStretched,int);
  vtkBooleanMacro(GenerateAverageCompressedStretched,int);
  
protected:
  vtkInflateSmoothPolyData();
  ~vtkInflateSmoothPolyData() {};

  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  double RegularSmoothingStrength;
  int RegularSmoothingIterations;
  double CompressStretchThreshold;
  int FingerSmoothingIterations;
  double FingerSmoothingStrength;
  double InflationFactor;
  int NumberOfSmoothingCycles;
  int GenerateAverageCompressedStretched;

  vtkSmoothPoints *SmoothPoints;
private:
  vtkInflateSmoothPolyData(const vtkInflateSmoothPolyData&);  // Not implemented.
  void operator=(const vtkInflateSmoothPolyData&);  // Not implemented.
};

#endif
