#include <string>

#ifndef VTK_MAJOR_VERSION
#  include "vtkVersion.h"
#endif

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
#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkCamera.h"

#include "vtkInteractorStyleCAT.h"
#include "vtkSurfaceReader.h"
#include "vtkScalarBarWidgetCAT.h"
#if VTK_MAJOR_VERSION==5 && VTK_MINOR_VERSION>=4
  #include "vtkLookupTableWithEnabling.h"
#endif
#include <float.h>

typedef  enum  { JET, GRAY, REDYELLOW, BLUECYAN, YELLOWRED, CYANBLUE, BLUEGREEN, GREENBLUE };

static void usage(const char* const prog);
vtkDoubleArray* readScalars(char* filename);

static double defaultScalarRange[2] = { 0.0, -1.0 };
static double defaultScalarRangeBkg[2] = { 0.0, -1.0 };
static double defaultAlpha = 1.0;
static int defaultColorbar = 0;
static int defaultInverse = 0;
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
  char *colorbarTitle = NULL;
  int colormap = JET;
  int colorbar = defaultColorbar;
  int inverse = defaultInverse;
  double alpha = defaultAlpha;
  int scalar = 0, scalarBkg = 0, png = 0, logScale = 0, title = 0;
  double scalarRange[2] = {defaultScalarRange[0], defaultScalarRange[1]};
  double scalarRangeBkg[2] = {defaultScalarRangeBkg[0], defaultScalarRangeBkg[1]};
  double clipRange[2] = {defaultClipRange[0], defaultClipRange[1]};
  int WindowSize[2] = {defaultWindowSize[0], defaultWindowSize[1]};
  double rotate[3] = {defaultRotate[0], defaultRotate[1], defaultRotate[2]};
  int definedMatrix = 0;
  vtkMatrix4x4 *matrix = vtkMatrix4x4::New();

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
    #if VTK_MAJOR_VERSION==5 && VTK_MINOR_VERSION<4
      cout << "Clipping of values only possible for VTK >5.4." << endl;
      j++;j++;
    #else
      j++; clipRange[0] = atof(argv[j]);
      j++; clipRange[1] = atof(argv[j]);
    #endif
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
   else if (strcmp(argv[j], "-matrix") == 0) {
     definedMatrix = 1;
     for (int xi = 0; xi < 4; xi++) {
       for (int yi = 0; yi < 4; yi++) {
         j++;
         matrix->SetElement(xi,yi,atof(argv[j]));
       }
     }
   }
   else if (strcmp(argv[j], "-scalar") == 0) {
    j++; scalarFileName = argv[j];
    scalar = 1;
   }
   else if (strcmp(argv[j], "-title") == 0) {
    j++; colorbarTitle = argv[j];
    title = 1;
   }
   else if (strcmp(argv[j], "-bkg") == 0) {
    j++; scalarFileNameBkg = argv[j];
    #if VTK_MAJOR_VERSION==5 && VTK_MINOR_VERSION<4
      cout << "Overlay onto backgound surface only possible for VTK >5.4." << endl;
      scalarBkg = 0;
    #else
      scalarBkg = 1;
    #endif
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
  vtkActor *actor  = vtkActor::New();
  vtkActor *actorBkg = vtkActor::New();
  vtkScalarBarWidgetCAT *scalarBarWidget = vtkScalarBarWidgetCAT::New();
  vtkInteractorStyleCAT *interactorStyleCAT = vtkInteractorStyleCAT::New();
  vtkRenderWindowInteractor *renderWindowInteractor =  vtkRenderWindowInteractor::New();
  vtkRenderWindow *renderWindow = vtkRenderWindow::New();
  vtkTransform *transform = vtkTransform::New();

  renderer->SetBackground( 0.0, 0.0, 0.0 );
  
  // limit range of alpha to 0..1
  if( alpha > 1.0) alpha = 1.0;
  if( alpha < 0.0) alpha = 0.0;
  
  polyDataReader->SetFileName( inputFileName );
  polyDataReader->Update();

  polyDataMapper->SetInput( polyDataReader->GetOutput() );

  cerr << "input number of points/polys " <<   polyDataReader->GetOutput()->GetNumberOfPoints() <<
    "/" <<  polyDataReader->GetOutput()->GetNumberOfPolys() << endl ;
      
#if VTK_MAJOR_VERSION==5 && VTK_MINOR_VERSION>=4
  vtkLookupTableWithEnabling *lookupTable = vtkLookupTableWithEnabling::New();
  
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
#else
    vtkLookupTable *lookupTable = vtkLookupTable::New();
#endif
  
  actor->SetMapper( polyDataMapper );
  actor->RotateX(rotate[0]);
  actor->RotateY(rotate[1]);
  actor->RotateZ(rotate[2]);

  renderer->AddActor( actor );

  if(definedMatrix) {
    matrix->Invert();
    transform->SetMatrix(matrix);
    renderer->GetActiveCamera()->ApplyTransform(transform);
  }

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

    if(inverse) {
      for(int i=0; i < polyDataReader->GetOutput()->GetNumberOfPoints(); i++) {
          scalars->SetValue(i,-(scalars->GetValue(i)));
      }
    }
    
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
#if VTK_MAJOR_VERSION==5 && VTK_MINOR_VERSION>=4
  if (scalarBkg) {
    cout << "Read background scalars: " << scalarFileNameBkg << endl; 
    vtkDoubleArray *scalarsBkg = NULL;
    scalarsBkg = readScalars(scalarFileNameBkg);
    polyDataReaderBkg->GetOutput()->GetPointData()->SetScalars(scalarsBkg);
    if (scalarRangeBkg[1] < scalarRangeBkg[0])
      polyDataReaderBkg->GetOutput()->GetScalarRange( scalarRangeBkg );
  }
#endif

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
#if VTK_MAJOR_VERSION==5 && VTK_MINOR_VERSION>=4
  if (clipRange[1] >= clipRange[0])
    lookupTable->SetEnabledArray(polyDataReader->GetOutput()->GetPointData()->GetScalars());
#endif
  lookupTable->Build();

  // background surface is alway gray
#if VTK_MAJOR_VERSION==5 && VTK_MINOR_VERSION>=4
  vtkLookupTable *lookupTableBkg = vtkLookupTable::New();  
  
  if(scalarBkg) {
    lookupTableBkg->SetTableRange( scalarRangeBkg );
    lookupTableBkg->SetHueRange( 0.0, 0.0 );
    lookupTableBkg->SetSaturationRange( 0, 0 );
    lookupTableBkg->SetValueRange( 0.5, 1.0 );
    lookupTableBkg->Build();
  }
#endif
  
  // plot colorbar only if scalar vector data is defined
  if (polyDataReader->GetOutput()->GetPointData()->GetScalars()) {
    if (polyDataReader->GetOutput()->GetPointData()->GetScalars()->GetNumberOfComponents() == 1) {
      // if scalarRange is not defined use range from reader
      polyDataMapper->SetScalarRange( scalarRange );
      polyDataMapper->SetLookupTable( lookupTable );
#if VTK_MAJOR_VERSION==5 && VTK_MINOR_VERSION>=4
      if(scalarBkg) {
        polyDataMapperBkg->SetScalarRange( scalarRangeBkg );
        polyDataMapperBkg->SetLookupTable( lookupTableBkg );
      }
#endif
      if (colorbar == 1) {
        scalarBarWidget->SetInteractor(renderWindowInteractor);
        scalarBarWidget->lookupTable = lookupTable;
        scalarBarWidget->GetScalarBarActor()->SetLookupTable(lookupTable);
        scalarBarWidget->GetScalarBarActor()->SetOrientationToHorizontal();
        scalarBarWidget->GetScalarBarActor()->SetWidth(0.4);
        scalarBarWidget->GetScalarBarActor()->SetHeight(0.075);
        scalarBarWidget->GetScalarBarActor()->SetPosition(0.3, 0.05);
        if (scalar == 1) {
          if (title == 0)
            scalarBarWidget->GetScalarBarActor()->SetTitle(scalarFileName);
          else
            scalarBarWidget->GetScalarBarActor()->SetTitle(colorbarTitle);
        }
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
  actor->Delete();
  actorBkg->Delete();
  polyDataMapper->Delete();
  polyDataMapperBkg->Delete();
  renderer->Delete();
  renderWindow->Delete();
  interactorStyleCAT->Delete();
  renderWindowInteractor->Delete();
  scalarBarWidget->Delete();
  matrix->Delete();
#if VTK_MAJOR_VERSION==5 && VTK_MINOR_VERSION>=4
  if (scalarBkg) lookupTableBkg->Delete();
#endif

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
  
  int i, magic, nValues, fNum, valsPerVertex, errno;
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
  << "  -title  " << endl
  << "     Set name for colorbar (default scalar-file)." << endl
#if VTK_MAJOR_VERSION==5 && VTK_MINOR_VERSION>=4
  << " Background surface:" << endl
  << "  -bkg scalarInputBkg.txt  " << endl
  << "     File with scalar values for background surface (either ascii or Freesurfer format)." << endl
  << "  -range-bkg lower upper  " << endl
  << "     Range of scalar values for background surface." << endl
  << "     Default value: " << defaultScalarRange[0] << " " << defaultScalarRange[1] << endl
#endif
  << "  -inverse  " << endl
  << "     Invert input values." << endl
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
  << "  -matrix m1 m2 m3 m4 m5 m6 m7 m8 m9 m10 m11 m12 m13 m14 m15 m16 " << endl
  << "     Use transformation matrix (which was dump with v-key)." << endl
  << " Output:" << endl
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
  << "  v " << endl
  << "     Print transformation matrix for use with matrix flag." << endl
  << "  g " << endl
  << "     Grab image to file render.png." << endl
  << "  i " << endl
  << "     Enable/disable colorbar." << endl
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
