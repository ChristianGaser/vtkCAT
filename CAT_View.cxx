#include <string>

#include "vtkFileOutputWindow.h"
#include "vtkPolyData.h"
#include "vtkPointData.h"
#include "vtkLookupTable.h"
#include "vtkDoubleArray.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkPolyDataReader.h"
#include "vtkCommand.h"
#include "vtkScalarBarActor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkPNGWriter.h"
#include "vtkWindowToImageFilter.h"
#include <vtksys/SystemTools.hxx>
#include "vtkErrorCode.h"

#include "vtkInteractorStyleCAT.h"
#include "vtkSurfaceReader.h"
#include "vtkScalarBarWidgetCAT.h"
#include "vtkLookupTableWithEnabling.h"
#include <float.h>

typedef  enum  { JET, GRAY, REDYELLOW, BLUECYAN, YELLOWRED, CYANBLUE, BLUEGREEN, GREENBLUE };

static void usage(const char* const prog);
vtkDoubleArray* readScalars(char* filename);

static double defaultScalarRange[2] = { 0.0, -1.0 };
static double defaultScalarRangeBkg[2] = { 0.0, -1.0 };
static double defaultAlpha = 1.0;
static int defaultColorbar = 0;
static double defaultClipRange[2] = { 0.0, -1.0 };
static double defaultRotate[3] = { 270.0, 0.0, -90.0 };
static int defaultWindowSize[2] = { 600, 600 };

