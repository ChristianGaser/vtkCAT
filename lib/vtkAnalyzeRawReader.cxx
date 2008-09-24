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
  Module:    $RCSfile: vtkAnalyzeRawReader.cxx,v $
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

#include <string>
#include "vtkAnalyzeRawReader.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"

//--------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkAnalyzeRawReader, "$Revision: 1.00 $");
vtkStandardNewMacro(vtkAnalyzeRawReader);

// Construct object with NULL file prefix; file pattern "%s.%d"; image range 
// set to (1,1); 
vtkAnalyzeRawReader::vtkAnalyzeRawReader()
{
  this->Dimensions[0] = this->Dimensions[1] = this->Dimensions[2] = 0;
  this->DataOrigin[0] = this->DataOrigin[1] = this->DataOrigin[2] = 0.0;
  this->DataSpacing[0] = this->DataSpacing[1] = this->DataSpacing[2] = 1.0;
  this->DataScaling = 1.0;

  this->VolumeDataOffset = 0;
  this->VolumeDataType   = VTK_CHAR;
  this->VolumeDataSwapBytes = 0;
  
  this->FileName = NULL;
}


vtkAnalyzeRawReader::~vtkAnalyzeRawReader ()
{
   if (this->FileName != NULL) delete[] this->FileName;
}


void vtkAnalyzeRawReader::PrintSelf(ostream& os, vtkIndent indent)
{
   // the reader ivar's source will be this reader. we must do this to
   // prevent infinite printing
   //if (!recursing) {
   this->Superclass::PrintSelf(os,indent);

      os << indent << "Data Origin: (" << this->DataOrigin[0] << ", "
                                       << this->DataOrigin[1] << ", "
                                       << this->DataOrigin[2] << ")\n";
      os << indent << "Data Spacing: (" << this->DataSpacing[0] << ", "
                                        << this->DataSpacing[1] << ", "
                                        << this->DataSpacing[2] << ")\n";
      os << indent << "Data Dimensions: (" << this->Dimensions[0] << ", "
                                        << this->Dimensions[1] << ", "
                                        << this->Dimensions[2] << ")\n";
      os << indent << "Data Scaling: " << this->DataScaling << "\n";
      os << indent << "FileName: " << this->FileName << "\n";
   //}
   //recursing = 0;
}

void vtkAnalyzeRawReader::Execute()
{
   this->ReadVolumeData();
}

