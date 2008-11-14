/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkFreesurferReader.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkFreesurferReader.h"

#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkProperty.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkByteSwap.h"
#include "vtkUnsignedCharArray.h"

vtkCxxRevisionMacro(vtkFreesurferReader, "$Revision: 1.00 $");
vtkStandardNewMacro(vtkFreesurferReader);

// Description:
// Instantiate object with NULL filename.
vtkFreesurferReader::vtkFreesurferReader()
{
  this->FileName = NULL;
  this->SetNumberOfInputPorts(0);
}

vtkFreesurferReader::~vtkFreesurferReader()
{
  if (this->FileName)
    {
    delete [] this->FileName;
    this->FileName = NULL;
    }
}

int vtkFreesurferReader::FreadInt(FILE *fp)
{
    unsigned char b1,b2,b3,b4;
    int count;
    
    count = fread(&b1, 1, 1, fp);
    count = fread(&b2, 1, 1, fp);
    count = fread(&b3, 1, 1, fp);
    count = fread(&b4, 1, 1, fp);
    return((b2 << 16) + (b3 << 8) + b4) ;

}

float vtkFreesurferReader::FreadFloat(FILE *fp)
{
    unsigned char b1,b2,b3,b4;
    int count;
		
    count = fread(&b1, 1, 1, fp);
    count = fread(&b2, 1, 1, fp);
    count = fread(&b3, 1, 1, fp);
    count = fread(&b4, 1, 1, fp);
	long temp = (long)(b1 << 24 | b2 << 16 | b3 << 8 | b4);
	return *((float *) &temp);		// convert long into a float

}

int vtkFreesurferReader::Fread3(FILE *fp)
{
    unsigned char b1,b2,b3;
    int count;
    
    count = fread(&b1, 1, 1, fp);
    count = fread(&b2, 1, 1, fp);
    count = fread(&b3, 1, 1, fp);
    return((b1 << 16) + (b2 << 8) + b3) ;

}

int vtkFreesurferReader::RequestData(
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
    
  vtkDebugMacro(<<"Reading file");

  // intialise some structures to store the file contents in
  vtkPoints *points = vtkPoints::New(); 
  vtkCellArray *polys = vtkCellArray::New();
  vtkProperty *property = vtkProperty::New();

  char line[1024];
  float xyz[3];
  int nPoly, nPoint, i, index, magic;
  FILE *fp;
          
  if((fp = fopen(this->FileName, "rb")) == 0) {
    fprintf(stderr, "input_freesurfer: Couldn't open file %s.\n", this->FileName);
    return(0);
  }

  property->SetAmbient(0.3);
  property->SetDiffuse(0.3);
  property->SetSpecular(0.4);
  property->SetSpecularPower(10);
  property->SetOpacity(1);
  property->SetColor(1,1,1);

  // read magic number for checking filetype
  magic = Fread3(fp);

  if( magic == QUAD_FILE_MAGIC_NUMBER) {
    fprintf(stderr, "QUAD_FILE_MAGIC_NUMBER not yet prepared.\n");
    return(0);
  } else if( magic == TRIANGLE_FILE_MAGIC_NUMBER) {
    fgets(line, 1024, fp);
    fscanf(fp, "\n") ;
    /* read # of vertices and faces */
    nPoint = FreadInt(fp);
    nPoly  = FreadInt(fp);

	// read points
	for (i=0; i<nPoint; ++i) {
		xyz[0] = FreadFloat(fp);
		xyz[1] = FreadFloat(fp);
		xyz[2] = FreadFloat(fp);
		points->InsertNextPoint(xyz);
	}

	// read indices
	for (i=0; i<nPoly; ++i) {
		polys->InsertNextCell(3);
		for (int j=0; j<3; ++j) {
			index = FreadInt(fp);
			polys->InsertCellPoint(index);
		}
	}

    fclose(fp);

	output->SetPoints(points);
	output->SetPolys(polys);

  }
  
  points->Delete();
  polys->Delete();
  property->Delete();

  return 1;
}

void vtkFreesurferReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "File Name: " 
     << (this->FileName ? this->FileName : "(none)") << "\n";

}

