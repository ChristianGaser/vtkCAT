// (c) Sylvain JAUME sylvain@mit.edu 2005

/*
  This example creates a surface mesh from a 3D image.
  If an isovalue outside of the image scalar range is specified, the center
  of the scalar range is picked.
  This code uses the Visualization Toolkit.
*/

const char *Specification =
  "This example creates the largest component of a surface mesh from a 3D image.\n"
  "If an isovalue outside of the image scalar range is specified, the center "
  "of the scalar range is picked.\n"
  "Optionally you can define the maximum number of Polygons. The default is 100000.\n"
  "This code uses the Visualization Toolkit.";

#include "vtkImageGaussianSmooth.h"
#include "vtkSmoothPolyDataFilter.h"
#include "vtkCellArray.h"
#include "vtkCleanPolyData.h"
#include "vtkDecimatePro.h"
#include "vtkPolyDataConnectivityFilter.h"
#include "vtkFileOutputWindow.h"
#include "vtkImageData.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkImageMarchingCubes.h"
#include "vtkMetaImageReader.h"
#include "vtkPolyDataWriter.h"
#include "vtkExtractEdges.h"
#include "vtkMINCImageReader.h"
#include <vtksys/SystemTools.hxx>
#include "macro.h"
#include "vtkFeatureEdges.h"
#include "vtkCurvatures.h"

#ifdef vtkCAT_USE_ITK
#include <vtkMetaImageData.h>
#endif

