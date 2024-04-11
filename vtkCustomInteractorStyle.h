#ifndef vtkCustomInteractorStyle_h
#define vtkCustomInteractorStyle_h

#include <vtkInteractorStyle.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include "vtkCamera.h"
#include <string>

class CustomInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
  static CustomInteractorStyle* New();
  vtkTypeMacro(CustomInteractorStyle, vtkInteractorStyleTrackballCamera);

  // Override mouse movement
  virtual void OnMouseMove() override
  {
    // Do nothing to suppress mouse movement
  }

  // Override keyboard events
  virtual void OnChar() override
  {
    vtkRenderWindowInteractor* rwi = this->Interactor;

    switch (rwi->GetKeyCode())
    {
        // use some of the old keycodes
      case 'Q' :    case 'q' :
      case 'e' :    case 'E' :
      case 'p' :    case 'P' :
      case 's' :    case 'S' :
      case 't' :    case 'T' :
      case 'j' :    case 'J' :
      case 'w' :    case 'W' :
      case 'm' :    case 'M' :
      case 'f' :    case 'F' :
        vtkInteractorStyle::OnChar();
        break;
      case 'u' :    case 'U' :
      {
        vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
        if (this->Interactor->GetShiftKey()) camera->Elevation(1.0);
        else if (this->Interactor->GetControlKey()) camera->Elevation(180);
        else camera->Elevation(45.0);
        camera->OrthogonalizeViewUp();
        rwi->Render();
        break;
      }
      case 'd' :    case 'D' :
      {
        vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
        if (this->Interactor->GetShiftKey()) camera->Elevation(-1.0);
        else if (this->Interactor->GetControlKey()) camera->Elevation(-180);
        else camera->Elevation(-45.0);
        camera->OrthogonalizeViewUp();
        rwi->Render();
        break;
      }
      case 'l' :    case 'L' :
      {
        vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
        if (this->Interactor->GetShiftKey()) camera->Azimuth(1.0);
        else if (this->Interactor->GetControlKey()) camera->Azimuth(180);
        else camera->Azimuth(45.0);
        camera->OrthogonalizeViewUp();
        rwi->Render();
        break;
      }
      case 'r' :    case 'R' :
      {
        vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
        if (this->Interactor->GetShiftKey()) camera->Azimuth(-1.0);
        else if (this->Interactor->GetControlKey()) camera->Azimuth(180);
        else camera->Azimuth(-45.0);
        camera->OrthogonalizeViewUp();
        rwi->Render();
        break;
      }
      case 'g' :
      {
        string strTmp = rwi->GetRenderWindow()->GetWindowName();
        vtksys::SystemTools::ReplaceString(strTmp,string(".gii"),string(""));
        vtksys::SystemTools::ReplaceString(strTmp,string(".txt"),string(""));
    
        string extension = ".png";
    
        // Adding extension using simple concatenation
        string fullFilename = strTmp + extension;
  
        vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
        windowToImageFilter->SetInput( rwi->GetRenderWindow() );
        windowToImageFilter->Update();  // Ensure the filter processes the current render window content
        vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();      
        writer->SetFileName(fullFilename.c_str());
        writer->SetInputConnection(windowToImageFilter->GetOutputPort());
        writer->Write();
        cout << "Save " << fullFilename << endl;
        break;
      }
      case 'h' :
      {
        cout << endl
        << "KEYBOARD INTERACTIONS" << endl
        << "  u d l r" << endl
        << "     Rotate up/down/left/right by 45 degree." << endl
        << "  U D L R" << endl
        << "     Rotate up/down/left/right by 1 degree." << endl
        << "  Ctrl/Cmd + u d l r" << endl
        << "     Rotate up/down/left/right by 180 degree." << endl
        << "  w " << endl
        << "     Show wireframe." << endl
        << "  s " << endl
        << "     Show shaded." << endl
        << "  g " << endl
        << "     Grab image to png-file." << endl
        << "  q e" << endl
        << "     Quit." << endl
        << endl
        << endl;
        break;
      }
    }

  }
};
vtkStandardNewMacro(CustomInteractorStyle);

#endif // vtkCustomInteractorStyle_h
