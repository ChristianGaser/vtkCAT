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
  Module:    $RCSfile: vtkEllipsoidProjectPolyDataFilter.cxx,v $
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

#include "vtkEllipsoidProjectPolyDataFilter.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkCellData.h"
#include "vtkPointData.h"

//------------------------------------------------------------------------------
vtkEllipsoidProjectPolyDataFilter* vtkEllipsoidProjectPolyDataFilter::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret =
         vtkObjectFactory::CreateInstance("vtkEllipsoidProjectPolyDataFilter");
  if(ret)
    {
    return (vtkEllipsoidProjectPolyDataFilter*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkEllipsoidProjectPolyDataFilter;
}

//------------------------------------------------------------------------------
void vtkEllipsoidProjectPolyDataFilter::SetEllipsoidDimensions(float x, float y, float z)
{
   this->EllipsoidSizeX = x;
   this->EllipsoidSizeY = y;
   this->EllipsoidSizeZ = z;
   this->Modified();
}

//------------------------------------------------------------------------------
void vtkEllipsoidProjectPolyDataFilter::Execute()
{

   vtkDebugMacro(<<"Ellipsoid projecting surface");

   vtkPolyData *input = this->GetInput();
   vtkPolyData *output = this->GetOutput();

   const int numPoints = input->GetNumberOfPoints();
   if (numPoints < 1) {
      vtkErrorMacro(<<"No data to generate ellipsoidt for!");
      return;
   }

   // input points and polygons 
   vtkPoints *inputPoints = input->GetPoints();
   //vtkCellArray *inPolys = input->GetPolys();
   //vtkPointData* pointData = input->GetPointData();
   
   const int numInputPoints = input->GetNumberOfPoints();
   
   // copy ellipsoid points with inflation
   vtkPoints* ellipsoidPoints = vtkPoints::New();
   ellipsoidPoints->SetNumberOfPoints(numInputPoints);
   
   for (int i = 0; i < numInputPoints; i++) {
      double xyz[3];
      inputPoints->GetPoint(i, xyz);
      float x = xyz[0] / this->EllipsoidSizeX;
      float y = xyz[1] / this->EllipsoidSizeY;
      float z = xyz[2] / this->EllipsoidSizeZ;
      float r = sqrt(x*x + y*y + z*z);
      float k = 1.0 + this->InflationFactor * (1.0 - r);
      xyz[0] *= k;
      xyz[1] *= k;
      xyz[2] *= k;
      ellipsoidPoints->SetPoint(i, xyz);
   }
   
   // "Reference Copy" the geometric and topological data.
   //output->CopyStructure(input);

   // "Reference Copy" the cells' attribute data.
   output->GetCellData()->PassData(input->GetCellData());
   
   // Point the output to the input    
   // NEED TO COPY ALL EXCEPT POINT FIELD DATA SINCE WE WILL BE MODIFYING IT.
   
   // "Reference Copy" points' attribute.
   output->GetPointData()->PassData(input->GetPointData());

   // replace output points with points projected to ellipsoid
   output->SetPoints(ellipsoidPoints);
   ellipsoidPoints->Delete();
   
   output->SetVerts(input->GetVerts());
   output->SetLines(input->GetLines());
   output->SetPolys(input->GetPolys());
   output->SetStrips(input->GetStrips());

}


//------------------------------------------------------------------------------
// Constructor
vtkEllipsoidProjectPolyDataFilter::vtkEllipsoidProjectPolyDataFilter()
{
   EllipsoidSizeX  = 1.0;
   EllipsoidSizeY  = 1.0;
   EllipsoidSizeZ  = 1.0;
   InflationFactor = 1.0;
}

//------------------------------------------------------------------------------
void vtkEllipsoidProjectPolyDataFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolyDataToPolyDataFilter::PrintSelf(os,indent);
}
