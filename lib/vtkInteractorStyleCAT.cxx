/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkInteractorStyleCAT.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkInteractorStyleCAT.h"

#include "vtkActor.h"
#include "vtkAssemblyNode.h"
#include "vtkAssemblyPath.h"
#include "vtkProperty.h"
#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"

vtkCxxRevisionMacro(vtkInteractorStyleCAT, "$Revision: 1.00 $");
vtkStandardNewMacro(vtkInteractorStyleCAT);

//----------------------------------------------------------------------------
vtkInteractorStyleCAT::vtkInteractorStyleCAT() 
{
}

//----------------------------------------------------------------------------
vtkInteractorStyleCAT::~vtkInteractorStyleCAT() 
{
}

//----------------------------------------------------------------------------
void vtkInteractorStyleCAT::OnChar()
{
  vtkRenderWindowInteractor *rwi = this->Interactor;

  switch (rwi->GetKeyCode())
    {
      // use some of the old keycodes
    case 'Q' :    case 'q' :
    case 'e' :    case 'E' :
    case 'p' :    case 'P' :
    case 's' :    case 'S' :
    case 'w' :    case 'W' :
    case 'm' :    case 'M' :
    case 'f' :    case 'F' :
      vtkInteractorStyle::OnChar();
      break;
    case 'u' :    case 'U' :
      {
      vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
      if (this->Interactor->GetShiftKey()) camera->Elevation(1.0);
      else camera->Elevation(45.0);
      camera->OrthogonalizeViewUp();
      rwi->Render();
      break;
      }
    case 'd' :    case 'D' :
      {
      vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
      if (this->Interactor->GetShiftKey()) camera->Elevation(-1.0);
      else camera->Elevation(-45.0);
      camera->OrthogonalizeViewUp();

      rwi->Render();
      break;
      }
    case 'l' :    case 'L' :
      {
      vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
      if (this->Interactor->GetShiftKey()) camera->Azimuth(1.0);
      else camera->Azimuth(45.0);
      camera->OrthogonalizeViewUp();
      rwi->Render();
      break;
      }
    case 'r' :    case 'R' :
      {
      vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
      if (this->Interactor->GetShiftKey()) camera->Azimuth(-1.0);
      else camera->Azimuth(-45.0);
      camera->OrthogonalizeViewUp();
      rwi->Render();
      break;
      }
    }

}

//----------------------------------------------------------------------------
void vtkInteractorStyleCAT::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

}
