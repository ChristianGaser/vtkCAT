#include <vtkActor.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkPolyDataReader.h>
#include "vtkDoubleArray.h"
#include <vtkPointData.h> 
#include "vtkSurfaceReader.h"

vtkDoubleArray* readScalars(char* filename);

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
    std::cerr << "Usage: " << argv[0] << " left.vtk right.vtk"
              << std::endl;
    return (EXIT_FAILURE);
  }

  char *scalarFileNameL = NULL;
  char *scalarFileNameR = NULL;
  char *scalarFileNameBkg = NULL;
  char *outputFileName = NULL;
  char *colorbarTitle = NULL;
  double alpha = defaultAlpha;
  double scalarRange[2] = {defaultScalarRange[0], defaultScalarRange[1]};
  double scalarRangeBkg[2] = {defaultScalarRangeBkg[0], defaultScalarRangeBkg[1]};
  double clipRange[2] = {defaultClipRange[0], defaultClipRange[1]};
  int colormap = 1;
  int colorbar = defaultColorbar;
  int inverse = defaultInverse;
  int scalar = 0, scalarBkg = 0, png = 0, logScale = 0, title = 0;
  int WindowSize[2] = {defaultWindowSize[0], defaultWindowSize[1]};
  int indx = -1;

  for (int j = 1; j < argc; j++) {
    if (argv[j][0] != '-') {
      indx = j;
      break;
   }
   else if (strcmp(argv[j], "-range") == 0) {
     j++; scalarRange[0] = atof(argv[j]);
     j++; scalarRange[1] = atof(argv[j]);
   }
   else if (strcmp(argv[j], "-range-bkg") == 0) {
     j++; scalarRangeBkg[0] = atof(argv[j]);
     j++; scalarRangeBkg[1] = atof(argv[j]);
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
     j++; scalarFileNameL = argv[j];
     j++; scalarFileNameR = argv[j];
     scalar = 1;
   }
   else if (strcmp(argv[j], "-title") == 0) {
     j++; colorbarTitle = argv[j];
     title = 1;
   }
   else if (strcmp(argv[j], "-bkg") == 0) {
     j++; scalarFileNameBkg = argv[j];
     scalarBkg = 1;
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

  vtkSmartPointer<vtkPolyData> polyData0;
  vtkSmartPointer<vtkPolyData> polyData1;
  
  try {
    polyData0 = ReadGIFTIFile(argv[indx]);
  } catch (const std::exception& e) {
    std::cerr << "Failed to read GIFTI file: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  try {
    polyData1 = ReadGIFTIFile(argv[indx+1]);
  } catch (const std::exception& e) {
    std::cerr << "Failed to read GIFTI file: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  // Create a mapper
  std::array<vtkNew<vtkPolyDataMapper>, 2> mapper;
  mapper[0]->SetInputData(polyData0);
  mapper[1]->SetInputData(polyData1);

  // Create an actor
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
    actor[i]->SetMapper(mapper[order[i]]);
    actor[i]->GetProperty()->SetColor(0.9, 0.9, 0.9);
    actor[i]->GetProperty()->SetInterpolationToPBR();
  
    // configure the basic properties
    actor[i]->GetProperty()->SetMetallic(0.5);
    actor[i]->GetProperty()->SetRoughness(0.8);
    actor[i]->AddPosition(position.data());
    actor[i]->RotateX(rotatex[i]);
    actor[i]->RotateY(rotatey[i]);
    actor[i]->RotateZ(rotatez[i]);
  }
  
  // A renderer and render window
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);
  renderWindow->SetWindowName("Visualize");
  renderWindow->SetSize(defaultWindowSize[0], defaultWindowSize[1]);

  // read scalars if defined
  if (scalar) {
    cout << "Read scalars: " << scalarFileNameL << endl;
    vtkSmartPointer<vtkDoubleArray> scalarsL = vtkSmartPointer<vtkDoubleArray>::New();
    vtkSmartPointer<vtkDoubleArray> scalarsR = vtkSmartPointer<vtkDoubleArray>::New();

    scalarsL = readScalars(scalarFileNameL);
    scalarsR = readScalars(scalarFileNameR);

    if(inverse) {
      for(int i=0; i < polyData0->GetNumberOfPoints(); i++)
          scalarsL->SetValue(i,-(scalarsL->GetValue(i)));
      for(int i=0; i < polyData1->GetNumberOfPoints(); i++)
          scalarsR->SetValue(i,-(scalarsR->GetValue(i)));
    }
    
    if(scalarsL == NULL) {
      cerr << "Error reading file " << scalarFileNameL << endl;
      return(-1);
    }
    
    polyData0->GetPointData()->SetScalars(scalarsL);
    polyData1->GetPointData()->SetScalars(scalarsR);
    
    // clip values if defined
    if (clipRange[1] > clipRange[0]) {
      for(int i=0; i < polyData0->GetNumberOfPoints(); i++) {
        if((scalarsL->GetValue(i) > clipRange[0]) && (scalarsL->GetValue(i) < clipRange[1]))
          scalarsL->SetValue(i,0);
      }
      for(int i=0; i < polyData1->GetNumberOfPoints(); i++) {
        if((scalarsR->GetValue(i) > clipRange[0]) && (scalarsR->GetValue(i) < clipRange[1]))
          scalarsR->SetValue(i,0);
      }
    }
  }

  if (scalarRange[1] < scalarRange[0]) {
    polyData0->GetScalarRange( scalarRange );
    polyData1->GetScalarRange( scalarRange );
  }
  
  // read scalars if defined
/*  if (scalarBkg) {
    cout << "Read background scalars: " << scalarFileNameBkg << endl; 
    vtkDoubleArray *scalarsBkg = NULL;
    scalarsBkg = readScalars(scalarFileNameBkg);
    polyDataReaderBkg->GetOutput()->GetPointData()->SetScalars(scalarsBkg);
    if (scalarRangeBkg[1] < scalarRangeBkg[0])
      polyDataReaderBkg->GetOutput()->GetScalarRange( scalarRangeBkg );
  }
*/

  // An interactor
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);
  
  // Add the actors to the scene
  for (auto i = 0; i < actor.size(); ++i)
  {
    renderer->AddActor(actor[i]);
  }

  // Render an image (lights and cameras are created automatically)
  renderWindow->Render();

  // Begin mouse interaction
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}

vtkDoubleArray* readScalars(char* filename)
{
  
  FILE* fp = fopen(filename, "r");
  if (fp == NULL) {
    cerr << "Unable to open " << filename << endl;
    return NULL;
  }

  vtkDoubleArray* scalars = vtkDoubleArray::New();
  
  int i, magic, nValues, fNum, valsPerVertex, errno;
  double x;
  const int LINE_SIZE = 10240;
  char line[LINE_SIZE];
  
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
  // BIC scalars as ascii file
  else
  {
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
  }
  
  fclose(fp);
  return scalars;
}


