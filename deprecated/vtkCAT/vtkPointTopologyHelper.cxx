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

#include "vtkPointTopologyHelper.h"

vtkPointTopologyHelperNew::vtkPointTopologyHelperNew(vtkPolyData* polyData)
{
   for (int i = 0; i < polyData->GetNumberOfPoints(); i++) {
      points.push_back(PointInfo(i));
   }
   
   vtkCellArray* polys = polyData->GetPolys();
   int cellId = 0;
   vtkIdType npts;
   vtkIdType* pts;
   for (polys->InitTraversal(); polys->GetNextCell(npts,pts); cellId++) {
      if (npts != 3) {
         cerr << " Polygon is not a triangle in vtkPointTopologyHelperNew" 
              << endl;
         return;
      }
      const int n1 =  pts[0];
      const int n2 =  pts[1];
      const int n3 =  pts[2];
      points[n1].addNeighbors(n3, n2);
      points[n2].addNeighbors(n1, n3);  // (n3, n1) to preserve orientation
      points[n3].addNeighbors(n2, n1);
   }

   maxNeighbors = 0;
   
   for (unsigned int k = 0; k < points.size(); k++) {
      points[k].sortNeighbors();
      if ((int)points[k].neighbors.size() > maxNeighbors) {
         maxNeighbors = points[k].neighbors.size();
      }
   }
   
   neighborStorage = NULL;
   if (maxNeighbors > 0) {
      neighborStorage = new int[maxNeighbors];
   }
}

int 
vtkPointTopologyHelperNew::GetNeighborsForPoint(const int pointNumberIn, 
                               int* &neighborsOut, 
                               int& numNeighborsOut)
{
   neighborsOut = NULL;
   numNeighborsOut = 0;
   if ( (pointNumberIn < 0) || (pointNumberIn > (int)points.size()) ) {
      return 1;
   }
   else if (neighborStorage == NULL) {
      return 1;
   }
   
   neighborsOut = neighborStorage;
   numNeighborsOut = points[pointNumberIn].neighbors.size();
   for (int i = 0; i < numNeighborsOut; i++) {
      neighborsOut[i] = points[pointNumberIn].neighbors[i];
   }
   
   return 0;
}
