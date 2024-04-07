/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkFreesurferReader.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkFreesurferReader - read Freesurfer files
// .SECTION Description
// vtkFreesurferReader is a source object that reads Freesurfer
// files. The output of this source object is polygonal data.
// .SECTION See Also
// vtkOBJImporter

#ifndef __vtkFreesurferReader_h
#define __vtkFreesurferReader_h

#ifndef TRIANGLE_FILE_MAGIC_NUMBER
#define TRIANGLE_FILE_MAGIC_NUMBER  16777214
#endif
#ifndef QUAD_FILE_MAGIC_NUMBER
#define QUAD_FILE_MAGIC_NUMBER  16777215
#endif

#include "vtkPolyDataReader.h"

//class VTKIOLEGACY_EXPORT vtkFreesurferReader : public vtkPolyDataReader 
class VTK_IO_EXPORT vtkFreesurferReader : public vtkPolyDataReader 
{
public:
  static vtkFreesurferReader *New();
  vtkTypeMacro(vtkFreesurferReader,vtkPolyDataReader);
  void PrintSelf(ostream& os, vtkIndent indent);

  int FreadInt(FILE *fp);
  float FreadFloat(FILE *fp);
  int Fread3(FILE *fp);

  // Description:
  // Specify file name of Freesurfer file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

protected:
  vtkFreesurferReader();
  ~vtkFreesurferReader();
  
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  char *FileName;
private:
  vtkFreesurferReader(const vtkFreesurferReader&);  // Not implemented.
  void operator=(const vtkFreesurferReader&);  // Not implemented.
};

#endif
