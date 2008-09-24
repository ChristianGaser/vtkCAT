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


#ifndef __vtkAnalyzeImageReader_H__
#define __vtkAnalyzeImageReader_H__

#include "vtkAnalyzeRawReader.h"

class VTK_EXPORT vtkAnalyzeImageReader : public vtkAnalyzeRawReader 
{
public:
  vtkTypeRevisionMacro(vtkAnalyzeImageReader,vtkAnalyzeRawReader);

  static vtkAnalyzeImageReader *New();
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the file name.
  virtual void SetFileName(const char *name);

  // Description:
  // Get the entension for this file format. 
  virtual const char* GetFileExtensions() {
    return ".hdr"; }

  // Description:
  // Get the name of this file format.
  virtual const char* GetDescriptiveName() {
    return "ANALYZE"; }

  enum ANALYZE_DATA_TYPE { 
      ANALYZE_DATA_TYPE_UNKNOWN = 0,
      ANALYZE_DATA_TYPE_BINARY = 1,
      ANALYZE_DATA_TYPE_UNSIGNED_BYTE = 2, 
      ANALYZE_DATA_TYPE_SHORT = 4, 
      ANALYZE_DATA_TYPE_INT = 8,
      ANALYZE_DATA_TYPE_FLOAT = 16, 
      ANALYZE_DATA_TYPE_COMPLEX = 32,   // two float, real and imaginary
      ANALYZE_DATA_TYPE_DOUBLE = 64, 
      ANALYZE_DATA_TYPE_RGB = 128
  };
  
  // Description
  // AFNI files may contain multiple volumes.
  vtkSetMacro(NumberOfVolumes,int);
  vtkGetMacro(NumberOfVolumes,int);
  
  // Description:
  // Read AFNI Header file for dimensions, number of volumes, etc.
  void ReadHeader();
  
  // Description:
  // May have more than one volume in an AFNI file.
  // Volumes are numbered starting at zero.
  vtkSetMacro(VolumeToRead,int);
  vtkGetMacro(VolumeToRead,int);
  
  // Description
  // AFNI volume are also used by SPM with some additional parmaters.
  vtkSetMacro(SpmFlag,int);
  vtkGetMacro(SpmFlag,int);
  vtkBooleanMacro(SpmFlag,int);
  
  // Description:
  // SPM maintains location of Anterior Commissure.
  vtkSetVector3Macro(SpmAcPosition,int);
  vtkGetVectorMacro(SpmAcPosition,int,3);
  
protected:
  vtkAnalyzeImageReader();
  ~vtkAnalyzeImageReader();
    
  int FileNameHasChanged;

  int SpmFlag;
  int SpmAcPosition[3];
  
  int NumberOfVolumes;
  int haveSwapBytes;
  int haveDimensions;
  ANALYZE_DATA_TYPE analyzeDataType;
  int VolumeToRead;
  
  void Execute();

private:
  vtkAnalyzeImageReader(const vtkAnalyzeImageReader&); // Not implemented
  void operator=(const vtkAnalyzeImageReader&);  // Not implemented
};

#endif
