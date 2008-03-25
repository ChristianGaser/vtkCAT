/*LICENSE_START*/
/*
 *  Copyright 1995-2002 Washington University School of Medicine
 *
 *  http://brainmap.wustl.edu
 *
 *  This file is part of CARET.
 *
 *  CARET is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  CARET is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CARET; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*LICENSE_END*/

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVolumeRawReader.h,v $
  Language:  C++
  Date:      $Date: 2002/11/19 16:32:00 $
  Version:   $Revision: 1.1.1.1 $


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkVolumeRawReader - read image files
// .SECTION Description
// vtkVolumeRawReader is a source object that reads image files.
//
// VolumeRawReader creates structured point datasets.  Reading from a single
// file, it reads a 3D volume.  
//
// This is a modified version of vtkVolumeReader designed to read the entire
// volume from a single file.
//
// The data is assumed to being at the beginning of the file.
// SetDataOffset can be used to skip over any header information.
//
// SetDataType is used to set the type of data in the file.  Regardless of
// the data type in the file, the data is stored in memory as floats.
//
// 

#ifdef HAVE_ZLIB
#define INPUT_STREAM gzFile
#else
#define INPUT_STREAM ifstream*
#endif // HAVE_ZLIB

#ifndef __vtkVolumeRawReader_h
#define __vtkVolumeRawReader_h

#include "vtkFloatArray.h"
#include "vtkStructuredPointsSource.h"
#include "vtkDataObject.h"

#ifdef HAVE_ZLIB
#include "zlib.h"
#endif

class VTK_EXPORT vtkVolumeRawReader : public vtkStructuredPointsSource
{
public:
  static vtkVolumeRawReader *New();
  vtkTypeMacro(vtkVolumeRawReader,vtkStructuredPointsSource);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify file name of vtk data file to read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Specify the dimensions for the data.
  vtkSetVector3Macro(Dimensions,int);
  vtkGetVectorMacro(Dimensions,int,3);

  // Description:
  // Specify the spacing for the data.
  vtkSetVector3Macro(DataSpacing,double);
  vtkGetVectorMacro(DataSpacing,double,3);

  // Description:
  // Specify the origin for the data.
  vtkSetVector3Macro(DataOrigin,double);
  vtkGetVectorMacro(DataOrigin,double,3);

  // Description:
  // Specify the scaling for the data.
  vtkSetMacro(DataScaling,float);
  vtkGetMacro(DataScaling,float);

  // Description
  // Specify the offset in bytes of the volume data (used if file is not 
  // raw but begins with a header).
  vtkSetMacro(VolumeDataOffset,int);
  vtkGetMacro(VolumeDataOffset,int);
  
  // Description
  // Specify the type of data in the volume.  Can be any of:
  //      VTK_CHAR  (default)
  //      VTK_UNSIGNED_CHAR
  //      VTK_SHORT
  //      VTK_UNSIGNED_SHORT
  //      VTK_INT
  //      VTK_UNSIGNED_INT
  //      VTK_LONG
  //      VTK_UNSIGNED_LONG
  //      VTK_FLOAT
  //      VTK_DOUBLE
  vtkSetMacro(VolumeDataType,int);
  vtkGetMacro(VolumeDataType,int);
  
  // Description:
  // Turn on/off byte swapping.
  vtkSetMacro(VolumeDataSwapBytes,int);
  vtkGetMacro(VolumeDataSwapBytes,int);
  vtkBooleanMacro(VolumeDataSwapBytes,int);

  
protected:
  vtkVolumeRawReader();
  ~vtkVolumeRawReader();
  vtkVolumeRawReader(const vtkVolumeRawReader&) {};
  void operator=(const vtkVolumeRawReader&) {};

  int   Dimensions[3];
  double DataSpacing[3];
  double DataOrigin[3];
  float DataScaling;
  int   VolumeDataOffset;
  int   VolumeDataType;
  int   VolumeDataSwapBytes;
  char* FileName;
  
  // The extent type is a 3D extent
  int GetExtentType() { return VTK_3D_EXTENT; };

  void Execute();
  void ReadVolumeData();
  
  int readCharData(INPUT_STREAM input, vtkFloatArray* scalars, int numScalars);
  int readUnsignedCharData(INPUT_STREAM input, vtkFloatArray* scalars, int numScalars);
  int readShortData(INPUT_STREAM input, vtkFloatArray* scalars, int numScalars);
  int readUnsignedShortData(INPUT_STREAM input, vtkFloatArray* scalars, int numScalars);
  int readIntData(INPUT_STREAM input, vtkFloatArray* scalars, int numScalars);
  int readUnsignedIntData(INPUT_STREAM input, vtkFloatArray* scalars, int numScalars);
  int readLongData(INPUT_STREAM input, vtkFloatArray* scalars, int numScalars);
  int readUnsignedLongData(INPUT_STREAM input, vtkFloatArray* scalars, int numScalars);
  int readFloatData(INPUT_STREAM input, vtkFloatArray* scalars, int numScalars);
  int readDoubleData(INPUT_STREAM input, vtkFloatArray* scalars, int numScalars);
  
  void SwapBytes(short* n, int numToSwap);
  void SwapBytes(unsigned short* n, int numToSwap);
  void SwapBytes(int* n, int numToSwap);
  void SwapBytes(unsigned int* n, int numToSwap);
  void SwapBytes(long* n, int numToSwap);
  void SwapBytes(unsigned long* n, int numToSwap);
  void SwapBytes(float* n, int numToSwap);
  void SwapBytes(double* n, int numToSwap);
  
};

#endif


