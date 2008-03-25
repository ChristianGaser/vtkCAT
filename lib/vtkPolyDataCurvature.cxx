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

#ifdef USE_NAMESPACE_STD
using namespace std;
#endif

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkPolyDataCurvature.cxx,v $
  Language:  C++
  Date:      $Date: 2002/11/19 16:32:00 $
  Version:   $Revision: 1.1.1.1 $


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
#include <vector>

#include "vtkFieldData.h"
#include "vtkFloatArray.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPointData.h"
#include "vtkPolyDataCurvature.h"
#include "vtkTriangle.h"
#include "vtkCellData.h"
#include "vtkCellArray.h"

class NodeInfo2 {
   private:
      class EdgeInfo {
         public:
            int node1, node2;
            EdgeInfo(const int node1In, const int node2In) {
               node1 = node1In;
               node2 = node2In;
            }
            int getOtherNode(const int node) const {
               if (node1 == node) return node2;
               else               return node1;
            }
      };

   public:
      double xyz[3];
      double normal[3];
      double gauss;
      double mean;
      double prin1;
      double prin2;
      double fold;
      int   nodeNumber;
      vector<int> neighbors;
      vector<EdgeInfo> edges;

      NodeInfo2(const int nodeNumberIn, const double xyzIn[3], 
               const double normalIn[]) {
         nodeNumber = nodeNumberIn;
         xyz[0] = xyzIn[0];
         xyz[1] = xyzIn[1];
         xyz[2] = xyzIn[2];
         normal[0] = normalIn[0];
         normal[1] = normalIn[1];
         normal[2] = normalIn[2];
         gauss = mean = prin1 = prin2 = fold = 0;
      }

      void addNeighbors(const int newNeighbor1, const int newNeighbor2) {
         edges.push_back(EdgeInfo(newNeighbor1, newNeighbor2));
      }

      const EdgeInfo* findEdgeWithNode(const int nodeSeeking, const int notNode) {
         for (unsigned int i = 0; i < edges.size(); i++) {
            if ( ((edges[i].node1 == nodeSeeking) &&
                  (edges[i].node2 != notNode)) ||
                 ((edges[i].node2 == nodeSeeking) &&
                  (edges[i].node1 != notNode)) ) {
               return &edges[i];
            }
         }
         return NULL;
      }
      
      void setCurvature(const double curveValues[5]) {
         gauss = curveValues[0];
         mean  = curveValues[1];
         prin1 = curveValues[2];
         prin2 = curveValues[3];
         fold  = curveValues[4];
      }

      void sortNeighbors() {
         if (edges.size() > 0) {
            int currentNode  = edges[0].node1;
            int nextNeighbor = edges[0].node2;
            neighbors.push_back(currentNode);

            for (unsigned int i = 1; i < edges.size(); i++) {
               neighbors.push_back(nextNeighbor);
               const EdgeInfo* e = findEdgeWithNode(nextNeighbor, currentNode);
               if (e != NULL) {
                  currentNode = nextNeighbor;
                  nextNeighbor = e->getOtherNode(nextNeighbor);
/*                  if (nextNeighbor < 0) {
                     printf("ERROR: Unable to find neighbor of %d\n",
                            nextNeighbor);
                  }
*/
               }
               else {
                  printf("ERROR: Unable to find edge for Node %d\n",
                         nodeNumber);
                  break;
               }
            }
         }
         else {
//            printf("WARNING: Node %d has no neighbors\n", nodeNumber);
         }
         edges.clear();
      }
};

//------------------------------------------------------------------------------

static const int MAX_NEIGHBORS = 50;

static int      curvature (vector<NodeInfo2>& nodes);
static void     compute_curvature (vector<NodeInfo2>& nodes,int snum, int i, 
                double *C0, double *N0, double *answer);
