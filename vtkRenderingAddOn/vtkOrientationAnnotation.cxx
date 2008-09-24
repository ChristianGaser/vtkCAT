/*=========================================================================

Program:   vtkINRIA3D
Module:    $Id: vtkOrientationAnnotation.cxx 477 2007-11-20 17:46:10Z filus $
Language:  C++
Author:    $Author: filus $
Date:      $Date: 2007-11-20 18:46:10 +0100 (Di, 20 Nov 2007) $
Version:   $Revision: 477 $

Copyright (c) 2007 INRIA - Asclepios Project. All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "vtkOrientationAnnotation.h"
#include "vtkObjectFactory.h"
#include "vtkTextProperty.h"
#include "vtkTextMapper.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkOrientationAnnotation);
vtkCxxRevisionMacro(vtkOrientationAnnotation, "$Revision: 477 $");

//----------------------------------------------------------------------------
vtkOrientationAnnotation::vtkOrientationAnnotation()
{
}

//----------------------------------------------------------------------------
vtkOrientationAnnotation::~vtkOrientationAnnotation()
{
}


//----------------------------------------------------------------------------
void vtkOrientationAnnotation::SetTextActorsPosition(int vsize[2])
{
  this->TextActor[2]->SetPosition(5, vsize[1]/2);
  this->TextActor[3]->SetPosition(vsize[0]/2, 5);
  this->TextActor[0]->SetPosition(vsize[0]-5, vsize[1]/2);
  this->TextActor[1]->SetPosition(vsize[0]/2, vsize[1]-5);
}
      
//----------------------------------------------------------------------------
void vtkOrientationAnnotation::SetTextActorsJustification()
{
  vtkTextProperty *tprop = this->TextMapper[2]->GetTextProperty();
  tprop->SetJustificationToLeft();
  tprop->SetVerticalJustificationToCentered();

  tprop = this->TextMapper[3]->GetTextProperty();
  tprop->SetJustificationToCentered();
  tprop->SetVerticalJustificationToBottom();
        
  tprop = this->TextMapper[0]->GetTextProperty();
  tprop->SetJustificationToRight();
  tprop->SetVerticalJustificationToCentered();
        
  tprop = this->TextMapper[1]->GetTextProperty();
  tprop->SetJustificationToCentered();
  tprop->SetVerticalJustificationToTop();
}
