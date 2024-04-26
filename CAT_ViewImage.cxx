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
    
    vtkSmartPointer<vtkTransform> transforms[3];
    transforms[0] = vtkSmartPointer<vtkTransform>::New();
    transforms[0]->Identity();  // Axial

    transforms[1] = vtkSmartPointer<vtkTransform>::New();
    transforms[1]->RotateX(90);  // Coronal

    transforms[2] = vtkSmartPointer<vtkTransform>::New();
    transforms[2]->RotateY(90);  // Sagittal

    vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
    lut->SetHueRange(0.0, 0.0); 
    lut->SetRange(range[0], range[1]);  // Set range to the image data range
    lut->SetValueRange(0.0, 1.0);
    lut->SetSaturationRange(0.0, 0.0);
    lut->Build();

if (0) {
    for (int i = 0; i < 3; i++) {
        vtkSmartPointer<vtkImageReslice> reslice = vtkSmartPointer<vtkImageReslice>::New();
        reslice->SetInputConnection(reader->GetOutputPort());
        reslice->SetOutputDimensionality(3);
        reslice->SetResliceAxes(transforms[i]->GetMatrix());

    vtkSmartPointer<vtkImageSliceMapper> imageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
    imageSliceMapper->SetInputConnection(reslice->GetOutputPort());
//    imageSliceMapper->SetSliceNumber(reslice->GetOutput()->GetExtent()[10]); // Middle slice in the Z-direction
    imageSliceMapper->SetSliceNumber(50); // Middle slice in the Z-direction

    vtkSmartPointer<vtkImageActor> imageActor = vtkSmartPointer<vtkImageActor>::New();
    imageActor->SetMapper(imageSliceMapper);
//    renderer->AddActor(imageActor);
/*
        vtkSmartPointer<vtkImageActor> actor = vtkSmartPointer<vtkImageActor>::New();
        vtkSmartPointer<vtkImageMapToColors> colorMap = vtkSmartPointer<vtkImageMapToColors>::New();
        colorMap->SetLookupTable(lut);
        colorMap->SetInputConnection(reslice->GetOutputPort());
        actor->SetInputData(colorMap->GetOutput());
        renderer->AddActor(actor);
*/

//    renderer->SetBackground(0.5, 0.5, 0.5); // Dark grey background
//    renderer->ResetCamera();

    }
} else {

    // Setup for Axial
    vtkSmartPointer<vtkImageSliceMapper> axialMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
    axialMapper->SetInputConnection(reader->GetOutputPort());
    axialMapper->SetSliceNumber(30);
    vtkSmartPointer<vtkImageActor> axialActor = vtkSmartPointer<vtkImageActor>::New();
    axialActor->SetMapper(axialMapper);
    
    axialRenderer->AddActor(axialActor);
    axialRenderer->SetViewport(0.0, 0.0, 0.33, 1.0);
    axialRenderer->ResetCamera();  // Auto adjust the camera based on the actor

    // Setup for Coronal
    vtkSmartPointer<vtkImageSliceMapper> coronalMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
    coronalMapper->SetInputConnection(reader->GetOutputPort());
    coronalMapper->SetOrientationToX();
    coronalMapper->SetSliceNumber(40);
    vtkSmartPointer<vtkImageActor> coronalActor = vtkSmartPointer<vtkImageActor>::New();
    coronalActor->SetMapper(coronalMapper);
    
    coronalRenderer->AddActor(coronalActor);
    coronalRenderer->SetViewport(0.33, 0.0, 0.66, 1.0);
    vtkCamera* coronalCamera = coronalRenderer->GetActiveCamera();
    coronalCamera->Yaw(-90);  // Rotate 90 degrees around the vertical axis

    // Setup for Sagittal
    vtkSmartPointer<vtkImageSliceMapper> sagittalMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
    sagittalMapper->SetInputConnection(reader->GetOutputPort());
    sagittalMapper->SetOrientationToY();
    sagittalMapper->SetSliceNumber(50);
    vtkSmartPointer<vtkImageActor> sagittalActor = vtkSmartPointer<vtkImageActor>::New();
    sagittalActor->SetMapper(sagittalMapper);
    sagittalRenderer->SetViewport(0.66, 0.0, 1.0, 1.0);
    vtkCamera* sagittalCamera = sagittalRenderer->GetActiveCamera();
    sagittalCamera->Elevation(-90);  // Rotate 90 degrees around the vertical axis
    
    sagittalRenderer->AddActor(sagittalActor);
}


    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(axialRenderer);
    renderWindow->AddRenderer(coronalRenderer);
    renderWindow->AddRenderer(sagittalRenderer);

    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetRenderWindow(renderWindow);

    renderWindow->SetSize(1800, 1000);
    renderWindow->Render();
    renderWindowInteractor->Initialize();
    renderWindowInteractor->Start();

    return EXIT_SUCCESS;
}
