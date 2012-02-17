/*=========================================================================

Program:   vtkINRIA3D
Module:    $Id$
Language:  C++
Author:    $Author$
Date:      $Date$
Version:   $Revision$

Copyright (c) 2007 INRIA - Asclepios Project. All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
// version vtkRenderingAddOn
#include <vtkViewImage3D.h>

#ifndef VTK_MAJOR_VERSION
#  include "vtkVersion.h"
#endif

#include <vtkInteractorStyleTrackball.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkImageData.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkVolumeTextureMapper3D.h>
#include <vtkVolumeMapper.h>
#include <vtkFiniteDifferenceGradientEstimator.h>
#include <vtkVolumeTextureMapper2D.h>
#include <vtkProperty.h>
#include <vtkVolume.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkImageDataGeometryFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkImageActor.h>
#include <vtkAxes.h>
#include <vtkMatrix4x4.h>
#include <vtkTubeFilter.h>
#include <vtkLookupTable.h>
#include <vtkAnnotatedCubeActor.h>
#include <vtkPropAssembly.h>
#include <vtkAxesActor.h>
#include <vtkTextProperty.h>
#include <vtkCaptionActor2D.h>
#include <vtkPointData.h>
#include <vtkImageBlend.h>
#include <vtkImageReslice.h>
#include "vtkRenderWindow.h"
#include "vtkScalarsToColors.h"
#include <vtkDataSet3DCroppingPlaneCallback.h>
#include "vtkColorTransferFunction.h"
#include <vtkCamera.h>
#include <vtkImageShiftScale.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkPlane.h>
#include <vtkPlaneCollection.h>
#include <vtkPolyDataMapper.h>
#include <vtkGeometryFilter.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkCellData.h>


#include <iostream>


extern int vtkrint(double a);


vtkCxxRevisionMacro(vtkViewImage3D, "$Revision$");
vtkStandardNewMacro(vtkViewImage3D);


vtkViewImage3D::vtkViewImage3D ()
{

  this->OverlappingImage = 0;
  
  // Volume Rendering stuff
  this->VolumeProperty  = vtkVolumeProperty::New(); 
  this->VolumeActor     = vtkVolume::New();
  this->VolumeRayCastMapper             = vtkVolumeRayCastMapper::New();
  this->VolumeRayCastMIPFunction        = vtkVolumeRayCastMIPFunction::New();
  this->VolumeRayCastCompositeFunction  = vtkVolumeRayCastCompositeFunction::New();
  this->VolumeRayCastIsosurfaceFunction = vtkVolumeRayCastIsosurfaceFunction::New();
  this->OpacityFunction                 = vtkPiecewiseFunction::New();
  this->ColorFunction                   = vtkColorTransferFunction::New(); 
  
				
  // Multiplanar reconstruction stuff
  this->AxialColorMapper       = vtkImageMapToColors::New();
  this->SagittalColorMapper    = vtkImageMapToColors::New();
  this->CoronalColorMapper     = vtkImageMapToColors::New();
  this->ActorSagittal          = vtkImageActor::New();
  this->ActorCoronal           = vtkImageActor::New();
  this->ActorAxial             = vtkImageActor::New();
  this->Axes                   = vtkAxes::New();
  this->TubeFilter             = vtkTubeFilter::New();
  this->AxesActor              = vtkActor::New();
  this->BoxWidget              = vtkBoxWidget::New();
  this->Callback               = vtkImage3DCroppingBoxCallback::New();
  this->DataSetCallback        = vtkDataSet3DCroppingPlaneCallback::New();
  this->AxialBlender           = vtkImageBlend::New();
  this->SagittalBlender        = vtkImageBlend::New();
  this->CoronalBlender         = vtkImageBlend::New();
  this->Blender                = vtkImageBlend::New();
  this->PlaneWidget            = vtkPlaneWidget::New();
  
  
  this->RenderingMode = PLANAR_RENDERING;
  this->Visibility          = true;
  this->BoxWidgetVisibility = true;
  this->AxesVisibility      = false;
  
  
  // set up 3D axes
  this->Axes->SetOrigin (0,0,0);
  this->Axes->SymmetricOn();
  this->Axes->SetScaleFactor (50.0);
  this->TubeFilter->SetInputConnection( this->Axes->GetOutputPort() );
  this->TubeFilter->SetRadius(1.0);
  this->TubeFilter->SetNumberOfSides(6);  
  vtkPolyDataMapper* axesMapper = vtkPolyDataMapper::New();
  axesMapper->SetInputConnection ( this->TubeFilter->GetOutputPort() );
  this->Axes->Update();
  this->Axes->GetOutput()->GetPointData()->GetScalars()->SetTuple1 (0, 0.0);
  this->Axes->GetOutput()->GetPointData()->GetScalars()->SetTuple1 (1, 0.0);
  this->Axes->GetOutput()->GetPointData()->GetScalars()->SetTuple1 (2, 0.5);
  this->Axes->GetOutput()->GetPointData()->GetScalars()->SetTuple1 (3, 0.5);
  this->Axes->GetOutput()->GetPointData()->GetScalars()->SetTuple1 (4, 1.0);
  this->Axes->GetOutput()->GetPointData()->GetScalars()->SetTuple1 (5, 1.0);    
  this->AxesActor->SetMapper (axesMapper);
  axesMapper->Delete();
  

  // set up the filters (properties, etc.)
  this->AxialBlender->SetBlendModeToNormal();
  this->AxialBlender->SetOpacity (0, 0.25);
  this->AxialBlender->SetOpacity (1, 0.75);


  this->SagittalBlender->SetBlendModeToNormal();
  this->SagittalBlender->SetOpacity (0, 0.25);
  this->SagittalBlender->SetOpacity (1, 0.75);


  this->CoronalBlender->SetBlendModeToNormal();
  this->CoronalBlender->SetOpacity (0, 0.25);
  this->CoronalBlender->SetOpacity (1, 0.75);


  this->Blender->SetBlendModeToNormal();
  this->Blender->SetOpacity (0, 0.25);
  this->Blender->SetOpacity (1, 0.75);

  
  vtkVolumeTextureMapper3D* mapper3D = vtkVolumeTextureMapper3D::New();
  mapper3D->SetSampleDistance(0.5);
  mapper3D->SetPreferredMethodToNVidia();
  mapper3D->CroppingOn();
  mapper3D->SetCroppingRegionFlags (0x7ffdfff);

  this->VolumeMapper3D = mapper3D;

  this->VolumeRayCastMapper->SetSampleDistance(2.0);
  this->VolumeRayCastMapper->CroppingOn();
  this->VolumeRayCastMapper->SetCroppingRegionFlags (0x7ffdfff);
  vtkFiniteDifferenceGradientEstimator *gradest = 
    vtkFiniteDifferenceGradientEstimator::New();
  this->VolumeRayCastMapper->SetGradientEstimator(gradest);
  gradest->Delete();
  this->VolumeRayCastMapper->SetVolumeRayCastFunction ( this->VolumeRayCastCompositeFunction );
    

  this->OpacityFunction->AddPoint (0, 0.0);
  this->OpacityFunction->AddPoint (255, 1.0);
  this->ColorFunction->AddRGBPoint (0, 0.0, 0.0, 0.0);
  this->ColorFunction->AddRGBPoint (255, 1.0, 1.0, 1.0);


  this->VolumeProperty->IndependentComponentsOff();
  this->VolumeProperty->SetColor (this->ColorFunction);
  this->ColorFunction->Delete();
  this->VolumeProperty->SetScalarOpacity(this->OpacityFunction);
  this->VolumeProperty->SetInterpolationTypeToLinear();
  this->VolumeProperty->ShadeOff();
  this->VolumeProperty->SetDiffuse (0.9);
  this->VolumeProperty->SetAmbient (0.2);
  this->VolumeProperty->SetSpecular (0.3);
  this->VolumeProperty->SetSpecularPower (15.0);
  
  
  // set up the vtk pipeline: volume rendering
  this->VolumeActor->SetProperty ( this->VolumeProperty );
  this->VolumeActor->SetMapper ( this->VolumeMapper3D );
  this->VolumeActor->PickableOff();
  
  
  // set up the boxwidget/ callback
  this->Callback->SetVolumeMapper ( this->VolumeMapper3D );
  this->DataSetCallback->SetPlaneWidget ( this->PlaneWidget );
  
  this->BoxWidget->RotationEnabledOff();
  this->BoxWidget->SetPlaceFactor (0.5);
  this->BoxWidget->SetKeyPressActivationValue ('b');
  this->BoxWidget->AddObserver (vtkCommand::InteractionEvent, this->Callback);
  //this->PlaneWidget->AddObserver (vtkCommand::InteractionEvent, this->DataSetCallback);
  

  this->PlaneWidget->SetKeyPressActivationValue ('p');

  
  // init the interactor style:
  /*
  vtkInteractorStyleSwitch* interactorStyle = vtkInteractorStyleSwitch::New();
  interactorStyle->SetCurrentStyleToTrackballCamera();
  this->SetInteractorStyle (interactorStyle);
  interactorStyle->Delete();
  */


  
  // Create an annotated cube actor (directions)
  this->Cube = vtkAnnotatedCubeActor::New();
  this->Cube->SetXPlusFaceText ("L");
  this->Cube->SetXMinusFaceText ("R");
  this->Cube->SetYPlusFaceText ("P");
  this->Cube->SetYMinusFaceText ("A");
  this->Cube->SetZPlusFaceText ("S");
  this->Cube->SetZMinusFaceText ("I");
  this->Cube->SetZFaceTextRotation (90);
  this->Cube->SetFaceTextScale (0.65);
  this->Cube->GetCubeProperty()->SetColor (0.5, 1, 1);
  this->Cube->GetTextEdgesProperty()->SetLineWidth (1);
  this->Cube->GetTextEdgesProperty()->SetDiffuse (0);
  this->Cube->GetTextEdgesProperty()->SetAmbient (1);
  this->Cube->GetTextEdgesProperty()->SetColor (0.18, 0.28, 0.23);

