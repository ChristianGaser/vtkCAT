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

typedef enum ColorMap {
    JET, GRAY, REDYELLOW, BLUECYAN, YELLOWRED, CYANBLUE, BLUEGREEN, GREENBLUE
} ColorMap;

vtkSmartPointer<vtkDoubleArray> readScalars(char* filename);

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
        if (key == "a")
        {
            // Custom action for 'a' key
            std::cout << "The 'a' key was pressed." << std::endl;
        }
        else
        {
            // Call the parent class's OnKeyPress method to handle other keys
            vtkInteractorStyleTrackballCamera::OnKeyPress();
        }
    }
};
vtkStandardNewMacro(CustomInteractorStyle);

int main(int argc, char* argv[])
{
  if (argc < 3)
  {
    std::cerr << "Usage: " << argv[0] << " left right"
              << std::endl;
    return (EXIT_FAILURE);
  }

  char *overlayFileNameL = NULL;
  char *overlayFileNameR = NULL;
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
  int indx = -1;

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
   else if (strcmp(argv[j], "-scalar") == 0) {
     j++; overlayFileNameL = argv[j];
     j++; overlayFileNameR = argv[j];
     overlay = 2;
   }
   else if (strcmp(argv[j], "-overlay") == 0) {
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
    std::cerr << "Usage: " << argv[0] << " left.vtk right.vtk"
              << std::endl;
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
    
  try {
    polyData[0] = ReadGIFTIMesh(argv[indx]);
  } catch (const std::exception& e) {
    std::cerr << "Failed to read GIFTI file: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  try {
    polyData[1] = ReadGIFTIMesh(argv[indx+1]);
  } catch (const std::exception& e) {
    std::cerr << "Failed to read GIFTI file: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  curvature[0]->SetInputData(polyData[0]);
  curvature[0]->SetCurvatureTypeToMean();
  curvature[0]->Update();
  curvature[1]->SetInputData(polyData[1]);
  curvature[1]->SetCurvatureTypeToMean();
  curvature[1]->Update();

  std::array<vtkSmartPointer<vtkPolyData>, 2> curvaturesMesh;
  curvaturesMesh[0] = curvature[0]->GetOutput();  
  curvaturesMesh[1] = curvature[1]->GetOutput();  

  // Create mapper
  vtkSmartPointer<vtkPolyDataMapper> mapper[2];
  mapper[0] = vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper[1] = vtkSmartPointer<vtkPolyDataMapper>::New();

  if (overlay) {
    mapper[0]->SetInputData(polyData[0]);
    mapper[1]->SetInputData(polyData[1]);
  }
  
  vtkSmartPointer<vtkPolyDataMapper> mapperBkg[2];
  mapperBkg[0] = vtkSmartPointer<vtkPolyDataMapper>::New();
  mapperBkg[1] = vtkSmartPointer<vtkPolyDataMapper>::New();

  mapperBkg[0]->SetInputData(curvaturesMesh[0]);
  mapperBkg[1]->SetInputData(curvaturesMesh[1]);

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
    scalars[0] = vtkSmartPointer<vtkDoubleArray>::New();
    scalars[1] = vtkSmartPointer<vtkDoubleArray>::New();

    scalars[0] = readScalars(overlayFileNameL);
    
    // if defined read right hemispheric overlay
    if (overlay == 2)
      scalars[1] = readScalars(overlayFileNameR);
    else {
      // or split the single overlay (which is merged) into left and right
      // overlay
      scalars[1]->SetNumberOfTuples(polyData[0]->GetNumberOfPoints());
      for(int i=0; i < polyData[0]->GetNumberOfPoints(); i++)
          scalars[1]->SetValue(i,scalars[0]->GetValue(i+polyData[1]->GetNumberOfPoints()));
    }

    if(inverse) {
      for(int i=0; i < polyData[0]->GetNumberOfPoints(); i++)
        scalars[0]->SetValue(i,-(scalars[0]->GetValue(i)));
      for(int i=0; i < polyData[1]->GetNumberOfPoints(); i++)
        scalars[1]->SetValue(i,-(scalars[1]->GetValue(i)));
    }
    
    if(scalars[0] == NULL) {
      cerr << "Error reading file " << overlayFileNameL << endl;
      return(-1);
    }

    // clip values if defined
    if (clipRange[1] > clipRange[0]) {
      for(int i=0; i < polyData[0]->GetNumberOfPoints(); i++) {
        val = scalars[0]->GetValue(i);
        if (((val > clipRange[0]) && (val < clipRange[1])) || std::isnan(val))
          scalars[0]->SetValue(i,0.0);
      }
      for(int i=0; i < polyData[1]->GetNumberOfPoints(); i++) {
        val = scalars[1]->GetValue(i);
        if (((val > clipRange[0]) && (val < clipRange[1])) || std::isnan(val))
          scalars[1]->SetValue(i,0.0);
      }
    }
    
    polyData[0]->GetPointData()->SetScalars(scalars[0]);
    polyData[1]->GetPointData()->SetScalars(scalars[1]);
    
  }

  if (overlayRange[1] < overlayRange[0]) {
    polyData[0]->GetScalarRange( overlayRange );
    polyData[1]->GetScalarRange( overlayRange );
  }
  
  // build colormap
  for(int i=0; i < 2; i++) {
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
  
  if (overlay) {
    mapper[0]->SetScalarRange( overlayRange );
    mapper[1]->SetScalarRange( overlayRange );
    mapper[0]->SetLookupTable( lookupTable[0] );
    mapper[1]->SetLookupTable( lookupTable[1] );
  }
  
  mapperBkg[0]->SetScalarRange( -0.5, 0.5 );
  mapperBkg[1]->SetScalarRange( -0.5, 0.5 );
  mapperBkg[0]->SetLookupTable( lookupTableBkg );
  mapperBkg[1]->SetLookupTable( lookupTableBkg );

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

  // Render an image (lights and cameras are created automatically)
  renderWindow->Render();

  renderWindow->SetWindowName("CAT_View");

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

    if (title == 0) scalarBar->SetTitle(overlayFileNameL);
    else scalarBar->SetTitle(colorbarTitle);
    scalarBar->GetTitleTextProperty()->ShallowCopy(textProperty);
    
    renderer->AddActor2D(scalarBar);
  }


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

vtkSmartPointer<vtkDoubleArray> readScalars(char* filename)
{
  
  FILE* fp = fopen(filename, "r");
  if (fp == NULL) {
    cerr << "Unable to open " << filename << endl;
    return NULL;
  }

  vtkSmartPointer<vtkDoubleArray> scalars = vtkSmartPointer<vtkDoubleArray>::New();
  
  int i, magic, nValues, fNum, valsPerVertex, errno;
  
  std::string extension =
      vtksys::SystemTools::GetFilenameLastExtension(std::string(filename));

  if (extension == ".gii") {
    scalars = ReadGIFTICurv(filename);
  
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

