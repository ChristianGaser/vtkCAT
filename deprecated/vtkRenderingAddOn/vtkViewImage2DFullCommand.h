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
#ifndef _vtkViewImage2DFullCommand_h_
#define _vtkViewImage2DFullCommand_h_

#include "vtkINRIA3DConfigure.h"

#include<vtkCommand.h>
#include<vtkObjectFactory.h>

class vtkViewImage2D;
class VTK_RENDERINGADDON_EXPORT vtkViewImage2DFullCommand : public vtkCommand
{
    
public:

  static vtkViewImage2DFullCommand* New() 
  { return new vtkViewImage2DFullCommand; }
    
  void Execute(vtkObject *   caller, 
               unsigned long event, 
               void *        callData);    
    
  vtkViewImage2D*          IV;
  double                   InitialWindow;
  double                   InitialLevel;

  bool                     WindowEventStatus;

 protected:
  vtkViewImage2DFullCommand()
  {
    this->IV = 0;
    this->InitialWindow = 0;
    this->InitialLevel = 0;
    this->WindowEventStatus = false;
  }
};

#endif
