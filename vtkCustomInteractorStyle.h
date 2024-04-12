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
      case '3' :
        vtkInteractorStyle::OnChar();
        break;
      // Rotate upwards
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
      // Rotate downwards
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
      // Rotate left
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
      // Rotate right
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
      // Reset to original view
      case 'o' :    case 'O' :
      {
        vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
        this->CurrentRenderer->ResetCamera();
        camera->OrthogonalizeViewUp();
        camera->Zoom(2.0);
        rwi->Render();
        break;
      }
      // Flip top views backward to bottom views
      case 'b' :
      {
        vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
        vtkActorCollection* actors = this->CurrentRenderer->GetActors();

        actors->InitTraversal();
        vtkActor* actor;

        // Estimate number of actors
        int nActors = 0;
        while ((actor = actors->GetNextActor()))
          nActors++;
        
        // Reset
        actors->InitTraversal();
        int k = 0;
        while ((actor = actors->GetNextActor())) {
          k++;
          vtkMapper* mapper = actor->GetMapper();
          
          // If overlay is used we have 12 actors and have to rotate the 2 meshes
          // and the 2 overlay
          if (nActors == 12) {
            if ((k == 9) || (k == 10) || (k == 11) || (k == 12))
              actor->RotateX(180);
          } else {
          // Otherwise just rotate the 2 meshes if no overlay is given
            if ((k == 5) || (k == 6))
              actor->RotateX(180);
          }
        }
        
        camera->OrthogonalizeViewUp();
        rwi->Render();
        break;
      }
      // Grab image and save as png file
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
        windowToImageFilter->SetInputBufferTypeToRGBA(); // Also capture the alpha (transparency) channel
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
        << "  b " << endl
        << "     Flip top view backwards to bottom view." << endl
        << "  o " << endl
        << "     Reset to original view." << endl
        << "  w " << endl
        << "     Show wireframe." << endl
        << "  s " << endl
        << "     Show shaded." << endl
        << "  g " << endl
        << "     Grab image to png-file." << endl
        << "  3 " << endl
        << "     Stereo view" << endl
        << "  f " << endl
        << "     Zoom to the selected point." << endl
        << "  p " << endl
        << "     Select render and show its box." << endl
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