#if VTK_MAJOR_VERSION==5 && VTK_MINOR_VERSION>=1
  this->Cube->SetTextEdgesVisibility (1);  
  this->Cube->SetCubeVisibility(1);  
  this->Cube->SetFaceTextVisibility(1);
#else
  this->Cube->TextEdgesOn ();
  this->Cube->FaceTextOn();
  this->Cube->CubeOn();
#endif

  this->Cube->GetXPlusFaceProperty()->SetColor (1, 0, 0);
  this->Cube->GetXPlusFaceProperty()->SetInterpolationToFlat();
  this->Cube->GetXMinusFaceProperty()->SetColor (1, 0, 0);
  this->Cube->GetXMinusFaceProperty()->SetInterpolationToFlat();
  this->Cube->GetYPlusFaceProperty()->SetColor (0, 1, 0);
  this->Cube->GetYPlusFaceProperty()->SetInterpolationToFlat();
  this->Cube->GetYMinusFaceProperty()->SetColor (0, 1, 0);
  this->Cube->GetYMinusFaceProperty()->SetInterpolationToFlat();
  this->Cube->GetZPlusFaceProperty()->SetColor (0, 0, 1);
  this->Cube->GetZPlusFaceProperty()->SetInterpolationToFlat();
  this->Cube->GetZMinusFaceProperty()->SetColor (0, 0, 1);
  this->Cube->GetZMinusFaceProperty()->SetInterpolationToFlat();

  /*
    this->CubeAxes = vtkAxesActor::New(); 
    this->CubeAxes->SetShaftTypeToCylinder();
    this->CubeAxes->SetXAxisLabelText ("x");
    this->CubeAxes->SetYAxisLabelText ("y");
    this->CubeAxes->SetZAxisLabelText ("z");
    this->CubeAxes->SetTotalLength (1.5, 1.5, 1.5);
    vtkTextProperty* tprop = vtkTextProperty::New();
    tprop->ItalicOn();
    tprop->ShadowOn();
    tprop->SetFontFamilyToTimes();
    this->CubeAxes->GetXAxisCaptionActor2D()->SetCaptionTextProperty (tprop);
    this->CubeAxes->GetYAxisCaptionActor2D()->SetCaptionTextProperty (tprop);
    this->CubeAxes->GetZAxisCaptionActor2D()->SetCaptionTextProperty (tprop);
  */

  /*
    vtkPropAssembly *assembly = vtkPropAssembly::New();
    assembly->AddPart (this->CubeAxes);
    assembly->AddPart (this->Cube);
  */
  
  this->Marker = vtkOrientationMarkerWidget::New();
  this->Marker->SetOutlineColor (0.93, 0.57, 0.13);
  //this->Marker->SetOrientationMarker (assembly);
  this->Marker->SetOrientationMarker (this->Cube);
  this->Marker->SetViewport (0.0, 0.05, 0.15, 0.15);
  
  //assembly->Delete();
  //tprop->Delete();
  

  this->VolumeActor->SetVisibility (0);
  this->AxesActor->SetVisibility (0);

  
    
  this->ActorSagittal->SetVisibility (this->Visibility);
  this->ActorCoronal->SetVisibility (this->Visibility);
  this->ActorAxial->SetVisibility (this->Visibility);

  this->GetScalarBar()->GetLabelTextProperty()->SetColor (0.0,0.0,0.0);
  this->GetScalarBar()->GetTitleTextProperty()->SetColor (0.0,0.0,0.0);

  this->ShowAxial = 1;
  this->ShowCoronal = 1;
  this->ShowSagittal = 1;

  this->FirstImage = 1;
}


