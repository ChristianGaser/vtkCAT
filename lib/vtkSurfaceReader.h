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
// .NAME vtkSurfaceReader - read ascii BIC .obj and freesurfer files
// .SECTION Description
// vtkSurfaceReader is a source object that reads BIC .obj and freesurfer
// files. The output of this source object is PolyDataReader.
// .SECTION See Also
// vtkFreesurferReader vtkBICOBJReader

#ifndef __vtkSurfaceReader_h
#define __vtkSurfaceReader_h

#include "vtkBICOBJReader.h"
#include "vtkFreesurferReader.h"
#include "vtkDataReader.h"

class vtkPolyData;

class VTK_IO_EXPORT vtkSurfaceReader : public vtkDataReader 
{
public:
  static vtkSurfaceReader *New();
  vtkTypeRevisionMacro(vtkSurfaceReader,vtkDataReader);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  enum SURFACE_TYPE {
    SURFACE_TYPE_FREESURFER  = 0,
    SURFACE_TYPE_BICOBJ      = 1,    
    SURFACE_TYPE_VTKPOLYDATA = 2
  };

  vtkPolyData *GetOutput();
  vtkPolyData *GetOutput(int idx);
  void SetOutput(vtkPolyData *output);

  virtual int GetSurfaceType(void);

  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  unsigned int CanReadFile(char* fn);

  void ReadBICOBJ(char* filename);
  void ReadVTK(char* filename);
  void ReadFreesurfer(char* filename);

protected:
  vtkSurfaceReader();
  ~vtkSurfaceReader();
  
  virtual int RequestData(vtkInformation *, vtkInformationVector **,
                          vtkInformationVector *);

  virtual int FillOutputPortInformation(int, vtkInformation*);

private:
  vtkSurfaceReader(const vtkSurfaceReader&);  // Not implemented.
  void operator=(const vtkSurfaceReader&);  // Not implemented.
};

#endif