int main( int argc, char **argv )
{
  if ( argc < 2 )
  {
    usage(argv[0]);
    exit( 1 );
  }

  char *scalarFileName = NULL;
  char *scalarFileNameBkg = NULL;
  char *outputFileName = NULL;
  int colormap = JET;
  int colorbar = defaultColorbar;
  double alpha = defaultAlpha;
  int scalar = 0, scalarBkg = 0, png = 0, logScale = 0;
  double scalarRange[2] = {defaultScalarRange[0], defaultScalarRange[1]};
  double scalarRangeBkg[2] = {defaultScalarRangeBkg[0], defaultScalarRangeBkg[1]};
  double clipRange[2] = {defaultClipRange[0], defaultClipRange[1]};
  int WindowSize[2] = {defaultWindowSize[0], defaultWindowSize[1]};
  double rotate[3] = {defaultRotate[0], defaultRotate[1], defaultRotate[2]};

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
   else if (strcmp(argv[j], "-rotate") == 0) {
    j++; rotate[0] = atof(argv[j]);
    j++; rotate[1] = atof(argv[j]);
    j++; rotate[2] = atof(argv[j]);
   }
   else if (strcmp(argv[j], "-scalar") == 0) {
    j++; scalarFileName = argv[j];
    scalar = 1;
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
   else if (strcmp(argv[j], "-left") == 0)
    rotate[2] = 90;
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
   usage(argv[0]);
   exit(1);
  }
  
  const int numArgs = argc - indx;
  
  if (numArgs != 1) {
   usage(argv[0]);
   exit(1);
  }
  
  char *inputFileName = argv[indx];

  vtkSurfaceReader *polyDataReader = vtkSurfaceReader::New();
  vtkSurfaceReader *polyDataReaderBkg = vtkSurfaceReader::New();
  vtkRenderer *renderer = vtkRenderer::New();
  vtkPolyDataMapper *polyDataMapper  = vtkPolyDataMapper::New();
  vtkPolyDataMapper *polyDataMapperBkg = vtkPolyDataMapper::New();
  vtkLookupTable *lookupTableBkg = vtkLookupTable::New();
  vtkLookupTableWithEnabling *lookupTable = vtkLookupTableWithEnabling::New();
  vtkActor *actor  = vtkActor::New();
  vtkActor *actorBkg = vtkActor::New();
  vtkScalarBarWidgetCAT *scalarBarWidget = vtkScalarBarWidgetCAT::New();
  vtkInteractorStyleCAT *interactorStyleCAT = vtkInteractorStyleCAT::New();
  vtkRenderWindowInteractor *renderWindowInteractor =  vtkRenderWindowInteractor::New();
  vtkRenderWindow *renderWindow = vtkRenderWindow::New();

  renderer->SetBackground( 0.0, 0.0, 0.0 );
  
  // limit range of alpha to 0..1
  if( alpha > 1.0) alpha = 1.0;
  if( alpha < 0.0) alpha = 0.0;
  
  polyDataReader->SetFileName( inputFileName );
  polyDataReader->Update();

  polyDataMapper->SetInput( polyDataReader->GetOutput() );

  cerr << "input number of points/polys " <<   polyDataReader->GetOutput()->GetNumberOfPoints() <<
    "/" <<  polyDataReader->GetOutput()->GetNumberOfPolys() << endl ;
      
  if (scalarBkg) {
    polyDataReaderBkg->SetFileName( inputFileName );
    polyDataReaderBkg->Update();
    polyDataMapperBkg->SetInput( polyDataReaderBkg->GetOutput() );

    actorBkg->SetMapper( polyDataMapperBkg );
    actorBkg->RotateX(rotate[0]);
    actorBkg->RotateY(rotate[1]);
    actorBkg->RotateZ(rotate[2]);

    renderer->AddActor( actorBkg );
  }
  
  actor->SetMapper( polyDataMapper );
  actor->RotateX(rotate[0]);
  actor->RotateY(rotate[1]);
  actor->RotateZ(rotate[2]);

  renderer->AddActor( actor );

  renderWindow->AddRenderer( renderer );
  renderWindow->SetSize( WindowSize[0], WindowSize[1] );

  renderWindowInteractor->SetRenderWindow( renderWindow );
  renderWindowInteractor->SetInteractorStyle( interactorStyleCAT );
  renderWindowInteractor->Initialize();

  // read scalars if defined
  if (scalar) {
    cout << "Read scalars: " << scalarFileName << endl; 
    vtkDoubleArray *scalars = NULL;
    scalars = readScalars(scalarFileName);
    
    if(scalars == NULL) {
      cerr << "Error reading file " << scalarFileName << endl;
      return(-1);
    }
    
    polyDataReader->GetOutput()->GetPointData()->SetScalars(scalars);
    
    // clip values if defined
    if (clipRange[1] > clipRange[0]) {
      for(int i=0; i < polyDataReader->GetOutput()->GetNumberOfPoints(); i++) {
        if((scalars->GetValue(i) > clipRange[0]) && (scalars->GetValue(i) < clipRange[1]))
          scalars->SetValue(i,0);
      }
    }
  }

  if (scalarRange[1] < scalarRange[0])
    polyDataReader->GetOutput()->GetScalarRange( scalarRange );
  
  // read scalars if defined
  if (scalarBkg) {
    cout << "Read background scalars: " << scalarFileNameBkg << endl; 
    vtkDoubleArray *scalarsBkg = NULL;
    scalarsBkg = readScalars(scalarFileNameBkg);
    polyDataReaderBkg->GetOutput()->GetPointData()->SetScalars(scalarsBkg);
    if (scalarRangeBkg[1] < scalarRangeBkg[0])
      polyDataReaderBkg->GetOutput()->GetScalarRange( scalarRangeBkg );
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
  
  // set opacity  
  lookupTable->SetAlphaRange( alpha, alpha );

  if(logScale) lookupTable->SetScaleToLog10();
  lookupTable->SetTableRange( scalarRange );
  if (clipRange[1] >= clipRange[0])
    lookupTable->SetEnabledArray(polyDataReader->GetOutput()->GetPointData()->GetScalars());
  lookupTable->Build();

  // background surface is alway gray
  if(scalarBkg) {
    lookupTableBkg->SetTableRange( scalarRangeBkg );
    lookupTableBkg->SetHueRange( 0.0, 0.0 );
    lookupTableBkg->SetSaturationRange( 0, 0 );
    lookupTableBkg->SetValueRange( 0.5, 1.0 );
    lookupTableBkg->Build();
  }
  
  // plot colorbar only if scalar vector data is defined
  if (polyDataReader->GetOutput()->GetPointData()->GetScalars()) {
    if (polyDataReader->GetOutput()->GetPointData()->GetScalars()->GetNumberOfComponents() == 1) {
      // if scalarRange is not defined use range from reader
      polyDataMapper->SetScalarRange( scalarRange );
      polyDataMapper->SetLookupTable( lookupTable );
      if(scalarBkg) {
        polyDataMapperBkg->SetScalarRange( scalarRangeBkg );
        polyDataMapperBkg->SetLookupTable( lookupTableBkg );
      }
      if (colorbar == 1) {
        scalarBarWidget->SetInteractor(renderWindowInteractor);
        scalarBarWidget->lookupTable = lookupTable;
        scalarBarWidget->GetScalarBarActor()->SetLookupTable(lookupTable);
        scalarBarWidget->GetScalarBarActor()->SetOrientationToHorizontal();
        scalarBarWidget->GetScalarBarActor()->SetWidth(0.4);
        scalarBarWidget->GetScalarBarActor()->SetHeight(0.075);
        scalarBarWidget->GetScalarBarActor()->SetPosition(0.3, 0.05);
        if (scalar == 1)
          scalarBarWidget->GetScalarBarActor()->SetTitle(scalarFileName);
        scalarBarWidget->SetEnabled(1);
        renderer->AddActor( scalarBarWidget->GetScalarBarActor() );
      }
    }
  }

  renderWindow->Render();

  renderWindow->SetWindowName(inputFileName);

  // save png-file if defined
  if (png == 1) {

    cout << "Write: " << outputFileName << endl; 

    vtkWindowToImageFilter *windowToImageFilter = vtkWindowToImageFilter::New();
    windowToImageFilter->SetInput( renderWindow );
                                                	
    vtkPNGWriter *PNGWriter = vtkPNGWriter::New();
    PNGWriter->SetInput( windowToImageFilter->GetOutput() );
    PNGWriter->SetFileName( outputFileName );
    windowToImageFilter->Update();
    PNGWriter->Write();
    PNGWriter->Delete();
    windowToImageFilter->Delete();
  } else renderWindowInteractor->Start();

  polyDataReader->Delete();
  polyDataReaderBkg->Delete();
  lookupTable->Delete();
  lookupTableBkg->Delete();
  actor->Delete();
  actorBkg->Delete();
  polyDataMapper->Delete();
  polyDataMapperBkg->Delete();
  renderer->Delete();
  renderWindow->Delete();
  interactorStyleCAT->Delete();
  renderWindowInteractor->Delete();
  scalarBarWidget->Delete();

  return 0;
}

vtkDoubleArray* readScalars(char* filename)
{
  
  FILE* fp = fopen(filename, "r");
  if (fp == NULL) {
    cerr << "Unable to open " << filename << endl;
    return NULL;
  }

  vtkDoubleArray* scalars = vtkDoubleArray::New();
  vtkFreesurferReader *FreesurferReader = vtkFreesurferReader::New();
  
  int i, magic, nValues, fNum, valsPerVertex;
  double x;
  const int LINE_SIZE = 10240;
  char line[LINE_SIZE];
  
  magic = FreesurferReader->Fread3(fp);
  
  // freesurfer scalars
  if (magic==16777215)
  {
  
    nValues = FreesurferReader->FreadInt(fp);
    fNum = FreesurferReader->FreadInt(fp);
    valsPerVertex = FreesurferReader->FreadInt(fp);

    for (i = 0; i < nValues; i++) {
      x = FreesurferReader->FreadFloat(fp);
      scalars->InsertNextValue(x);
    }
  }
  // BIC scalars as ascii file
  else
  {
    while (fgets(line, sizeof(line), fp) != NULL) {
      if (sscanf(line, "%lf", &x) != 1) {
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

static void
usage(const char* const prog)
{
  cout << endl
  << "NAME " << endl
  << "    " << prog << " - render surface"<< endl
  << "" << endl
  << "SYNOPSIS" << endl
  << "    " << prog << " [options] <input.vtk>" << endl
  << "" << endl
  << "DESCRIPTION" << endl
  << "    This program will render the <input.vtk> surface." << endl
  << endl
  << "OPTIONS" << endl
  << " Overlay:" << endl
  << "  -scalar scalarInput.txt  " << endl
  << "     File with scalar values (either ascii or Freesurfer format)." << endl
  << "  -range lower upper  " << endl
  << "     Range of scalar values." << endl
  << "     Default value: " << defaultScalarRange[0] << " " << defaultScalarRange[1] << endl
  << "  -clip lower upper  " << endl
  << "     Clip scalar values. These values will be not displayed." << endl
  << "     Default value: " << defaultClipRange[0] << " " << defaultClipRange[1] << endl
  << "  -colorbar  " << endl
  << "     Show colorbar (default no)." << endl
  << " Background surface:" << endl
  << "  -bkg scalarInputBkg.txt  " << endl
  << "     File with scalar values for background surface (either ascii or Freesurfer format)." << endl
  << "  -range-bkg lower upper  " << endl
  << "     Range of scalar values for background surface." << endl
  << "     Default value: " << defaultScalarRange[0] << " " << defaultScalarRange[1] << endl
  << " Colors:" << endl
  << "  -opacity value  " << endl
  << "     Value for opacity of overlay." << endl
  << "     Default value: " << defaultAlpha << endl
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
  << " Orientation:" << endl
  << "  -left  " << endl
  << "     Show left hemisphere (default right)." << endl
  << "  -rotate x y z " << endl
  << "     Rotate xyz-axis." << endl
  << "     Default value: " << defaultRotate[0] << " " << defaultRotate[1] << " " << defaultRotate[2] << endl
  << " Window:" << endl
  << "  -size xsize ysize  " << endl
  << "     Window size." << endl
  << "     Default value: " << defaultWindowSize[0] << " " << defaultWindowSize[1] << endl
  << "  -output output.png  " << endl
  << "     Save as png-file." << endl
  << endl
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
  << "REQUIRED PARAMETERS" << endl
  << "    <input.vtk> " << endl
  << "" << endl
  << "EXAMPLE" << endl
  << "    " << prog << " -range -1 1 -scalar scalarInput.txt input.vtk" << endl
  << endl
  << endl;
}
