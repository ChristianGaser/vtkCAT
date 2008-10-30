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

static void usage(const char* const prog);
vtkDoubleArray* readScalars(char* filename);
vtkDoubleArray* readFreesurferScalars(char* filename);

static double defaultScalarRange[2] = { 0, -1 };
static int defaultColorbar = 0;
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
  char *outputFileName = NULL;
  int colorbar = defaultColorbar;
  int scalar = 0;
  int png = 0;
  double scalarRange[2] = {defaultScalarRange[0], defaultScalarRange[1]};
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
   else if (strcmp(argv[j], "-size") == 0) {
    j++; WindowSize[0] = atoi(argv[j]);
    j++; WindowSize[1] = atoi(argv[j]);
   }
   else if (strcmp(argv[j], "-colorbar") == 0) {
    colorbar = 1;
   }
   else if (strcmp(argv[j], "-left") == 0) {
    rotate[2] = 90;
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
   else if (strcmp(argv[j], "-output") == 0) {
    j++; outputFileName = argv[j];
    png = 1;
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
  
  const int numArgs = argc - indx;
  
  if (numArgs != 1) {
   usage(argv[0]);
   exit(1);
  }
  
  char *inputFileName = argv[indx];

  vtkSurfaceReader *polyDataReader = vtkSurfaceReader::New();
  vtkRenderer *renderer = vtkRenderer::New();
  vtkPolyDataMapper *polyDataMapper = vtkPolyDataMapper::New();
  vtkLookupTable *lookupTable = vtkLookupTable::New();
  vtkActor *actor = vtkActor::New();
  vtkScalarBarWidgetCAT *scalarBarWidget = vtkScalarBarWidgetCAT::New();
  vtkInteractorStyleCAT *interactorStyleCAT = vtkInteractorStyleCAT::New();
  vtkRenderWindowInteractor *renderWindowInteractor =  vtkRenderWindowInteractor::New();
  vtkRenderWindow *renderWindow = vtkRenderWindow::New();

  renderer->SetBackground( 0.0, 0.0, 0.0 );
    
  polyDataReader->SetFileName( inputFileName );
  polyDataReader->Update();
    
  cerr << "input number of points/polys " <<   polyDataReader->GetOutput()->GetNumberOfPoints() <<
    "/" <<  polyDataReader->GetOutput()->GetNumberOfPolys() << endl ;
  polyDataMapper->SetInput( polyDataReader->GetOutput() );
  
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
  if (scalar == 1) {
    cout << "Read scalars: " << scalarFileName << endl; 
    vtkDoubleArray *scalars = NULL;
    scalars = readScalars(scalarFileName);
    polyDataReader->GetOutput()->GetPointData()->SetScalars(scalars);
  }

  if (scalarRange[1] < scalarRange[0])
    polyDataReader->GetOutput()->GetScalarRange( scalarRange );
  lookupTable->SetTableRange( scalarRange );
  lookupTable->SetHueRange( 0.667, 0.0 );
  lookupTable->SetSaturationRange( 1, 1 );
  lookupTable->SetValueRange( 1, 1 );
  lookupTable->Build();

  // plot colorbar only if scalar vector data is defined
  if (polyDataReader->GetOutput()->GetPointData()->GetScalars()) {
    if (polyDataReader->GetOutput()->GetPointData()->GetScalars()->GetNumberOfComponents() == 1) {
      // if scalarRange is not defined use range from reader
      polyDataMapper->SetScalarRange( scalarRange );
      polyDataMapper->SetLookupTable( lookupTable );
      if (colorbar == 1) {
        scalarBarWidget->SetInteractor(renderWindowInteractor);
        scalarBarWidget->GetScalarBarActor()->SetLookupTable(polyDataMapper->GetLookupTable());
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
  lookupTable->Delete();
  actor->Delete();
  polyDataMapper->Delete();
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
  << "  -size xsize ysize  " << endl
  << "     Window size." << endl
  << "     Default value: " << defaultWindowSize[0] << " " << defaultWindowSize[1] << endl
  << "  -range lower upper  " << endl
  << "     Range of scalar values." << endl
  << "     Default value: " << defaultScalarRange[0] << " " << defaultScalarRange[1] << endl
  << "  -scalar scalarInput.txt  " << endl
  << "     File with scalar values (either ascii or Freesurfer format)." << endl
  << "  -colorbar  " << endl
  << "     Show colorbar (default no)." << endl
  << "  -left  " << endl
  << "     Show left hemisphere (default right)." << endl
  << "  -rotate x y z " << endl
  << "     Rotate xyz-axis." << endl
  << "     Default value: " << defaultRotate[0] << " " << defaultRotate[1] << " " << defaultRotate[2] << endl
  << "  -output output.png  " << endl
  << "     Save png-file." << endl
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