vtkViewImage3D::~vtkViewImage3D()
{
  this->Marker->SetEnabled (0);
  this->BoxWidget->SetEnabled (0);
  this->PlaneWidget->SetEnabled (0);

  this->BoxWidget->RemoveObserver (this->Callback);

  // delete all vtk objetcts:
  this->VolumeMapper3D->Delete();
  this->VolumeProperty->Delete();
  this->VolumeActor->Delete();
  this->OpacityFunction->Delete();
  //this->ColorFunction->Delete();
  this->AxialColorMapper->Delete();
  this->SagittalColorMapper->Delete();
  this->CoronalColorMapper->Delete();
  this->ActorSagittal->Delete();
  this->ActorCoronal->Delete();
  this->ActorAxial->Delete();
  this->Axes->Delete();
  this->TubeFilter->Delete();
  this->AxesActor->Delete();
  this->BoxWidget->Delete();
  this->Callback->Delete();
  this->DataSetCallback->Delete();
  this->Cube->Delete();
  //this->CubeAxes->Delete();
  this->Marker->Delete();
  this->AxialBlender->Delete();
  this->CoronalBlender->Delete();
  this->SagittalBlender->Delete();
  this->Blender->Delete();
  this->PlaneWidget->Delete();
  this->VolumeRayCastMapper->Delete();
  this->VolumeRayCastMIPFunction->Delete();
  this->VolumeRayCastCompositeFunction->Delete();
  this->VolumeRayCastIsosurfaceFunction->Delete();
}


void vtkViewImage3D::Initialize()
{
	vtkViewImage::Initialize();

	if( this->GetRenderWindowInteractor() )
	{
      this->BoxWidget->SetInteractor ( this->GetRenderWindowInteractor() );
	  this->PlaneWidget->SetInteractor ( this->GetRenderWindowInteractor() );
	  this->Marker->SetInteractor ( this->GetRenderWindowInteractor() );

	  vtkInteractorStyleSwitch* interactorStyle = vtkInteractorStyleSwitch::New();
	  interactorStyle->SetCurrentStyleToTrackballCamera();
      this->GetRenderWindowInteractor()->SetInteractorStyle (interactorStyle);
      interactorStyle->Delete();
	}
}

void vtkViewImage3D::Uninitialize()
{
  this->BoxWidget->SetInteractor (NULL);
  this->Marker->SetInteractor ( NULL );
  this->PlaneWidget->SetInteractor ( NULL );

  if (this->GetRenderWindowInteractor())
	  this->GetRenderWindowInteractor()->SetInteractorStyle( NULL );

  this->RemoveActor (this->ActorSagittal);
  this->RemoveActor (this->ActorCoronal);
  this->RemoveActor (this->ActorAxial);
  
  this->RemoveActor (this->AxesActor);
  this->RemoveActor (this->VolumeActor);
  for (unsigned int i=0; i<this->DataSetActorList.size(); i++)
  {
	this->RemoveActor( this->DataSetActorList[i] );
  }

  vtkViewImage::Uninitialize();
}



