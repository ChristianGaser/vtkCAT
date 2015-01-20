/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkGiftiReader.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkGiftiReader - read ascii BIC .obj files
// .SECTION Description
// vtkGiftiReader is a source object that reads BIC .obj
// files. The output of this source object is polygonal data.
// .SECTION See Also
// vtkOBJImporter

#ifndef __vtkGiftiReader_h
#define __vtkGiftiReader_h

#include "vtkPolyDataReader.h"

class VTK_IO_EXPORT vtkGiftiReader : public vtkPolyDataReader 
{
public:
  static vtkGiftiReader *New();
  vtkTypeRevisionMacro(vtkGiftiReader,vtkPolyDataReader);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

protected:
  vtkGiftiReader();
  ~vtkGiftiReader();
  
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  char *FileName;
private:
  vtkGiftiReader(const vtkGiftiReader&);  // Not implemented.
  void operator=(const vtkGiftiReader&);  // Not implemented.
};

#endif
