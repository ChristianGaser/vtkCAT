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

#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include "vtkStringArray.h"
#include "vtkCharArray.h"
#include "vtkUnsignedCharArray.h"
#include "vtkShortArray.h"
#include "vtkIntArray.h"
#include "vtkFloatArray.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkMatrix4x4.h"
#include "vtkSmartPointer.h"
#include "vtkMath.h"

#include "vtkType.h"

#include "vtkAnalyzeImageReader.h"
#include "dbh.h"


//--------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkAnalyzeImageReader, "$Revision: 1.00 $");
vtkStandardNewMacro(vtkAnalyzeImageReader);


// Construct object 
vtkAnalyzeImageReader::vtkAnalyzeImageReader()
{
  this->haveDimensions = 0;
  this->haveSwapBytes  = 0;
  this->NumberOfVolumes = 0;
  this->analyzeDataType = ANALYZE_DATA_TYPE_UNKNOWN; 
  this->VolumeToRead = 0;
  this->SpmFlag = 0;
  this->SpmAcPosition[0] = 0;
  this->SpmAcPosition[1] = 0;
  this->SpmAcPosition[2] = 0;
  this->FileNameHasChanged = 0;
}

// Destruct object
vtkAnalyzeImageReader::~vtkAnalyzeImageReader ()
{
}

void vtkAnalyzeImageReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  if (this->FileName)
    {
    os << "FileName : " << this->FileName << "\n";
    }
  else
    {
    os << "FileName : (NULL)" << "\n";
    }
}

//-------------------------------------------------------------------------
void vtkAnalyzeImageReader::SetFileName(const char *name)
{
  // Set FileNameHasChanged even if the file name hasn't changed,
  // because it is possible that the user is re-reading a file after
  // changing it.
  if (!(name == 0 && this->GetFileName() == 0))
    { 
    this->FileNameHasChanged = 1;
    }

  this->Superclass::SetFileName(name);
}

void vtkAnalyzeImageReader::ReadHeader()
{
  vtkDebugMacro(<<"Reading Analyze volume file...");
  
  ifstream in(this->FileName);
  if (!in) {
     vtkErrorMacro(<<"Unable to open file "<<this->FileName);
     return;
  }
  
  haveDimensions = 0;
  haveSwapBytes  = 0;
  analyzeDataType = ANALYZE_DATA_TYPE_UNKNOWN; 
  
  struct dsr hdr;
  in.read((char*)&hdr, sizeof(hdr));
  in.close();

  this->haveSwapBytes = 1;
  this->SetVolumeDataSwapBytes(0);
  
  short* spmAcPtr = (short*)hdr.hist.originator;
  
  if ((hdr.dime.dim[0] < 0) || (hdr.dime.dim[0] > 15)) {
     this->SetVolumeDataSwapBytes(1);
     
     this->SwapBytes(&hdr.hk.sizeof_hdr, 1);
     this->SwapBytes(&hdr.hk.extents, 1);
     this->SwapBytes(&hdr.hk.session_error, 1);
     
     this->SwapBytes(hdr.dime.dim, 8);
     this->SwapBytes(&hdr.dime.unused1, 1);
     this->SwapBytes(&hdr.dime.datatype, 1);
     this->SwapBytes(&hdr.dime.bitpix, 1);
     this->SwapBytes(hdr.dime.pixdim, 8);
     this->SwapBytes(&hdr.dime.vox_offset, 1);
     this->SwapBytes(&hdr.dime.roi_scale, 1);
     this->SwapBytes(&hdr.dime.funused1, 1);
     this->SwapBytes(&hdr.dime.funused2, 1);
     this->SwapBytes(&hdr.dime.cal_max, 1);
     this->SwapBytes(&hdr.dime.cal_min, 1);
     this->SwapBytes(&hdr.dime.compressed, 1);
     this->SwapBytes(&hdr.dime.verified, 1);
     this->SwapBytes(&hdr.dime.glmax, 1);
     this->SwapBytes(&hdr.dime.glmin, 1);
     
     this->SwapBytes(&hdr.hist.views, 1);
     this->SwapBytes(&hdr.hist.vols_added, 1);
     this->SwapBytes(&hdr.hist.start_field, 1);
     this->SwapBytes(&hdr.hist.field_skip, 1);
     this->SwapBytes(&hdr.hist.omax, 1);
     this->SwapBytes(&hdr.hist.omin, 1);
     this->SwapBytes(&hdr.hist.smax, 1);
     this->SwapBytes(&hdr.hist.smin, 1);
     
     if (this->SpmFlag) {
        this->SwapBytes(spmAcPtr, 3);
     }
  }
  
  if ((hdr.dime.dim[0] > 0) &&
      (hdr.dime.dim[1] > 0) &&
      (hdr.dime.dim[2] > 0)) {
     this->haveDimensions = 1;
     this->Dimensions[0] = hdr.dime.dim[1];
     this->Dimensions[1] = hdr.dime.dim[2];
     this->Dimensions[2] = hdr.dime.dim[3];
  }
  //cout << "Dimensions: " 
  //     << this->Dimensions[0] << " "
  //     << this->Dimensions[1] << " "
  //     << this->Dimensions[2] << endl;
       
  if (hdr.dime.dim[0] >= 4) {
     this->NumberOfVolumes = hdr.dime.dim[4];
  }
  else {
     this->NumberOfVolumes = 1;
  }
  //cout << "Number of Volumes: " << this->NumberOfVolumes << endl;
  
  this->analyzeDataType = static_cast<ANALYZE_DATA_TYPE>(hdr.dime.datatype);
  //cout << "Data type: " << this->analyzeDataType << endl;
  
  if (haveDimensions == 0) {
     vtkErrorMacro("Unable to find dimensions in ANALYZE .hdr file");
     return;
  }
  if (haveSwapBytes == 0) {
     vtkErrorMacro("Unable to find byte ordering in ANALYZE .hdr file");
     return;
  }
  
  this->SetDataSpacing(1, 1, 1);
  
  if (this->SpmFlag) {
     this->SetDataSpacing(hdr.dime.pixdim[1], hdr.dime.pixdim[2], 
                          hdr.dime.pixdim[3]);
     this->SetSpmAcPosition(spmAcPtr[0], spmAcPtr[1], spmAcPtr[2]);
     if (hdr.dime.funused1 > 0.0) {
        this->SetDataScaling(hdr.dime.funused1);
     }
     
     cout << "Spm Voxel Size: " 
          << this->DataSpacing[0] << " "
          << this->DataSpacing[1] << " "
          << this->DataSpacing[2] << endl;
     cout << "Spm AC Position: " 
          << this->SpmAcPosition[0] << " "
          << this->SpmAcPosition[1] << " "
          << this->SpmAcPosition[2] << endl;
     cout << "Spm Scaling: " << this->DataScaling << endl;
  }
}
  