void vtkViewImage3D::PrepareForDelete()
{
  this->SetRenderingModeToPlanar();
  this->PlaneWidget->Off();
}


void vtkViewImage3D::SetImage ( vtkImageData* image )
{
  if( !image )
  {
    return;
  }

  int* extent = image->GetExtent();
  if( extent[1]<extent[0] || extent[3]<extent[2] || extent[5]<extent[4] )
  {
    vtkErrorMacro ( << "Image extent is not valid: " << extent[0] << " "
		    << extent[1] << " "
		    << extent[2] << " "
		    << extent[3] << " "
		    << extent[4] << " "
		    << extent[5]);
    return;
  }
  

  if (!this->GetRenderWindow() || !this->GetRenderWindowInteractor())
  {
    return;
  }
  
  this->RegisterImage (image);

  this->VolumeMapper3D->SetInput ( this->GetImage() );
  this->VolumeRayCastMapper->SetInput ( this->GetImage() );

  /*
    double* spacing = this->GetImage()->GetSpacing();
    double  sampleDistance = 0.33*0.1*(spacing[0]+spacing[1]+spacing[2]);
  */

  vtkVolumeTextureMapper3D* mapper3D = vtkVolumeTextureMapper3D::SafeDownCast ( this->VolumeActor->GetMapper() );

  if( mapper3D && !this->GetRenderWindow()->GetNeverRendered() )
  {
    if( !mapper3D->IsRenderSupported ( this->VolumeProperty
#if (VTK_MAJOR_VERSION == 5 && ((VTK_MINOR_VERSION >= 6 && VTK_BUILD_VERSION >=1) || VTK_MINOR_VERSION >= 7)) || VTK_MAJOR_VERSION > 5
					 , this->GetRenderer()
#endif
	    ) )
    {
      //try the ATI fragment program implementation
      mapper3D->SetPreferredMethodToFragmentProgram();
      if ( !mapper3D->IsRenderSupported ( this->VolumeProperty
#if (VTK_MAJOR_VERSION == 5 && ((VTK_MINOR_VERSION >= 6 && VTK_BUILD_VERSION >=1) || VTK_MINOR_VERSION >= 7)) || VTK_MAJOR_VERSION > 5
				       , this->GetRenderer()
#endif
))
      {
        vtkWarningMacro (<<"Warning: 3D texture volume rendering is not supported by your hardware, switching to 2D texture rendering."<<endl);

	vtkVolumeTextureMapper2D* newMapper = vtkVolumeTextureMapper2D::New();
        newMapper->CroppingOn();
        newMapper->SetCroppingRegionFlags (0x7ffdfff);
        

	double* range = this->GetImage()->GetScalarRange();
	double shift = 0 - range[0];
	double scale = 65535.0/(range[1] - range[0]);
	
	vtkImageShiftScale* scaler = vtkImageShiftScale::New();
	scaler->SetInput (image);
	scaler->SetShift (shift);
	scaler->SetScale (scale);
	scaler->SetOutputScalarTypeToUnsignedShort();
	
	scaler->Update();
	
	this->SetShift(scaler->GetShift());
	this->SetScale(scaler->GetScale());
	newMapper->SetInput ( scaler->GetOutput() );

	scaler->Delete();
	
	this->Callback->SetVolumeMapper (newMapper);
        
        mapper3D->Delete();
        this->VolumeMapper3D = newMapper;
        this->VolumeActor->SetMapper ( this->VolumeMapper3D );
      }    
    }    
  }

  
  // Before all: remove the overlapping image (if any)
  this->RemoveOverlappingImage();
  
  // If an image is already of type unsigned char, there is no
  // need to map it through a lookup table
  if( this->GetImage()->GetScalarType() == VTK_UNSIGNED_CHAR )
  {
    this->ActorSagittal->SetInput ( this->GetImage() );
    this->ActorCoronal->SetInput  ( this->GetImage() );
    this->ActorAxial->SetInput    ( this->GetImage() );
  }
  else
  {
    this->AxialColorMapper->SetInput    (this->GetImage());
    this->SagittalColorMapper->SetInput (this->GetImage());
    this->CoronalColorMapper->SetInput  (this->GetImage());
    
    this->ActorSagittal->SetInput (this->SagittalColorMapper->GetOutput());
    this->ActorCoronal->SetInput (this->CoronalColorMapper->GetOutput());
    this->ActorAxial->SetInput (this->AxialColorMapper->GetOutput());
  }


  this->VolumeActor->SetProperty (this->VolumeProperty);
  

  this->BoxWidget->SetInput (this->GetImage());
  this->BoxWidget->PlaceWidget();
  this->Callback->Execute (this->BoxWidget, 0, NULL);

  
  if( this->RenderingMode==VOLUME_RENDERING && this->Visibility && this->BoxWidgetVisibility)
  {
    this->BoxWidget->On();
  }
  else
  {
    this->BoxWidget->Off();
  }
  

  
  this->PlaneWidget->SetInput (this->GetImage());
  this->PlaneWidget->NormalToZAxisOn();
  this->PlaneWidget->PlaceWidget();
  
  
  double pt[3];
  this->GetCurrentPoint (pt);
  this->SetCurrentPoint (pt);
     
  // add actors to the renderwindow
  this->AddActor (this->ActorSagittal);
  this->AddActor (this->ActorCoronal);
  this->AddActor (this->ActorAxial);
  
  this->AddActor (this->AxesActor);
  this->AddActor (this->VolumeActor);

  if( this->FirstImage )
  {
    this->Center();
    this->FirstImage = 0;
  }
  
  
}


