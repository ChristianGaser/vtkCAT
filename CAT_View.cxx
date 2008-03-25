#include <string>

#include "vtkFileOutputWindow.h"
#include "vtkPolyData.h"
#include "vtkPointData.h"
#include "vtkLookupTable.h"
#include "vtkDoubleArray.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkProperty.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkPolyDataReader.h"
#include "vtkCommand.h"
#include "vtkBICOBJReader.h"
#include "vtkFreesurferReader.h"
#include "vtkScalarBarWidget.h"
#include "vtkScalarBarActor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkPNGWriter.h"
#include "vtkWindowToImageFilter.h"
#include <vtksys/SystemTools.hxx>
#include "vtkInteractorStyleCAT.h"


static void usage(const char* const prog);
vtkDoubleArray* readScalars(char* filename);
vtkDoubleArray* readFreesurferScalars(char* filename);

static double defaultScalarRange[2] = { 0, -1 };
static int defaultColorbar = 0;
static double defaultRotate[3] = { 270.0, 0.0, -90.0 };

int main( int argc, char **argv )
{
  if ( argc < 2 )
  {
    usage(argv[0]);
    exit( 1 );
  }

  int inputLength = -1;
  int polyFormat = -1;
  char *scalarFileName = NULL;
  char *outputFileName = NULL;
  int colorbar = defaultColorbar;
  int scalar = 0;
  int png = 0;
  double scalarRange[2];
  scalarRange[0] = defaultScalarRange[0];
  scalarRange[1] = defaultScalarRange[1];
  double rotate[3];
  rotate[0] = defaultRotate[0];
  rotate[1] = defaultRotate[1];
  rotate[2] = defaultRotate[2];

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

  vtkPolyDataReader *polyDataReader = NULL;
  vtkRenderer *renderer = vtkRenderer::New();
  vtkPolyDataMapper *polyDataMapper = vtkPolyDataMapper::New();
  vtkLookupTable *lookupTable = vtkLookupTable::New();
  vtkActor *actor = vtkActor::New();
  vtkScalarBarWidget *scalarBarWidget = vtkScalarBarWidget::New();
  vtkInteractorStyleCAT *interactorStyleTrackballCamera = vtkInteractorStyleCAT::New();

  vtkRenderWindowInteractor *renderWindowInteractor =  vtkRenderWindowInteractor::New();
  vtkRenderWindow *renderWindow = vtkRenderWindow::New();

  renderer->SetBackground( 0.0, 0.0, 0.0 );

  cerr << "read " << inputFileName << endl;

  inputLength = strlen( inputFileName );
    
  if (!strcmp(vtksys::SystemTools::GetFilenameLastExtension(inputFileName).c_str(),".vtk")) 
    polyFormat = 0;
  
  if (!strcmp(vtksys::SystemTools::GetFilenameLastExtension(inputFileName).c_str(),".obj")) 
    polyFormat = 1;

  switch(polyFormat) {
    case -1:
      polyDataReader = vtkFreesurferReader::New();
      break;      
    case  0:
      polyDataReader = vtkPolyDataReader::New();
      break;      
    case  1:
      polyDataReader = vtkBICOBJReader::New();
      break;      
  }
  
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
  renderWindow->SetSize( 600, 600 );

  renderWindowInteractor->SetRenderWindow( renderWindow );
  renderWindowInteractor->SetInteractorStyle( interactorStyleTrackballCamera );
  renderWindowInteractor->Initialize();

  // read scalars if defined
  if (scalar == 1) {
    cout << "Read scalars: " << scalarFileName << endl; 
    vtkDoubleArray *scalars = NULL;
    if(polyFormat == -1)
      scalars = readFreesurferScalars(scalarFileName);
    else
      scalars = readScalars(scalarFileName);
    polyDataReader->GetOutput()->GetPointData()->SetScalars(scalars);
  }
  
  if (scalarRange[1] < scalarRange[0])
    polyDataReader->GetOutput()->GetScalarRange( scalarRange );
  lookupTable->SetTableRange( scalarRange );
  lookupTable->SetHueRange( 0.667, 0.0 );
  lookupTable->SetSaturationRange( 1, 1 );
  lookupTable->SetValueRange( 1, 1 );

  // plot colorbar only if scalar vector data is defined
  if (polyDataReader->GetOutput()->GetPointData()->GetScalars()) {
    if (polyDataReader->GetOutput()->GetPointData()->GetScalars()->GetNumberOfComponents() == 1) {
      // if scalarRange is not defined use range form reader
      polyDataMapper->SetScalarRange( scalarRange );
      polyDataMapper->SetLookupTable( lookupTable );
      if (colorbar == 1) {
        scalarBarWidget->SetInteractor(renderWindowInteractor);
        if (scalar == 1)
          scalarBarWidget->GetScalarBarActor()->SetTitle(scalarFileName);
        scalarBarWidget->GetScalarBarActor()->SetLookupTable(polyDataMapper->GetLookupTable());
        scalarBarWidget->GetScalarBarActor()->SetOrientationToHorizontal();
        scalarBarWidget->GetScalarBarActor()->SetWidth(0.4);
        scalarBarWidget->GetScalarBarActor()->SetHeight(0.075);
        scalarBarWidget->GetScalarBarActor()->SetPosition(0.3, 0.05);
        scalarBarWidget->EnabledOn();
      }
    }
  }


  renderWindow->Render();

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
  interactorStyleTrackballCamera->Delete();
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
  
  double x;
  const int LINE_SIZE = 10240;
  char line[LINE_SIZE];
  
  while (fgets(line, sizeof(line), fp) != NULL) {
    if (sscanf(line, "%lf", &x) != 1) {
      cerr << "Error reading value from line " << line << " from file " << filename << endl;
      fclose(fp);
      return NULL;
    }
    scalars->InsertNextValue(x);
  }
   
  fclose(fp);
  return scalars;
}

vtkDoubleArray* readFreesurferScalars(char* filename)
{
  
  FILE* fp = fopen(filename, "rb");
  if (fp == NULL) {
    cerr << "Unable to open " << filename << endl;
    return NULL;
  }

  vtkDoubleArray* scalars = vtkDoubleArray::New();
  vtkFreesurferReader *FreesurferReader = vtkFreesurferReader::New();
  
  double x;
  int i, magic, nValues, fNum, valsPerVertex;
  
  magic = FreesurferReader->Fread3(fp);

  nValues = FreesurferReader->FreadInt(fp);
  fNum = FreesurferReader->FreadInt(fp);
  valsPerVertex = FreesurferReader->FreadInt(fp);

  for (i = 0; i < nValues; i++) {
    x = FreesurferReader->FreadFloat(fp);
    scalars->InsertNextValue(x);
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
  << "  -range lower upper  " << endl
  << "     Range of scalar values." << endl
  << "     Default value: " << defaultScalarRange[0] << " " << defaultScalarRange[1] << endl
  << "  -scalar scalarInput.txt  " << endl
  << "     Ascii file with scalar values." << endl
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
  << "REQUIRED PARAMETERS" << endl
  << "    <inputFiducial.vtk> " << endl
  << "" << endl
  << "EXAMPLE" << endl
  << "    " << prog << " -range -1 1 -scalar scalarInput.txt input.vtk" << endl
  << endl
  << endl;
}
