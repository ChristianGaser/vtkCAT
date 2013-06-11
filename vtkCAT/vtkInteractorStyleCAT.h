/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkInteractorStyleCAT.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkInteractorStyleCAT - interactive manipulation of the camera
// .SECTION Description
// vtkInteractorStyleCAT allows the user to interactively
// manipulate (rotate, pan, etc.) the camera, the viewpoint of the scene.  In
// trackball interaction, the magnitude of the mouse motion is proportional
// to the camera motion associated with a particular mouse binding. For
// example, small left-button motions cause small changes in the rotation of
// the camera around its focal point. For a 3-button mouse, the left button
// is for rotation, the right button for zooming, the middle button for
// panning, and ctrl + left button for spinning.  (With fewer mouse buttons,
// ctrl + shift + left button is for zooming, and shift + left button is for
// panning.)

// .SECTION See Also
// vtkInteractorStyleTrackballActor vtkInteractorStyleJoystickCamera
// vtkInteractorStyleJoystickActor

#ifndef __vtkInteractorStyleCAT_h
#define __vtkInteractorStyleCAT_h

#include "vtkInteractorStyleTrackballCamera.h"

class VTK_RENDERING_EXPORT vtkInteractorStyleCAT : public vtkInteractorStyleTrackballCamera
{
public:
  static vtkInteractorStyleCAT *New();
  vtkTypeRevisionMacro(vtkInteractorStyleCAT,vtkInteractorStyleTrackballCamera);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void OnChar();

protected:
  vtkInteractorStyleCAT();
  ~vtkInteractorStyleCAT();
  
  float rollIncrement, azimuthIncrement, elevationIncrement;

private:
  vtkInteractorStyleCAT(const vtkInteractorStyleCAT&);
  void operator=(const vtkInteractorStyleCAT&);
};

#endif