void vtkAnalyzeRawReader::ReadVolumeData()
{
   vtkImageData *output = this->GetOutput();

   vtkDebugMacro(<<"Reading raw volume file...");

   if ( (this->Dimensions[0] <= 0) ||
        (this->Dimensions[1] <= 0) ||
        (this->Dimensions[2] <= 0) ) {
      vtkErrorMacro(<< "Data Dimensions not set: ");
      return;
   }
   output->SetDimensions(this->Dimensions);
   
   if ( (this->DataSpacing[0] <= 0) ||
        (this->DataSpacing[1] <= 0) ||
        (this->DataSpacing[2] <= 0) ) {
      vtkErrorMacro(<< "Data Spacing not set: ");
      return;
   }
   
#ifdef HAVE_ZLIB
   gzFile input = gzopen(this->FileName, "rb");
   if (input == NULL) {
      string gzipFileName(this->FileName);
      gzipFileName.append(".gz");
      input = gzopen(gzipFileName.c_str(), "rb");
      if (input == NULL) {
         vtkErrorMacro(<<"Unable to open: "<<this->FileName<<" or "<<gzipFileName);
      return;
      }
   }
#else
   ifstream* input = new ifstream(this->FileName);
   if (input == NULL) {
      vtkErrorMacro(<<"Unable to open: "<<this->FileName);
      return;
   }
   else if (input->bad()) {
      vtkErrorMacro(<<"Unable to open: "<<this->FileName);
      delete input;
      return;
   }
#endif // HAVE_ZLIB
//    printf("data type: %d\n", VolumeDataType);
//    printf("dimensions: %d %d %d\n", Dimensions[0],Dimensions[1], Dimensions[2]); 
//    printf("swap bytes: %d\n", VolumeDataSwapBytes);
//    printf("Filename: %s\n", FileName);
   
   output->SetSpacing(this->DataSpacing);
   
   output->SetOrigin(this->DataOrigin);
   
   const int outputScalarsSize = this->Dimensions[0] 
                               * this->Dimensions[1]
                               * this->Dimensions[2];
                                        
   vtkFloatArray* outputScalars = vtkFloatArray::New();

   int dataValid = 0;
   
#ifdef HAVE_ZLIB

#ifdef MAC_OSX_SYSTEM          
   //
   // gzseek does not work on OSX, so just read data until offset
   //
   const int dummySize = 2048;
   unsigned char dummy[dummySize];
   int amountLeft = VolumeDataOffset;

   bool done = false;
   while(!done) {
      int amountToRead = dummySize;
      if (amountLeft < amountToRead) {
         amountToRead = amountLeft;
      }
      const int numRead = gzread(input, dummy, amountToRead);
      if (numRead != amountToRead) {
         done = true;
         printf("ERROR: gzeek failed and backup method failed\n");
      }
      else {
         amountLeft -= amountToRead;
         if (amountLeft <= 0) {
            done = true;
            dataValid = 1;
         }
      }
   }
#else   // MAC_OSX_SYSTEM
   const z_off_t actualOffset = gzseek(input, VolumeDataOffset, SEEK_SET); 
   if (actualOffset != VolumeDataOffset) {
      printf("gzseek failed: requested offset: %d, actual offset: %d\n",
          (int)VolumeDataOffset, (int)actualOffset);
   }
   else {
      dataValid = 1;
   }
#endif  // MAC_OSX_SYSTEM

#else   // HAVE_ZLIB
   input->seekg(VolumeDataOffset, ios::beg);
   dataValid = 1;
#endif  // HAVE_ZLIB

   if (dataValid != 0) {   
      switch(VolumeDataType) {
         case VTK_CHAR:
            dataValid = readCharData(input, outputScalars, outputScalarsSize);
            break;
         case VTK_UNSIGNED_CHAR:
            dataValid = readUnsignedCharData(input, outputScalars, outputScalarsSize);
            break;
         case VTK_SHORT:
            dataValid = readShortData(input, outputScalars, outputScalarsSize);
            break;
         case VTK_UNSIGNED_SHORT:
            dataValid = readUnsignedShortData(input, outputScalars, outputScalarsSize);
            break;
         case VTK_INT:
            dataValid = readIntData(input, outputScalars, outputScalarsSize);
            break;
         case VTK_UNSIGNED_INT:
            dataValid = readUnsignedIntData(input, outputScalars, outputScalarsSize);
            break;
         case VTK_LONG:
            dataValid = readLongData(input, outputScalars, outputScalarsSize);
            break;
         case VTK_UNSIGNED_LONG:
            dataValid = readUnsignedLongData(input, outputScalars, outputScalarsSize);
            break;
         case VTK_FLOAT:
            dataValid = readFloatData(input, outputScalars, outputScalarsSize);
            break;
         case VTK_DOUBLE:
            dataValid = readDoubleData(input, outputScalars, outputScalarsSize);
            break;
      }
   }
        
   if (dataValid) {
      float scaledMin = outputScalars->GetValue(0) * this->DataScaling;
      float scaledMax = outputScalars->GetValue(0) * this->DataScaling;
      float unscaledMin = outputScalars->GetValue(0);
      float unscaledMax = outputScalars->GetValue(0);
      for (int i = 0; i < outputScalarsSize; i++) {
         float s = outputScalars->GetValue(i);
         if (s < unscaledMin) unscaledMin = s;
         if (s > unscaledMax) unscaledMax = s;
         s *= this->DataScaling;
         if (s < scaledMin) scaledMin = s;
         if (s > scaledMax) scaledMax = s;
         outputScalars->SetValue(i, s);
      }
      cout << "Unscaled range: " << unscaledMin << " " << unscaledMax << endl;
      cout << "Scaled range: " << scaledMin << " " << scaledMax << endl;
//      output->SetScalars(outputScalars);
   } 

#ifdef HAVE_ZLIB
   gzclose(input);
#else   
   input->close();
   delete input;
#endif // HAVE_ZLIB
   outputScalars->Delete();
}

