/*=========================================================================

Program:   vtkINRIA3D
Module:    $Id: vtkBoxCutPolyLines.cxx 1080 2009-02-18 13:27:02Z acanale $
Language:  C++
Author:    $Author: acanale $
Date:      $Date: 2009-02-18 14:27:02 +0100 (Mi, 18 Feb 2009) $
Version:   $Revision: 1080 $

Copyright (c) 2007 INRIA - Asclepios Project. All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
// version vtkRenderingAddOn
#include <vtkBoxCutPolyLines.h>

#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkObjectFactory.h>
#include <vtkInformationVector.h>
#include <vtkInformation.h>
#include <vtkCellArray.h>

vtkCxxRevisionMacro(vtkBoxCutPolyLines, "$Revision: 1080 $");
vtkStandardNewMacro(vtkBoxCutPolyLines);

vtkBoxCutPolyLines::vtkBoxCutPolyLines()
{

}

//----------------------------------------------------------------------------
// Specify the bounding box for clipping

void vtkBoxCutPolyLines::SetBoxClip(double xmin,double xmax,
                                   double ymin,double ymax,
                                   double zmin,double zmax)
{
  this->XMin = xmin;
  this->XMax = xmax;
  this->YMin = ymin;
  this->YMax = ymax;
  this->ZMin = zmin;
  this->ZMax = zmax;
  this->Modified();
  
}


int vtkBoxCutPolyLines::RequestData (vtkInformation *vtkNotUsed(request),
                                     vtkInformationVector **inputVector,
                                     vtkInformationVector *outputVector)
{
  
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkPolyData *input = vtkPolyData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
  output->Initialize();
  output->Allocate();

  vtkPoints* points = input->GetPoints();
  output->SetPoints ( points );
  output->GetPointData()->SetScalars ( input->GetPointData()->GetScalars() );

  vtkCellArray* lines = input->GetLines();
  if( lines==0 )
    return 0;
  lines->InitTraversal();


  vtkIdType npt, *pto;
  int test = lines->GetNextCell (npt, pto);
  
  while( test!=0 )
  {
    int i=0;
    
    while( i<npt)
    {
      int test2 = 0;
     double* pt = points->GetPoint (pto[i]);

      if(( pt[0]>this->XMin && pt[0]<this->XMax &&
	   pt[1]>this->YMin && pt[1]<this->YMax &&
	   pt[2]>this->ZMin && pt[2]<this->ZMax ))
	test2 = 1;
      
      vtkIdList* idlist = vtkIdList::New();
      
      while (test2 == 1 && i<npt)
      {
	
    	idlist->InsertNextId (pto[i]);
	
	i++;
	if (i<npt)
	  pt = points->GetPoint (pto[i]);
	
	if(( pt[0]>this->XMin && pt[0]<this->XMax &&
	     pt[1]>this->YMin && pt[1]<this->YMax &&
	     pt[2]>this->ZMin && pt[2]<this->ZMax ))
	  test2 = 1;
	else
	  test2 = 0;

	
      }

      output->InsertNextCell (VTK_POLY_LINE, idlist);

      idlist->Delete();
    
      i++;
    }
    
    test = lines->GetNextCell ((vtkIdType&)npt, pto);
  }


  return 1;
  
}
