#ifndef vtkStat_h
#define vtkStat_h

#include <vtkSmartPointer.h>
#include <vtkDoubleArray.h>

double get_mean(vtkSmartPointer<vtkDoubleArray> arr);
double get_median(vtkSmartPointer<vtkDoubleArray> arr);
double get_std(vtkSmartPointer<vtkDoubleArray> arr);
double get_sum(vtkSmartPointer<vtkDoubleArray> arr);

#endif // vtkStat_h
