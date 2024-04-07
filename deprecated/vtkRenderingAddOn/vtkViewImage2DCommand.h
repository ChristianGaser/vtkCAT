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
#ifndef _vtkViewImage2DCommand_h_
#define _vtkViewImage2DCommand_h_

#include "vtkINRIA3DConfigure.h"

#include<vtkCommand.h>
#include<vtkObjectFactory.h>


class vtkInteractorStyleImage;
class vtkInteractorStyleImage2D;
class vtkViewImage2D;
  
class VTK_RENDERINGADDON_EXPORT vtkViewImage2DCommand : public vtkCommand
{    
 public:
  vtkViewImage2DCommand(); 
  
  static  vtkViewImage2DCommand* New() 
    { return new vtkViewImage2DCommand; }
  
  void Execute(vtkObject *   caller, 
               unsigned long event, 
               void *        callData);    
  
  void SetView(vtkViewImage2D *p_view);

  //BTX
  enum EventIds
  {
    ZSliceMoveEvent=(vtkCommand::UserEvent+1),
    StartZSliceMoveEvent,
    EndZSliceMoveEvent,
    StartMeasureEvent,
    MeasureEvent,
    EndMeasureEvent,
    FullPageEvent,
    ResetPositionEvent,
    ZoomEvent,
    ResetZoomEvent
  };
  //ETX
    
 private:
  void ChangeZSlice(vtkInteractorStyleImage2D* p_isi);
  void EndWindowing();
  void Windowing(vtkInteractorStyleImage2D* p_isi);
  void StartWindowing();  
  void StartPicking(vtkInteractorStyleImage* p_isi);
  void EndPicking (void);
  void Zoom (vtkInteractorStyleImage* p_isi);
  
 private:
  vtkViewImage2D* View;
  double          InitialWindow;
  double          InitialLevel;

  bool            WindowEventStatus;
  
};

#endif
