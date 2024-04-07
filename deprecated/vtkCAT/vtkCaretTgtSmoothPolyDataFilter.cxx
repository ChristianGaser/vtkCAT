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
  Module:    $RCSfile: vtkCaretTgtSmoothPolyDataFilter.cxx,v $
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
#include "vtkTriangle.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkPointLocator.h"
#include "vtkPolyData.h"

#include "vtkCaretTgtSmoothPolyDataFilter.h"
#include "vtkPointTopologyHelper.h"

//------------------------------------------------------------------------------
vtkCaretTgtSmoothPolyDataFilter* vtkCaretTgtSmoothPolyDataFilter::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkCaretTgtSmoothPolyDataFilter");
  if(ret)
    {
    return (vtkCaretTgtSmoothPolyDataFilter*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkCaretTgtSmoothPolyDataFilter;
}





// The following code defines methods for the vtkCaretTgtSmoothPolyDataFilter class
//

// Construct object with number of iterations 20; relaxation factor 1.0;
// boundary smoothing turned on. 
vtkCaretTgtSmoothPolyDataFilter::vtkCaretTgtSmoothPolyDataFilter()
{
  this->NumberOfIterations = 20;

  this->RelaxationFactor = 1.0;

  this->BoundarySmoothing = 1;
  
  this->TargetedSmoothing = 0;
  this->TargetedPoints    = NULL;
  this->TargetedPointsSize = 0;

}

void vtkCaretTgtSmoothPolyDataFilter::SetTargetedPoints(int* pointsToSmooth, 
                                                        int numPoints)
{
   if (this->TargetedPoints != NULL) {
      if (numPoints != this->TargetedPointsSize) {
         delete this->TargetedPoints;
         this->TargetedPoints = NULL;
         this->TargetedPointsSize = 0;
      }
   }
   
   if (this->TargetedPoints == NULL) {
      this->TargetedPoints = new int[numPoints];
      this->TargetedPointsSize = numPoints;
   }
   for (int i = 0; i < numPoints; i++) {
      this->TargetedPoints[i] = pointsToSmooth[i];
   }
} 
    
void vtkCaretTgtSmoothPolyDataFilter::Execute()
{
  int iterationNumber, abortExecute;
      int* neighbors;
      int numNeighbors;
  vtkPolyData *input=this->GetInput();
  vtkPolyData *output=this->GetOutput();

  // Check input
  //
  int numPts=input->GetNumberOfPoints();
  if (numPts < 1)
    {
    vtkErrorMacro(<<"No data to smooth!");
    return;
    }

  vtkDebugMacro(<<"Smoothing " << numPts 
               << "\tIterations= " << this->NumberOfIterations << "\n"
               << "\tRelaxation Factor= " << this->RelaxationFactor << "\n"
               << "\tBoundary Smoothing " << (this->BoundarySmoothing ? "On\n" : "Off\n"));

  if ( this->NumberOfIterations <= 0 || this->RelaxationFactor == 0.0) 
    { //don't do anything! pass data through
    output->CopyStructure(input);
    output->GetPointData()->PassData(input->GetPointData());
    output->GetCellData()->PassData(input->GetCellData());
    return;
    }

  // build a list of neighbors for all points
  vtkDebugMacro(<<"Determining topology...");
  vtkPointTopologyHelperNew* topology = new vtkPointTopologyHelperNew(input);

  vtkPoints* inPts = input->GetPoints();
  
  this->UpdateProgress(0.10);

  vtkDebugMacro(<<"Beginning smoothing iterations...");

  // We've setup the topology...now perform Laplacian smoothing
  //
  vtkPoints *newPts = vtkPoints::New();
  newPts->SetNumberOfPoints(numPts);
  vtkPoints *tempPts = vtkPoints::New();
  tempPts->SetNumberOfPoints(numPts);
  for (int i=0; i<numPts; i++) //initialize to old coordinates
    {
    newPts->SetPoint(i,inPts->GetPoint(i));
    }

  for ( iterationNumber=0, abortExecute=0; 
       (iterationNumber < this->NumberOfIterations) && !abortExecute;
       iterationNumber++ )
    {
    if ( iterationNumber && !(iterationNumber % 5) )
      {
      this->UpdateProgress (0.1 + 0.9*iterationNumber/this->NumberOfIterations);
      if (this->GetAbortExecute())
        {
        abortExecute = 1;
        break;
        }
      }

    for (int h = 0; h < numPts; h++) {
       tempPts->SetPoint(h, newPts->GetPoint(h));
    }
    
    for (int i=0; i<numPts; i++) 
      {
      if (topology->GetNeighborsForPoint(i, neighbors, numNeighbors) != 0) {
         vtkErrorMacro(<<"topology->GetNeighborsForPoint failed");
         return;
      }
      const int MAX_NEIGHBORS = 100;
      
      int smoothPoint = 0;
      if ( (numNeighbors > 1) && (numNeighbors < MAX_NEIGHBORS) ) {
         if (this->TargetedSmoothing && (this->TargetedPoints != NULL)) {
            smoothPoint = this->TargetedPoints[i];
         }
         else {
            smoothPoint = 1;
         }
      }
      
      if (smoothPoint) {
         double myXyz[3];
         newPts->GetPoint(i, myXyz);
         float averageCenter[MAX_NEIGHBORS][3]; 
         float areas[MAX_NEIGHBORS];
         float totalArea = 0.0;
         int j;
         for (j = 0; j < numNeighbors; j++) {
            int neighbor = neighbors[j];
            int nextNeighborIndex = j + 1;
            if (nextNeighborIndex >= numNeighbors) {
               nextNeighborIndex = 0;
            }
            int nextNeighbor = neighbors[nextNeighborIndex];
            double n1Xyz[3], n2Xyz[3];
            newPts->GetPoint(neighbor, n1Xyz);
            newPts->GetPoint(nextNeighbor, n2Xyz);

            areas[j] = vtkTriangle::TriangleArea(myXyz, n1Xyz, n2Xyz);
            for (int k = 0; k < 3; k++) {
               averageCenter[j][k] = (myXyz[k] + n1Xyz[k] + n2Xyz[k]) / 3.0;
            }
            totalArea += areas[j];
         }
         
         float xw = 0.0, yw = 0.0, zw = 0.0;
         
         if (totalArea > 0.0) {
            for (j = 0; j < numNeighbors; j++) {
               const float weight = areas[j] / totalArea;
               xw += weight * averageCenter[j][0];
               yw += weight * averageCenter[j][1];
               zw += weight * averageCenter[j][2];
            }

            float xyz[3];
            for (j = 0; j < numNeighbors; j++) {
               xyz[0] = myXyz[0] * (1.0 - this->RelaxationFactor) 
                      + xw * this->RelaxationFactor;
               xyz[1] = myXyz[1] * (1.0 - this->RelaxationFactor) 
                      + yw * this->RelaxationFactor;
               xyz[2] = myXyz[2] * (1.0 - this->RelaxationFactor) 
                      + zw * this->RelaxationFactor;
            }
            tempPts->SetPoint(i, xyz);
         }
      }
    }
    
    for (int j = 0; j < numPts; j++) {
       newPts->SetPoint(j, tempPts->GetPoint(j));
    }
  } //for within iteration count

  vtkDebugMacro(<<"Performed " << iterationNumber << " smoothing passes");

  // Update output. Only point coordinates have changed.
  //
  output->GetPointData()->PassData(input->GetPointData());
  output->GetCellData()->PassData(input->GetCellData());

  // update geometry to output 
  output->SetPoints(newPts);
  newPts->Delete();

  output->SetVerts(input->GetVerts());
  output->SetLines(input->GetLines());
  output->SetPolys(input->GetPolys());
  output->SetStrips(input->GetStrips());

}

void vtkCaretTgtSmoothPolyDataFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolyDataToPolyDataFilter::PrintSelf(os,indent);

  os << indent << "Number of Iterations: " << this->NumberOfIterations << "\n";
  os << indent << "Relaxation Factor: " << this->RelaxationFactor << "\n";
  os << indent << "Boundary Smoothing: " << (this->BoundarySmoothing ? "On\n" : "Off\n");
}
