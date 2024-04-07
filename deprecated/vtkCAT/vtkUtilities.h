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


// .NAME vtkUtilities - utility functions for use with VTK.

#ifndef __vtkUtilities_h
#define __vtkUtilities_h

#include "vtkDataSet.h"
#include "vtkPolyData.h"

class vtkUtilities 
{
public:
   // Description:
   // Get the center (average) for the data set's point data.
   static void GetPointsCenter(vtkDataSet* dataSet, double center[3]);
   
   // Description:
   // Get the surface area for a poly data set.
   static double GetSurfaceArea(vtkPolyData* polyData);
   
   // Description:
   // Average float values for each point with its neighboring points
   static int AverageFloatDataWithNeighbors(vtkPolyData* polyData,
                                            double* values);
                                            
   // Description:
   // Write the field data as a metric file
   static int WriteMetricFile(vtkFloatArray* data, const char* fileName,
                              bool writeAsInteger);
};

#endif
