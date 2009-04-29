/*=========================================================================

Program:   vtkINRIA3D
Module:    $Id: vtkImageOrientedPointWidgetCallback.h 1080 2009-02-18 13:27:02Z acanale $
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
#ifndef _vtk_ImageOrientedPointWidgetCallback_h_
#define _vtk_ImageOrientedPointWidgetCallback_h_

#include "vtkINRIA3DConfigure.h"

#include "vtkCommand.h"
#include "vtkPolyData.h"
#include <vtkViewImage2DWithOrientedPoint.h>
#include "vtkObject.h"
#include "vtkImageMapToWindowLevelColors.h"

class VTK_RENDERINGADDON_EXPORT vtkImageOrientedPointWidgetCallback : public vtkCommand
{

 public:
  
  static vtkImageOrientedPointWidgetCallback *New()
  { return new vtkImageOrientedPointWidgetCallback; }

  
  virtual void Execute (vtkObject* caller, unsigned long, void*);
  
  void SetView ( vtkViewImage2DWithOrientedPoint* view);

  
  vtkViewImage2DWithOrientedPoint* GetView (void) const
  {
    return this->View;
  }

  vtkPolyData* GetPolyData (void) const
  {
    return this->PathPoly;
  }

/*   void SetPositionText (wxStaticText* text) */
/*   { this->PositionText = text; } */
/*   wxStaticText* GetPositionText (void) const */
/*   { return this->PositionText; } */
    
/*   void SetDirectionText (wxStaticText* text) */
/*   { this->DirectionText = text; } */
/*   wxStaticText* GetDirectionText (void) const */
/*   { return this->DirectionText; } */

 
  void RefreshText();
  
  void UpdatePoints(vtkObject* caller);

  const double* GetPosition (void) const
  {
    return this->Position;
  }
  const double* GetDirection (void) const
  {
    return this->Direction;
  }
  

 protected:
 
 vtkImageOrientedPointWidgetCallback();
  ~vtkImageOrientedPointWidgetCallback();

  void OnEndInteractionEvent(vtkObject* caller);
  void OnMouseMoveEvent(vtkObject* caller);
  void OnKeyPressEvent (vtkObject* caller, unsigned char);

 
 private:
  vtkImageOrientedPointWidgetCallback (const vtkImageOrientedPointWidgetCallback&);
  void operator=(const vtkImageOrientedPointWidgetCallback&);


  vtkPolyData*                 PathPoly;
  vtkViewImage2DWithOrientedPoint*  View;
/*   wxStaticText*              m_PositionText; */
/*   wxStaticText*              m_DirectionText; */
  vtkPoints*                   Points;
  double*                      Position;
  double*                      Direction;
  
};



#endif
