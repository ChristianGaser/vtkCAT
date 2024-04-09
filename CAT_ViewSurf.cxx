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
#include <vtksys/SystemTools.hxx>
#include "vtkSurfaceReader.h"
#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::__fs::filesystem;

typedef enum ColorMap {
    JET, GRAY, REDYELLOW, BLUECYAN, YELLOWRED, CYANBLUE, BLUEGREEN, GREENBLUE
} ColorMap;

static void usage(const char* const prog);
vtkSmartPointer<vtkDoubleArray> readScalars(const char* filename);

static double defaultScalarRange[2] = { 0.0, -1.0 };
static double defaultScalarRangeBkg[2] = { 0.0, -1.0 };
static double defaultAlpha = 0.8;
static double defaultClipRange[2] = { 0.0, -1.0 };
static double defaultRotate[3] = { 270.0, 0.0, -90.0 };
static int defaultColorbar = 0;
static int defaultInverse = 0;
static int defaultBkg = 0;
static int defaultWindowSize[2] = { 1600, 1600 };

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
  virtual void OnKeyPress() override
  {
    vtkRenderWindowInteractor* rwi = this->Interactor;
    std::string key = rwi->GetKeySym();
    
    // Handle custom keys; for example, if the 'a' key is pressed
    if (key == "h")
    {
      // Custom action for 'a' key
      std::cout << "The 'h' key was pressed." << std::endl;
    }
    else
    {
      // Call the parent class's OnKeyPress method to handle other keys
      vtkInteractorStyleTrackballCamera::OnKeyPress();
    }
  }
};
vtkStandardNewMacro(CustomInteractorStyle);

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
  << "  -scalar scalarInputLeft  " << endl
  << "     File with scalar values (gifit, ascii or Freesurfer format), either for the left or merged hemispheres." << endl
  << "  -overlay scalarInput  " << endl
  << "     File with scalar values (gifit, ascii or Freesurfer format), either for the left or merged hemispheres." << endl
  << "  -range lower upper  " << endl
  << "     Range of scalar values." << endl
  << "     Default value: " << defaultScalarRange[0] << " " << defaultScalarRange[1] << endl
  << "  -clip lower upper  " << endl
  << "     Clip scalar values. These values will be not displayed." << endl
  << "     Default value: " << defaultClipRange[0] << " " << defaultClipRange[1] << endl
  << "  -colorbar  " << endl
  << "     Show colorbar (default no)." << endl
  << "  -title  " << endl
  << "     Set name for colorbar (default scalar-file)." << endl
  << "  -inverse  " << endl
  << "     Invert input values." << endl
  << " Colors:" << endl
  << "  -opacity value  " << endl
  << "     Value for opacity of overlay." << endl
  << "     Default value: " << defaultAlpha << endl
  << "  -white  " << endl
  << "     Use white background" << endl
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
  << " Output:" << endl
  << "  -size xsize ysize  " << endl
  << "     Window size." << endl
  << "     Default value: " << defaultWindowSize[0] << " " << defaultWindowSize[1] << endl
  << "  -output output.png  " << endl
  << "     Save as png-file." << endl
  << endl
  << "KEYBOARD INTERACTIONS" << endl
  << "  w " << endl
  << "     Show wireframe." << endl
  << "  s " << endl
  << "     Show shaded." << endl
  << "  q e" << endl
  << "     Quit." << endl
  << endl
  << "REQUIRED PARAMETERS" << endl
  << "    <input.vtk> " << endl
  << "" << endl
  << "EXAMPLE" << endl
  << "    " << prog << " -range -1 1 -scalar scalarInput.txt input.vtk" << endl
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

  char *overlayFileNameL = NULL;
  char *overlayFileNameBkg = NULL;
  char *outputFileName = NULL;
  char *colorbarTitle = NULL;
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
  int bkg = defaultBkg;
  int overlay = 0, overlayBkg = 0, png = 0, title = 0;
  int WindowSize[2] = {defaultWindowSize[0], defaultWindowSize[1]};
  int indx = -1, nMeshes;
  bool rhExists, bothHemis;

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
     j++; colorbarTitle = argv[j];
     title = 1;
   }
   else if (strcmp(argv[j], "-bkg") == 0) {
     j++; overlayFileNameBkg = argv[j];
     overlayBkg = 1;
   }
   else if (strcmp(argv[j], "-output") == 0) {
     j++; outputFileName = argv[j];
     png = 1;
   }
   else if (strcmp(argv[j], "-opacity") == 0) {
     j++; alpha = atof(argv[j]);
   }
   else if (strcmp(argv[j], "-inverse") == 0)
     inverse = 1;
   else if (strcmp(argv[j], "-colorbar") == 0) 
     colorbar = 1;
   else if (strcmp(argv[j], "-white") == 0) 
    bkg = 1;
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

  vtkSmartPointer<vtkCurvatures> curvature[2];
  curvature[0] = vtkSmartPointer<vtkCurvatures>::New();
  curvature[1] = vtkSmartPointer<vtkCurvatures>::New();

  // check whether filename contains "lh." or "left" and replace filename with
  // name for the right hemisphere and check whether the file exists  
  std::string rhSurfName = argv[indx];
  if (rhSurfName.find("lh.") != std::string::npos) {
    vtksys::SystemTools::ReplaceString(rhSurfName,std::string("lh."),std::string("rh."));
    rhExists = vtksys::SystemTools::FileExists(rhSurfName);
  } else if (rhSurfName.find("left") != std::string::npos) {
    std::string rhSurfName = argv[indx];
    vtksys::SystemTools::ReplaceString(rhSurfName,std::string("left"),std::string("right"));
    rhExists = vtksys::SystemTools::FileExists(rhSurfName);
  } else rhExists = false;
  
  try {
    polyData[0] = ReadGIFTIMesh(argv[indx]);
  } catch (const std::exception& e) {
    std::cerr << "Failed to read GIFTI file: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  
  if (rhExists) {
    try {
      polyData[1] = ReadGIFTIMesh(rhSurfName.c_str());
    } catch (const std::exception& e) {
      std::cerr << "Failed to read GIFTI file: " << e.what() << std::endl;
      return EXIT_FAILURE;
    }
    nMeshes = 2;
  } else nMeshes = 1;
  
  
  std::array<vtkSmartPointer<vtkPolyData>, 2> curvaturesMesh;
  vtkSmartPointer<vtkPolyDataMapper> mapper[2];
  vtkSmartPointer<vtkPolyDataMapper> mapperBkg[2];

  for (auto i = 0; i < nMeshes; i++) {
    curvature[i]->SetInputData(polyData[i]);
    curvature[i]->SetCurvatureTypeToMean();
    curvature[i]->Update();
  
    curvaturesMesh[i] = curvature[i]->GetOutput();  

    // Create mapper
    mapper[i] = vtkSmartPointer<vtkPolyDataMapper>::New();

    if (overlay)
      mapper[i]->SetInputData(polyData[i]);

    mapperBkg[i] = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapperBkg[i]->SetInputData(curvaturesMesh[i]);
  }

  // Create actor
  auto const numberOfViews = 6;
  std::array<vtkNew<vtkActor>, numberOfViews> actor;
  std::array<vtkNew<vtkActor>, numberOfViews> actorBkg;
  std::array<double, 3> position{{0, 0, 0}};
  
  double shift = 180;
  std::array<double, numberOfViews> positionx{{0, 2*shift, 0.15*shift, 1.85*shift, shift, shift}};
  std::array<double, numberOfViews> positiony{{0, 0, shift, shift, 0.7*shift, 0.7*shift}};
  std::array<double, numberOfViews> rotatex{{270, 270, 270, 270, 0, 0}};
  std::array<double, numberOfViews> rotatey{{0, 0, 0, 0, 0, 0}};
  std::array<double, numberOfViews> rotatez{{90, -90, -90, 90, 0, 0}};
  std::array<int,    numberOfViews> order{{0, 1, 0, 1, 0, 1}};

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
  renderWindow->SetSize(defaultWindowSize[0], defaultWindowSize[1]);

  if (bkg) renderer->SetBackground(white);
  else renderer->SetBackground(black);

  // read scalars if defined
  if (overlay) {

    cout << "Read overlays: " << overlayFileNameL << endl;
    
    vtkSmartPointer<vtkDoubleArray> scalars[2];
    for (auto i = 0; i < nMeshes; i++)
      scalars[i] = vtkSmartPointer<vtkDoubleArray>::New();

    scalars[0] = readScalars(overlayFileNameL);
    
    // if defined, read right hemispheric overlay
    if ((nMeshes > 1) && rhExists) {

      // try replacing lh/left by rh/right in the overlay filename
      std::string overlayFileNameR = overlayFileNameL;
      if (overlayFileNameR.find("lh.") != std::string::npos) {
        vtksys::SystemTools::ReplaceString(overlayFileNameR,std::string("lh."),std::string("rh."));
        rhExists = vtksys::SystemTools::FileExists(overlayFileNameR);
      } else if (rhSurfName.find("left") != std::string::npos) {
        std::string overlayFileNameR = overlayFileNameL;
        vtksys::SystemTools::ReplaceString(overlayFileNameR,std::string("left"),std::string("right"));
        rhExists = vtksys::SystemTools::FileExists(overlayFileNameR);
      } else rhExists = false;

      if (rhExists)
        scalars[1] = readScalars(overlayFileNameR.c_str());
      else {
        // or split the single overlay (which is merged) into left and right
        // overlay
        try {
          scalars[1]->SetNumberOfTuples(polyData[0]->GetNumberOfPoints());
          for(int i=0; i < polyData[0]->GetNumberOfPoints(); i++)
            scalars[1]->SetValue(i,scalars[0]->GetValue(i+polyData[1]->GetNumberOfPoints()));
        } catch (const std::exception& e) {
          cerr << "Error splitting values for left/right hemisphere in file " << overlayFileNameL << endl;
          return(-1);
        }
      }
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
          if (((val > clipRange[0]) && (val < clipRange[1])) || std::isnan(val))
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
  
  // build colormap
  for (auto i = 0; i < nMeshes; i++) {
    switch(colormap) {
    case JET:
      lookupTable[i]->SetHueRange( 0.667, 0.0 );
      lookupTable[i]->SetSaturationRange( 1.0, 1.0 );
      lookupTable[i]->SetValueRange( 1.0, 1.0 );
      break;
    case GRAY:
      lookupTable[i]->SetHueRange( 0.0, 0.0 );
      lookupTable[i]->SetSaturationRange( 0.0, 0.0 );
      lookupTable[i]->SetValueRange( 0.0, 1.0 );
      break;
    case REDYELLOW:
      lookupTable[i]->SetHueRange( 0.0, 0.1667 );
      lookupTable[i]->SetSaturationRange( 1.0, 1.0 );
      lookupTable[i]->SetValueRange( 1.0, 1.0 );
      break;
    case BLUECYAN:
      lookupTable[i]->SetHueRange( 0.66667, 0.5);
      lookupTable[i]->SetSaturationRange( 1.0, 1.0 );
      lookupTable[i]->SetValueRange( 1.0, 1.0 );
      break;
    case YELLOWRED:
      lookupTable[i]->SetHueRange( 0.1667, 0.0 );
      lookupTable[i]->SetSaturationRange( 1.0, 1.0 );
      lookupTable[i]->SetValueRange( 1.0, 1.0 );
      break;
    case CYANBLUE:
      lookupTable[i]->SetHueRange( 0.5, 0.66667);
      lookupTable[i]->SetSaturationRange( 1.0, 1.0 );
      lookupTable[i]->SetValueRange( 1.0, 1.0 );
      break;
    case BLUEGREEN:
      lookupTable[i]->SetHueRange( 0.66667, 0.33333);
      lookupTable[i]->SetSaturationRange( 1.0, 1.0 );
      lookupTable[i]->SetValueRange( 1.0, 1.0 );
      break;
    case GREENBLUE:
      lookupTable[i]->SetHueRange( 0.33333, 0.66667);
      lookupTable[i]->SetSaturationRange( 1.0, 1.0 );
      lookupTable[i]->SetValueRange( 1.0, 1.0 );
      break;
    }

    // set opacity  
    lookupTable[i]->SetAlphaRange( alpha, alpha );
  
    lookupTable[i]->SetTableRange( overlayRange );
    if (clipRange[1] > clipRange[0]) {
      lookupTable[i]->SetEnabledArray(polyData[i]->GetPointData()->GetScalars());
    }
    lookupTable[i]->Build();
  }
  
  lookupTableBkg->SetHueRange( 0.0, 0.0 );
  lookupTableBkg->SetSaturationRange( 0.0, 0.0 );
  lookupTableBkg->SetValueRange( 0.0, 1.0 );

  lookupTableBkg->SetTableRange( -0.5, 0.5 );
  lookupTableBkg->Build();
  
  for (auto i = 0; i < nMeshes; i++) {
    if (overlay) {
      mapper[i]->SetScalarRange( overlayRange );
      mapper[i]->SetLookupTable( lookupTable[i] );
    }
    
    mapperBkg[i]->SetScalarRange( -0.5, 0.5 );
    mapperBkg[i]->SetLookupTable( lookupTableBkg );
  }
  
  // An interactor
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);

  vtkSmartPointer<CustomInteractorStyle> customStyle = vtkSmartPointer<CustomInteractorStyle>::New();
  renderWindowInteractor->SetInteractorStyle(customStyle);
  
  // Add the actors to the scene
  for (auto i = 0; i < actor.size(); ++i)
  {
    renderer->AddActor(actorBkg[i]);
    if (overlay) renderer->AddActor(actor[i]);
  }

  // Create the scalarBar.
  if (colorbar == 1) {
    vtkSmartPointer<vtkTextProperty> textProperty = vtkSmartPointer<vtkTextProperty>::New();
    vtkNew<vtkScalarBarActor> scalarBar;
    scalarBar->SetOrientationToHorizontal();
    scalarBar->SetLookupTable(lookupTable[0]);
    scalarBar->SetWidth(0.3);
    scalarBar->SetHeight(0.05);
    scalarBar->SetPosition(0.35, 0.05);
    
    if (bkg) textProperty->SetColor(black);
    else textProperty->SetColor(white);
    scalarBar->GetLabelTextProperty()->ShallowCopy(textProperty);

    fs::path filePath(overlayFileNameL);

    // Obtain the folder name (directory path)
    fs::path folderName = filePath.parent_path();

    // Obtain the basename (filename with extension)
    fs::path baseName = filePath.filename();
    
    if (title == 0) scalarBar->SetTitle(baseName.c_str());
    else scalarBar->SetTitle(colorbarTitle);
    scalarBar->GetTitleTextProperty()->ShallowCopy(textProperty);
    
    renderer->AddActor2D(scalarBar);
  }

  // Render an image (lights and cameras are created automatically)
  renderWindow->Render();

  renderWindow->SetWindowName("CAT_View");

  // save png-file if defined
  if (png) {

    cout << "Write: " << outputFileName << endl; 

    // Create a window to image filter
    vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
    windowToImageFilter->SetInput(renderWindow);
    windowToImageFilter->SetScale(1); // Adjust the scale of the output image if needed
    windowToImageFilter->ReadFrontBufferOff(); // Read from the back buffer
    windowToImageFilter->Update();
    
    // Create a PNG writer
    vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();
    writer->SetFileName(outputFileName);
    writer->SetInputConnection(windowToImageFilter->GetOutputPort());
    writer->Write();
  } else renderWindowInteractor->Start(); // Begin mouse interaction otherwise


  return EXIT_SUCCESS;
}

vtkSmartPointer<vtkDoubleArray> readScalars(const char* filename)
{
  
  FILE* fp = fopen(filename, "r");
  if (fp == NULL) {
    cerr << "Unable to open " << filename << endl;
    return NULL;
  }

  vtkSmartPointer<vtkDoubleArray> scalars = vtkSmartPointer<vtkDoubleArray>::New();
  
  int i, magic, nValues, fNum, valsPerVertex, errno;
  double x;
  const int LINE_SIZE = 10240;
  char line[LINE_SIZE];
  
  std::string extension =
      vtksys::SystemTools::GetFilenameLastExtension(std::string(filename));

  if (extension == ".gii") {
    scalars = ReadGIFTICurv(filename);
  } else if (extension == ".txt") {

    rewind(fp);
    while (fgets(line, sizeof(line), fp) != NULL) {
      errno = 0;
      x = strtod(line, NULL);
      if (errno != 0) {
        cerr << "Error reading value from line " << line << " from file " << filename << endl;
        fclose(fp);
        return NULL;
      }
      scalars->InsertNextValue(x);
    }
  
  } else {
  
    magic = Fread3(fp);
    
    // freesurfer scalars
    if (magic==16777215)
    {
    
      nValues = FreadInt(fp);
      fNum = FreadInt(fp);
      valsPerVertex = FreadInt(fp);
      scalars->SetNumberOfTuples(nValues);
  
      for (i = 0; i < nValues; i++) 
        scalars->SetValue(i, FreadFloat(fp));
    } else {
      cerr << "Format in %s not supported." << filename << endl;
      return NULL;
    }
  }
  
  fclose(fp);
  return scalars;
}

