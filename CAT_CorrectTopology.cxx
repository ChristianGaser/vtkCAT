
#include "lib/topologysimplification/Utils.h"
#include "lib/topologysimplification/SurfaceVtk.h"
#include "lib/topologysimplification/FastLoop.h"
#include "lib/topologysimplification/PatchDisk.h"
#include "vtkBICOBJReader.h"
#include "vtkFreesurferReader.h"
#include <vtksys/SystemTools.hxx>

static void usage(const char* const prog);

int main(int argc, char *argv[])
{
   if (argc < 3) {
      usage(argv[0]);
      exit(1);
   }

	char* inputFileName = argv[1];
	char* outputFileName = argv[2];

	// create a surfaace
	Surface surface=Surface();
	
	int inputLength = strlen( inputFileName );
    
	vtkPolyDataReader *polyDataReader = NULL;

	int polyFormat = -1;
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

	polyDataReader->SetFileName(inputFileName);
	polyDataReader->Update();
	surface.VtkToSurface(polyDataReader->GetOutput());

	int euler = surface.GetEuler();
	
	cout << "Initial Euler number is " << surface.euler << " ( = " << surface.nvertices 
		<< " - " << surface.nedges << " + " << surface.nfaces << " ) " << endl;	

	// the patching disks
	surface.disk = new PatchDisk[4];
	for(int n = 0 ; n < 4 ; n++)
		surface.disk[n].Create(n);

	surface.InitSurface();
	surface.CorrectTopology();

	vtkPolyData* polyData = vtkPolyData::New();
	polyData = surface.SurfaceToVtk();
	vtkPolyDataWriter* writer = vtkPolyDataWriter::New();
	writer->SetInput(polyData);
	writer->SetFileName(outputFileName);
	writer->SetFileTypeToASCII();
	writer->Write();
	polyData->Delete();
	writer->Delete();
       
	cout << "Final Euler number is " << surface.euler << " ( = " << surface.nvertices 
		<< " - " << surface.nedges << " + " << surface.nfaces << " ) " << endl;	

	return 0;
}

static void
usage(const char* const prog)
{
   cout << endl
   << "NAME " << endl
   << "     " << prog << " - correct topology to a sphere"<< endl
   << "" << endl
   << "SYNOPSIS" << endl
   << "     " << prog << " <input.vtk> <output.vtk>" << endl
   << "" << endl
   << "DESCRIPTION" << endl
   << endl
   << "REQUIRED PARAMETERS" << endl
   << "     <input.vtk> " << endl
   << "     <output.vtk> " << endl
   << "" << endl
   << "EXAMPLE" << endl
   << "     " << prog << " input.vtk output.vtk" << endl
   << endl
   << endl;
}

