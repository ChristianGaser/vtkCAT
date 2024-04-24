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

    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetRenderWindow(renderWindow);

    vtkSmartPointer<vtkTransform> transforms[3];
    transforms[0] = vtkSmartPointer<vtkTransform>::New();
    transforms[0]->Identity();  // Axial

    transforms[1] = vtkSmartPointer<vtkTransform>::New();
    transforms[1]->RotateX(90);  // Coronal

    transforms[2] = vtkSmartPointer<vtkTransform>::New();
    transforms[2]->RotateY(90);  // Sagittal

    vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
    lut->SetRange(range[0], range[1]);  // Set range to the image data range
    lut->SetValueRange(0.0, 1.0);
    lut->SetSaturationRange(0.0, 0.0);
    lut->Build();

    for (int i = 0; i < 3; i++) {
        vtkSmartPointer<vtkImageReslice> reslice = vtkSmartPointer<vtkImageReslice>::New();
        reslice->SetInputConnection(reader->GetOutputPort());
        reslice->SetOutputDimensionality(2);
        reslice->SetResliceAxes(transforms[i]->GetMatrix());

        vtkSmartPointer<vtkImageMapToColors> colorMap = vtkSmartPointer<vtkImageMapToColors>::New();
        colorMap->SetLookupTable(lut);
        colorMap->SetInputConnection(reslice->GetOutputPort());

        vtkSmartPointer<vtkImageActor> actor = vtkSmartPointer<vtkImageActor>::New();
        actor->SetInputData(colorMap->GetOutput());
        renderer->AddActor(actor);
    }

    renderer->ResetCamera();
    renderWindow->SetSize(1800, 1000);
    renderWindow->Render();
    renderWindowInteractor->Initialize();
    renderWindowInteractor->Start();

    return EXIT_SUCCESS;
}