int vtkAnalyzeRawReader::readCharData(INPUT_STREAM input, 
                                      vtkFloatArray* scalars,
                                      int numScalars)
{
   const int length = numScalars * sizeof(char);
   char* data = new char[length];
   
#ifdef HAVE_ZLIB
   const int numRead = gzread(input, (char*)data, (unsigned)length);
   if (numRead != length) {
      vtkErrorMacro(<<"Premature EOF reading file");
      return 0;
   }
#else
   input->read((char*)data, length);
   if (input->eof()) {
      vtkErrorMacro(<<"Premature EOF reading file");
      return 0;
   }
#endif
   
   for (int i = 0; i < numScalars; i++) {
      float f = data[i];
      scalars->InsertNextTuple(&f);
   }
   delete[] data;
   return 1;
}

int vtkAnalyzeRawReader::readUnsignedCharData(INPUT_STREAM input, 
                                      vtkFloatArray* scalars,
                                      int numScalars)
{
   const int length = numScalars * sizeof(unsigned char);
   unsigned char* data = new unsigned char[length];
   
#ifdef HAVE_ZLIB
   const int numRead = gzread(input, (char*)data, (unsigned)length);
   if (numRead != length) {
      vtkErrorMacro(<<"Premature EOF reading file");
      return 0;
   }
#else
   input->read((char*)data, length);
   if (input->eof()) {
      vtkErrorMacro(<<"Premature EOF reading file");
      return 0;
   }
#endif
   
   for (int i = 0; i < numScalars; i++) {
      float f = data[i];
      scalars->InsertNextTuple(&f);
   }
   delete[] data;
   return 1;
}

int vtkAnalyzeRawReader::readShortData(INPUT_STREAM input, 
                                      vtkFloatArray* scalars,
                                      int numScalars)
{
   const int length = numScalars * sizeof(short);
   short* data = new short[length];
   
#ifdef HAVE_ZLIB
   const int numRead = gzread(input, (char*)data, (unsigned)length);
   if (numRead != length) {
      vtkErrorMacro(<<"Premature EOF reading file");
      return 0;
   }
#else
   input->read((char*)data, length);
   if (input->eof()) {
      vtkErrorMacro(<<"Premature EOF reading file");
      return 0;
   }
#endif
   
   if (this->VolumeDataSwapBytes) {
      this->SwapBytes(data, numScalars);
   }
   
   for (int i = 0; i < numScalars; i++) {
      float f = data[i];
      scalars->InsertNextTuple(&f);
   }
   delete[] data;
   return 1;
}

int vtkAnalyzeRawReader::readUnsignedShortData(INPUT_STREAM input, 
                                      vtkFloatArray* scalars,
                                      int numScalars)
{
   const int length = numScalars * sizeof(unsigned short);
   unsigned short* data = new unsigned short[length];
   
#ifdef HAVE_ZLIB
   const int numRead = gzread(input, (char*)data, (unsigned)length);
   if (numRead != length) {
      vtkErrorMacro(<<"Premature EOF reading file");
      return 0;
   }
#else
   input->read((char*)data, length);
   if (input->eof()) {
      vtkErrorMacro(<<"Premature EOF reading file");
      return 0;
   }
#endif
   
   if (this->VolumeDataSwapBytes) {
      this->SwapBytes(data, numScalars);
   }
   
   for (int i = 0; i < numScalars; i++) {
      float f = data[i];
      scalars->InsertNextTuple(&f);
   }
   delete[] data;
   return 1;
}

