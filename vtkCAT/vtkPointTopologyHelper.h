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

// .NAME vtkPointTopologyHelperNew
// .SECTION Description
// vtkPointTopologyHelperNew creates topology information for each Point
// in a vtkPolyData object.

#ifndef __vtkPointTopologyHelperNew_h
#define __vtkPointTopologyHelperNew_h

#ifdef USE_NAMESPACE_STD
using namespace std;
#endif

#include <vector>
#include "vtkPolyData.h"
class vtkPointTopologyHelperNew {
   private:
      class EdgeInfo {
         public:
            int point1, point2;
            EdgeInfo(const int point1In, const int point2In) {
               point1 = point1In;
               point2 = point2In;
            }
            int getOtherPoint(const int point) const {
               if (point1 == point) return point2;
               else                 return point1;
            }
      };
      
      class PointInfo {
         public:
            int   pointNumber;
            vector<int> neighbors;
            vector<EdgeInfo> edges;

            PointInfo(const int pointNumberIn) {
               pointNumber = pointNumberIn;
            }
            void addNeighbors(const int newNeighbor1, const int newNeighbor2) {
               edges.push_back(EdgeInfo(newNeighbor1, newNeighbor2));
            }

            const EdgeInfo* findEdgeWithPoint(const int pointSeeking, 
                                              const int notPoint) {
               for (unsigned int i = 0; i < edges.size(); i++) {
                  if ( ((edges[i].point1 == pointSeeking) &&
                        (edges[i].point2 != notPoint)) ||
                       ((edges[i].point2 == pointSeeking) &&
                        (edges[i].point1 != notPoint)) ) {
                     return &edges[i];
                  }
               }
               return NULL;
            }

            void sortNeighbors() {
               if (edges.size() > 0) {
                  int currentPoint  = edges[0].point1;
                  int nextNeighbor  = edges[0].point2;
                  neighbors.push_back(currentPoint);

                  for (unsigned int i = 1; i < edges.size(); i++) {
                     neighbors.push_back(nextNeighbor);
                     const EdgeInfo* e = findEdgeWithPoint(nextNeighbor, currentPoint);
                     if (e != NULL) {
                        currentPoint = nextNeighbor;
                        nextNeighbor = e->getOtherPoint(nextNeighbor);
/*                        if (nextNeighbor < 0) {
                           printf("ERROR: Unable to find neighbor of %d\n",
                                  nextNeighbor);
                        }
*/
                     }
                     else {
/*                        printf("ERROR: Unable to find edge for Point %d\n",
                               pointNumber);
*/
                        break;
                     }
                  }
               }
               else {
      //            printf("WARNING: Point %d has no neighbors\n", pointNumber);
               }
               edges.clear();
            }
      };
      
      vector<PointInfo> points;
      int* neighborStorage;
      int maxNeighbors;
      
   public:
   
      // Description:
      // Constructor
      static vtkPointTopologyHelperNew* New(vtkPolyData* polyData) {
                     return new vtkPointTopologyHelperNew(polyData); }
      void Delete() { delete this; }
      vtkPointTopologyHelperNew(vtkPolyData* polyData);  
      
      // Description:
      // Get the neighbors for a point.
      // Note "neighborsOut" will point to memory allocated by this class.
      // Returns non-zero if an error occurs.  Zero if successful.
      int GetNeighborsForPoint(const int pointNumberIn, int* &neighborsOut, 
                               int& numNeighborsOut);
};

#endif
