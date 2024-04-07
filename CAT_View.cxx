#include <vtkActor.h>
#include <vtkNamedColors.h>
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
#include <vtksys/SystemTools.hxx>
#include "vtkSurfaceReader.h"

typedef enum ColorMap {
    JET, GRAY, REDYELLOW, BLUECYAN, YELLOWRED, CYANBLUE, BLUEGREEN, GREENBLUE
} ColorMap;

vtkSmartPointer<vtkDoubleArray> readScalars(char* filename);

static double defaultScalarRange[2] = { 0.0, -1.0 };
static double defaultScalarRangeBkg[2] = { 0.0, -1.0 };
static double defaultAlpha = 1.0;
static double defaultClipRange[2] = { 0.0, -1.0 };
static double defaultRotate[3] = { 270.0, 0.0, -90.0 };
static int defaultColorbar = 0;
static int defaultInverse = 0;
static int defaultWindowSize[2] = { 1600, 1600 };

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
  int colormap = JET;
  int colorbar = defaultColorbar;
  int inverse = defaultInverse;
  int overlay = 0, overlayBkg = 0, png = 0, logScale = 0, title = 0;
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
   else if ((strcmp(argv[j], "-scalar") == 0) || (strcmp(argv[j], "-overlay") == 0)) {
     j++; overlayFileNameL = argv[j];
     j++; overlayFileNameR = argv[j];
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
   else if (strcmp(argv[j], "-log") == 0) 
     logScale = 1;
   else if (strcmp(argv[j], "-colorbar") == 0) 
     colorbar = 1;
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
  vtkSmartPointer<vtkLookupTableWithEnabling> lookupTable = vtkSmartPointer<vtkLookupTableWithEnabling>::New();
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
  std::array<double, numberOfViews> positionx{{0, 2*shift, 0, 2*shift, shift, shift}};
  std::array<double, numberOfViews> positiony{{0, 0, shift, shift, 0.7*shift, 0.7*shift}};
  std::array<double, numberOfViews> rotatex{{270, 270, 270, 270, 0, 0}};
  std::array<double, numberOfViews> rotatey{{0, 0, 0, 0, 0, 0}};
  std::array<double, numberOfViews> rotatez{{90, -90, 90, -90, 0, 0}};
  std::array<int,    numberOfViews> order{{0, 1, 1, 0, 0, 1}};

  for (auto i = 0; i < actor.size(); ++i)
  {
    position[0] = positionx[i];
    position[1] = positiony[i];
    
    actorBkg[i]->SetMapper(mapperBkg[order[i]]);
    actorBkg[i]->GetProperty()->SetColor(0.9, 0.9, 0.9);
    actorBkg[i]->GetProperty()->SetInterpolationToPBR();
  
  
    // configure the basic properties
    actorBkg[i]->GetProperty()->SetMetallic(0.1);
    actorBkg[i]->GetProperty()->SetRoughness(0.5);
    actorBkg[i]->AddPosition(position.data());
    actorBkg[i]->RotateX(rotatex[i]);
    actorBkg[i]->RotateY(rotatey[i]);
    actorBkg[i]->RotateZ(rotatez[i]);

    if (overlay) {
      actor[i]->SetMapper(mapper[order[i]]);
      actor[i]->GetProperty()->SetColor(0.9, 0.9, 0.9);
      actor[i]->GetProperty()->SetInterpolationToPBR();
  
      // configure the basic properties
      actor[i]->GetProperty()->SetMetallic(0.1);
      actor[i]->GetProperty()->SetRoughness(0.5);
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
  renderWindow->SetWindowName("CAT_View");
  renderWindow->SetSize(defaultWindowSize[0], defaultWindowSize[1]);

  // read scalars if defined
  if (overlay) {

    cout << "Read overlays: " << overlayFileNameL << endl;

    vtkSmartPointer<vtkDoubleArray> scalars[2];
    scalars[0] = vtkSmartPointer<vtkDoubleArray>::New();
    scalars[1] = vtkSmartPointer<vtkDoubleArray>::New();

    scalars[0] = readScalars(overlayFileNameL);
    scalars[1] = readScalars(overlayFileNameR);

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
        if((scalars[0]->GetValue(i) > clipRange[0]) && (scalars[0]->GetValue(i) < clipRange[1]))
          scalars[0]->SetValue(i,0.0);
      }
      for(int i=0; i < polyData[1]->GetNumberOfPoints(); i++) {
        if((scalars[1]->GetValue(i) > clipRange[0]) && (scalars[1]->GetValue(i) < clipRange[1]))
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
  switch(colormap) {
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
  
  lookupTableBkg->SetHueRange( 0.0, 0.0 );
  lookupTableBkg->SetSaturationRange( 0.0, 0.0 );
  lookupTableBkg->SetValueRange( 0.0, 1.0 );

  // set opacity  
  lookupTable->SetAlphaRange( alpha, alpha );

  if(logScale) lookupTable->SetScaleToLog10();
  lookupTable->SetTableRange( overlayRange );
  if (clipRange[1] >= clipRange[0]) {
    lookupTable->SetEnabledArray(polyData[0]->GetPointData()->GetScalars());
    lookupTable->SetEnabledArray(polyData[1]->GetPointData()->GetScalars());
  }
  lookupTable->Build();

  lookupTableBkg->SetTableRange( -0.5, 0.5 );
  lookupTableBkg->Build();
  
  if (overlay) {
    mapper[0]->SetScalarRange( overlayRange );
    mapper[1]->SetScalarRange( overlayRange );
    mapper[0]->SetLookupTable( lookupTable );
    mapper[1]->SetLookupTable( lookupTable );
  }
  
  mapperBkg[0]->SetScalarRange( -0.5, 0.5 );
  mapperBkg[1]->SetScalarRange( -0.5, 0.5 );
  mapperBkg[0]->SetLookupTable( lookupTableBkg );
  mapperBkg[1]->SetLookupTable( lookupTableBkg );

  // An interactor
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);
  
  // Add the actors to the scene
  for (auto i = 0; i < actor.size(); ++i)
  {
    renderer->AddActor(actorBkg[i]);
    if (overlay) renderer->AddActor(actor[i]);
  }

  // Render an image (lights and cameras are created automatically)
  renderWindow->Render();

  // Create the scalarBar.
  if (colorbar == 1) {
    vtkNew<vtkScalarBarActor> scalarBar;
    scalarBar->SetOrientationToHorizontal();
    scalarBar->SetLookupTable(lookupTable);
    scalarBar->SetWidth(0.3);
    scalarBar->SetHeight(0.05);
    scalarBar->SetPosition(0.3, 0.05);
    if (overlay) {
      if (title == 0)
        scalarBar->SetTitle("test");
      else
        scalarBar->SetTitle(colorbarTitle);
    }
    renderer->AddActor2D(scalarBar);
  }


  // Begin mouse interaction
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}

vtkSmartPointer<vtkDoubleArray> readScalars(char* filename)
{
  
  FILE* fp = fopen(filename, "r");
  if (fp == NULL) {
    cerr << "Unable to open " << filename << endl;
    return NULL;
  }

  vtkSmartPointer<vtkDoubleArray> scalars;
  
  int i, magic, nValues, fNum, valsPerVertex, errno;
  double x;
  
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
  
      for (i = 0; i < nValues; i++) {
        x = FreadFloat(fp);
        scalars->InsertNextValue(x);
      }
    }
  }
  
  fclose(fp);
  return scalars;
}


