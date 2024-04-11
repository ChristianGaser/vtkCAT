#include "vtkStat.h"

void quicksort(vtkSmartPointer<vtkDoubleArray> arr, int start, int end) {
  if (end > start + 1) {
    double pivot = arr->GetValue(start);
    int left = start + 1, right = end;
    while (left < right) {
      if (arr->GetValue(left) <= pivot) {
        left++;
      } else {
        double temp = arr->GetValue(left);
        arr->SetValue(left, arr->GetValue(--right));
        arr->SetValue(right, temp);
      }
    }
    double temp = arr->GetValue(--left);
    arr->SetValue(left, arr->GetValue(start));
    arr->SetValue(start, temp);
    quicksort(arr, start, left);
    quicksort(arr, right, end);
  }
}

double get_median(vtkSmartPointer<vtkDoubleArray> arr) {
  int n = arr->GetNumberOfTuples();
  quicksort(arr, 0, n);

  if (n % 2 != 0)
    return arr->GetValue(n / 2);
  else
    return (arr->GetValue((n - 1) / 2) + arr->GetValue(n / 2)) / 2.0;
}

double get_sum(vtkSmartPointer<vtkDoubleArray> arr) {
  double sum = 0.0;
  for(vtkIdType i = 0; i < arr->GetNumberOfTuples(); ++i) {
    double value = arr->GetValue(i);
    if (!std::isnan(value)) // Check if the value is not NaN
        sum += value;
  }
  return sum;
}

double get_mean(vtkSmartPointer<vtkDoubleArray> arr) {
  int n = arr->GetNumberOfTuples();
  return get_sum(arr) / static_cast<double>(n);
}

double get_std(vtkSmartPointer<vtkDoubleArray> arr) {
  int n = arr->GetNumberOfTuples();
  double mean = get_mean(arr);
  double variance = 0.0;
  
  for(vtkIdType i = 0; i < arr->GetNumberOfTuples(); ++i) {
    double value = arr->GetValue(i);
    if (!std::isnan(value)) // Check if the value is not NaN
      variance += (value - mean) * (value - mean);
  }
  variance /= static_cast<double>(n);

  return std::sqrt(variance);
}

