// (c) Sylvain JAUME <sylvain at mit.edu> 2005

/*
  Smoothes a triangle mesh.
  This code uses the Visualization Toolkit.
*/

const char *Specification =
  "Smoothes a triangle mesh.\n"
  "This code uses the Visualization Toolkit.";

#include "vtkFileOutputWindow.h"
#include "vtkPolyData.h"
#include "vtkSmoothPolyDataFilter.h"
#include "vtkPolyDataReader.h"
#include "vtkPolyDataWriter.h"
#include "macro.h"

int main( int argc, char **argv )
{
  MAIN_MACRO;

  if ( argc != 4 )
    {
      USAGE_MACRO( "number_of_iterations input.vtk output.vtk\n"
		   << Specification );
      exit( 0 );
    }

  const char *inputFileName = argv[2];
  const char *outputFileName = argv[3];

  int numberOfIterations = -1;

  if ( sscanf( argv[1], "%d", &numberOfIterations ) != 1 )
    {
      PRINTOUT_MACRO( "argv[1] " << argv[1] );
      ERROR_MACRO( "number of iterations " << numberOfIterations );
      exit( 0 );
    }

  DEBUG_MACRO( "number of iterations " << numberOfIterations );
  ASSERT_MACRO( numberOfIterations > 0 );

  ASSERT_MACRO( inputFileName );
  DEBUG_MACRO( "input file name " << inputFileName );

  int inputLength = strlen( inputFileName );

  ASSERT_MACRO( inputLength > 4 );
  ASSERT_MACRO( inputLength < 256 );

  ASSERT_MACRO( inputFileName[ inputLength - 4 ] == '.' );
  ASSERT_MACRO( inputFileName[ inputLength - 3 ] == 'v' );
  ASSERT_MACRO( inputFileName[ inputLength - 2 ] == 't' );
  ASSERT_MACRO( inputFileName[ inputLength - 1 ] == 'k' );

  ASSERT_MACRO( outputFileName );
  DEBUG_MACRO( "output file name " << outputFileName );

  int outputLength = strlen( outputFileName );

  ASSERT_MACRO( outputLength > 4 );
  ASSERT_MACRO( outputLength < 256 );

  ASSERT_MACRO( outputFileName[ outputLength - 4 ] == '.' );
  ASSERT_MACRO( outputFileName[ outputLength - 3 ] == 'v' );
  ASSERT_MACRO( outputFileName[ outputLength - 2 ] == 't' );
  ASSERT_MACRO( outputFileName[ outputLength - 1 ] == 'k' );

  PRINTOUT_MACRO( "read " << inputFileName );
  vtkPolyDataReader *polyDataReader = vtkPolyDataReader::New();
  polyDataReader->SetFileName( inputFileName );
  polyDataReader->Update();

  ASSERT_MACRO( polyDataReader->GetErrorCode() == 0 );
  ASSERT_MACRO( polyDataReader->GetOutput() );

  int inputNumberOfPoints = polyDataReader->GetOutput()->GetNumberOfPoints();
  int inputNumberOfPolys = polyDataReader->GetOutput()->GetNumberOfPolys();
  int inputNumberOfStrips = polyDataReader->GetOutput()->GetNumberOfStrips();
  int inputNumberOfCells = polyDataReader->GetOutput()->GetNumberOfCells();

  DEBUG_MACRO( "input number of points " << inputNumberOfPoints );
  DEBUG_MACRO( "input number of polygons " << inputNumberOfPolys );
  DEBUG_MACRO( "input number of strips " << inputNumberOfStrips );
  DEBUG_MACRO( "input number of cells " << inputNumberOfCells );

  ASSERT_MACRO( inputNumberOfPoints > 0 );
  ASSERT_MACRO( inputNumberOfCells > 0 );

  DEBUG_MACRO( "smooth poly data filter" );
  vtkSmoothPolyDataFilter *smoothPolyDataFilter =
    vtkSmoothPolyDataFilter::New();
  smoothPolyDataFilter->SetInput( polyDataReader->GetOutput() );
  polyDataReader->Delete();
  smoothPolyDataFilter->SetRelaxationFactor( 0.1 );
  smoothPolyDataFilter->SetNumberOfIterations( numberOfIterations );
  smoothPolyDataFilter->FeatureEdgeSmoothingOff();
  smoothPolyDataFilter->BoundarySmoothingOff();
  smoothPolyDataFilter->Update();

  ASSERT_MACRO( smoothPolyDataFilter->GetErrorCode() == 0 );
  ASSERT_MACRO( smoothPolyDataFilter->GetOutput() );

  int smoothNumberOfPoints =
    smoothPolyDataFilter->GetOutput()->GetNumberOfPoints();
  int smoothNumberOfPolys =
    smoothPolyDataFilter->GetOutput()->GetNumberOfPolys();
  int smoothNumberOfCells =
    smoothPolyDataFilter->GetOutput()->GetNumberOfCells();

  DEBUG_MACRO( "smooth number of points " << smoothNumberOfPoints );
  DEBUG_MACRO( "smooth number of polygons " << smoothNumberOfPolys );
  DEBUG_MACRO( "smooth number of cells " << smoothNumberOfCells );

  ASSERT_MACRO( smoothNumberOfPoints == inputNumberOfPoints );
  ASSERT_MACRO( smoothNumberOfPolys == inputNumberOfPolys );
  ASSERT_MACRO( smoothNumberOfCells == inputNumberOfCells );

  PRINTOUT_MACRO( "write " << outputFileName );
  vtkPolyDataWriter *polyDataWriter = vtkPolyDataWriter::New();
  polyDataWriter->SetInput( smoothPolyDataFilter->GetOutput() );
  smoothPolyDataFilter->Delete();
  polyDataWriter->SetFileName( outputFileName );
  polyDataWriter->SetFileTypeToBinary();
  polyDataWriter->Write();

  ASSERT_MACRO( polyDataWriter->GetErrorCode() == 0 );
  polyDataWriter->Delete();

  END_MACRO( argv[0] );
  return 0;
}
