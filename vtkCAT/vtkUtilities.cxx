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

#include "vtkFieldData.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkTriangle.h"
#include "vtkTriangleFilter.h"

#include "vtkPointTopologyHelper.h"
#include "vtkUtilities.h"

// #define sgn(a)          ((a) < 0.0 ? -1.0 : 1.0)        /* NB 0.0 -> 1.0 */
// 
// static double   angle (double *pi, double *pj, double *pk, double *n)
// {
// 
//   double         x1, x2, y1, y2, z1, z2, dx, dy, dz, s, c, phi;
// 
//   x1 = pj [0] - pi [0];
//   y1 = pj [1] - pi [1];
//   z1 = pj [2] - pi [2];
//   x2 = pk [0] - pi [0];
//   y2 = pk [1] - pi [1];
//   z2 = pk [2] - pi [2];
// 
//   /* s = |(ji)||(ki)| sin(phi) by cross product */
//   dx = y1*z2 - y2*z1;
//   dy = x2*z1 - x1*z2;
//   dz = x1*y2 - x2*y1;
//   s = sgn((dx*n [0]) + (dy*n [1]) + (dz*n [2])) *
//                 sqrtf ((dx*dx) + (dy*dy) + (dz*dz));
// 
//   /* c = |(ji)||(ki)| cos(phi) by inner product */
//   c = x1*x2 + y1*y2 + z1*z2;
//   phi = atan2f (s,c);
//   return (phi);
// }
// static void    subtract_vectors (double *v1, double *v2, double *diff)
// {
//         int     i;
// 
//         for (i = 0; i < 3; i++)
//            diff [i] = v1 [i] - v2 [i];
// }
// static double  magnitude (double *vector)
// {
//         return (sqrtf (vector [0]*vector [0] + vector [1]*vector [1] + vector
// [2]*vector [2]));
// }
// static void    unit_normal (double *po, double *pn, double *pm, double *normal)
// {
//         register        int     i;
//         register        double   d_o [3],
//                         d_m [3],
//                         nmag;
// 
//         subtract_vectors (po, pn, d_o);
//         subtract_vectors (pm, pn, d_m);
// 
//         normal [0] = d_o [1]*d_m [2] - d_m [1]*d_o [2];
//         normal [1] = d_m [0]*d_o [2] - d_o [0]*d_m [2];
//         normal [2] = d_o [0]*d_m [1] - d_m [0]*d_o [1];
//         nmag = magnitude (normal);
//         if (nmag > 0.0) {
//            for (i = 0; i < 3; i++)
//               normal [i] /= nmag;
//         }
//         else {
//            for (i = 0; i < 3; i++)
//               normal [i] = 0.0;
//         }
// }
// double   distance  (double *v1, double *v2)
// {
//         register double x, y, z, temp;
// 
// 
//         x = v1 [0] - v2 [0];
//         y = v1 [1] - v2 [1];
//         z = v1 [2] - v2 [2];
//         temp = x*x + y*y + z*z;
//         if (temp > 0.0)
//            return (sqrtf (temp));
//         else
//            return (0.0);
// }
// static void
// caretTriangleArea(const double vtkArea, double p1[3], double p2[3], double p3[3])
// {
//    double   area, w, h, angle2;
//    double normal[3] = { 0.0, 0.0, 0.0 };
//    unit_normal (p1, p2, p3, normal);
//    w = distance (p2, p1);
//    h = distance (p2, p3);
//    if ((w <= 0.0) || (h <= 0.0)){
//       area = 0.0;
//    }
//    else{
//       angle2 = angle (p2, p1, p3, normal);
//       area = 0.5*w*h*sinf (angle2);
//    }
//    const double diff = area - vtkArea;
//    if (diff > 0.000001) {
//       cout << "Triangle area formulas different: " << area << " vs. " 
//            << vtkArea << endl;
//    }
// }

//---------------------------------------------------------------------------
void
vtkUtilities::GetPointsCenter(vtkDataSet* dataSet, double centerOut[3])
{
   dataSet->Update();
   
   double center[3];
   
   center[0] = 0.0;
   center[1] = 0.0;
   center[2] = 0.0;
   for (int i = 0; i < dataSet->GetNumberOfPoints(); i++) {
      double xyz[3];
      dataSet->GetPoint(i, xyz);
      center[0] += xyz[0];
      center[1] += xyz[1];
      center[2] += xyz[2];
   }
   center[0] /= dataSet->GetNumberOfPoints();
   center[1] /= dataSet->GetNumberOfPoints();
   center[2] /= dataSet->GetNumberOfPoints();
   
   centerOut[0] = center[0];
   centerOut[1] = center[1];
   centerOut[2] = center[2];
}


double 
vtkUtilities::GetSurfaceArea(vtkPolyData* polyDataIn)
{
   // vtkTriangleFilter converts triangle strips into triangles
   vtkTriangleFilter* tf = vtkTriangleFilter::New();
      tf->SetInput(polyDataIn);
      tf->Update();
      
   vtkPolyData* polyData = tf->GetOutput();
   if (polyData->GetNumberOfStrips() > 0) {
      cerr<<"Strips not supported in vtkUtilities::GetSurfaceArea\n";
      return 0.0;
   }
   double area = 0.0;
   vtkCellArray *polys = polyData->GetPolys();
   int cellId = 0;
   int npts;
   int* pts;
   for (polys->InitTraversal(); polys->GetNextCell(npts,pts); cellId++) {
      if (npts != 3) {
         cerr<<"Polygon "<<cellId<<" is not a triangle\n";
         return 0.0;
      }
      double xyz1[3], xyz2[3], xyz3[3];
      
      polyData->GetPoint(pts[0], xyz1);
      polyData->GetPoint(pts[1], xyz2);
      polyData->GetPoint(pts[2], xyz3);
      const double triangleArea = vtkTriangle::TriangleArea(xyz1, xyz2, xyz3);
      area += triangleArea;
      
      //caretTriangleArea(triangleArea, xyz1, xyz2, xyz3);
   }
   tf->Delete();
   return area;     
}

int 
vtkUtilities::AverageFloatDataWithNeighbors(vtkPolyData* polyData,
                                            double* values)
{
   
   const int numberOfPoints = polyData->GetNumberOfPoints();
   double* averageValue      = new double[numberOfPoints];
   vtkPointTopologyHelperNew* topology = new vtkPointTopologyHelperNew(polyData);
   for (int i = 0; i < numberOfPoints; i++) {
      int* neighbors;
      int  numNeighbors;
      averageValue[i] = values[i];
      if (topology->GetNeighborsForPoint(i, neighbors, numNeighbors) == 0) {
         for (int j = 0; j < numNeighbors; j++) {
            averageValue[i] += values[neighbors[j]];
         }
      }
      averageValue[i] /= (double)(numNeighbors + 1);
   }
   
   for (int j = 0; j < numberOfPoints; j++) {
      values[j] = averageValue[j];
   }
   
   delete topology;

   delete[] averageValue;
   
   return 0;
}

int 
vtkUtilities::WriteMetricFile(vtkFloatArray* data, const char* fileName,
                              bool writeAsInteger)
{
   ofstream out(fileName);
   if (!out) {
      cout << "ERROR: Unable to open " << fileName 
           << "for output." << endl;
      return 1;
   }
   else {
      for (int i = 0; i < data->GetNumberOfTuples(); i++) {
         if (writeAsInteger) {
            out << i << " " << (int)data->GetValue(i) << endl;
         }
         else {
            out << i << " " << data->GetValue(i)  << endl;
         }
      }
      out.close();
   }  
   return 0; 
}

