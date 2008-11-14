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
#include "vtkWindowToImageFilter.h"
#include "vtkPNGWriter.h"

vtkCxxRevisionMacro(vtkInteractorStyleCAT, "$Revision: 1.00 $");
vtkStandardNewMacro(vtkInteractorStyleCAT);

static void usage();

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
    case 'h' :    case 'H' :
      {
      usage();
      break;
      }
    case 'g' :    case 'G' :
      {
  	  vtkWindowToImageFilter *windowToImageFilter = vtkWindowToImageFilter::New();
	  windowToImageFilter->SetInput( rwi->GetRenderWindow() );
                                                	
	  vtkPNGWriter *PNGWriter = vtkPNGWriter::New();
	  PNGWriter->SetInput( windowToImageFilter->GetOutput() );
	  PNGWriter->SetFileName( "render.png" );
	  windowToImageFilter->Update();
	  PNGWriter->Write();
	  PNGWriter->Delete();
	  windowToImageFilter->Delete();
	  cout << "Save render.png" << endl;
      break;
      }
    }

}

//----------------------------------------------------------------------------
void vtkInteractorStyleCAT::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

}

//----------------------------------------------------------------------------
static void
usage()
{
  cout << endl
  << "KEYBOARD INTERACTIONS" << endl
  << "  u d l r" << endl
  << "     Rotate up/down/left/right by 45 degree." << endl
  << "  U D L R" << endl
  << "     Rotate up/down/left/right by 1 degree." << endl
  << "  w " << endl
  << "     Show wireframe." << endl
  << "  s " << endl
  << "     Show shaded." << endl
  << "  g " << endl
  << "     Grab image to file render.png." << endl
  << "  q e" << endl
  << "     Quit." << endl
  << endl
  << endl;
}