void vtkViewImage3D::SetWindow (double w)
{

  if( w<0.0 )
  {
    w = 0.0;
  }
    
  // scale the value w.r.t Scale/Shift parameters
  vtkViewImage::SetWindow ( this->GetShift() + w*this->GetScale() );

  
  double v_min = this->GetLevel() - 0.5*this->GetWindow();
  double v_max = this->GetLevel() + 0.5*this->GetWindow();
  
  if( this->GetLookupTable() )
  {      
    this->GetLookupTable()->SetRange ( (v_min-0.5*this->GetShift())/this->GetScale(),
                                       (v_max-1.5*this->GetShift())/this->GetScale());
    
    this->AxialColorMapper->GetLookupTable()->SetRange (v_min, v_max);
    this->SagittalColorMapper->GetLookupTable()->SetRange (v_min, v_max);
    this->CoronalColorMapper->GetLookupTable()->SetRange (v_min, v_max);
      
    vtkLookupTable* lut = vtkLookupTable::SafeDownCast ( this->GetLookupTable() );
    
    if( lut )
    {
      this->ColorFunction->RemoveAllPoints();              
      int numColors = lut->GetNumberOfTableValues();
      for( int i=0; i<numColors; i++)
      {
        double* color = lut->GetTableValue (i);
        this->ColorFunction->AddRGBPoint (v_min + (double)(i)*(v_max-v_min)/(numColors-1), color[0], color[1], color[2]);
      }
    }      
  }
  
  
  this->OpacityFunction->RemoveAllPoints();
  
    
  // In case there is an overlapping image, the input image in the
  // volume renderer is already set between 0 and 255. Thus, one needs
  // to adapt the window/level value with respect to the range [0 255].
  if( this->GetOverlappingImage() )
  {
    double s1 = this->GetLevel()-0.5*this->GetWindow();
    double s2 = this->GetLevel()+0.5*this->GetWindow();
    double* range = this->GetImage()->GetScalarRange();
    
    if( s1<range[0] )
    {
      s1=range[0];
    }
    else
    {
      if( s1>range[1] )
      {
        s1 = range[1];
      }
      else
      {
        s1 = 255.0/(range[1]-range[0])*(s1-range[0]);
      }
    }
    
    if( s2<range[0] )
    {
      s2=range[0];
    }
    else
    {
      if( s2>range[1] )
      {
        s2=range[1];
      }
      else
      {
        s2 = 255.0/(range[1]-range[0])*(s2-range[0]);      
      }
    }
    
    this->OpacityFunction->AddPoint (0.0, 0.0);
    this->OpacityFunction->AddPoint (s1,  0.0);
    this->OpacityFunction->AddPoint (s2,  1.0);
  }
  else
  {
    this->OpacityFunction->AddPoint (0.0,                  0.0);
    //this->OpacityFunction->AddPoint (this->GetLevel()-0.5*this->GetWindow(), 0.0);
    //this->OpacityFunction->AddPoint (this->GetLevel()+0.5*this->GetWindow(), 1.0);
    this->OpacityFunction->AddPoint (v_min, 0.0);
    this->OpacityFunction->AddPoint (v_max, 1.0);
  }
  
}



void vtkViewImage3D::SetLevel (double l)
{

  vtkViewImage::SetLevel( this->GetShift() + l*this->GetScale() );

  double v_min = this->GetLevel() - 0.5*this->GetWindow();
  double v_max = this->GetLevel() + 0.5*this->GetWindow();
  
  
  if( this->GetLookupTable() )
  {
    this->GetLookupTable()->SetRange ( (v_min-0.5*this->GetShift())/this->GetScale(),
                                       (v_max-1.5*this->GetShift())/this->GetScale() );
    this->AxialColorMapper->GetLookupTable()->SetRange (v_min, v_max);
    this->SagittalColorMapper->GetLookupTable()->SetRange (v_min, v_max);
    this->CoronalColorMapper->GetLookupTable()->SetRange (v_min, v_max);
    
    
    vtkLookupTable* lut = vtkLookupTable::SafeDownCast ( this->GetLookupTable() );
    
    if( lut )
    {
      this->ColorFunction->RemoveAllPoints();
      int numColors = lut->GetNumberOfTableValues();
      for( int i=0; i<numColors; i++)
      {
        double* color = lut->GetTableValue (i);
        this->ColorFunction->AddRGBPoint (v_min + (double)(i)*(v_max-v_min)/(numColors-1), color[0], color[1], color[2]);
      }
    }
  }
  
  
  this->OpacityFunction->RemoveAllPoints();
  
  // In case there is an overlapping image, the input image in the
  // volume renderer is already set between 0 and 255. Thus, one needs
  // to adapt the window/level value with respect to the range [0 255].
  if( this->GetOverlappingImage() )
  {
    double s1 = this->GetLevel()-0.5*this->GetWindow();
    double s2 = this->GetLevel()+0.5*this->GetWindow();
    double* range = this->GetImage()->GetScalarRange();
    
    if( s1<range[0] )
    {
      s1=range[0];
    }
    else
    {
      if( s1>range[1] )
      {
        s1=range[1];
      }
      else
      {
        s1 = 255.0/(range[1]-range[0])*(s1-range[0]);
      }
    }
    
    
    if( s2<range[0] )
    {
      s2=range[0];
    }
    else
    {
      if( s2>range[1] )
      {
        s2=range[1];
      }
      else
      {
        s2 = 255.0/(range[1]-range[0])*(s2-range[0]);
      }
    }
    
    this->OpacityFunction->AddPoint (0.0, 0.0);
    this->OpacityFunction->AddPoint (s1,  0.0);
    this->OpacityFunction->AddPoint (s2,  1.0);
  }
  else
  {
    this->OpacityFunction->AddPoint (0.0,                  0.0);
    //this->OpacityFunction->AddPoint (this->GetLevel()-0.5*this->GetWindow(), 0.0);
    //this->OpacityFunction->AddPoint (this->GetLevel()+0.5*this->GetWindow(), 1.0);
    this->OpacityFunction->AddPoint (v_min, 0.0);
    this->OpacityFunction->AddPoint (v_max, 1.0);
  }
  
}



