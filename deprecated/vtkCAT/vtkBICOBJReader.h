/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkBICOBJReader.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkBICOBJReader - read ascii BIC .obj files
// .SECTION Description
// vtkBICOBJReader is a source object that reads BIC .obj
// files. The output of this source object is polygonal data.
// .SECTION See Also
// vtkOBJImporter

#ifndef __vtkBICOBJReader_h
#define __vtkBICOBJReader_h

#include "vtkPolyDataReader.h"

//class VTKIOLEGACY_EXPORT vtkBICOBJReader : public vtkPolyDataReader 
class VTK_IO_EXPORT vtkBICOBJReader : public vtkPolyDataReader 
{
public:
  static vtkBICOBJReader *New();
  vtkTypeMacro(vtkBICOBJReader,vtkPolyDataReader);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify file name of BIC .obj file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

protected:
  vtkBICOBJReader();
  ~vtkBICOBJReader();
  
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  char *FileName;
private:
  vtkBICOBJReader(const vtkBICOBJReader&);  // Not implemented.
  void operator=(const vtkBICOBJReader&);  // Not implemented.
};

#endif
