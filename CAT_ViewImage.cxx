#include <vtkSmartPointer.h>
#include <vtkNIFTIImageReader.h>
#include <vtkImageReslice.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>
#include <vtkLookupTable.h>
#include <vtkImageMapToColors.h>
#include <vtkImageData.h>
#include <vtkImageActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkCamera.h> 
#include <vtkImagePermute.h>

#include <vtkImageSliceMapper.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <NIfTIFile>" << std::endl;
        return EXIT_FAILURE;
    }

    vtkSmartPointer<vtkNIFTIImageReader> reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
    reader->SetFileName(argv[1]);
    reader->Update();

    // Accessing the transformation matrices
    vtkSmartPointer<vtkMatrix4x4> qFormMatrix = reader->GetQFormMatrix();
    vtkSmartPointer<vtkMatrix4x4> sFormMatrix = reader->GetSFormMatrix();

    // Getting data range directly from the image
    double range[2];
    reader->GetOutput()->GetScalarRange(range);

    vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
    lut->SetHueRange(0.0, 0.0); 
    lut->SetRange(1.2*range[0], 0.8*range[1]);  // Set range to the image data range
    lut->SetValueRange(0.0, 1.0);
    lut->SetSaturationRange(0.0, 0.0);
    lut->Build();

    vtkSmartPointer<vtkImageMapToColors> colorMap = vtkSmartPointer<vtkImageMapToColors>::New();
    colorMap->SetLookupTable(lut);
    colorMap->SetInputConnection(reader->GetOutputPort());

    // Visualization setup
    vtkSmartPointer<vtkRenderer> axialRenderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderer> coronalRenderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderer> sagittalRenderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    
    // Setup for Axial
    vtkSmartPointer<vtkImageSliceMapper> axialMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
    axialMapper->SetInputConnection(colorMap->GetOutputPort());
    axialMapper->SetSliceNumber(50);
    
    vtkSmartPointer<vtkImageActor> axialActor = vtkSmartPointer<vtkImageActor>::New();
    axialActor->SetMapper(axialMapper);
    
    axialRenderer->SetViewport(0.0, 0.0, 0.33, 1.0);
    axialRenderer->ResetCamera();  // Auto adjust the camera based on the actor
    axialRenderer->AddActor(axialActor);

    // Setup for Coronal using permute filter
    vtkSmartPointer<vtkImagePermute> permuteCoronal = vtkSmartPointer<vtkImagePermute>::New();
    permuteCoronal->SetInputConnection(colorMap->GetOutputPort());
    permuteCoronal->SetFilteredAxes(2, 1, 0); // New order of axes: Z, Y, X
    permuteCoronal->Update();

    // Setup for Coronal
    vtkSmartPointer<vtkImageSliceMapper> coronalMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
    coronalMapper->SetInputConnection(permuteCoronal->GetOutputPort());
    coronalMapper->SetSliceNumber(50);
    
    vtkSmartPointer<vtkImageActor> coronalActor = vtkSmartPointer<vtkImageActor>::New();
    coronalActor->SetMapper(coronalMapper);
    //coronalActor->SetUserTransform(transforms[1]);
    
    coronalRenderer->SetViewport(0.33, 0.0, 0.66, 1.0);
    coronalRenderer->ResetCamera();  // Auto adjust the camera based on the actor
    coronalRenderer->AddActor(coronalActor);
    
    // Setup for Sagittal using permute filter
    vtkSmartPointer<vtkImagePermute> permuteSagittal = vtkSmartPointer<vtkImagePermute>::New();
    permuteSagittal->SetInputConnection(colorMap->GetOutputPort());
    permuteSagittal->SetFilteredAxes(1, 0, 2);  // Adjust axes for sagittal view
    permuteSagittal->Update();

    vtkSmartPointer<vtkImageSliceMapper> sagittalMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
    sagittalMapper->SetInputConnection(permuteSagittal->GetOutputPort());
    sagittalMapper->SetSliceNumber(80);
    
    vtkSmartPointer<vtkImageActor> sagittalActor = vtkSmartPointer<vtkImageActor>::New();
    sagittalActor->SetMapper(sagittalMapper);
    
    sagittalRenderer->SetViewport(0.66, 0.0, 1.0, 1.0);
    sagittalRenderer->ResetCamera();  // Auto adjust the camera based on the actor
    sagittalRenderer->AddActor(sagittalActor);
    
    renderWindow->AddRenderer(axialRenderer);
    renderWindow->AddRenderer(coronalRenderer);
    renderWindow->AddRenderer(sagittalRenderer);

    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    vtkSmartPointer<vtkInteractorStyleImage> style = vtkSmartPointer<vtkInteractorStyleImage>::New();

    renderWindowInteractor->SetInteractorStyle(style);

    renderWindowInteractor->SetRenderWindow(renderWindow);

    renderWindow->SetSize(1800, 1000);
    renderWindow->Render();
    renderWindowInteractor->Start();

    return EXIT_SUCCESS;
}
