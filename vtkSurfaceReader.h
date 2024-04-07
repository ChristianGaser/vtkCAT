#ifndef vtkSurfaceReader_h
#define vtkSurfaceReader_h

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>

// Forward declaration
class gifti_image2;

// Function to convert a GIFTI image to a VTK polydata
vtkSmartPointer<vtkPolyData> ConvertGIFTIToVTK(gifti_image2* gimage);

// Function to read a GIFTI file and return VTK polydata
vtkSmartPointer<vtkPolyData> ReadGIFTIFile(const char* filename);

int FreadInt(FILE *fp);
float FreadFloat(FILE *fp);
int Fread3(FILE *fp);

#endif // vtkSurfaceReader_h