int vtkAnalyzeRawReader::readIntData(INPUT_STREAM input, 
                                      vtkFloatArray* scalars,
                                      int numScalars)
{
   const int length = numScalars * sizeof(int);
   int* data = new int[length];
   
#ifdef HAVE_ZLIB
   const int numRead = gzread(input, (char*)data, (unsigned)length);
   if (numRead != length) {
      vtkErrorMacro(<<"Premature EOF reading file");
      return 0;
   }
#else
   input->read((char*)data, length);
   if (input->eof()) {
      vtkErrorMacro(<<"Premature EOF reading file");
      return 0;
   }
#endif
   
   if (this->VolumeDataSwapBytes) {
      this->SwapBytes(data, numScalars);
   }
   
   for (int i = 0; i < numScalars; i++) {
      //scalars->SetValue(i, (float)data[i]);
      float f = data[i];
      scalars->InsertNextTuple(&f);
   }
   delete[] data;
   return 1;
}

int vtkAnalyzeRawReader::readUnsignedIntData(INPUT_STREAM input, 
                                      vtkFloatArray* scalars,
                                      int numScalars)
{
   const int length = numScalars * sizeof(unsigned int);
   unsigned int* data = new unsigned int[length];
   
#ifdef HAVE_ZLIB
   const int numRead = gzread(input, (char*)data, (unsigned)length);
   if (numRead != length) {
      vtkErrorMacro(<<"Premature EOF reading file");
      return 0;
   }
#else
   input->read((char*)data, length);
   if (input->eof()) {
      vtkErrorMacro(<<"Premature EOF reading file");
      return 0;
   }
#endif
   
   if (this->VolumeDataSwapBytes) {
      this->SwapBytes(data, numScalars);
   }
   
   for (int i = 0; i < numScalars; i++) {
      float f = data[i];
      scalars->InsertNextTuple(&f);
   }
   delete[] data;
   return 1;
}

int vtkAnalyzeRawReader::readLongData(INPUT_STREAM input, 
                                      vtkFloatArray* scalars,
                                      int numScalars)
{
   const int length = numScalars * sizeof(long);
   long* data = new long[length];
   
#ifdef HAVE_ZLIB
   const int numRead = gzread(input, (char*)data, (unsigned)length);
   if (numRead != length) {
      vtkErrorMacro(<<"Premature EOF reading file");
      return 0;
   }
#else
   input->read((char*)data, length);
   if (input->eof()) {
      vtkErrorMacro(<<"Premature EOF reading file");
      return 0;
   }
#endif
   
   if (this->VolumeDataSwapBytes) {
      this->SwapBytes(data, numScalars);
   }
   
   for (int i = 0; i < numScalars; i++) {
      float f = data[i];
      scalars->InsertNextTuple(&f);
   }
   delete[] data;
   return 1;
}

int vtkAnalyzeRawReader::readUnsignedLongData(INPUT_STREAM input, 
                                      vtkFloatArray* scalars,
                                      int numScalars)
{
   const int length = numScalars * sizeof(unsigned long);
   unsigned long* data = new unsigned long[length];
   
#ifdef HAVE_ZLIB
   const int numRead = gzread(input, (char*)data, (unsigned)length);
   if (numRead != length) {
      vtkErrorMacro(<<"Premature EOF reading file");
      return 0;
   }
#else
   input->read((char*)data, length);
   if (input->eof()) {
      vtkErrorMacro(<<"Premature EOF reading file");
      return 0;
   }
#endif
   
   if (this->VolumeDataSwapBytes) {
      this->SwapBytes(data, numScalars);
   }
   
   for (int i = 0; i < numScalars; i++) {
      float f = data[i];
      scalars->InsertNextTuple(&f);
   }
   delete[] data;
   return 1;
}

