/*=========================================================================

Program:   vtkINRIA3D
Module:    $Id: vtkInteractorStyleImage2D.h 1080 2009-02-18 13:27:02Z acanale $
Language:  C++
Author:    $Author: acanale $
Date:      $Date: 2009-02-18 14:27:02 +0100 (Mi, 18 Feb 2009) $
Version:   $Revision: 1080 $

Copyright (c) 2007 INRIA - Asclepios Project. All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
// version vtkRenderingAddOn
#ifndef _vtk_InteractorStyleImage2D_h_
#define _vtk_InteractorStyleImage2D_h__

#include "vtkINRIA3DConfigure.h"

#include <vtkInteractorStyleImage.h>
#include <vtkRenderingAddOn/vtkViewImage2D.h>

// Motion flags (See VTKIS_PICK and so on in vtkInteractorStyleImage.h)
#define VTKIS_MEASURE      5050
#define VTKIS_ZSLICE_MOVE  5051

 
class VTK_RENDERINGADDON_EXPORT vtkInteractorStyleImage2D : public vtkInteractorStyleImage
{
 public:
  static vtkInteractorStyleImage2D *New();
  vtkTypeRevisionMacro (vtkInteractorStyleImage2D, vtkInteractorStyleImage);
  //{ return new vtkInteractorStyleImage2D; }
  
  virtual void OnMouseMove();
  virtual void OnLeftButtonDown();
  virtual void OnLeftButtonUp();
  virtual void OnMiddleButtonDown();
  virtual void OnMiddleButtonUp();
  virtual void OnRightButtonDown();
  virtual void OnRightButtonUp();
  virtual void OnMouseWheelForward();
  virtual void OnMouseWheelBackward();
  virtual void OnChar();
  
  void SetView (vtkViewImage2D* view)
  {
    this->View = view;
  }
  
  vtkGetObjectMacro (View, vtkViewImage2D);
  vtkSetMacro (ZSliceStep, int);
  vtkGetMacro (ZSliceStep, int);

  vtkSetMacro (WindowStep, double);
  vtkGetMacro (WindowStep, double);

  vtkSetMacro (LevelStep,  double);
  vtkGetMacro (LevelStep,  double);


  virtual void StartZSliceMove();
  virtual void ZSliceMove();
  virtual void ZSliceWheelForward();
  virtual void ZSliceWheelBackward();
  virtual void EndZSliceMove();
  virtual void FullPage();
  virtual void StartMeasure();
  virtual void Measure();
  virtual void EndMeasure();
  virtual void WindowLevel ();
  virtual void WindowLevelWheelForward ();
  virtual void WindowLevelWheelBackward ();

  void PropagateCameraFocalAndPosition();
  
 protected:
  vtkInteractorStyleImage2D();
  ~vtkInteractorStyleImage2D();

 private:
  
  vtkInteractorStyleImage2D(const vtkInteractorStyleImage2D&);  // Not implemented.
  void operator=(const vtkInteractorStyleImage2D&);  // Not implemented.
  
 private:
  vtkViewImage2D*  View;
  int              ZSliceStep;
  double           WindowStep;
  double           LevelStep;
  
};

#endif
