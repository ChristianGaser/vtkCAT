/*=========================================================================

Program:   vtkINRIA3D
Module:    $Id: vtkImage3DCroppingBoxCallback.h 608 2008-01-14 08:21:23Z filus $
Language:  C++
Author:    $Author: filus $
Date:      $Date: 2008-01-14 09:21:23 +0100 (Mo, 14 Jan 2008) $
Version:   $Revision: 608 $

Copyright (c) 2007 INRIA - Asclepios Project. All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
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
