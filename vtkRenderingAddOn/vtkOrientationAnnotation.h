/*=========================================================================

Program:   vtkINRIA3D
Module:    $Id: vtkOrientationAnnotation.h 608 2008-01-14 08:21:23Z filus $
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
// .NAME vtkOrientationAnnotation  - text annotation in four corners
// .SECTION Description
// This is an annotation object that manages four text actors / mappers
// to provide annotation in the four corners of a viewport
//
// .SECTION See Also
// vtkActor2D vtkTextMapper

#ifndef __vtkOrientationAnnotation_h
#define __vtkOrientationAnnotation_h

#include "vtkINRIA3DConfigure.h"

#include "vtkCornerAnnotation.h"

class VTK_RENDERINGADDON_EXPORT vtkOrientationAnnotation : public vtkCornerAnnotation
{
public:
  vtkTypeRevisionMacro(vtkOrientationAnnotation,vtkCornerAnnotation);
  
  // Description:
  // Instantiate object with a rectangle in normaled view coordinates
  // of (0.2,0.85, 0.8, 0.95).
  static vtkOrientationAnnotation *New();

protected:
  vtkOrientationAnnotation();
  ~vtkOrientationAnnotation();

  // Description:
  // Set text actor positions given a viewport size and justification
  virtual void SetTextActorsPosition(int vsize[2]);
  virtual void SetTextActorsJustification();

private:
  vtkOrientationAnnotation(const vtkOrientationAnnotation&);  // Not implemented.
  void operator=(const vtkOrientationAnnotation&);  // Not implemented.
};


#endif



