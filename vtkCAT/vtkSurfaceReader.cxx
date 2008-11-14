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
#include "vtkFieldData.h"
#include "vtkFloatArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include <vtksys/SystemTools.hxx>
#include <vtkErrorCode.h>
#include "vtkExecutive.h"

vtkCxxRevisionMacro(vtkSurfaceReader, "$Revision: 1.00 $");
vtkStandardNewMacro(vtkSurfaceReader);

// Description:
// Instantiate object with NULL this->FileName.
vtkSurfaceReader::vtkSurfaceReader()
{
  vtkPolyData *output = vtkPolyData::New();
  this->SetOutput(output);
  this->FileName = NULL;
  output->SetMaximumNumberOfPieces(1);
  output->ReleaseData();
  output->Delete();
}

vtkSurfaceReader::~vtkSurfaceReader()
{
}

vtkPolyData *vtkSurfaceReader::GetOutput()
{
  return this->GetOutput(0);
}

vtkPolyData *vtkSurfaceReader::GetOutput(int port)
{
  return vtkPolyData::SafeDownCast(this->GetOutputDataObject(port));
}

void vtkSurfaceReader::SetOutput(vtkPolyData *output)
{
  this->GetExecutive()->SetOutputData(0, output);
}

int vtkSurfaceReader::GetSurfaceType()
{
  int surfaceType = 0;
  
  if (!strcmp(vtksys::SystemTools::GetFilenameLastExtension(this->FileName).c_str(),".obj")) 
      surfaceType = 1;
  if (!strcmp(vtksys::SystemTools::GetFilenameLastExtension(this->FileName).c_str(),".vtk")) 
      surfaceType = 2;
  return(surfaceType);
}

void vtkSurfaceReader::ReadBICOBJ(char* filename)
{

  vtkBICOBJReader* reader = vtkBICOBJReader::New();
  reader->SetFileName (filename);
  
  try
  {
    reader->Update();
    this->SetFileName(reader->GetFileName());
    this->SetOutput(reader->GetOutput());
    this->GetOutput()->SetMaximumNumberOfPieces(1);
  }
  catch (vtkErrorCode::ErrorIds error)
  {
    reader->Delete();
    throw error;
  }
  reader->Delete();
}

void vtkSurfaceReader::ReadVTK(char* filename)
{
  vtkPolyDataReader* reader = vtkPolyDataReader::New();
  reader->SetFileName (filename);
  
  try
  {
    reader->Update();
    this->SetFileName(reader->GetFileName());
    this->SetOutput(reader->GetOutput());
    this->GetOutput()->SetMaximumNumberOfPieces(1);
  }
  catch (vtkErrorCode::ErrorIds error)
  {
    reader->Delete();
    throw error;
  }
  reader->Delete();
}


void vtkSurfaceReader::ReadFreesurfer(char* filename)
{

  vtkFreesurferReader* reader = vtkFreesurferReader::New();
  reader->SetFileName (filename);
  
  try
  {
    reader->Update();
    this->SetFileName(reader->GetFileName());
    this->SetOutput(reader->GetOutput());
    this->GetOutput()->SetMaximumNumberOfPieces(1);
  }
  catch (vtkErrorCode::ErrorIds error)
  {
    reader->Delete();
    throw error;
  }
  reader->Delete();
}

unsigned int vtkSurfaceReader::CanReadFile (char* filename)
{

  if (!strcmp(vtksys::SystemTools::GetFilenameLastExtension(filename).c_str(),".obj")) 
  {
    vtkBICOBJReader* reader = vtkBICOBJReader::New();
    reader->SetFileName (filename);
    try
    {
      reader->Update();
    }
    catch(vtkErrorCode::ErrorIds)
    {
      reader->Delete();
      return 0;
    }
    reader->Delete();
    return vtkSurfaceReader::SURFACE_TYPE_BICOBJ;
  }
  
  if (!strcmp(vtksys::SystemTools::GetFilenameLastExtension(filename).c_str(),".vtk")) 
  {
    try
    {
      vtkPolyDataReader* reader = vtkPolyDataReader::New();
      reader->SetFileName (filename);
      if (reader->IsFilePolyData ())
      {
        reader->Delete();
        return vtkSurfaceReader::SURFACE_TYPE_VTKPOLYDATA;
      }
      reader->Delete();
    }
    catch (vtkErrorCode::ErrorIds)
    {
    }
  }

  vtkFreesurferReader* reader = vtkFreesurferReader::New();
  reader->SetFileName (filename);
  try
  {
    reader->Update();
  }
  catch(vtkErrorCode::ErrorIds)
  {
    reader->Delete();
    return 0;
  }
  reader->Delete();
  return vtkSurfaceReader::SURFACE_TYPE_FREESURFER;

}


int vtkSurfaceReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
    
  output->SetMaximumNumberOfPieces(1);
  
  if (!this->FileName) 
  {
    vtkErrorMacro(<< "A FileName must be specified.");
    return 1;
  }

  try
  {
    
    switch (vtkSurfaceReader::CanReadFile(this->FileName))
    {
	case vtkSurfaceReader::SURFACE_TYPE_FREESURFER :
	  vtkSurfaceReader::ReadFreesurfer(this->FileName);
	  break;
	case vtkSurfaceReader::SURFACE_TYPE_BICOBJ :
	  vtkSurfaceReader::ReadBICOBJ(this->FileName);
	  break;
	case vtkSurfaceReader::SURFACE_TYPE_VTKPOLYDATA :
	  vtkSurfaceReader::ReadVTK(this->FileName);
	  break;
	default :
	  vtkErrorMacro(<<"unknown dataset type : "<<this->FileName<<endl);
	  throw vtkErrorCode::UnrecognizedFileTypeError;
    }
    
  }
  catch (vtkErrorCode::ErrorIds error)
  {
    throw error; 
  }  
}

int vtkSurfaceReader::FillOutputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkPolyData::DATA_TYPE_NAME(), "vtkPolyData");
  return 1;
}


void vtkSurfaceReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "File Name: " 
     << (this->FileName ? this->FileName : "(none)") << "\n";

}

