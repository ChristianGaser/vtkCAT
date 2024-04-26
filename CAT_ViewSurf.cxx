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
#include "vtkUtils.h"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>

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
  int logColorbar = 0;
  int fontSize = 0;
  int inverse = defaultInverse;
  int discrete = defaultDiscrete;
  int bkgWhite = defaultBkg;
  int printStats = 0;
  int overlay = 0, overlayBkg = 0, saveImage = 0, title = 0;
  int WindowSize[2] = {defaultWindowSize[0], defaultWindowSize[1]};
  int indx = -1, nMeshes;
  bool rhExists;

  for (auto j = 1; j < argc; j++) {
    if (argv[j][0] != '-') {
      indx = j;
      break;
    }
    else if (!strcmp(argv[j], "-range") || !strcmp(argv[j], "-r")) {
      j++; overlayRange[0] = atof(argv[j]);
      j++; overlayRange[1] = atof(argv[j]);
    }
    else if (!strcmp(argv[j], "-range-bkg") || !strcmp(argv[j], "-rb")) {
      j++; overlayRangeBkg[0] = atof(argv[j]);
      j++; overlayRangeBkg[1] = atof(argv[j]);
    }
    else if (!strcmp(argv[j], "-clip") || !strcmp(argv[j], "-cl")) {
      j++; clipRange[0] = atof(argv[j]);
      j++; clipRange[1] = atof(argv[j]);
    }
    else if (!strcmp(argv[j], "-size") || !strcmp(argv[j], "-sz")) {
      j++; WindowSize[0] = atoi(argv[j]);
      j++; WindowSize[1] = atoi(argv[j]);
    }
    else if (!strcmp(argv[j], "-scalar") || !strcmp(argv[j], "-overlay") || !strcmp(argv[j], "-ov")) {
      j++; overlayFileNameL = argv[j];
      overlay = 1;
    }
    else if (!strcmp(argv[j], "-title")) {
      j++; Title = argv[j];
      title = 1;
    }
    else if (!strcmp(argv[j], "-bkg")) {
      j++; overlayFileNameBkgL = argv[j];
      overlayBkg = 1;
    }
    else if (!strcmp(argv[j], "-output") || !strcmp(argv[j], "-save")) {
      j++; outputFileName = argv[j];
      saveImage = 1;
    }
    else if (!strcmp(argv[j], "-fontsize") || !strcmp(argv[j], "-fs")) {
      j++; fontSize = atoi(argv[j]);
    }
    else if (!strcmp(argv[j], "-opacity") || !strcmp(argv[j], "-op")) {
      j++; alpha = atof(argv[j]);
    }
    else if (!strcmp(argv[j], "-stats"))
      printStats = 1;
    else if (!strcmp(argv[j], "-inverse"))
      inverse = 1;
    else if (!strcmp(argv[j], "-colorbar") || !strcmp(argv[j], "-cb")) 
      colorbar = 1;
    else if (!strcmp(argv[j], "-discrete") || !strcmp(argv[j], "-dsc")) {
      j++; discrete = atoi(argv[j]);
    }
    else if (!strcmp(argv[j], "-log"))
      logColorbar = 1;
    else if (!strcmp(argv[j], "-white")) 
     bkgWhite = 1;
    else if (!strcmp(argv[j], "-fire")) 
     colormap = FIRE;
    else if (!strcmp(argv[j], "-bipolar")) 
     colormap = BIPOLAR;
    else if (!strcmp(argv[j], "-c1")) 
     colormap = C1;
    else if (!strcmp(argv[j], "-c2")) 
     colormap = C2;
    else if (!strcmp(argv[j], "-c3")) 
     colormap = C3;
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

  if ((discrete < 0) || (discrete > 4)) {
    cerr << "ERROR: Parameter 'discrete' should be 0..4." << endl;
    return EXIT_FAILURE;
  } else if (discrete)
    discrete = pow(2, discrete+2);

  const int numArgs = argc - indx;

  vtkSmartPointer<vtkPolyData> polyData[2];
  vtkSmartPointer<vtkLookupTableWithEnabling> lookupTable[2];
  lookupTable[0] = vtkSmartPointer<vtkLookupTableWithEnabling>::New();
  lookupTable[1] = vtkSmartPointer<vtkLookupTableWithEnabling>::New();
  vtkSmartPointer<vtkLookupTableWithEnabling> lookupTableBkg = vtkSmartPointer<vtkLookupTableWithEnabling>::New();
  vtkSmartPointer<vtkLookupTableWithEnabling> lookupTableColorBar = vtkSmartPointer<vtkLookupTableWithEnabling>::New();

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
  
  // Check that file exists
  if (!vtksys::SystemTools::FileExists(argv[indx])) {
    cerr << "ERROR: File " << argv[indx] << " not found." << endl;
    return EXIT_FAILURE;
  }

  // Check whether filename contains "lh." or "left" and replace filename with
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

  // Read background scalars if defined
  if (overlayBkg) {
    // Check that file exists
    if (!vtksys::SystemTools::FileExists(overlayFileNameBkgL)) {
      cerr << "ERROR: File " << overlayFileNameBkgL << " not found." << endl;
      return EXIT_FAILURE;
    }
    if (ReadBackgroundScalars(overlayFileNameBkgL, polyData, scalarsBkg, nMeshes, rhExists, rhSurfName)) {
      cerr << "ERROR: File " << overlayFileNameBkgL << " could not be read." << endl;
      return EXIT_FAILURE;
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

    // Automatically estimate background range
    if (overlayRangeBkg[1] < overlayRangeBkg[0])
      curvaturesMesh[i]->GetScalarRange( overlayRangeBkg );

    // Use symmetric background range
    if (overlayRangeBkg[0] < 0 && overlayRangeBkg[1] > 0) {
      if (-overlayRangeBkg[0] > overlayRangeBkg[1])
        overlayRangeBkg[0] = -overlayRangeBkg[1];
      else
        overlayRangeBkg[1] = -overlayRangeBkg[0];
    }
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
    // If only left hemisphere exists, then skip display of right hemipshere
    if ((nMeshes == 1) && (i % 2)) continue;
    
    position[0] = positionx[i];
    position[1] = positiony[i];
    
    actorBkg[i]->SetMapper(mapperBkg[order[i]]);  
  
    // Configure the basic properties
    actorBkg[i]->GetProperty()->SetAmbient(0.8);
    actorBkg[i]->GetProperty()->SetDiffuse(0.7);
    actorBkg[i]->GetProperty()->SetSpecular(0.0);
    actorBkg[i]->AddPosition(position.data());
    actorBkg[i]->RotateX(rotatex[i]);
    actorBkg[i]->RotateY(rotatey[i]);
    actorBkg[i]->RotateZ(rotatez[i]);

    if (overlay) {
      actor[i]->SetMapper(mapper[order[i]]);
  
      // Configure the basic properties
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

  //renderer->SetBackgroundAlpha(0.0); // Set transparency

  // Add the actors to the scene
  for (auto i = 0; i < actor.size(); ++i)
  {
    renderer->AddActor(actorBkg[i]);
    if (overlay) renderer->AddActor(actor[i]);
  }

  // Read scalars if defined
  if (overlay) {
    if (!vtksys::SystemTools::FileExists(overlayFileNameL)) {
      cerr << "ERROR: File " << overlayFileNameL << " not found." << endl;
      return EXIT_FAILURE;
    }
    if (ReadAndUpdateScalars(overlayFileNameL, polyData, scalars, nMeshes, rhExists, rhSurfName, inverse, clipRange, currentPath, overlayRange)) {
      cerr << "ERROR: File " << overlayFileNameL << " could not be read." << endl;
      return EXIT_FAILURE;
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
  if (!title) Title = strTmp.c_str();

  // Build colormap
  for (auto i = 0; i < nMeshes; i++) {

    // Get LUT for colormap
    lookupTable[i] = getLookupTable(colormap,alpha,overlayRange,clipRange);
  
    if (overlayRange[1] > overlayRange[0])
      lookupTable[i]->SetTableRange( overlayRange );
    if (clipRange[1] > clipRange[0])
      lookupTable[i]->SetEnabledArray(polyData[i]->GetPointData()->GetScalars());

    // Set opacity  
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
  
  // Create interactor
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);

  vtkSmartPointer<CustomInteractorStyle> customStyle = vtkSmartPointer<CustomInteractorStyle>::New();
  renderWindowInteractor->SetInteractorStyle(customStyle);

  customStyle->SetCurrentRenderer(renderer);
  
  // Create the scalarBar
  if (overlay) {
    int n1 = polyData[0]->GetNumberOfPoints();
    int n2 = polyData[1]->GetNumberOfPoints();
    UpdateScalarBarAndLookupTable(n1, n2, scalars, lookupTable, lookupTableColorBar, overlayRange, clipRange, colorbar, discrete, bkgWhite, fontSize, logColorbar, printStats, renderer, Title, alpha);
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
    
    // We have to rescue fullpath, since we changed folder
    cout << "Write: " << imageFilename << endl; 

    // Create a window to image filter
    vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
    windowToImageFilter->SetInput(renderWindow);
    windowToImageFilter->SetScale(1); // Adjust the scale of the output image if needed
    windowToImageFilter->ReadFrontBufferOff(); // Read from the back buffer
    windowToImageFilter->Update();
    
    // Create a PNG writer
    vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();
    writer->SetFileName(imageFilename.c_str());
    writer->SetInputConnection(windowToImageFilter->GetOutputPort());
    writer->Write();
  } else renderWindowInteractor->Start(); // Begin interaction otherwise


  return EXIT_SUCCESS;
}

