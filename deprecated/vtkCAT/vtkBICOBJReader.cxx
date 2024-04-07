/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkBICOBJReader.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkBICOBJReader.h"

#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkProperty.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
//#include "vtkUnsignedCharArray.h"

//vtkCxxRevisionMacro(vtkBICOBJReader, "$Revision$");
vtkStandardNewMacro(vtkBICOBJReader);

// Description:
// Instantiate object with NULL filename.
vtkBICOBJReader::vtkBICOBJReader()
{
  this->FileName = NULL;
  this->SetNumberOfInputPorts(0);
}

vtkBICOBJReader::~vtkBICOBJReader()
{
  if (this->FileName)
    {
    delete [] this->FileName;
    this->FileName = NULL;
    }
}

int vtkBICOBJReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (!this->FileName) 
    {
    vtkErrorMacro(<< "A FileName must be specified.");
    return 1;
    }
    
  ifstream input(this->FileName);
  vtkDebugMacro(<<"Reading file");

  // intialise some structures to store the file contents in
  vtkPoints *points = vtkPoints::New(); 
  vtkFloatArray *normals = vtkFloatArray::New();
  normals->SetNumberOfComponents(3);
  vtkCellArray *polys = vtkCellArray::New();
  vtkProperty *property = vtkProperty::New();
  vtkUnsignedCharArray *scalars = vtkUnsignedCharArray::New();
  scalars->SetNumberOfComponents(4);

  char line[1024], Id;
  float xyz[3], ambient, diffuse, specular, specular_exponent, opacity;
  float r, g, b, a;
  int nPoly, nPoint, colorId, i, index;
  
  input >> Id >> ambient >> diffuse >> specular >> specular_exponent >> opacity >> nPoint;
  if(Id == 'p') {
    vtkDebugMacro(<<"Only ascii BIC obj format can be read.");
    return 1;
  } else if(! Id == 'P') {
    vtkDebugMacro(<<"Wrong format.");
    return 1;
  }
  property->SetAmbient(ambient);
  property->SetDiffuse(diffuse);
  property->SetSpecular(specular);
  property->SetSpecularPower(specular_exponent);
  property->SetOpacity(opacity);
  scalars->SetNumberOfTuples(nPoint);
        
  // read points
  for (i=0; i<nPoint; ++i) {
    input.getline(line, 1024);
    input >> xyz[0] >> xyz[1] >> xyz[2];
    points->InsertNextPoint(xyz);
  }
  
  // read normals
  for (i=0; i<nPoint; ++i) {
    input.getline(line, 1024);
    // skip blank lines
    while (line == "")
      input.getline(line, 1024);    
    input >> xyz[0] >> xyz[1] >> xyz[2];
    normals->InsertNextTuple(xyz);
  }

  // skip blank lines
  while (line == "")
     input.getline(line, 1024);    
  
  // number of polygons
  input >> nPoly;
  // get color information
  input.getline(line, 1024); 
  input >> colorId >> r >> g >> b >> a;
  if((colorId != 2) && (colorId !=0)) {
    vtkErrorMacro(<< "Bad color information");
    return 1;
  }
  scalars->SetTuple4(0, 255*r, 255*g, 255*b, 255*a);
  
  // read color information
  if(colorId == 0) {
   property->SetColor(r,g,b);
  } else {
    for (i=1; i<nPoint; ++i) {
      input.getline(line, 1024);
      input >> r >> g >> b >> a;
      scalars->SetTuple4(i, 255*r, 255*g, 255*b, 255*a);
    }
  }
  
  // skip lines with redundant information
  input.getline(line, 1024);
  for (i=0; i<nPoly; ++i) {
    input >> index;
  }
  
  // blank line
  input.getline(line, 1024);

  // read indices
  for (i=0; i<nPoly; ++i) {
    polys->InsertNextCell(3);
    for (int j=0; j<3; ++j) {
      input >> index;
      polys->InsertCellPoint(index);
    }
  }

  output->SetPoints(points);
  output->SetPolys(polys);
  output->GetPointData()->SetNormals(normals);
  if(colorId == 2) {
        output->GetPointData()->SetScalars(scalars);
  }
  
  points->Delete();
  normals->Delete();
  polys->Delete();
  scalars->Delete();
  property->Delete();

  return 1;
}

void vtkBICOBJReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "File Name: " 
     << (this->FileName ? this->FileName : "(none)") << "\n";

}

