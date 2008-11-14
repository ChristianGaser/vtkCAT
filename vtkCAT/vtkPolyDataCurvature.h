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


// .NAME vtkPolyDataCurvature
// .SECTION Description
// vtkPolyDataCurvature computes curvature for a vtkPolyData object.

#ifndef __vtkPolyDataCurvature_h
#define __vtkPolyDataCurvature_h

#include "vtkPolyDataToPolyDataFilter.h"

class VTK_EXPORT vtkPolyDataCurvature : public vtkPolyDataToPolyDataFilter
{
public:
   vtkTypeMacro(vtkPolyDataCurvature, vtkPolyDataToPolyDataFilter);
   
   void PrintSelf(ostream& os, vtkIndent indent);
   
   // Description: 
   // Construct an instance of vtkPolyDataCurvature.
   static vtkPolyDataCurvature *New();

protected:
   vtkPolyDataCurvature();
   ~vtkPolyDataCurvature() {};
   vtkPolyDataCurvature(const vtkPolyDataCurvature&) {};
   void operator=(const vtkPolyDataCurvature&) {};
   
   void Execute();
   
};

// these names should probably be in vtkCaretData
static char* CARET_GAUSSIAN_CURVATURE = "CaretGaussianCurvature";
static char* CARET_MEAN_CURVATURE     = "CaretMeanCurvature";


#endif
