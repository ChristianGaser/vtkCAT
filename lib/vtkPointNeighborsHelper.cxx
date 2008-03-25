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

#include "vtkCellArray.h"

#include "vtkPointNeighborsHelper.h"

vtkPointNeighborsHelper::vtkPointNeighborsHelper(vtkPolyData* polyData)
{
   pointNeighbors = NULL;
   neighborsValid = false;
   
   const int numberOfPoints = polyData->GetNumberOfPoints();
   if (numberOfPoints <= 0) {
      cerr << "No points in the polygon data" << endl;
   }
   
   pointNeighbors = new PointNeighbors[numberOfPoints];
   
   vtkCellArray* polys = polyData->GetPolys();
   int cellId = 0;
   int npts;
   int* pts;
   for (polys->InitTraversal(); polys->GetNextCell(npts,pts); cellId++) {
      if (npts != 3) {
         cerr << " Polygon is not a triangle in "
              << "vtkPointNeighborsHelper" << endl; 
         delete[] pointNeighbors;
         return;
      }
      const int n1 =  pts[0];
      const int n2 =  pts[1];
      const int n3 =  pts[2];
      pointNeighbors[n1].addNeighbor(n2);
      pointNeighbors[n1].addNeighbor(n3);
      pointNeighbors[n2].addNeighbor(n1);
      pointNeighbors[n2].addNeighbor(n3);
      pointNeighbors[n3].addNeighbor(n1);
      pointNeighbors[n3].addNeighbor(n2);
   }
   
   neighborsValid = true;
}

vtkPointNeighborsHelper::~vtkPointNeighborsHelper()
{
   if (pointNeighbors != NULL) {
      delete[] pointNeighbors;
   }
}

void
vtkPointNeighborsHelper::getNeighborsForPoint(const int pointNumber,
                                              vector<int>& neighbors) const
{
   neighbors = pointNeighbors[pointNumber].neighbors;
}