void vtkAnalyzeImageReader::Execute()
{
  this->ReadHeader();
  if ((haveSwapBytes == 0) ||
      (haveDimensions == 0)) {
     return;
  }
  
  this->SetDataOrigin(0, 0, 0);
  this->SetVolumeDataOffset(0);
  
  int dataSizeInBytes = 0;
  switch(analyzeDataType) {
     case ANALYZE_DATA_TYPE_UNSIGNED_BYTE:
        this->SetVolumeDataType(VTK_UNSIGNED_CHAR);
        dataSizeInBytes = 1;
        break;
     case ANALYZE_DATA_TYPE_SHORT:
        this->SetVolumeDataType(VTK_SHORT);
        dataSizeInBytes = 2;
        break;
     case ANALYZE_DATA_TYPE_INT:
        this->SetVolumeDataType(VTK_INT);
        dataSizeInBytes = 4;
        break;
     case ANALYZE_DATA_TYPE_FLOAT:
        this->SetVolumeDataType(VTK_FLOAT);
        dataSizeInBytes = 4;
        break;
     case ANALYZE_DATA_TYPE_DOUBLE:
        this->SetVolumeDataType(VTK_DOUBLE);
        dataSizeInBytes = 8;
        break;
     case ANALYZE_DATA_TYPE_COMPLEX:
        vtkErrorMacro("Complex Data type is not supported.");
        return;
     case ANALYZE_DATA_TYPE_RGB:
        vtkErrorMacro("RGB Data type is not supported.");
        return;
     case ANALYZE_DATA_TYPE_BINARY:
        vtkErrorMacro("Binary Data type is not supported.");
        return;
     case ANALYZE_DATA_TYPE_UNKNOWN:
     default:
        vtkErrorMacro("Unknown data type: "<<(int)analyzeDataType);
        return;
  }

  this->SetVolumeDataOffset(0);
  if (VolumeToRead > 0) {
     if (VolumeToRead > NumberOfVolumes) {
        vtkErrorMacro("Volume for reading ("<<VolumeToRead<<" does not exist");
        return;
     }
     else {
        int d[3];
        this->GetDimensions(d);
        const int offset = VolumeToRead *
                           (dataSizeInBytes * d[0] * d[1] * d[2]);
        cout << "Offset for reading volume: " << offset << endl; 
        this->SetVolumeDataOffset(offset * VolumeToRead);
     }
  }
  
  char filename[1024], savedFilename[1024];
  strcpy(filename, this->GetFileName());
  strcpy(savedFilename, this->GetFileName());
  
  char* img = strstr(filename, ".hdr");
  if (img == NULL) {
     vtkErrorMacro("Must supply a filename that ends with \".img\" for "<<
                   "the Analyze volume reader");
     return;
  }
  else {
     img[1] = 'i';
     img[2] = 'm';
     img[3] = 'g';
     this->SetFileName(filename);
     this->ReadVolumeData();
     this->SetFileName(savedFilename);
  }
}

