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

    // Getting data range directly from the image
    double range[2];
    reader->GetOutput()->GetScalarRange(range);

    vtkSmartPointer<vtkRenderer> axialRenderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderer> coronalRenderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderer> sagittalRenderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    
    vtkSmartPointer<vtkTransform> transforms[3];
    transforms[0] = vtkSmartPointer<vtkTransform>::New();
    transforms[0]->Identity();  // Axial

    transforms[1] = vtkSmartPointer<vtkTransform>::New();
    transforms[1] ->Translate(reader->GetOutput()->GetExtent()[1]*0.5, reader->GetOutput()->GetExtent()[3]*0.5, 0);
    transforms[1] ->RotateY(90);
    transforms[1] ->RotateX(-90);
    transforms[1] ->Translate(-reader->GetOutput()->GetExtent()[3]*0.5, -reader->GetOutput()->GetExtent()[1]*0.5, 0);


    transforms[2] = vtkSmartPointer<vtkTransform>::New();
    transforms[2] ->Translate(reader->GetOutput()->GetExtent()[1]*0.5, reader->GetOutput()->GetExtent()[3]*0.5, 0);
    transforms[2] ->RotateX(90);
    transforms[2] ->RotateY(180);
    transforms[2] ->Translate(-reader->GetOutput()->GetExtent()[3]*0.5, -reader->GetOutput()->GetExtent()[1]*0.5, 0);

    vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
    lut->SetHueRange(0.0, 0.0); 
    lut->SetRange(range[0], range[1]);  // Set range to the image data range
    lut->SetValueRange(0.0, 1.0);
    lut->SetSaturationRange(0.0, 0.0);
    lut->Build();

    // Setup for Axial
    vtkSmartPointer<vtkImageSliceMapper> axialMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
    axialMapper->SetInputConnection(reader->GetOutputPort());
    axialMapper->SetSliceNumber(50);
    vtkSmartPointer<vtkImageActor> axialActor = vtkSmartPointer<vtkImageActor>::New();
    axialActor->SetMapper(axialMapper);
    
    axialActor->SetUserTransform(transforms[0]);
    axialRenderer->SetViewport(0.0, 0.0, 0.33, 1.0);
    axialRenderer->ResetCamera();  // Auto adjust the camera based on the actor
    axialRenderer->AddActor(axialActor);

    // Create a permute filter
    vtkSmartPointer<vtkImagePermute> permuteCoronal = vtkSmartPointer<vtkImagePermute>::New();
    permuteCoronal->SetInputConnection(reader->GetOutputPort());
    permuteCoronal->SetFilteredAxes(2, 1, 0); // New order of axes: Z, Y, X
    permuteCoronal->Update();

    // Setup for Coronal
    vtkSmartPointer<vtkImageSliceMapper> coronalMapper = vtkSmartPointer<vtkImageSliceMapper>::New();

    coronalMapper->SetInputConnection(reader->GetOutputPort());
    coronalMapper->SetOrientationToX();
    coronalMapper->SetSliceNumber(50);
    vtkSmartPointer<vtkImageActor> coronalActor = vtkSmartPointer<vtkImageActor>::New();
    coronalActor->SetMapper(coronalMapper);
    
    coronalActor->SetUserTransform(transforms[1]);
    coronalRenderer->SetViewport(0.33, 0.0, 0.66, 1.0);
    coronalRenderer->ResetCamera();  // Auto adjust the camera based on the actor
    coronalRenderer->AddActor(coronalActor);
    
    // Setup for Sagittal
    vtkSmartPointer<vtkImagePermute> permuteSagiital = vtkSmartPointer<vtkImagePermute>::New();
    permuteSagiital->SetInputConnection(reader->GetOutputPort());
    permuteSagiital->SetFilteredAxes(1, 0, 2); 
    permuteSagiital->Update();

    vtkSmartPointer<vtkImageSliceMapper> sagittalMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
    sagittalMapper->SetInputConnection(reader->GetOutputPort());
    sagittalMapper->SetOrientationToY();
    sagittalMapper->SetSliceNumber(80);
    vtkSmartPointer<vtkImageActor> sagittalActor = vtkSmartPointer<vtkImageActor>::New();
    sagittalActor->SetMapper(sagittalMapper);
    
    sagittalActor->SetUserTransform(transforms[2]);
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
