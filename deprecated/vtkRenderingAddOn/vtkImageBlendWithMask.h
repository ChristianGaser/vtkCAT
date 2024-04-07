/*=========================================================================

Program:   vtkINRIA3D
Module:    $Id$
Language:  C++
Author:    $Author$
Date:      $Date$
Version:   $Revision$

Copyright (c) 2007 INRIA - Asclepios Project. All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
// version vtkRenderingAddOn
#ifndef _vtk_ImageBlendWithMask_h_
#define _vtk_ImageBlendWithMask_h_

#include "vtkINRIA3DConfigure.h"

#include "vtkThreadedImageAlgorithm.h"
#include "vtkLookupTable.h"

class VTK_RENDERINGADDON_EXPORT vtkImageBlendWithMask : public vtkThreadedImageAlgorithm
{
 public:
  static vtkImageBlendWithMask *New();
  vtkTypeRevisionMacro (vtkImageBlendWithMask, vtkThreadedImageAlgorithm);
  void PrintSelf (ostream &os, vtkIndent indent);


  // Set/Get the LUT to map the mask
  vtkSetObjectMacro (LookupTable, vtkLookupTable);
  vtkGetObjectMacro (LookupTable, vtkLookupTable);
  
  
  
  
  // Description:
  // Set the input to be masked.
  void SetImageInput(vtkImageData *in);

  // Description:
  // Set the mask to be used.
  void SetMaskInput(vtkImageData *in);

  // Description:
  // Set the two inputs to this filter
  virtual void SetInput1(vtkDataObject *in) { this->SetInput(0,in); }
  virtual void SetInput2(vtkDataObject *in) { this->SetInput(1,in); }

 protected:
  vtkImageBlendWithMask();
  ~vtkImageBlendWithMask();

  vtkLookupTable* LookupTable;

  virtual int RequestInformation (vtkInformation *, 
                                  vtkInformationVector **,
                                  vtkInformationVector *);
  
 
  virtual void ThreadedRequestData(vtkInformation *request, 
                                   vtkInformationVector **inputVector, 
                                   vtkInformationVector *outputVector,
                                   vtkImageData ***inData, 
                                   vtkImageData **outData,
                                   int extent[6], int threadId);
  

 private:
  vtkImageBlendWithMask (const vtkImageBlendWithMask&);
  void operator=(const vtkImageBlendWithMask&);  

};



#endif
