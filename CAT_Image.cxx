
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPolyData.h>
#include <vtksys/SystemTools.hxx>
#include <vtkProperty.h>
#include <vtkActor.h>
#include <vtkImageData.h>
#include "vtkMINCImageReader.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkSmartPointer.h"

#ifdef vtkCAT_USE_ITK
#include <vtkMetaImageData.h>
#endif

#include "vtkViewImage2D.h"
#include "vtkViewImage3D.h"
#include "vtkSurfaceReader.h"

#define mirrorSurface 1

static void usage(const char* const prog);

static int defaultWindowSize = 400;

int main (int argc, char* argv[])
{

  int isMinc = 0;
  int windowSize = defaultWindowSize;
  double origin[3];

  int indx = -1;
  for (int j = 1; j < argc; j++) {
   if (argv[j][0] != '-') {
    indx = j;
    break;
   }
   else if (strcmp(argv[j], "-size") == 0) {
    j++; windowSize = atof(argv[j]);
   }
   else {
    cout << endl;
    cout << "ERROR: Unrecognized argument: " << argv[j] << endl; 
    cout << endl;
    exit(1);
   }
  }
  if (indx < 0) {
   usage(argv[0]);
   exit(1);
  }

  const char *imageFileName = argv[indx];
  const int numArgs = argc - indx;

  if (!strcmp(vtksys::SystemTools::GetFilenameLastExtension(imageFileName).c_str(),".mnc")) 
    isMinc = 1;

  vtkImageData *image = vtkImageData::New();
  vtkMINCImageReader *reader = vtkMINCImageReader::New();
  
#ifdef vtkCAT_USE_ITK
  vtkMetaImageData* metaimage = vtkMetaImageData::New();
  if(isMinc==0) 
  {
    metaimage->Read(imageFileName);
    image = metaimage->GetImageData();
    vtkMatrix4x4* matrix = metaimage->GetOrientationMatrix();
    for (int i = 0; i < 3; i++) origin[i] = matrix->GetElement(i,3)/matrix->GetElement(i,i);
    image->SetOrigin(origin);
    matrix->Delete();
  }
#endif

  if(isMinc) 
  {
    reader->SetFileName (imageFileName);
    reader->GetOutput()->Update();
    image = reader->GetOutput();
  }
  
  // read up to 3 surfaces
  vtkSurfaceReader *polyDataReader  = vtkSurfaceReader::New();
  vtkSurfaceReader *polyDataReader2 = vtkSurfaceReader::New();
  vtkSurfaceReader *polyDataReader3 = vtkSurfaceReader::New();

  vtkPolyData *polyData  = vtkPolyData::New();
  vtkPolyData *polyData2 = vtkPolyData::New();
  vtkPolyData *polyData3 = vtkPolyData::New();

  /* create transformation to mirrow surfaces */
  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  transform->Scale(-1.0, 1.0, 1.0);
  vtkTransformPolyDataFilter *st  = vtkTransformPolyDataFilter::New();
  vtkTransformPolyDataFilter *st2 = vtkTransformPolyDataFilter::New();
  vtkTransformPolyDataFilter *st3 = vtkTransformPolyDataFilter::New();

    if (numArgs>1) 
  {
    polyDataReader->SetFileName(argv[indx+1]);
    polyDataReader->Update();

#ifdef mirrorSurface
    st -> SetInput(polyDataReader->GetOutput());
    st -> SetTransform(transform);
    st -> Update();
    polyData = st->GetOutput();
#else
    polyData = polyDataReader->GetOutput();
#endif

    polyData->Update();
  
      cout << "\033[22;31m" << argv[indx+1] << std::endl;
  }

  if (numArgs>2) 
  {
    polyDataReader2->SetFileName(argv[indx+2]);
    polyDataReader2->Update();

#ifdef mirrorSurface
    st2 -> SetInput(polyDataReader2->GetOutput());
    st2 -> SetTransform(transform);
    st2 -> Update();
    polyData2 = st2->GetOutput();
#else
    polyData2 = polyDataReader2->GetOutput();
#endif

    polyData2->Update();
    cout << "\033[22;32m" << argv[indx+2] << std::endl;
  }

  if (numArgs>3) 
  {
    polyDataReader3->SetFileName(argv[indx+3]);
    polyDataReader3->Update();

#ifdef mirrorSurface
    st3 -> SetInput(polyDataReader3->GetOutput());
    st3 -> SetTransform(transform);
    st3 -> Update();
    polyData3 = st3->GetOutput();
#else
    polyData3 = polyDataReader3->GetOutput();
#endif

    polyData3->Update();
    cout << "\033[22;34m" << argv[indx+3] << std::endl;
  }

  /* reset colored text */
  cout << "\033[0m";
  
  /**
     Create 3 views, each of them will have a different orientation, .i.e.
     axial, sagittal and coronal.
     Create one 3D view.
   */
  vtkViewImage2D* view1 = vtkViewImage2D::New();
  vtkViewImage2D* view2 = vtkViewImage2D::New();
  vtkViewImage2D* view3 = vtkViewImage2D::New();

  vtkRenderWindowInteractor* renderWindowInteractor1 = vtkRenderWindowInteractor::New();
  vtkRenderWindowInteractor* renderWindowInteractor2 = vtkRenderWindowInteractor::New();
  vtkRenderWindowInteractor* renderWindowInteractor3 = vtkRenderWindowInteractor::New();

  vtkRenderWindow* renderWindow1 = vtkRenderWindow::New();
  vtkRenderWindow* renderWindow2 = vtkRenderWindow::New();
  vtkRenderWindow* renderWindow3 = vtkRenderWindow::New();

  vtkRenderer* renderer1 = vtkRenderer::New();
  vtkRenderer* renderer2 = vtkRenderer::New();
  vtkRenderer* renderer3 = vtkRenderer::New();

  renderWindow1->AddRenderer (renderer1);
  renderWindow2->AddRenderer (renderer2);
  renderWindow3->AddRenderer (renderer3);

  // windowsize
  renderWindow1->SetSize(windowSize, windowSize);
  renderWindow2->SetSize(windowSize, windowSize);
  renderWindow3->SetSize(windowSize, windowSize);

  renderWindow1->SetPosition (0, 40);
  renderWindow2->SetPosition (0, 60+windowSize);
  renderWindow3->SetPosition (windowSize, 60+windowSize);

  renderWindowInteractor1->SetRenderWindow (renderWindow1);
  renderWindowInteractor2->SetRenderWindow (renderWindow2);
  renderWindowInteractor3->SetRenderWindow (renderWindow3);

  view1->SetRenderWindow (renderWindow1);
  view2->SetRenderWindow (renderWindow2);
  view3->SetRenderWindow (renderWindow3);

  view1->SetRenderer (renderer1);
  view2->SetRenderer (renderer2);
  view3->SetRenderer (renderer3);

  /**
     Set some properties to the views, like the interaction style, orientation and
     background color.
   */

  // One can also associate to each button (left, middle, right and even wheel)
  // a specific interaction like this:
  
  view1->SetLeftButtonInteractionStyle   (vtkViewImage2D::SELECT_INTERACTION);
  view1->SetMiddleButtonInteractionStyle (vtkViewImage2D::SELECT_INTERACTION);
  view1->SetWheelInteractionStyle        (vtkViewImage2D::ZOOM_INTERACTION);
  view1->SetRightButtonInteractionStyle  (vtkViewImage2D::WINDOW_LEVEL_INTERACTION);

  view2->SetLeftButtonInteractionStyle   (vtkViewImage2D::SELECT_INTERACTION);
  view2->SetMiddleButtonInteractionStyle (vtkViewImage2D::SELECT_INTERACTION);
  view2->SetWheelInteractionStyle        (vtkViewImage2D::ZOOM_INTERACTION);
  view2->SetRightButtonInteractionStyle  (vtkViewImage2D::WINDOW_LEVEL_INTERACTION);

  view3->SetLeftButtonInteractionStyle   (vtkViewImage2D::SELECT_INTERACTION);
  view3->SetMiddleButtonInteractionStyle (vtkViewImage2D::SELECT_INTERACTION);
  view3->SetWheelInteractionStyle        (vtkViewImage2D::ZOOM_INTERACTION);
  view3->SetRightButtonInteractionStyle  (vtkViewImage2D::WINDOW_LEVEL_INTERACTION);
  
  view1->SetConventionsToNeurological();
  view2->SetConventionsToNeurological();
  view3->SetConventionsToNeurological();
  
  view1->SetLinkZoom (true);
  view2->SetLinkZoom (true);
  view3->SetLinkZoom (true);

  view1->SetOrientation (vtkViewImage2D::AXIAL_ID);
  view2->SetOrientation (vtkViewImage2D::CORONAL_ID);
  view3->SetOrientation (vtkViewImage2D::SAGITTAL_ID);

  view1->SetBackgroundColor (0.0,0.0,0.0);
  view2->SetBackgroundColor (0.0,0.0,0.0);
  view3->SetBackgroundColor (0.0,0.0,0.0);  

  view1->SetAboutData ("Axial");
  view2->SetAboutData ("Coronal");
  view3->SetAboutData ("Sagittal");

  /**
     Link the views together for synchronization.
   */
  view1->AddChild (view2);
  view2->AddChild (view3);
  view3->AddChild (view1);
  
  view1->SetImage (image);
  view2->SetImage (image);
  view3->SetImage (image);

  vtkProperty* prop  = vtkProperty::New();
  vtkProperty* prop2 = vtkProperty::New();
  vtkProperty* prop3 = vtkProperty::New();

  if (numArgs>1) 
  {
    prop->SetColor (1,0,0);
    view1->AddDataSet(polyData, prop);
    view2->AddDataSet(polyData, prop);
    view3->AddDataSet(polyData, prop);
  }

  if (numArgs>2) 
  {
    prop2->SetColor (0,1,0);
    view1->AddDataSet(polyData2, prop2);
    view2->AddDataSet(polyData2, prop2);
    view3->AddDataSet(polyData2, prop2);
  }

  if (numArgs>3) 
  {
    prop3->SetColor (0,0,1);
    view1->AddDataSet(polyData3, prop3);
    view2->AddDataSet(polyData3, prop3);
    view3->AddDataSet(polyData3, prop3);
  }

  /**
     Reset the window/level and the current position.
  */

  view1->SyncResetCurrentPoint();
  view1->SyncResetWindowLevel();
  
  renderWindowInteractor1->Initialize();
  renderWindowInteractor2->Initialize();
  renderWindowInteractor3->Initialize();

  renderWindow1->Render();
  renderWindow2->Render();
  renderWindow3->Render();

  renderWindow1->SetWindowName(imageFileName);
  renderWindow2->SetWindowName(imageFileName);
  renderWindow3->SetWindowName(imageFileName);

  renderWindowInteractor1->Start();
  renderWindowInteractor2->Start();
  renderWindowInteractor3->Start();

  view1->Detach();
  view2->Detach();
  view3->Detach();

  view1->Delete();
  view2->Delete();
  view3->Delete();

  renderWindow1->Delete();
  renderWindow2->Delete();
  renderWindow3->Delete();

  renderWindowInteractor1->Delete();
  renderWindowInteractor2->Delete();
  renderWindowInteractor3->Delete();

  renderer1->Delete();
  renderer2->Delete();
  renderer3->Delete();

  reader->Delete();
#ifdef vtkCAT_USE_ITK
  metaimage->Delete();
#endif  
  image->Delete();

  prop->Delete();
  prop2->Delete();
  prop3->Delete();

  transform->Delete();
  st->Delete();
  st2->Delete();
  st3->Delete();

  polyDataReader->Delete();
  polyDataReader2->Delete();
  polyData->Delete();
  polyData2->Delete();

  return 0;
}

static void
usage(const char* const prog)
{
  cout << endl
  << "NAME " << endl
  << "    " << prog << " - display image file and optionally overlay surface or image data"<< endl
  << "" << endl
  << "SYNOPSIS" << endl
  << "    " << prog << " <input.mnc> [surf1.obj [surf2.obj] [surf3.obj]]" << endl
  << "" << endl
  << "DESCRIPTION" << endl
  << "    This program will display the <input.mnc> image." << endl
  << endl
  << "OPTIONS" << endl
  << "  -size WindowSize  " << endl
  << "     Size of window." << endl
  << "     Default value: " << defaultWindowSize << endl
  << endl
  << "REQUIRED PARAMETERS" << endl
  << "    <input.mnc> " << endl
  << "" << endl
  << "EXAMPLE" << endl
  << "    " << prog << " input.mnc surf1.obj surf2.obj surf3.obj" << endl
  << endl
  << endl;
}

