#include <vtkActor.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkPolyDataReader.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h> 
#include <vtkLookupTable.h>
#include <vtkLookupTableWithEnabling.h>
#include <vtkScalarBarActor.h>
#include <vtkCurvatures.h>
#include <vtkTextProperty.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>
#include <vtkInteractorStyle.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkColorTransferFunction.h>
#include <vtksys/SystemTools.hxx>
#include "vtkSurfaceReader.h"
#include "vtkCustomInteractorStyle.h"
#include "vtkStat.h"
#include "vtkCamera.h"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>

namespace fs = std::__fs::filesystem;
using namespace std;

typedef enum ColorMap {
    C1, C2, C3, JET, GRAY, REDYELLOW, BLUECYAN, YELLOWRED, CYANBLUE, BLUEGREEN, GREENBLUE
} ColorMap;

static void usage(const char* const prog);

static double defaultScalarRange[2] = { 0.0, -1.0 };
static double defaultScalarRangeBkg[2] = { 0.0, -1.0 };
static double defaultAlpha = 0.8;
static double defaultClipRange[2] = { 0.0, -1.0 };
static double defaultRotate[3] = { 270.0, 0.0, -90.0 };
static int defaultColorbar = 0;
static int defaultInverse = 0;
static int defaultBkg = 0;
static int defaultWindowSize[2] = { 1800, 1000 };