static void     proj2plane (double *answer, double *projected, double basis [2][3]);
static void     projection (double *answer, double *vector, double *normal);
static void     least_squares (int N, double dc [MAX_NEIGHBORS][2], 
                double dn [MAX_NEIGHBORS][2], double *gaussianCurvature, 
                double *meanCurvature, double *prin1, double *prin2, double *fold);
                
//------------------------------------------------------------------------------
vtkPolyDataCurvature* vtkPolyDataCurvature::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkPolyDataCurvature");
  if(ret)
    {
    return (vtkPolyDataCurvature*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkPolyDataCurvature;
}

//------------------------------------------------------------------------------
void vtkPolyDataCurvature::Execute()
{

   vtkDebugMacro(<<"Generating polygon curvature");

   vtkPolyData *input = this->GetInput();
   vtkPolyData *output = this->GetOutput();

   if (input->GetNumberOfStrips() > 0) {
      vtkErrorMacro(<<"Strips not supported for generating curvature!");
      return;
   }
   const int numPolys = input->GetNumberOfPolys();
   const int numPoints = input->GetNumberOfPoints();
   if ( (numPoints < 1) || (numPolys < 1) ) {
      vtkErrorMacro(<<"No data to generate curvature for!");
      return;
   }

   // input points and polygons 
   vtkPoints *inPts = input->GetPoints();
   vtkCellArray *inPolys = input->GetPolys();
   vtkPointData* pointData = input->GetPointData();

   vtkDataArray* normals = pointData->GetNormals();
   if (normals == NULL) {
      vtkErrorMacro(<<"No normals area associated with dataset!");
      return;
   }
   const int numNormals = normals->GetNumberOfTuples();
   
   if (numNormals != numPoints) {
      vtkErrorMacro(<<"Number of normals != number of points in dataset!");
      return;
   }
   
   vector<NodeInfo2> nodes;
   unsigned int i = 0;
   for (i = 0; static_cast<int>(i) < numPoints; i++) {
      double xyz[3];
      inPts->GetPoint(i, xyz);
      double norm[3];
      normals->GetTuple(i, norm);
      nodes.push_back(NodeInfo2(i, xyz, norm));
   }
   
   //printf("Number of polygons is: %d\n", numPolys);
   int cellId = 0;
   int npts;
   int* pts;
   for (inPolys->InitTraversal(); inPolys->GetNextCell(npts,pts); cellId++) {
      if (npts != 3) {
         vtkErrorMacro(<<"Polygon %d is not a triangle\n");
         return;
      }
      const int n1 =  pts[0];
      const int n2 =  pts[1];
      const int n3 =  pts[2];
      nodes[n1].addNeighbors(n3, n2);
      nodes[n2].addNeighbors(n1, n3);  // (n3, n1) to preserve orientation
      nodes[n3].addNeighbors(n2, n1);
   }

   for (unsigned int k = 0; k < nodes.size(); k++) {
      nodes[k].sortNeighbors();
   }
   
   curvature(nodes);
   
   // "Reference Copy" the geometric and topological data.
   output->CopyStructure(input);

   // "Reference Copy" the cells' attribute data.
   output->GetCellData()->PassData(input->GetCellData());
   
   // Point the output to the input    
   // NEED TO COPY ALL EXCEPT POINT FIELD DATA SINCE WE WILL BE MODIFYING IT.
   
   // "Reference Copy" points' attribute.
   output->GetPointData()->PassData(input->GetPointData());

   // field data is used to store attributes such as curvature 
   vtkFloatArray* meanCurvatureArray = NULL;
   vtkFloatArray* gaussianCurvatureArray = NULL;

   
   int meanCurvatureIsNew = 0;
   if (meanCurvatureArray == NULL) {
      meanCurvatureIsNew = 1;
      meanCurvatureArray = vtkFloatArray::New();
      meanCurvatureArray->SetNumberOfTuples(numPoints);
      meanCurvatureArray->SetNumberOfComponents(1);
   }
   
   for (i = 0; static_cast<int>(i) < numPoints; i++) {
      meanCurvatureArray->SetValue(i, nodes[i].mean);
   }
   if (meanCurvatureIsNew) {
      meanCurvatureArray->SetName(CARET_MEAN_CURVATURE);
      output->GetPointData()->AddArray(meanCurvatureArray);
      // can now "dereference" since fieldData now references it
      meanCurvatureArray->Delete();
   }
   
   int gaussianCurvatureIsNew = 0;
   if (gaussianCurvatureArray == NULL) {
      gaussianCurvatureIsNew = 1;
      gaussianCurvatureArray = vtkFloatArray::New();
      gaussianCurvatureArray->SetNumberOfTuples(numPoints);
      gaussianCurvatureArray->SetNumberOfComponents(1);
   }
   
   for (i = 0; static_cast<int>(i) < numPoints; i++) {
      gaussianCurvatureArray->SetValue(i, nodes[i].gauss);
   }
   if (gaussianCurvatureIsNew) {
      gaussianCurvatureArray->SetName(CARET_GAUSSIAN_CURVATURE);
      output->GetPointData()->AddArray(gaussianCurvatureArray);
      
      // can now "dereference" since fieldData now references it
      gaussianCurvatureArray->Delete();
   }   
}

//------------------------------------------------------------------------------
static int      curvature (vector<NodeInfo2>& nodes)
{
        double   min_gauss, max_gauss, avg_gauss = 0.0;
        double   min_mean, max_mean, avg_mean = 0.0;
        double   min_prin1, max_prin1, avg_prin1 = 0.0;
        double   min_prin2, max_prin2, avg_prin2 = 0.0;
        double   min_fold, max_fold;
        double   mag;   // factor, temp,
//         FILE    *fd_out;
//         char    gaussfile [100];
//         char    meanfile [100];
// 
//         strcpy (gaussfile, "vtk.gauss");
//         strcpy (meanfile, "vtk.mean");
        unsigned int i = 0;
        for (i = 0; i < nodes.size(); i++){
           NodeInfo2& node = nodes[i];

           if (node.neighbors.size() > 0){
              //mag = magnitude (node.normal);
              mag = vtkMath::Norm(node.normal);
              if (mag < 0.01){
                 printf ("WARNING: Normal for point %d: %f %f %f, magnitude %f\n",
                        i, node.normal[0], node.normal[1], node.normal[2], mag);                 const int m = node.neighbors[0];
                 const int o = node.neighbors[1];
                 //unit_normal (nodes[o].xyz, node.xyz,
                 //     nodes[m].xyz, node.normal);
                 vtkTriangle::ComputeNormal(nodes[o].xyz, node.xyz,
                        nodes[m].xyz, node.normal);
                 //mag = magnitude (node.normal);
                 mag = vtkMath::Norm(node.normal);
                 printf ("WARNING: Normal for point %d: %f %f %f, magnitude %f\n",
                        i, node.normal[0], node.normal[1], node.normal[2], mag);
              }
           }
        }
        min_fold = min_prin2 = min_prin1 = min_gauss = min_mean = 100000;
        max_fold = max_prin2 = max_prin1 = max_gauss = max_mean = -100000;

        double answer[5];
        for (i = 0; i < nodes.size(); i++){
           if (nodes[i].neighbors.size() > 0){
              compute_curvature (nodes, 0, i, nodes[i].xyz,
                        nodes[i].normal, answer);

              nodes[i].setCurvature(answer);
//               if ((i % 5000) == 0)
//                  printf ("Curvature %7d %7.3f %7.3f %7.3f %7.3f\n",
//                         i, answer  [0], answer  [1],
//                         answer [2], answer [3]);
              if (answer[0] < min_gauss)
                 min_gauss = answer[0];
              if (answer[0] > max_gauss)
                 max_gauss = answer[0];
              if (answer[1] < min_mean)
                 min_mean = answer[1];
              if (answer[1] > max_mean)
                 max_mean = answer[1];
              if (answer[2] < min_prin1)
                 min_prin1 = answer[2];
              if (answer[2] > max_prin1)
                 max_prin1 = answer[2];
              if (answer[3] < min_prin2)
                 min_prin2 = answer[3];
              if (answer[3] > max_prin2)
                 max_prin2 = answer[3];
              if (answer[4] < min_fold)
                 min_fold = answer[4];
              if (answer[4] > max_fold)
                 max_fold = answer[4];
              avg_gauss += fabs (answer[0]);
              avg_mean  += fabs (answer[1]);
              avg_prin1 += fabs (answer[2]);
              avg_prin2 += fabs (answer[3]);
           }
        }

        avg_gauss /= nodes.size();
        avg_mean  /= nodes.size();
        avg_prin1 /= nodes.size();
        avg_prin2 /= nodes.size();
//         printf ("Gauss:  %.2f %.2f, avg %.2f\n", min_gauss, max_gauss, avg_gauss);
//         printf ("Mean :  %.2f %.2f, avg %.2f\n", min_mean, max_mean, avg_mean);
//         printf ("Prin1:  %.2f %.2f, avg %.2f\n", min_prin1, max_prin1, avg_prin1);
//         printf ("Prin2:  %.2f %.2f, avg %.2f\n", min_prin2, max_prin2, avg_prin2);
//         printf ("Fold :  %.2f %.2f\n", min_fold, max_fold);

//         if ((fd_out = fopen (meanfile, "w")) == NULL){
//            printf ("ERROR: could not open file %s\n", meanfile);
//            return -1;
//         }
//         for (i = 0; i < nodes.size(); i++){
//            fprintf (fd_out, "%d %f\n", i, answer [1]);
//         }
// 
//         fclose (fd_out);
// 
//         if ((fd_out = fopen (gaussfile, "w")) == NULL){
//            printf ("ERROR: could not open file %s\n", gaussfile);
//            return -1;
//         }
//         for (i = 0; i < nodes.size(); i++){
//            fprintf (fd_out, "%d %f\n", i, -answer[0]);
//         }
//         fclose (fd_out);

        return 0;
}

static void
compute_curvature (vector<NodeInfo2>& nodes,
                   int snum, int i, double *C0, double *N0, double *answer)
{
        int     n, neigh, k;
        double   projected [3];
        double   t1 [3];
        double   dN [MAX_NEIGHBORS][3];
        double   dC [MAX_NEIGHBORS][3];
        double   dn [MAX_NEIGHBORS][2];
        double   dc [MAX_NEIGHBORS][2];
        double   basis [2][3];
        double   gauss, mean, prin1, prin2, fold; //, gauss1, mean1;
        double   mag;  //, temp;

        for (n = 0; n < static_cast<int>(nodes[i].neighbors.size()); n++){
           neigh = nodes[i].neighbors[n];
           for (k = 0; k < 3; k++){
              dN [n][k] = nodes[neigh].normal[k] - N0 [k];
              dC [n][k] = nodes[neigh].xyz[k] - C0 [k];
           }
        }

        /* Compute the basis functions */
        projection (basis [0], dC [0], N0);
        //mag = magnitude (basis [0]);
        mag = vtkMath::Norm(basis [0]);
        for (k = 0; k < 3; k++){
           basis [0][k] = basis [0][k]/mag;
           t1 [k] = -basis [0][k];
        }
        //cross_product (t1, N0, basis [1]);
        vtkMath::Cross(t1, N0, basis [1]);
        vtkMath::Normalize(basis[1]);
        //mag = magnitude (basis [1]);
        //mag = vtkMath::Norm(basis [1]);
        //for (k = 0; k < 3; k++)
        //   basis [1][k] = basis [1][k]/mag;
//      temp = dot_product (basis [0], basis [1]);

        for (n = 0; n < static_cast<int>(nodes[i].neighbors.size()); n++){
           projection (projected, dN [n], N0);
           proj2plane (dn [n], projected, basis);

           projection (projected, dC [n], N0);
           proj2plane (dc [n], projected, basis);

        }
        least_squares (nodes[i].neighbors.size(), dc, dn, &gauss,
                        &mean, &prin1, &prin2, &fold);
        answer [0] = gauss;
        answer [1] = mean;
        answer [2] = prin1;
        answer [3] = prin2;
        answer [4] = fold;
}

static void proj2plane (double *answer, double *projected, double basis [2][3])
{
        //answer [0] = dot_product (projected, basis [0]);
        //answer [1] = dot_product (projected, basis [1]);
        answer [0] = vtkMath::Dot(projected, basis [0]);
        answer [1] = vtkMath::Dot(projected, basis [1]);
}

static void projection (double *answer, double *vector, double *normal)
{
        int     i;
        double   t2;

        //t2 = dot_product (vector, normal);
        t2 = vtkMath::Dot(vector, normal);
        for (i = 0; i < 3; i++)
           answer [i] = vector [i] - (t2*normal [i]);
}

static void least_squares (int N, double dc [MAX_NEIGHBORS][2], double dn [MAX_NEIGHBORS][2],
                double *gaussianCurvature, double *meanCurvature, double *prin1,
                double *prin2, double *fold)
{
        int     i;
        double   sum1, sum2, sum3;
        double   a, b, c;
        double   trC, detC, deltaPlus, deltaMinus, k1, k2;
        double   wxy, wx, wy;
        double   t1, temp;  //t2, t3;

        a = b = c = 0.0;
        sum1 = sum2 = sum3 = 0.0;
        wx = wy = wxy = 0.0;
        for (i = 0; i < N; i++){
           sum1 += (dc [i][0] * dn [i][0]);
           sum2 += ((dc [i][0] * dn [i][1]) + (dc [i][1] * dn [i][0]));
           sum3 += (dc [i][1] * dn [i][1]);
           wx   += (dc [i][0] * dc [i][0]);
           wy   += (dc [i][1] * dc [i][1]);
           wxy  += (dc [i][0] * dc [i][1]);
        }

        t1 = (wx + wy)*(-pow (wxy,2) + wx*wy);

        if (t1 > 0.0){
           a = (sum3*pow (wxy,2) - sum2*wxy*wy +
                sum1*(-pow (wxy,2) + wx*wy + pow (wy,2)))/t1;

           b = (-(sum3*wx*wxy) + sum2*wx*wy - sum1*wxy*wy)/t1;

           c = (-(sum2*wx*wxy) + sum1*pow (wxy,2) +
                sum3*(pow (wx,2) - pow (wxy,2) + wx*wy))/t1;
        }
        //else
           //printf ("WARNING: Coefficients zero...\n");

        trC = a + c;
        detC = a*c - b*b;
        temp = trC*trC - 4 *detC;
        if (temp > 0.0){
           deltaPlus = sqrt (temp);
           deltaMinus = -deltaPlus;
           k1 = (trC + deltaPlus)/2.0; /* principal curvature */
           k2 = (trC + deltaMinus)/2.0;
        }
        else{
           //printf ("WARNING: Delta < 0...\n");
           k1 = k2 = deltaPlus = deltaMinus = 0.0;
        }

        *gaussianCurvature = k1 * k2;
        *meanCurvature = (k1 + k2)/2.0;
        *prin1 = k1;
        *prin2 = k2;
        *fold = 0.0;
}

//------------------------------------------------------------------------------



// Constructor
vtkPolyDataCurvature::vtkPolyDataCurvature()
{
   // nothing to initialize at this time
}

//------------------------------------------------------------------------------
void vtkPolyDataCurvature::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolyDataToPolyDataFilter::PrintSelf(os,indent);
}
