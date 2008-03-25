/*=========================================================================

	Based on:
	Segonne F, Grimson E, Fischl B.	Related Articles
	A genetic algorithm for the topology correction of cortical surfaces.
	Inf Process Med Imaging. 2005;19:393-405. 

=========================================================================*/
// .NAME vtkCorrectTopologyPolyData.h - Targeted Smoothing of Points with weighting determined by neighboring tile area
// .SECTION Description
// By default all points are smoothed.  Targeted smoothing can be performed by
// enabling targeted smoothing and setting the points to be smoothed.
//
// .SECTION See Also
// vtkPolyDataAlgorithm 

#ifndef __vtkCorrectTopologyPolyData_h
#define __vtkCorrectTopologyPolyData_h

#include "vtkPolyDataAlgorithm.h"
#include "topologysimplification/Utils.h"
#include "topologysimplification/SurfaceVtk.h"
#include "topologysimplification/FastLoop.h"
#include "topologysimplification/PatchDisk.h"

class vtkPolyData;

class VTK_GRAPHICS_EXPORT vtkCorrectTopologyPolyData : public vtkPolyDataAlgorithm
{
public:
  // Description:
  // Correct topology by removing handles and holes;
  static vtkCorrectTopologyPolyData *New();
  vtkTypeRevisionMacro(vtkCorrectTopologyPolyData,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify the source object which is used to correct topology.
//  virtual void SetInput(vtkPolyData *input);
//  vtkPolyData *GetInput();
    
protected:

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int port, vtkInformation *info);

};


#endif


