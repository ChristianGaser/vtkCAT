/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSurfaceReader.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSurfaceReader - read various surface files
// .SECTION Description
// vtkSurfaceReader is a source object that reads surface
// files. The output of this source object is polygonal data.
// .SECTION See Also
// vtkOBJImporter

#ifndef __vtkSurfaceReader_h
#define __vtkSurfaceReader_h

#include "vtkPolyDataReader.h"

class VTK_IO_EXPORT vtkSurfaceReader : public vtkPolyDataReader 
{
public:
  static vtkSurfaceReader *New();
  vtkTypeRevisionMacro(vtkSurfaceReader,vtkPolyDataReader);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify file name of Surface file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

protected:
  vtkSurfaceReader();
  ~vtkSurfaceReader();
  
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  char *FileName;
private:
  vtkSurfaceReader(const vtkSurfaceReader&);  // Not implemented.
  void operator=(const vtkSurfaceReader&);  // Not implemented.
};

#endif
