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
  Module:    $RCSfile: vtkCaretTgtSmoothPolyDataFilter.h,v $
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
// .NAME vtkCaretTgtSmoothPolyDataFilter.h - Targeted Smoothing of Points with weighting determined by neighboring tile area
// .SECTION Description
// By default all points are smoothed.  Targeted smoothing can be performed by
// enabling targeted smoothing and setting the points to be smoothed.
//
// .SECTION See Also
// vtkWindowedSincPolyDataFilter vtkSmoothPolyDataFilter

#ifndef __vtkCaretTgtSmoothPolyDataFilter_h
#define __vtkCaretTgtSmoothPolyDataFilter_h

#include "vtkPolyDataToPolyDataFilter.h"

class vtkSmoothPoints;

class VTK_EXPORT vtkCaretTgtSmoothPolyDataFilter : public vtkPolyDataToPolyDataFilter
{
public:
  vtkTypeMacro(vtkCaretTgtSmoothPolyDataFilter,vtkPolyDataToPolyDataFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct object with number of iterations 20; relaxation factor 1.0;
  // All points enabled for smoothing. 
  static vtkCaretTgtSmoothPolyDataFilter *New();

  // Description:
  // Specify the number of iterations for Laplacian smoothing,
  vtkSetClampMacro(NumberOfIterations,int,0,VTK_LARGE_INTEGER);
  vtkGetMacro(NumberOfIterations,int);

  // Description:
  // Specify the relaxation factor for Laplacian smoothing. As in all
  // iterative methods, the stability of the process is sensitive to
  // this parameter. In general, small relaxation factors and large
  // numbers of iterations are more stable than larger relaxation
  // factors and smaller numbers of iterations.
  vtkSetMacro(RelaxationFactor,float);
  vtkGetMacro(RelaxationFactor,float);

  // Description:
  // Turn on/off the smoothing of vertices on the boundary of the mesh.
  vtkSetMacro(BoundarySmoothing,int);
  vtkGetMacro(BoundarySmoothing,int);
  vtkBooleanMacro(BoundarySmoothing,int);

  // Description:
  // Specify the source object which is used to constrain smoothing. The 
  // source defines a surface that the input (as it is smoothed) is 
  // constrained to lie upon.
  void SetSource(vtkPolyData *source);
  vtkPolyData *GetSource();
  
  // Description:
  // Specify the points for smoothing
  void SetTargetedPoints(int* pointsToSmooth, int numPoints);
  
  // Description:
  // Set/Get Targeted Smoothing
  vtkSetMacro(TargetedSmoothing, int);
  vtkGetMacro(TargetedSmoothing, int);
  vtkBooleanMacro(TargetedSmoothing, int);
  
protected:
  vtkCaretTgtSmoothPolyDataFilter();
  ~vtkCaretTgtSmoothPolyDataFilter() {};
  vtkCaretTgtSmoothPolyDataFilter(const vtkCaretTgtSmoothPolyDataFilter&) {};
  void operator=(const vtkCaretTgtSmoothPolyDataFilter&) {};

  void Execute();

  int NumberOfIterations;
  float RelaxationFactor;
  int BoundarySmoothing;

  vtkSmoothPoints *SmoothPoints;

  int* TargetedPoints;
  int  TargetedPointsSize;
  int  TargetedSmoothing;
};


#endif