int vtkAnalyzeRawReader::readFloatData(INPUT_STREAM input, 
                                      vtkFloatArray* scalars,
                                      int numScalars)
{
   const int length = numScalars * sizeof(float);
   float* data = new float[length];
   
#ifdef HAVE_ZLIB
   const int numRead = gzread(input, (char*)data, (unsigned)length);
   if (numRead != length) {
      vtkErrorMacro(<<"Premature EOF reading file");
      return 0;
   }
#else
   input->read((char*)data, length);
   if (input->eof()) {
      vtkErrorMacro(<<"Premature EOF reading file");
      return 0;
   }
#endif
   
   if (this->VolumeDataSwapBytes) {
      this->SwapBytes(data, numScalars);
   }
   
   for (int i = 0; i < numScalars; i++) {
      scalars->InsertNextTuple(&data[i]);
   }
   delete[] data;
   return 1;
}

int vtkAnalyzeRawReader::readDoubleData(INPUT_STREAM input, 
                                       vtkFloatArray* scalars,
                                       int numScalars)
{
   const int length = numScalars * sizeof(float);
   float* data = new float[length];
   
#ifdef HAVE_ZLIB
   const int numRead = gzread(input, (char*)data, (unsigned)length);
   if (numRead != length) {
      vtkErrorMacro(<<"Premature EOF reading file");
      return 0;
   }
#else
   input->read((char*)data, length);
   if (input->eof()) {
      vtkErrorMacro(<<"Premature EOF reading file");
      return 1;
   }
#endif
   
   if (this->VolumeDataSwapBytes) {
      this->SwapBytes(data, numScalars);
   }
   
   for (int i = 0; i < numScalars; i++) {
      float f = data[i];
      scalars->InsertNextTuple(&f);
   }
   delete[] data;
   return 0;
}

void vtkAnalyzeRawReader::SwapBytes(short* n, int numToSwap)
{
   for (int i = 0; i < numToSwap; i++) {
      char* bytes = (char*)&n[i];
      char  temp = bytes[0];
      bytes[0] = bytes[1];
      bytes[1] = temp;
   }
}

void vtkAnalyzeRawReader::SwapBytes(unsigned short* n, int numToSwap)
{
   SwapBytes((short*)n, numToSwap);
}

void vtkAnalyzeRawReader::SwapBytes(int* n, int numToSwap)
{
   for (int i = 0; i < numToSwap; i++) {
      char* bytes = (char*)&n[i];
      char  temp = bytes[0];
      bytes[0] = bytes[3];
      bytes[3] = temp;

      temp = bytes[1];
      bytes[1] = bytes[2];
      bytes[2] = temp;
   }
}

void vtkAnalyzeRawReader::SwapBytes(unsigned int* n, int numToSwap)
{
   SwapBytes((int*)n, numToSwap);
}

void vtkAnalyzeRawReader::SwapBytes(long* n, int numToSwap)
{
   for (int i = 0; i < numToSwap; i++) {
      char* bytes = (char*)&n[i];
      char  temp = bytes[0];
      bytes[0] = bytes[7];
      bytes[7] = temp;

      temp = bytes[1];
      bytes[1] = bytes[6];
      bytes[6] = temp;

      temp = bytes[2];
      bytes[2] = bytes[5];
      bytes[5] = temp;

      temp = bytes[3];
      bytes[3] = bytes[4];
      bytes[4] = temp;
   }
}

void vtkAnalyzeRawReader::SwapBytes(unsigned long* n, int numToSwap)
{
   SwapBytes((long*)n, numToSwap);
}

void vtkAnalyzeRawReader::SwapBytes(float* n, int numToSwap)
{
   SwapBytes((int*)n, numToSwap);
}

void vtkAnalyzeRawReader::SwapBytes(double* n, int numToSwap)
{
   SwapBytes((long*)n, numToSwap);
}

