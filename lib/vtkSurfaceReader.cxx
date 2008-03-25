/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSurfaceReader.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSurfaceReader.h"

#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkByteSwap.h"
#include "vtkUnsignedCharArray.h"
#include <vtksys/SystemTools.hxx>
#include "vtkPolyDataReader.h"
#include "vtkBICOBJReader.h"
#include "vtkFreesurferReader.h"

vtkCxxRevisionMacro(vtkSurfaceReader, "$Revision: 1.00 $");
vtkStandardNewMacro(vtkSurfaceReader);

// Description:
// Instantiate object with NULL filename.
vtkSurfaceReader::vtkSurfaceReader()
{
  this->FileName = NULL;
  this->SetNumberOfInputPorts(0);
}

vtkSurfaceReader::~vtkSurfaceReader()
{
  if (this->FileName)
    {
    delete [] this->FileName;
    this->FileName = NULL;
    }
}

int vtkSurfaceReader::RequestData(
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
  vtkFloatArray *normals = vtkFloatArray::New();
  vtkCellArray *polys = vtkCellArray::New();
  vtkUnsignedCharArray *scalars = vtkUnsignedCharArray::New();

  char line[1024], Id;
  float xyz[3];
  float s, r, g, b, a;
  int nPoly, nPoint, colorId, i, index, magic;
  FILE *fp;
          
  if((fp = fopen(this->FileName, "rb")) == 0) {
    fprintf(stderr, "input_freesurfer: Couldn't open file %s.\n", this->FileName);
    return(0);
  }

  // read magic number for checking filetype
  magic = Fread3(fp);

  if( magic == QUAD_FILE_MAGIC_NUMBER) {
    fprintf(stderr, "QUAD_FILE_MAGIC_NUMBER not yet prepared %s.\n");
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

  return 1;
}

void vtkSurfaceReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "File Name: " 
     << (this->FileName ? this->FileName : "(none)") << "\n";

}