void vtkViewImage3D::UpdatePosition ()
{

  if( this->GetImage() )
  {
    
    double*       spacing = this->GetImage()->GetSpacing();
    double*       origin  = this->GetImage()->GetOrigin();
    int*          dims    = this->GetImage()->GetWholeExtent();
    //double*      imBounds = this->GetImage()->GetBounds();

    const double* currentPoint = this->GetCurrentPoint();
    
    int imCoor[3];
    imCoor[0] = (int)(vtkrint ( ( currentPoint[0]-origin[0] )/spacing[0] ));
    imCoor[1] = (int)(vtkrint ( ( currentPoint[1]-origin[1] )/spacing[1] ));
    imCoor[2] = (int)(vtkrint ( ( currentPoint[2]-origin[2] )/spacing[2] ));

    
    if( imCoor[0]<dims[0] ) imCoor[0]=dims[0];
    if( imCoor[0]>dims[1] ) imCoor[0]=dims[1];
    if( imCoor[1]<dims[2] ) imCoor[1]=dims[2];
    if( imCoor[1]>dims[3] ) imCoor[1]=dims[3];
    if( imCoor[2]<dims[4] ) imCoor[2]=dims[4];
    if( imCoor[2]>dims[5] ) imCoor[2]=dims[5];

   
    double ppoint[3];
    for( int i=0; i<3; i++)
    {
      ppoint[i] = currentPoint[i];
    }
    
    
    this->ActorCoronal->SetDisplayExtent  (dims[0],   dims[1],   imCoor[1], imCoor[1], dims[4],   dims[5]);
    this->ActorSagittal->SetDisplayExtent (imCoor[0], imCoor[0], dims[2],   dims[3],   dims[4],   dims[5]);
    this->ActorAxial->SetDisplayExtent    (dims[0],   dims[1],   dims[2],   dims[3],   imCoor[2], imCoor[2]);
    
    
    this->Axes->SetOrigin(ppoint[0], ppoint[1], ppoint[2]);
    
  }
  
}


void vtkViewImage3D::SetRenderingMode (int mode)
{

  switch(mode)
  {
      case VOLUME_RENDERING :
        
        this->RenderingMode = VOLUME_RENDERING;

        this->ActorSagittal->SetVisibility (0);
        this->ActorCoronal->SetVisibility (0);
        this->ActorAxial->SetVisibility (0);

        this->VolumeActor->SetVisibility (this->Visibility);
        this->AxesActor->SetVisibility (this->Visibility && this->AxesVisibility);
        
          if( !this->GetRenderWindow()->GetNeverRendered() )
	  {
 
	    if( this->Visibility )
	    {
	      this->SetBoxWidget (this->BoxWidgetVisibility);
	    }
	    else
	    {
	      this->BoxWidget->Off ();
	    }
	  }
		
	  break;


      case PLANAR_RENDERING :

        this->RenderingMode = PLANAR_RENDERING;

        this->VolumeActor->SetVisibility (0);
        this->AxesActor->SetVisibility (this->Visibility && this->AxesVisibility);
        this->BoxWidget->Off();


        this->ActorSagittal->SetVisibility (this->Visibility && this->ShowSagittal);
        this->ActorCoronal->SetVisibility (this->Visibility && this->ShowCoronal);
        this->ActorAxial->SetVisibility (this->Visibility && this->ShowAxial);

        break;


      default :
        std::cerr << "Unknown rendering mode!" << std::endl;
  };
    
  
}


void vtkViewImage3D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Rendering mode: " << this->RenderingMode << std::endl;
}



void vtkViewImage3D::SetCameraPosition(double position[3])
{
  if (!this->GetRenderWindow()->GetNeverRendered())
    this->GetRenderer()->GetActiveCamera()->SetPosition(position);
}

void vtkViewImage3D::SetCameraFocalPoint(double position[3])
{
  if(!this->GetRenderWindow()->GetNeverRendered())
    this->GetRenderer()->GetActiveCamera()->SetFocalPoint(position);
}

void vtkViewImage3D::SetCameraViewUp(double position[3])
{
  if(!this->GetRenderWindow()->GetNeverRendered())
    this->GetRenderer()->GetActiveCamera()->SetViewUp(position);
}


void vtkViewImage3D::SetVisibility (bool isVisible)
{
  this->Visibility = isVisible;

  if( this->GetRenderingMode() == PLANAR_RENDERING )
  {
    this->VolumeActor->SetVisibility( 0 );
    this->AxesActor->SetVisibility( 0 );

    this->BoxWidget->Off();

    this->ActorSagittal->SetVisibility( isVisible );
    this->ActorCoronal->SetVisibility( isVisible );
    this->ActorAxial->SetVisibility( isVisible );
  }


  if( this->GetRenderingMode() == VOLUME_RENDERING )
  {
    this->ActorSagittal->SetVisibility( 0 );
    this->ActorCoronal->SetVisibility( 0 );
    this->ActorAxial->SetVisibility( 0 );

    this->VolumeActor->SetVisibility( isVisible );
    this->AxesActor->SetVisibility( isVisible && this->AxesVisibility);

    if( isVisible)
    {
      this->SetBoxWidget (this->BoxWidgetVisibility);
    }
    else
    { 
      this->BoxWidget->Off();
    }
  }  
}