// Obtain LookUpTable for defined colormap
vtkSmartPointer<vtkLookupTableWithEnabling> getLookupTable(int colormap, double alpha)
{
  vtkSmartPointer<vtkLookupTableWithEnabling> lookupTable = vtkSmartPointer<vtkLookupTableWithEnabling>::New();
  vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();

  switch(colormap) {
  case C1:

    // Add RGB points to the function
    colorTransferFunction->AddRGBPoint(100.0,213/255.0,62/255.0,79/255.0); 
    colorTransferFunction->AddRGBPoint(87.5, 244/255.0,109/255.0,67/255.0); 
    colorTransferFunction->AddRGBPoint(75.0, 253/255.0,174/255.0,97/255.0); 
    colorTransferFunction->AddRGBPoint(62.5, 254/255.0,224/255.0,139/255.0);
    colorTransferFunction->AddRGBPoint(50.0, 255/255.0,255/255.0,191/255.0);
    colorTransferFunction->AddRGBPoint(37.5, 230/255.0,245/255.0,152/255.0);
    colorTransferFunction->AddRGBPoint(25.0, 171/255.0,221/255.0,164/255.0);
    colorTransferFunction->AddRGBPoint(12.5, 102/255.0,194/255.0,165/255.0);
    colorTransferFunction->AddRGBPoint(0.0,  50/255.0,136/255.0,189/255.0);

    // Fill the lookup table using the color transfer function
    for (int i = 0; i < 256; i++) {
        double* rgb;
        double value = (static_cast<double>(i) / 255.0) * 100.0;
        rgb = colorTransferFunction->GetColor(value);
        lookupTable->SetTableValue(i, rgb[0], rgb[1], rgb[2], alpha); // Set RGBA, with full opacity
    }
    break;
  case C2:

    // Add RGB points to the function
    colorTransferFunction->AddRGBPoint(0.0, 0, 0.6, 1); 
    colorTransferFunction->AddRGBPoint(25.0, 0.5, 1, 0.5); 
    colorTransferFunction->AddRGBPoint(50.0, 1, 1, 0.5); 
    colorTransferFunction->AddRGBPoint(75.0, 1, 0.75, 0.5); 
    colorTransferFunction->AddRGBPoint(100.0, 1, 0.5, 0.5);

    // Fill the lookup table using the color transfer function
    for (int i = 0; i < 256; i++) {
        double* rgb;
        double value = (static_cast<double>(i) / 255.0) * 100.0;
        rgb = colorTransferFunction->GetColor(value);
        lookupTable->SetTableValue(i, rgb[0], rgb[1], rgb[2], alpha); // Set RGBA, with full opacity
    }
    break;
  case C3:

    // Add RGB points to the function
    colorTransferFunction->AddRGBPoint(0.0,  0/255.0,143/255.0,213/255.0); 
    colorTransferFunction->AddRGBPoint(25.0, 111/255.0,190/255.0,70/255.0); 
    colorTransferFunction->AddRGBPoint(50.0, 255/255.0,220/255.0,45/255.0); 
    colorTransferFunction->AddRGBPoint(75.0, 252/255.0,171/255.0,23/255.0); 
    colorTransferFunction->AddRGBPoint(100.0,238/255.0,28/255.0,58/255.0);

    // Fill the lookup table using the color transfer function
    for (int i = 0; i < 256; i++) {
        double* rgb;
        double value = (static_cast<double>(i) / 255.0) * 100.0;
        rgb = colorTransferFunction->GetColor(value);
        lookupTable->SetTableValue(i, rgb[0], rgb[1], rgb[2], alpha); // Set RGBA, with full opacity
    }
    break;
  case JET:
    lookupTable->SetHueRange( 0.667, 0.0 );
    lookupTable->SetSaturationRange( 1.0, 1.0 );
    lookupTable->SetValueRange( 1.0, 1.0 );
    break;
  case GRAY:
    lookupTable->SetHueRange( 0.0, 0.0 );
    lookupTable->SetSaturationRange( 0.0, 0.0 );
    lookupTable->SetValueRange( 0.0, 1.0 );
    break;
  case REDYELLOW:
    lookupTable->SetHueRange( 0.0, 0.1667 );
    lookupTable->SetSaturationRange( 1.0, 1.0 );
    lookupTable->SetValueRange( 1.0, 1.0 );
    break;
  case BLUECYAN:
    lookupTable->SetHueRange( 0.66667, 0.5);
    lookupTable->SetSaturationRange( 1.0, 1.0 );
    lookupTable->SetValueRange( 1.0, 1.0 );
    break;
  case YELLOWRED:
    lookupTable->SetHueRange( 0.1667, 0.0 );
    lookupTable->SetSaturationRange( 1.0, 1.0 );
    lookupTable->SetValueRange( 1.0, 1.0 );
    break;
  case CYANBLUE:
    lookupTable->SetHueRange( 0.5, 0.66667);
    lookupTable->SetSaturationRange( 1.0, 1.0 );
    lookupTable->SetValueRange( 1.0, 1.0 );
    break;
  case BLUEGREEN:
    lookupTable->SetHueRange( 0.66667, 0.33333);
    lookupTable->SetSaturationRange( 1.0, 1.0 );
    lookupTable->SetValueRange( 1.0, 1.0 );
    break;
  case GREENBLUE:
    lookupTable->SetHueRange( 0.33333, 0.66667);
    lookupTable->SetSaturationRange( 1.0, 1.0 );
    lookupTable->SetValueRange( 1.0, 1.0 );
    break;
  }
  return lookupTable;
}
static void
usage(const char* const prog)
{
  cout << endl
  << "NAME " << endl
  << "    " << prog << " - render left and right surfaces"<< endl
  << "" << endl
  << "SYNOPSIS" << endl
  << "    " << prog << " [options] <left_surface>" << endl
  << "" << endl
  << "DESCRIPTION" << endl
  << "    This program will render the left and right surfaces. Only the left hemipshere should be defined while the right is automatically selected." << endl
  << endl
  << "OPTIONS" << endl
  << " Overlay:" << endl
  << "  -scalar scalarInput (deprecated) " << endl
  << "     File with scalar values (gifit, ascii or Freesurfer format), either for the left or merged hemispheres." << endl
  << endl
  << "  -overlay scalarInput  " << endl
  << "     File with scalar values (gifit, ascii or Freesurfer format), either for the left or merged hemispheres." << endl
  << endl
  << "  -range lower upper  " << endl
  << "     Range of scalar values." << endl
  << "     Default value: " << defaultScalarRange[0] << " " << defaultScalarRange[1] << endl
  << endl
  << "  -clip lower upper  " << endl
  << "     Clip scalar values. These values will be not displayed." << endl
  << "     Default value: " << defaultClipRange[0] << " " << defaultClipRange[1] << endl
  << endl
  << "  -bkg scalarInputBkg  " << endl
  << "     File with scalar values for background surface (gifit, ascii or Freesurfer format), either for the left or merged hemispheres." << endl
  << endl
  << "  -range-bkg lower upper  " << endl
  << "     Range of background scalar values." << endl
  << "     Default value: " << defaultScalarRangeBkg[0] << " " << defaultScalarRangeBkg[1] << endl
  << endl
  << "  -colorbar  " << endl
  << "     Show colorbar (default no)." << endl
  << endl
  << "  -title  " << endl
  << "     Set name for colorbar (default scalar-file)." << endl
  << endl
  << "  -inverse  " << endl
  << "     Invert input values." << endl
  << endl
  << "  -stats  " << endl
  << "     Output mean/median/SD in colorbar." << endl
  << endl
  << " Colors:" << endl
  << "  -opacity value  " << endl
  << "     Value for opacity of overlay." << endl
  << "     Default value: " << defaultAlpha << endl
  << endl
  << "  -white  " << endl
  << "     Use white background" << endl
  << endl
  << "  -c1  " << endl
  << "     Use custom rainbow colorbar 1 (default jet)." << endl
  << "  -c2  " << endl
  << "     Use custom rainbow colorbar 2 (default jet)." << endl
  << "  -c3  " << endl
  << "     Use custom rainbow colorbar 3 (default jet)." << endl
  << "  -gray  " << endl
  << "     Use gray colorbar (default jet)." << endl
  << "  -redyellow  " << endl
  << "     Use red-yellow colorbar (default jet)." << endl
  << "  -bluecyan  " << endl
  << "     Use blue-cyan colorbar (default jet)." << endl
  << "  -yellowred  " << endl
  << "     Use yellow-red colorbar (default jet)." << endl
  << "  -cyanblue  " << endl
  << "     Use cyan-blue colorbar (default jet)." << endl
  << "  -bluegreen  " << endl
  << "     Use blue-green colorbar (default jet)." << endl
  << "  -greenblue  " << endl
  << "     Use green-blue colorbar (default jet)." << endl
  << endl
  << " Output:" << endl
  << "  -size xsize ysize  " << endl
  << "     Window size." << endl
  << "     Default value: " << defaultWindowSize[0] << " " << defaultWindowSize[1] << endl
  << endl
  << "  -output output.png  " << endl
  << "     Save as png-file or skip extension to save image with the overlay or mesh name as png." << endl
  << endl
  << "KEYBOARD INTERACTIONS" << endl
  << "      Use Shift-key for small rotations and Ctrl/Cmd-key for flipping." << endl
  << endl
  << "  l L Rotate left." << endl
  << endl
  << "  r R Rotate right." << endl
  << endl
  << "  u U Rotate up." << endl
  << endl
  << "  d D Rotate down." << endl
  << endl
  << "  w   Show wireframe." << endl
  << endl
  << "  s   Show shaded." << endl
  << endl
  << "  3   Stereo view." << endl
  << endl
  << "  g   Save image png-file." << endl
  << endl
  << "  f   Zoom to selected point." << endl
  << endl
  << "  q e Quit." << endl
  << endl
  << "REQUIRED PARAMETERS" << endl
  << "    <input.gii> " << endl
  << "" << endl
  << "EXAMPLE" << endl
  << "    " << prog << " -range -1 1 -overlay overlayInput.txt lh.central.freesurfer.gii" << endl
  << endl
  << endl;
}

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    usage(argv[0]);
    return (EXIT_FAILURE);
  }

  const char *overlayFileNameL = NULL;
  const char *overlayFileNameBkgL = NULL;
  const char *outputFileName = NULL;
  const char *Title = NULL;
  double alpha = defaultAlpha;
  double overlayRange[2] = {defaultScalarRange[0], defaultScalarRange[1]};
  double overlayRangeBkg[2] = {defaultScalarRangeBkg[0], defaultScalarRangeBkg[1]};
  double clipRange[2] = {defaultClipRange[0], defaultClipRange[1]};
  double white[3] = {1.0, 1.0, 1.0};
  double black[3] = {0.0, 0.0, 0.0};
  double val;
  int colormap = JET;
  int colorbar = defaultColorbar;
  int inverse = defaultInverse;
  int bkgWhite = defaultBkg;
  int printStats = 0;
  int overlay = 0, overlayBkg = 0, saveImage = 0, title = 0;
  int WindowSize[2] = {defaultWindowSize[0], defaultWindowSize[1]};
  int indx = -1, nMeshes;
  bool rhExists, rhExistsBkg, bothHemis;

  for (int j = 1; j < argc; j++) {
    if (argv[j][0] != '-') {
      indx = j;
      break;
   }
   else if (strcmp(argv[j], "-range") == 0) {
     j++; overlayRange[0] = atof(argv[j]);
     j++; overlayRange[1] = atof(argv[j]);
   }
   else if (strcmp(argv[j], "-range-bkg") == 0) {
     j++; overlayRangeBkg[0] = atof(argv[j]);
     j++; overlayRangeBkg[1] = atof(argv[j]);
   }
   else if (strcmp(argv[j], "-clip") == 0) {
     j++; clipRange[0] = atof(argv[j]);
     j++; clipRange[1] = atof(argv[j]);
   }
   else if (strcmp(argv[j], "-size") == 0) {
     j++; WindowSize[0] = atoi(argv[j]);
     j++; WindowSize[1] = atoi(argv[j]);
   }
   else if (strcmp(argv[j], "-scalar") == 0 || strcmp(argv[j], "-overlay") == 0) {
     j++; overlayFileNameL = argv[j];
     overlay = 1;
   }
   else if (strcmp(argv[j], "-title") == 0) {
     j++; Title = argv[j];
     title = 1;
   }
   else if (strcmp(argv[j], "-bkg") == 0) {
     j++; overlayFileNameBkgL = argv[j];
     overlayBkg = 1;
   }
   else if (strcmp(argv[j], "-output") == 0) {
     j++; outputFileName = argv[j];
     saveImage = 1;
   }
   else if (strcmp(argv[j], "-opacity") == 0) {
     j++; alpha = atof(argv[j]);
   }
   else if (strcmp(argv[j], "-stats") == 0)
     printStats = 1;
   else if (strcmp(argv[j], "-inverse") == 0)
     inverse = 1;
   else if (strcmp(argv[j], "-colorbar") == 0) 
     colorbar = 1;
   else if (strcmp(argv[j], "-white") == 0) 
    bkgWhite = 1;
   else if (strcmp(argv[j], "-c1") == 0) 
    colormap = C1;
   else if (strcmp(argv[j], "-c2") == 0) 
    colormap = C2;
   else if (strcmp(argv[j], "-c3") == 0) 
    colormap = C3;
   else if (strcmp(argv[j], "-gray") == 0) 
    colormap = GRAY;
   else if (strcmp(argv[j], "-redyellow") == 0) 
    colormap = REDYELLOW;
   else if (strcmp(argv[j], "-bluecyan") == 0) 
    colormap = BLUECYAN;
   else if (strcmp(argv[j], "-yellowred") == 0) 
    colormap = YELLOWRED;
   else if (strcmp(argv[j], "-cyanblue") == 0) 
    colormap = CYANBLUE;
   else if (strcmp(argv[j], "-bluegreen") == 0) 
    colormap = BLUEGREEN;
   else if (strcmp(argv[j], "-greenblue") == 0) 
    colormap = GREENBLUE;
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

  const int numArgs = argc - indx;

  vtkSmartPointer<vtkPolyData> polyData[2];
  vtkSmartPointer<vtkLookupTableWithEnabling> lookupTable[2];
  lookupTable[0] = vtkSmartPointer<vtkLookupTableWithEnabling>::New();
  lookupTable[1] = vtkSmartPointer<vtkLookupTableWithEnabling>::New();
  vtkSmartPointer<vtkLookupTableWithEnabling> lookupTableBkg = vtkSmartPointer<vtkLookupTableWithEnabling>::New();

  vtkSmartPointer<vtkDoubleArray> scalarsBkg[2];
  scalarsBkg[0] = vtkSmartPointer<vtkDoubleArray>::New();
  scalarsBkg[1] = vtkSmartPointer<vtkDoubleArray>::New();

  vtkSmartPointer<vtkDoubleArray> scalars[2];
  scalars[0] = vtkSmartPointer<vtkDoubleArray>::New();
  scalars[1] = vtkSmartPointer<vtkDoubleArray>::New();

  vtkSmartPointer<vtkCurvatures> curvature[2];
  curvature[0] = vtkSmartPointer<vtkCurvatures>::New();
  curvature[1] = vtkSmartPointer<vtkCurvatures>::New();

  vtkSmartPointer<vtkPolyData> curvaturesMesh[2];
  vtkSmartPointer<vtkPolyDataMapper> mapper[2];
  vtkSmartPointer<vtkPolyDataMapper> mapperBkg[2];
  
  // check whether filename contains "lh." or "left" and replace filename with
  // name for the right hemisphere and check whether the file exists  
  string rhSurfName = argv[indx];
  if (rhSurfName.find("lh.") != string::npos) {
    vtksys::SystemTools::ReplaceString(rhSurfName,string("lh."),string("rh."));
    rhExists = vtksys::SystemTools::FileExists(rhSurfName);
  } else if (rhSurfName.find("left") != string::npos) {
    string rhSurfName = argv[indx];
    vtksys::SystemTools::ReplaceString(rhSurfName,string("left"),string("right"));
    rhExists = vtksys::SystemTools::FileExists(rhSurfName);
  } else rhExists = false;
  
  try {
    polyData[0] = ReadGIFTIMesh(argv[indx]);
  } catch (const exception& e) {
    cerr << "Failed to read GIFTI file: " << e.what() << endl;
    return EXIT_FAILURE;
  }
  
  if (rhExists) {
    try {
      polyData[1] = ReadGIFTIMesh(rhSurfName.c_str());
    } catch (const exception& e) {
      cerr << "Failed to read GIFTI file: " << e.what() << endl;
      return EXIT_FAILURE;
    }
    nMeshes = 2;
  } else nMeshes = 1;
  
  fs::path currentPath = fs::current_path();

  // read background scalars if defined
  if (overlayBkg) {
    
    cout << "Read underlays: " << overlayFileNameBkgL << endl;
    
    scalarsBkg[0] = readScalars(overlayFileNameBkgL);
    
    // if defined, read right hemispheric overlay
    if ((nMeshes > 1) && rhExists) {

      // try replacing lh/left by rh/right in the overlay filename
      string overlayFileNameR = overlayFileNameBkgL;
      if (overlayFileNameR.find("lh.") != string::npos) {
        vtksys::SystemTools::ReplaceString(overlayFileNameR,string("lh."),string("rh."));
        rhExistsBkg = vtksys::SystemTools::FileExists(overlayFileNameR);
      } else if (rhSurfName.find("left") != string::npos) {
        string overlayFileNameR = overlayFileNameBkgL;
        vtksys::SystemTools::ReplaceString(overlayFileNameR,string("left"),string("right"));
        rhExistsBkg = vtksys::SystemTools::FileExists(overlayFileNameR);
      } else rhExistsBkg = false;

      if (rhExistsBkg)
        scalarsBkg[1] = readScalars(overlayFileNameR.c_str());
      else {
        // or split the single overlay (which is merged) into left and right
        // overlay
        try {
          scalarsBkg[1]->SetNumberOfTuples(polyData[0]->GetNumberOfPoints());
          for (auto i=0; i < polyData[0]->GetNumberOfPoints(); i++)
            scalarsBkg[1]->SetValue(i,scalarsBkg[0]->GetValue(i+polyData[1]->GetNumberOfPoints()));
        } catch (const exception& e) {
          cerr << "Error splitting values for left/right hemisphere in file " << overlayFileNameBkgL << endl;
          return(-1);
        }
      }
    }
    
    if (scalarsBkg[0] == NULL) {
      cerr << "Error reading file " << overlayFileNameBkgL << endl;
      return(-1);
    }
  }

  for (auto i = 0; i < nMeshes; i++) {
  
    curvature[i]->SetInputData(polyData[i]);
    curvature[i]->SetCurvatureTypeToMean();
    curvature[i]->Update();
    curvaturesMesh[i] = curvature[i]->GetOutput();

    if (overlayBkg)
      curvaturesMesh[i]->GetPointData()->SetScalars(scalarsBkg[i]);  

    // Create mapper
    mapper[i] = vtkSmartPointer<vtkPolyDataMapper>::New();

    if (overlay)
      mapper[i]->SetInputData(polyData[i]);

    mapperBkg[i] = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapperBkg[i]->SetInputData(curvaturesMesh[i]);

    if (overlayRangeBkg[1] < overlayRangeBkg[0])
      curvaturesMesh[i]->GetScalarRange( overlayRangeBkg );

  }

  // Create actor
  auto const numberOfViews = 6;
  array<vtkNew<vtkActor>, numberOfViews> actor;
  array<vtkNew<vtkActor>, numberOfViews> actorBkg;
  array<double, 3> position{{0, 0, 0}};
  
  double shift[2] = {180.0, 180.0};
  array<double, numberOfViews> positionx{{0, 2*shift[0], 0.15*shift[0], 1.85*shift[0], shift[0], shift[0]}};
  array<double, numberOfViews> positiony{{0, 0, 0.8*shift[1], 0.8*shift[1], 0.6*shift[1], 0.6*shift[1]}};
  array<double, numberOfViews> rotatex{{270, 270, 270, 270, 0, 0}};
  array<double, numberOfViews> rotatey{{0, 0, 0, 0, 0, 0}};
  array<double, numberOfViews> rotatez{{90, -90, -90, 90, 0, 0}};
  array<int,    numberOfViews> order{{0, 1, 0, 1, 0, 1}};

  for (auto i = 0; i < actor.size(); ++i)
  {
    // if only left hemisphere exists, then skip display of right hemipshere
    if ((nMeshes == 1) && (i % 2)) continue;
    
    position[0] = positionx[i];
    position[1] = positiony[i];
    
    actorBkg[i]->SetMapper(mapperBkg[order[i]]);  
  
    // configure the basic properties
    actorBkg[i]->GetProperty()->SetAmbient(0.8);
    actorBkg[i]->GetProperty()->SetDiffuse(0.7);
    actorBkg[i]->GetProperty()->SetSpecular(0.0);
    actorBkg[i]->AddPosition(position.data());
    actorBkg[i]->RotateX(rotatex[i]);
    actorBkg[i]->RotateY(rotatey[i]);
    actorBkg[i]->RotateZ(rotatez[i]);

    if (overlay) {
      actor[i]->SetMapper(mapper[order[i]]);
  
      // configure the basic properties
      actor[i]->GetProperty()->SetAmbient(0.3);
      actor[i]->GetProperty()->SetDiffuse(0.7);
      actor[i]->GetProperty()->SetSpecular(0.0);
      actor[i]->AddPosition(position.data());
      actor[i]->RotateX(rotatex[i]);
      actor[i]->RotateY(rotatey[i]);
      actor[i]->RotateZ(rotatez[i]);
    }    
  }
  
  // A renderer and render window
  vtkNew<vtkRenderer> renderer;

  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);
  renderWindow->SetSize(WindowSize[0], WindowSize[1]);
  renderWindow->SetAlphaBitPlanes(1);  // Enable usage of alpha channel

  if (bkgWhite) renderer->SetBackground(white);
  else renderer->SetBackground(black);

  renderer->SetBackgroundAlpha(0.0); // Set transparency

  // Add the actors to the scene
  for (auto i = 0; i < actor.size(); ++i)
  {
    renderer->AddActor(actorBkg[i]);
    if (overlay) renderer->AddActor(actor[i]);
  }

  // read scalars if defined
  if (overlay) {

    // Since the dat-file is otherwise not found, we have to go into the folder
    // of the overlay data to ensure that the gii/dat files are correctly read
    string directoryPath = vtksys::SystemTools::GetFilenamePath(overlayFileNameL);
    string baseNameL = vtksys::SystemTools::GetFilenameName(overlayFileNameL);
    
    // change current path
    fs::current_path(directoryPath);
    
    cout << "Read overlays: " << overlayFileNameL << endl;
    
    scalars[0] = readScalars(baseNameL.c_str());
    
    // if defined, read right hemispheric overlay
    if ((nMeshes > 1) && rhExists) {

      // try replacing lh/left by rh/right in the overlay filename
      string overlayFileNameR = baseNameL.c_str();
      if (overlayFileNameR.find("lh.") != string::npos) {
        vtksys::SystemTools::ReplaceString(overlayFileNameR,string("lh."),string("rh."));
        rhExists = vtksys::SystemTools::FileExists(overlayFileNameR);
      } else if (rhSurfName.find("left") != string::npos) {
        string overlayFileNameR = baseNameL.c_str();
        vtksys::SystemTools::ReplaceString(overlayFileNameR,string("left"),string("right"));
        rhExists = vtksys::SystemTools::FileExists(overlayFileNameR);
      } else rhExists = false;

      if (rhExists)
        scalars[1] = readScalars(overlayFileNameR.c_str());
      else {
        // or split the single overlay (which is merged) into left and right
        // overlay
        try {
          scalars[1]->SetNumberOfTuples(polyData[0]->GetNumberOfPoints());
          for (auto i=0; i < polyData[0]->GetNumberOfPoints(); i++)
            scalars[1]->SetValue(i,scalars[0]->GetValue(i+polyData[1]->GetNumberOfPoints()));
        } catch (const exception& e) {
          cerr << "Error splitting values for left/right hemisphere in file " << overlayFileNameL << endl;
          return(-1);
        }
      }
      
      // go back to current folder
      fs::current_path(currentPath);
    }
    
    if(inverse) {
      for (auto i = 0; i < nMeshes; i++) {
        for (auto k = 0; k < polyData[i]->GetNumberOfPoints(); k++)
          scalars[i]->SetValue(k,-(scalars[i]->GetValue(k)));
      }
    }
    
    if (scalars[0] == NULL) {
      cerr << "Error reading file " << overlayFileNameL << endl;
      return(-1);
    }

    // clip values if defined
    if (clipRange[1] > clipRange[0]) {
      for (auto i = 0; i < nMeshes; i++) {
        for (auto k = 0; k < polyData[0]->GetNumberOfPoints(); k++) {
          val = scalars[i]->GetValue(k);
          if (((val > clipRange[0]) && (val < clipRange[1])) || isnan(val))
            scalars[i]->SetValue(k,0.0);
        }
      }
    }
    
    for (auto i = 0; i < nMeshes; i++) {
      polyData[i]->GetPointData()->SetScalars(scalars[i]);
      if (overlayRange[1] < overlayRange[0])
        polyData[i]->GetScalarRange( overlayRange );
    }
  }
  
  // Obtain the Title for colorbar, window and saved image
  fs::path baseNameL;
  
  if (overlay) {
    fs::path filePath(overlayFileNameL);
    baseNameL = filePath.filename();
  } else {
    fs::path filePath(argv[indx]);
    baseNameL = filePath.filename();
  }

  string strTmp = baseNameL.c_str();
  vtksys::SystemTools::ReplaceString(strTmp,string(".gii"),string(""));
  vtksys::SystemTools::ReplaceString(strTmp,string(".txt"),string(""));

  // Set the title
  if (title == 0) Title = strTmp.c_str();


  // build colormap
  for (auto i = 0; i < nMeshes; i++) {

    // get LUT for colormap
    lookupTable[i] = getLookupTable(colormap,alpha);
  
    lookupTable[i]->SetTableRange( overlayRange );
    if (clipRange[1] > clipRange[0])
      lookupTable[i]->SetEnabledArray(polyData[i]->GetPointData()->GetScalars());

    // set opacity  
    lookupTable[i]->SetAlphaRange( alpha, alpha );

    lookupTable[i]->Build();
  }
  
  lookupTableBkg->SetHueRange( 0.0, 0.0 );
  lookupTableBkg->SetSaturationRange( 0.0, 0.0 );
  lookupTableBkg->SetValueRange( 0.0, 1.0 );

  if (overlayRangeBkg[1] > overlayRangeBkg[0])
    lookupTableBkg->SetTableRange( overlayRangeBkg );
  lookupTableBkg->Build();
  
  for (auto i = 0; i < nMeshes; i++) {
    if (overlay) {
      mapper[i]->SetScalarRange( overlayRange );
      mapper[i]->SetLookupTable( lookupTable[i] );
    }
    
    mapperBkg[i]->SetScalarRange( overlayRangeBkg );
    mapperBkg[i]->SetLookupTable( lookupTableBkg );
  }
  
  // An interactor
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);

  vtkSmartPointer<CustomInteractorStyle> customStyle = vtkSmartPointer<CustomInteractorStyle>::New();
  renderWindowInteractor->SetInteractorStyle(customStyle);

  customStyle->SetCurrentRenderer(renderer);
  
  // Create the scalarBar.
  if (colorbar && overlay) {
    
    vtkSmartPointer<vtkDoubleArray> scalarLR = vtkSmartPointer<vtkDoubleArray>::New();
    int nValuesLR = polyData[0]->GetNumberOfPoints() + polyData[1]->GetNumberOfPoints();
    scalarLR->SetNumberOfTuples(nValuesLR);

    for (auto i=0; i < polyData[0]->GetNumberOfPoints(); i++)
      scalarLR->SetValue(i,scalars[0]->GetValue(i));
    for (auto i=0; i < polyData[1]->GetNumberOfPoints(); i++)
      scalarLR->SetValue(i+polyData[0]->GetNumberOfPoints(),scalars[1]->GetValue(i));

    vtkSmartPointer<vtkTextProperty> textProperty = vtkSmartPointer<vtkTextProperty>::New();
    if (bkgWhite) textProperty->SetColor(black);
    else textProperty->SetColor(white);

    vtkNew<vtkScalarBarActor> scalarBar;
    scalarBar->SetOrientationToHorizontal();
    scalarBar->SetLookupTable(lookupTable[0]);
    scalarBar->SetWidth(0.3);
    scalarBar->SetHeight(0.05);
    scalarBar->SetPosition(0.35, 0.05);
    
    scalarBar->GetLabelTextProperty()->ShallowCopy(textProperty);
    scalarBar->GetTitleTextProperty()->ShallowCopy(textProperty);
    scalarBar->GetTitleTextProperty()->SetLineOffset(-10); // Apply additional specific settings after copying
    scalarBar->GetTitleTextProperty()->BoldOn(); 

    char info[150];
    snprintf(info, sizeof(info), "Mean=%.3f Median=%.3f SD=%.3f", get_mean(scalarLR), get_median(scalarLR), get_std(scalarLR));
    if (printStats) scalarBar->SetTitle(info);
    else scalarBar->SetTitle(Title);
    
    renderer->AddActor2D(scalarBar);
  }

  // Zoom in to remove large empty areas
  renderer->ResetCamera();
  renderer->GetActiveCamera()->Zoom(2.0); // Adjust the zoom factor as needed

  // Render an image (lights and cameras are created automatically)
  renderWindow->Render();  
  renderWindow->SetWindowName(Title);
  

  // Save png-file if defined
  if (saveImage) {
    
    string imageFilename;
    string ext = vtksys::SystemTools::GetFilenameLastExtension(outputFileName);
    
    if (strncmp(ext.c_str(), ".", 1)) {

      string extension = ".png";
  
      // Adding extension using simple concatenation
      imageFilename = Title + extension;    

    } else imageFilename = outputFileName;
    
    // we have to rescue fullpath, since we changed folder
    cout << "Write: " << imageFilename << endl; 

    // Create a window to image filter
    vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
    windowToImageFilter->SetInput(renderWindow);
    windowToImageFilter->SetScale(1); // Adjust the scale of the output image if needed
    windowToImageFilter->ReadFrontBufferOff(); // Read from the back buffer
    windowToImageFilter->SetInputBufferTypeToRGBA(); // Also capture the alpha (transparency) channel
    windowToImageFilter->Update();
    
    // Create a PNG writer
    vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();
    writer->SetFileName(imageFilename.c_str());
    writer->SetInputConnection(windowToImageFilter->GetOutputPort());
    writer->Write();
  } else renderWindowInteractor->Start(); // Begin interaction otherwise


  return EXIT_SUCCESS;
}

