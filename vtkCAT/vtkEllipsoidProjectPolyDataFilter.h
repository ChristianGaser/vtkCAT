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


// .NAME vtkEllipsoidProjectPolyDataFilter - project surface to ellipsoid.

#ifndef __vtkEllipsoidProjectPolyDataFilter_h
#define __vtkEllipsoidProjectPolyDataFilter_h

#include "vtkPolyDataToPolyDataFilter.h"

class VTK_EXPORT vtkEllipsoidProjectPolyDataFilter : 
                                            public vtkPolyDataToPolyDataFilter
{ 
public:
   vtkTypeMacro(vtkEllipsoidProjectPolyDataFilter, vtkPolyDataToPolyDataFilter);
   
   void PrintSelf(ostream& os, vtkIndent indent);
   
   // Description: 
   // Construct an instance of vtkEllipsoidProjectPolyDataFilter.
   static vtkEllipsoidProjectPolyDataFilter *New();

   // Description:
   // Set the X, Y, and Z dimensions of the output ellipse MUST BE CALLED!!
   void SetEllipsoidDimensions(float x, float y, float z);
   
   // Description:
   // Set/Get the inflation factor
   vtkSetMacro(InflationFactor, float);
   vtkGetMacro(InflationFactor, float);
   
protected:
   vtkEllipsoidProjectPolyDataFilter();
   ~vtkEllipsoidProjectPolyDataFilter() {};
   vtkEllipsoidProjectPolyDataFilter(const vtkEllipsoidProjectPolyDataFilter&) {};
   void operator=(const vtkEllipsoidProjectPolyDataFilter&) {};
   float EllipsoidSizeX;
   float EllipsoidSizeY;
   float EllipsoidSizeZ;
   
   float InflationFactor;
   void Execute();
   
};

#endif