int main( int argc, char **argv )
{
  MAIN_MACRO;

  if ( argc < 4 )
    {
      USAGE_MACRO( "isovalue input.mhd output.vtk [maxNumberOfPolys] [sdGaussianFiltering]\n" << Specification );
      return 1;
    }

  const char *inputFileName = argv[2];
  const char *outputFileName = argv[3];
  int isMinc = 0;
  double origin[3];
  
  double isovalue = -1.0;
  if ( sscanf( argv[1], "%lf", &isovalue ) != 1 )
    {
      PRINTOUT_MACRO( "argv[1] " << argv[1] );
      ERROR_MACRO( "cannot read isovalue in argv[1]" );
      exit( 0 );
    }

  double maxNumberOfPolys = 100000.0;
  if ( argc == 5 )
  {
      if ( sscanf( argv[4], "%lf", &maxNumberOfPolys ) != 1 )
        {
        PRINTOUT_MACRO( "argv[4] " << argv[4] );
        ERROR_MACRO( "cannot read maxNumberOfPolys in argv[4]" );
        exit( 0 );
        }
  }
  
  double sdGaussianFiltering = 1.0;
  if ( argc == 6 )
  {
      if ( sscanf( argv[5], "%lf", &sdGaussianFiltering ) != 1 )
        {
        PRINTOUT_MACRO( "argv[5] " << argv[5] );
        ERROR_MACRO( "cannot read std_gaussian in argv[5]" );
        exit( 0 );
        }
  }

// ----------------------------------------------------------------------------
  ASSERT_MACRO( inputFileName );

  if (!strcmp(vtksys::SystemTools::GetFilenameLastExtension(inputFileName).c_str(),".mnc")) 
    isMinc = 1;

// ----------------------------------------------------------------------------
  PRINTOUT_MACRO( "read image " << inputFileName );
  vtkImageData *image = vtkImageData::New();
  vtkMINCImageReader *reader = vtkMINCImageReader::New();
  
#ifdef vtkCAT_USE_ITK
  vtkMetaImageData* metaimage = vtkMetaImageData::New();
  if(isMinc==0) 
  {
    metaimage->Read(inputFileName);
    image = metaimage->GetImageData();
    vtkMatrix4x4* matrix = metaimage->GetOrientationMatrix();
    for (int i = 0; i < 3; i++) origin[i] = matrix->GetElement(i,3)/matrix->GetElement(i,i);
    image->SetOrigin(origin);
    matrix->Delete();
  }
#endif

  if(isMinc) 
  {
    reader->SetFileName (inputFileName);
    reader->GetOutput()->Update();
    image = reader->GetOutput();
  }
    
  double inputScalarRange[2] = { 0.0, -1.0 };

  image->GetScalarRange( inputScalarRange );

  if ( isovalue < inputScalarRange[0] || isovalue > inputScalarRange[1] )
    {
      PRINTOUT_MACRO( "isovalue " << isovalue << " out of scalar range "
		      << inputScalarRange[0] << " " << inputScalarRange[1] );
      isovalue = 0.5 * ( inputScalarRange[0] + inputScalarRange[1] );
      PRINTOUT_MACRO( "new isovalue " << isovalue );
    }

// ----------------------------------------------------------------------------
  DEBUG_MACRO( "gaussian filtering" );
  vtkImageGaussianSmooth* gaussian = vtkImageGaussianSmooth::New();
  gaussian->SetDimensionality(3);
  gaussian->SetStandardDeviation(sdGaussianFiltering, sdGaussianFiltering, sdGaussianFiltering);
  gaussian->SetInput(image);
  gaussian->Update();

// ----------------------------------------------------------------------------
  DEBUG_MACRO( "image marching cubes" );
  vtkImageMarchingCubes *imageMarchingCubes = vtkImageMarchingCubes::New();
  imageMarchingCubes->SetInput( gaussian->GetOutput() );
  imageMarchingCubes->SetValue( 0, isovalue );
  imageMarchingCubes->ComputeScalarsOff();
  imageMarchingCubes->ComputeNormalsOff();
  imageMarchingCubes->ComputeGradientsOff();
  imageMarchingCubes->Update();
  gaussian->Delete();

// ----------------------------------------------------------------------------
  DEBUG_MACRO( "poly data connectivity filter" );
  vtkPolyDataConnectivityFilter *polyDataConnectivityFilter = vtkPolyDataConnectivityFilter::New();
  polyDataConnectivityFilter->SetInput( imageMarchingCubes->GetOutput() );
  polyDataConnectivityFilter->SetExtractionModeToLargestRegion();
  polyDataConnectivityFilter->Update();
  imageMarchingCubes->Delete();

// ----------------------------------------------------------------------------
  DEBUG_MACRO( "clean poly data" );
  vtkCleanPolyData *clean = vtkCleanPolyData::New();
  clean->SetInput( polyDataConnectivityFilter->GetOutput() );
  clean->Update();

// ----------------------------------------------------------------------------
  DEBUG_MACRO( "decimate" );
  double targetReduction = 1 - (maxNumberOfPolys/polyDataConnectivityFilter->GetOutput()->GetNumberOfPolys());
  if (targetReduction < 0) targetReduction = 0;
  vtkDecimatePro *decimater = vtkDecimatePro::New(); 
  decimater->SetInput(clean->GetOutput());
  decimater->SetTargetReduction(targetReduction);
  decimater->PreserveTopologyOn();
  decimater->AccumulateErrorOn();
  decimater->SetMaximumError(1);
  decimater->Update();
  clean->Delete();

// ----------------------------------------------------------------------------
  DEBUG_MACRO( "smooth polygons" );
  vtkSmoothPolyDataFilter* smooth = vtkSmoothPolyDataFilter::New();
  smooth->SetInput(decimater->GetOutput());
  smooth->SetNumberOfIterations(10);
  smooth->SetRelaxationFactor(0.2);
  smooth->SetFeatureAngle(180.0);
  smooth->FeatureEdgeSmoothingOff();
  smooth->BoundarySmoothingOff();
  smooth->SetConvergence(0);
  smooth->Update();
  decimater->Delete();

// ----------------------------------------------------------------------------
  DEBUG_MACRO( "compute curvatures" );
  vtkCurvatures* curvature = vtkCurvatures::New();
  curvature->SetInput(smooth->GetOutput());
  curvature->SetCurvatureTypeToMean();
  curvature->Update();
  smooth->Delete();

// ----------------------------------------------------------------------------
  DEBUG_MACRO( "extract edges" );
  vtkExtractEdges *extractEdges = vtkExtractEdges::New();
  extractEdges->SetInput( curvature->GetOutput() );
  extractEdges->Update();

  int NumberOfPoints = curvature->GetOutput()->GetNumberOfPoints();
  int NumberOfPolys = curvature->GetOutput()->GetNumberOfPolys();
  int NumberOfCells = curvature->GetOutput()->GetNumberOfCells();
  int NumberOfLines = extractEdges->GetOutput()->GetNumberOfLines();

  extractEdges->Delete();

  DEBUG_MACRO( "number of points " << NumberOfPoints);
  DEBUG_MACRO( "number of polys " << NumberOfPolys);
  DEBUG_MACRO( "number of cells " << NumberOfCells);
  DEBUG_MACRO( "number of lines " << NumberOfLines);

  int EulerCharacteristic = NumberOfPoints - NumberOfLines +
    NumberOfPolys;

  DEBUG_MACRO( "Euler characteristic : V - E + F = 2C - 2g : "
	       << EulerCharacteristic );

  DEBUG_MACRO( "number of holes: genus " << 0.5 * ( 2 - EulerCharacteristic ) );

// ----------------------------------------------------------------------------
  PRINTOUT_MACRO( "write " << outputFileName );
  vtkPolyDataWriter *polyDataWriter = vtkPolyDataWriter::New();
  polyDataWriter->SetInput( curvature->GetOutput() );
  polyDataWriter->SetFileName( outputFileName );
  polyDataWriter->SetFileTypeToBinary();
  polyDataWriter->Write();

  polyDataWriter->Delete();
  curvature->Delete();

  reader->Delete();
#ifdef vtkCAT_USE_ITK
  metaimage->Delete();
#endif  
  image->Delete();

  END_MACRO( argv[0] );
  return 0;
}
