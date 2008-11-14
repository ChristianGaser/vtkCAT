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


//
// This class is used to allow for easy access of the unoredered 
// neighbors of a vtkPolyData object.
//
#ifndef __ve_vtkPointNeighborsHelper_h__
#define __ve_vtkPointNeighborsHelper_h__

#include <vector>
#include "vtkPolyData.h"

class vtkPointNeighborsHelper {
   private:
      class PointNeighbors {
         public:
            vector<int> neighbors;
         
            void addNeighbor(const int neigh) {
               for (unsigned int i = 0; i < neighbors.size(); i++) {
                  if (neighbors[i] == neigh) return;
               }
               neighbors.push_back(neigh);
            }
      };
      
      PointNeighbors* pointNeighbors;
      bool neighborsValid;
      
   public:
      vtkPointNeighborsHelper(vtkPolyData* polyData);
      ~vtkPointNeighborsHelper();
      void getNeighborsForPoint(const int pointNumber,
                                vector<int>& neighbors) const;
      bool getNeighborsValid() const { return neighborsValid; }
};

#endif