void vtkViewImage3D::SetLookupTable(vtkScalarsToColors* lut)
{
  
  if( !lut )
  {
    return;
  }
  
  vtkViewImage::SetLookupTable (lut);


  double v_min = this->GetLevel() - 0.5*this->GetWindow();
  double v_max = this->GetLevel() + 0.5*this->GetWindow();

  /**
     In the case of a shift/scale, one must set the lut range to values
     without this shift/scale, because the object can be shared by different
     views.
   */
  lut->SetRange ( (v_min-0.5*this->GetShift())/this->GetScale(),
                  (v_max-1.5*this->GetShift())/this->GetScale());


  vtkLookupTable* realLut = vtkLookupTable::SafeDownCast (lut);

  if( !realLut )
  {
    std::cerr << "Error: Cannot cast vtkScalarsToColors to vtkLookupTable." << std::endl;
    return;
  }

  /**
     Due to the same problem as above (shift/scale), one must copy the lut
     so that it does not change values of the shared object.
  */
  vtkLookupTable* newLut = vtkLookupTable::New();
  newLut->DeepCopy (realLut);
  newLut->SetRange (v_min, v_max);
  this->AxialColorMapper->SetLookupTable (newLut);
  this->SagittalColorMapper->SetLookupTable (newLut);
  this->CoronalColorMapper->SetLookupTable (newLut);
  
  newLut->Delete();


  this->ColorFunction->RemoveAllPoints();

  int numColors = newLut->GetNumberOfTableValues();
  for( int i=0; i<numColors; i++)
  {
    double* color = newLut->GetTableValue (i);
    this->ColorFunction->AddRGBPoint (v_min + (double)(i)*(v_max-v_min)/(double(numColors)-1.0), color[0], color[1], color[2]);
  }
}



vtkScalarsToColors* vtkViewImage3D::GetColorFunction() const
{
  return this->ColorFunction;
}


void vtkViewImage3D::SetOverlappingImage (vtkImageData* image)
{

  if( !this->GetImage() || !image )
  {
    return;
  }
  
  this->OverlappingImage = image;
  this->AxialBlender->RemoveAllInputs();
  this->SagittalBlender->RemoveAllInputs();
  this->CoronalBlender->RemoveAllInputs();
  this->Blender->RemoveAllInputs();
  
  if( this->GetImage()->GetScalarType() == VTK_UNSIGNED_CHAR )
  {
    this->AxialBlender->AddInput    ( this->GetImage() );
    this->SagittalBlender->AddInput ( this->GetImage() );
    this->CoronalBlender->AddInput  ( this->GetImage() );
    this->Blender->AddInput         ( this->GetImage() );
  }
  else
  {
    this->AxialBlender->AddInputConnection (0, this->AxialColorMapper->GetOutputPort() );
    this->SagittalBlender->AddInputConnection (0, this->SagittalColorMapper->GetOutputPort() );
    this->CoronalBlender->AddInputConnection (0, this->CoronalColorMapper->GetOutputPort() );
    
    vtkImageMapToWindowLevelColors* colorMapper = vtkImageMapToWindowLevelColors::New();
    colorMapper->SetInput ( this->GetImage() );
    //colorMapper->SetWindow ( this->AxialColorMapper->GetWindow() );
    colorMapper->SetWindow ( this->GetWindow() );
    //colorMapper->SetLevel ( this->AxialColorMapper->GetLevel() );
    colorMapper->SetLevel ( this->GetLevel() );
    colorMapper->Update();
    
    vtkImageData* outColor = colorMapper->GetOutput();
    int* dims = outColor->GetDimensions();
    int  tot  = dims[0]*dims[1]*dims[2];
    
    // Set the last component to be the grey-value of the original image.
    // Thus, when windowing the image in volume rendering mode, the last
    // component will be used to compute opacity.
    
	  
    unsigned char* outBuffer = (unsigned char*)(outColor->GetScalarPointer());
    for(int i=0; i<tot; i++)
    {
      int val = (int)(*outBuffer);
      outBuffer += 3;
      (*outBuffer++)=(unsigned char)(val);
    }
    
    
    this->Blender->AddInput ( outColor );
    colorMapper->Delete();
  }
  
  this->AxialBlender->AddInput ( image );
  this->SagittalBlender->AddInput ( image );
  this->CoronalBlender->AddInput ( image );
  this->Blender->AddInput ( image );
  
  this->ActorSagittal->SetInput ( this->AxialBlender->GetOutput() );
  this->ActorCoronal->SetInput ( this->SagittalBlender->GetOutput() );
  this->ActorAxial->SetInput ( this->CoronalBlender->GetOutput() );
  
  this->VolumeMapper3D->SetInput ( this->Blender->GetOutput() );
  
}


void vtkViewImage3D::RemoveOverlappingImage()
{
  if( !this->GetImage() || !this->GetOverlappingImage() )
  {
    return;
  }
  

  this->VolumeMapper3D->SetInput ( this->GetImage() );
  
  if( this->GetImage()->GetScalarType() == VTK_UNSIGNED_CHAR )
  {
    this->ActorSagittal->SetInput ( this->GetImage() );
    this->ActorCoronal->SetInput  ( this->GetImage() );
    this->ActorAxial->SetInput    ( this->GetImage() );
  }
  else
  {
    this->ActorSagittal->SetInput (this->SagittalColorMapper->GetOutput());
    this->ActorCoronal->SetInput (this->CoronalColorMapper->GetOutput());
    this->ActorAxial->SetInput (this->AxialColorMapper->GetOutput());
  }
  
  this->OverlappingImage = 0;
  
}

