/*=========================================================================

	Based on:
	Segonne F, Grimson E, Fischl B.	Related Articles
	A genetic algorithm for the topology correction of cortical surfaces.
	Inf Process Med Imaging. 2005;19:393-405. 

=========================================================================*/

#include "vtkCorrectTopologyPolyData.h"

vtkCxxRevisionMacro(vtkCorrectTopologyPolyData, "$Revision: 1.00 $");
vtkStandardNewMacro(vtkCorrectTopologyPolyData);

int vtkCorrectTopologyPolyData::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input
  vtkPolyData *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // Initialize
  vtkDebugMacro(<<"Correcting topology of polygonal data");

  if (input == NULL || input->GetPoints() == NULL)
    {
    return 1;
    }
  
	// create a surfaace
	Surface surface=Surface();
	
	surface.VtkToSurface(input);
	int euler = surface.GetEuler();
	
	cout << "Euler number is " << surface.euler << " ( = " << surface.nvertices 
		<< " - " << surface.nedges << " + " << surface.nfaces << " ) " << endl;	

	// the patching disks
	surface.disk = new PatchDisk[4];
	for(int n = 0 ; n < 4 ; n++)
		surface.disk[n].Create(n);

	surface.InitSurface();
	surface.CorrectTopology();

  return 1;
}

void vtkCorrectTopologyPolyData::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
//  os << indent << "Shrink Factor: " << this->ShrinkFactor << "\n";
}
