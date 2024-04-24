#ifndef vtkUtils_h
#define vtkUtils_h

#include <vtkRenderer.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h> 
#include <vtkLookupTable.h>
#include <vtkLookupTableWithEnabling.h>
#include <vtkScalarBarActor.h>
#include <vtkTextProperty.h>
#include <vtkColorTransferFunction.h>
#include <vtksys/SystemTools.hxx>
#include "vtkSurfaceReader.h"
#include "vtkStat.h"
#include <filesystem>

namespace fs = std::__fs::filesystem;
using namespace std;

typedef enum ColorMap {
    C1, C2, C3, JET, GRAY, REDYELLOW, BLUECYAN, YELLOWRED, CYANBLUE, BLUEGREEN, GREENBLUE
} ColorMap;

static double defaultScalarRange[2] = { 0.0, -1.0 };
static double defaultScalarRangeBkg[2] = { 0.0, -1.0 };
static double defaultAlpha = 0.8;
static double defaultClipRange[2] = { 0.0, -1.0 };
static double defaultRotate[3] = { 270.0, 0.0, -90.0 };
static int defaultColorbar = 0;
static int defaultInverse = 0;
static int defaultBkg = 0;
static int defaultWindowSize[2] = { 1800, 1000 };

void usage(const char* const prog);
void ReadBackgroundScalars(const char* overlayFileNameBkgL, vtkSmartPointer<vtkPolyData> polyData[], 
    vtkSmartPointer<vtkDoubleArray> scalarsBkg[], int nMeshes, bool& rhExists, 
    string& rhSurfName);
void ReadAndUpdateScalars(string overlayFileNameL, vtkSmartPointer<vtkPolyData> polyData[], 
    vtkSmartPointer<vtkDoubleArray> scalars[], int nMeshes, bool rhExists, 
    const string& rhSurfName, bool inverse, double clipRange[], fs::path currentPath, 
    double overlayRange[]);
void UpdateScalarBarAndLookupTable(int n1, int n2, vtkSmartPointer<vtkDoubleArray> scalars[], 
    vtkLookupTable* lookupTable, vtkLookupTable* lookupTableColorBar, const double overlayRange[], 
    const double clipRange[], int colorbar, bool bkgWhite, int fontSize, bool logColorbar, 
    bool printStats, vtkRenderer* renderer, const char* Title, double alpha);
vtkSmartPointer<vtkLookupTableWithEnabling> getLookupTable(int colormap, double alpha);

#endif // vtkUtils_h