void vtkViewImage3D::SetShowAxial (int val)
{
  this->ShowAxial = val;
  this->SetRenderingMode (this->RenderingMode);
}
void vtkViewImage3D::SetShowCoronal (int val)
{
  this->ShowCoronal = val;
  this->SetRenderingMode (this->RenderingMode);
}
void vtkViewImage3D::SetShowSagittal (int val)
{
  this->ShowSagittal = val;
  this->SetRenderingMode (this->RenderingMode);
}
 
vtkActor* vtkViewImage3D::AddDataSet (vtkDataSet* dataset, vtkProperty* property)
{
  
  bool doit = true;
    
  if (!dataset)
    doit = false;
  
  if( this->HasDataSet (dataset) )
    doit = false;
  
  vtkImageData* imagedata = NULL;
  imagedata = vtkImageData::SafeDownCast(dataset);
  
  if (imagedata)
  {
    this->SetImage(imagedata);
  }
  else
  {
      
    if (doit)
    {
	
      vtkDataSetSurfaceFilter* geometryextractor = vtkDataSetSurfaceFilter::New();
      vtkPolyDataNormals*      normalextractor = vtkPolyDataNormals::New();
      vtkPolyDataMapper*       m_mapper = vtkPolyDataMapper::New();
      vtkActor*                actor = vtkActor::New();
	
      normalextractor->SetFeatureAngle (90);
      ///\todo try to skip the normal extraction filter in order to enhance the visualization speed when the data is time sequence.
      geometryextractor->SetInput (dataset);
      normalextractor->SetInput (geometryextractor->GetOutput());
      m_mapper->SetInput (normalextractor->GetOutput());
      actor->SetMapper (m_mapper);
      if (property)
	actor->SetProperty (property);
      
      this->AddActor (actor);
      this->DataSetList.push_back (dataset);
      this->DataSetActorList.push_back (actor);	  
	
      m_mapper->Delete();
      normalextractor->Delete();
      geometryextractor->Delete();
      actor->Delete();
    }
  }
    
  return this->GetDataSetActor(dataset);
    
}


void vtkViewImage3D::ColorDataSetByArray(vtkDataSet* dataset, const char* arrayname, vtkColorTransferFunction* transfer)
{

  
  bool doit = true;
    
    
    if (!dataset)
      doit = false;

    vtkDataArray* array = NULL;
    vtkMapper* mapper = NULL;
    
    if (doit)
    {  
      for (unsigned int i=0; i<this->DataSetList.size(); i++)
      {
	if (dataset == this->DataSetList[i])
	{
	  mapper = this->DataSetActorList[i]->GetMapper();
	  break;
	}
      }
      if (!mapper)
      {
	doit = false;
      }
    }

    if (doit)
    {
      mapper->Modified();
      
      if (dataset->GetCellData())
      {
	array = dataset->GetCellData()->GetArray (arrayname);
	if (array)
	{
	  if (!mapper->GetInput()->GetCellData()->HasArray (arrayname))
	  {
	    this->UpdateDataSetPipeline (dataset);
	    for (unsigned int i=0; i<this->DataSetList.size(); i++)
	    {
	      if (dataset == this->DataSetList[i])
	      {
		mapper = this->DataSetActorList[i]->GetMapper();
		break;
	      }
	    }
	  }
	  
	  mapper->SetScalarModeToUseCellFieldData();
	  
	}
	
      }
      
      if (!array && dataset->GetPointData())
      {
	array = dataset->GetPointData()->GetArray (arrayname);
	if (array)
	{
	  if (!mapper->GetInput()->GetPointData()->HasArray (arrayname))
	  {
	    this->UpdateDataSetPipeline (dataset);
	    for (unsigned int i=0; i<this->DataSetList.size(); i++)
	    {
	      if (dataset == this->DataSetList[i])
	      {
		mapper = this->DataSetActorList[i]->GetMapper();
		break;
	      }
	    }
	  }
	  
	  mapper->SetScalarModeToUsePointFieldData();
	}
      }
      
      if (!array)
      {
	mapper->SetScalarModeToDefault();
	mapper->SetInterpolateScalarsBeforeMapping(0);
	doit = false;
      }
    }

    if (doit)
    {
      mapper->SetLookupTable (transfer);
      mapper->SetScalarRange (array->GetRange()[0], array->GetRange()[1]);
      mapper->SetInterpolateScalarsBeforeMapping(1);
      mapper->SelectColorArray (array->GetName());
    }
}



void vtkViewImage3D::UpdateDataSetPipeline (vtkDataSet* dataset)
{

//   for( unsigned int i=0; i<this->Children.size(); i++)
//   {
//     vtkViewImage* view = dynamic_cast<vtkViewImage*> (this->Children[i]);
//     if( view )
//     {
//       view->Lock ();
//     }
//   }

  vtkProperty* prop = NULL;
  vtkActor* actor = this->GetDataSetActor (dataset);
  if (actor)
  {
    prop = actor->GetProperty();
    prop->Register(this);
  }  
  
  this->RemoveDataSet (dataset);
  this->AddDataSet (dataset, prop);

  prop->UnRegister (this);

//   for( unsigned int i=0; i<this->Children.size(); i++)
//   {
//     vtkViewImage* view = dynamic_cast<vtkViewImage*> (this->Children[i]);
//     if( view )
//     {
//       view->UnLock();
//     }
//   }
  
  
}

  
