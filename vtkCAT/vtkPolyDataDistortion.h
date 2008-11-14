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

// .NAME vtkPolyDataDistortion
// .SECTION Description
// vtkPolyDataDistortion computes areal and linear distortion measurements
// for a vtkPolyData object.  Areal distortion is the ratio of polygonal area
// for two vtkPolyData objects of identical topology but differing geometry.
// Linear distortion is the ratio of linear distance to neighboring points.
// The output vtkPolyData object will be a reference counted copy of the 
// vtkPolyData object specified with the "SetInput()" method.  To compute 
// areal distortion, two methods must be used to the "numerator" and 
// "denominator" vtkPolyData objects.  The methods are 
// "SetDistortionNumeratorPolyData()" and "SetDistortionDenominatorPolyData()" 
// respectively.
//
// Distortion is stored in field data arrays named by the "#defines" as follows:
//  VTK_FIELD_DATA_AREAL_DISTORTION - average areal distortion for each point
//  VTK_FIELD_DATA_INVERSE_AREAL_DISTORTION - average areal distortion for each point
//  VTK_FIELD_DATA_MAXIMUM_AREAL_DISTORTION - max areal dist for each point
//  VTK_FIELD_DATA_LINEAR_DISTORTION - average areal distortion for each point
//  VTK_FIELD_DATA_MAXIMUM_LINEAR_DISTORTION - max areal dist for each point
 
#ifndef __vtkPolyDataDistortion_h
#define __vtkPolyDataDistortion_h

#include "vtkPolyDataToPolyDataFilter.h"

class VTK_EXPORT vtkPolyDataDistortion : public vtkPolyDataToPolyDataFilter
{
public:
   vtkTypeMacro(vtkPolyDataDistortion, vtkPolyDataToPolyDataFilter);
   
   void PrintSelf(ostream& os, vtkIndent indent);
   
   // Description: 
   // Construct an instance of vtkPolyDataDistortion.
   static vtkPolyDataDistortion *New();

   // Description:
   // Set the Areal Distortion Numerator Data Set
   vtkSetMacro(DistortionNumeratorPolyData, vtkPolyData*);
   vtkSetMacro(DistortionDenominatorPolyData, vtkPolyData*);
   
protected:
   vtkPolyDataDistortion();
   ~vtkPolyDataDistortion() {};
   vtkPolyDataDistortion(const vtkPolyDataDistortion&) {};
   void operator=(const vtkPolyDataDistortion&) {};
   
   void Execute();
   
   vtkPolyData* DistortionNumeratorPolyData;
   vtkPolyData* DistortionDenominatorPolyData;
};

#define VTK_FIELD_DATA_AREAL_DISTORTION "ArealDistortion"
#define VTK_FIELD_DATA_INVERSE_AREAL_DISTORTION "InverseArealDistortion"
#define VTK_FIELD_DATA_MAXIMUM_AREAL_DISTORTION "MaximumArealDistortion"
#define VTK_FIELD_DATA_LINEAR_DISTORTION "LinearDistortion"
#define VTK_FIELD_DATA_MAXIMUM_LINEAR_DISTORTION "MaximumLinearDistortion"


#endif

