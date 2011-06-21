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
#ifndef _vtk_Image3DCroppingBoxCallback_h_
#define _vtk_Image3DCroppingBoxCallback_h_

#include "vtkINRIA3DConfigure.h"

#include <vtkCommand.h>
#include <vtkVolumeMapper.h>
#include <vtkSetGet.h>
#include <vtkObjectFactory.h>

class VTK_RENDERINGADDON_EXPORT vtkImage3DCroppingBoxCallback: public vtkCommand
{

 public:
  static vtkImage3DCroppingBoxCallback* New()
  { return new vtkImage3DCroppingBoxCallback; }

  virtual void Execute ( vtkObject *caller, unsigned long, void* );

  void SetVolumeMapper (vtkVolumeMapper* mapper)
  {
    this->VolumeMapper = mapper;
  }
  vtkVolumeMapper* GetVolumeMapper (void) const
  {
    return this->VolumeMapper;
  }
  
  
 protected:
  vtkImage3DCroppingBoxCallback()
  {
    this->VolumeMapper = 0;
  }
  ~vtkImage3DCroppingBoxCallback(){};
  
 private:
  
  vtkVolumeMapper* VolumeMapper;
  
};


#endif
