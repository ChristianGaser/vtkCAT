/*=========================================================================

Program:   vtkINRIA3D
Module:    $Id: vtkViewImage2D.cxx 752 2008-03-11 16:32:52Z filus $
Language:  C++
Author:    $Author: filus $
Date:      $Date: 2008-03-11 17:32:52 +0100 (Di, 11 Mär 2008) $
Version:   $Revision: 752 $

Copyright (c) 2007 INRIA - Asclepios Project. All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "vtkViewImage2D.h"

#include "vtkInteractorObserver.h"
#include "vtkInteractorStyleSwitch.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRendererCollection.h"
#include "vtkProp.h"
#include "vtkTextActor.h"
#include "vtkCoordinate.h"
#include "vtkProperty.h"
#include "vtkProperty2D.h"
#include "vtkTextProperty.h"
#include <vtkCornerAnnotation.h>
#include <vtkLightCollection.h>
#include <vtkLight.h>
#include "vtkOrientationAnnotation.h"

#include "assert.h"
#include <iostream>
#include <sstream>
#include <cmath>

#include "vtkViewImage2DCommand.h"
#include "vtkViewImage2DFullCommand.h"
#include "vtkInteractorStyleImage2D.h"
#include "vtkCamera.h"
#include "vtkImageReslice.h"
#include "vtkRenderWindow.h"
#include "vtkTransform.h"
#include "vtkImageMapToWindowLevelColors.h"
#include "vtkImageMapToColors.h"
#include "vtkImageActor.h"
#include "vtkImageData.h"
#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkLineSource.h"
#include "vtkLookupTable.h"
#include "vtkImageBlendWithMask.h"
#include "vtkImageBlend.h"
#include <vtkPolyDataMapper2D.h>
#include <vtkActor2D.h>

#include <vtkColorTransferFunction.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkExtractGeometry.h>
#include <vtkDataSet.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSetMapper.h>
#include <vtkPlane.h>
#include <vtkPlaneCollection.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkActor2D.h>
#include <vtkClipDataSet.h>
#include <vtkCutter.h>
#include <vtkBox.h>
#include <vtkPolyDataWriter.h>
#include <vtkPolyDataNormals.h>


extern int vtkrint(double a);


vtkCxxRevisionMacro(vtkViewImage2D, "$Revision: 752 $");
vtkStandardNewMacro(vtkViewImage2D);

// Initialize static member that controls display convention (0: radiologic, 1: neurologic)
static int vtkViewImage2DDisplayConventions = 0;

void vtkViewImage2D::SetViewImage2DDisplayConventions(int val)
{
  vtkViewImage2DDisplayConventions = val;
}

int vtkViewImage2D::GetViewImage2DDisplayConventions()
{
  return vtkViewImage2DDisplayConventions;
}


vtkViewImage2D::vtkViewImage2D()
{

  this->FirstRender = 1;
  this->FirstImage  = 1;
  this->ShowCurrentPoint = true;
  this->ShowDirections   = true;
  this->ShowSliceNumber  = true;
  this->Orientation      = vtkViewImage::AXIAL_ID;
  this->InteractionStyle = SELECT_INTERACTION;
  this->LeftButtonInteractionStyle = SELECT_INTERACTION;
  this->MiddleButtonInteractionStyle = SELECT_INTERACTION;
  this->RightButtonInteractionStyle = SELECT_INTERACTION;
  this->WheelInteractionStyle = SELECT_INTERACTION;

  this->Conventions = RADIOLOGIC;
  
  this->InitialParallelScale = 1.0;
  

  this->OverlappingImage = 0;


  this->ImageReslice     = vtkImageReslice::New();
  this->ImageActor       = vtkImageActor::New();
  this->WindowLevelForCorner = vtkImageMapToWindowLevelColors::New();
  this->WindowLevel      = vtkImageMapToColors::New();
  this->MaskFilter       = vtkImageBlendWithMask::New();
  this->Blender          = vtkImageBlend::New();

  
  this->HorizontalLineSource = vtkLineSource::New();
  this->VerticalLineSource   = vtkLineSource::New();
  this->HorizontalLineActor  = vtkActor::New();
  this->VerticalLineActor    = vtkActor::New();
  
  this->DataSetCutPlane      = vtkPlane::New();
  this->DataSetCutBox        = vtkBox::New();
  

  this->DataSetCutPlane->SetOrigin (0,0,0);
  this->DataSetCutPlane->SetNormal (0,0,1);
  this->DataSetCutBox->SetBounds (0,0,0,0,0,0);
  this->BoxThickness = 2;
  
  

  this->LinkCameraFocalAndPosition = 0;
  

  // set the filters properties
  this->Blender->SetBlendModeToNormal();
  this->Blender->SetOpacity (0, 0.25);
  this->Blender->SetOpacity (1, 0.75);
  
  
  // set up the vtk pipeline  
  this->ImageReslice->SetOutputDimensionality(2);
  this->ImageReslice->InterpolateOff();
  this->ImageReslice->SetInputConnection( this->WindowLevel->GetOutputPort() );
  

  this->AuxInput     = this->WindowLevel->GetOutput();
  this->ResliceInput = this->WindowLevel->GetOutput();

 
  // Interactor Style
  this->InitInteractorStyle(vtkViewImage2D::SELECT_INTERACTION);
  
  
  // Initialize cursor lines
  vtkPolyDataMapper* mapper =  vtkPolyDataMapper::New();
  mapper->SetInputConnection( this->HorizontalLineSource->GetOutputPort() );

  this->HorizontalLineActor->SetMapper(mapper);
  this->HorizontalLineActor->GetProperty()->SetColor (1.0,0.0,0.0);
  mapper->Delete();
  this->HorizontalLineActor->SetVisibility (0);
  

  
  vtkPolyDataMapper* mapper2 =  vtkPolyDataMapper::New();
  mapper2->SetInputConnection( this->VerticalLineSource->GetOutputPort() );
  this->VerticalLineActor->SetMapper(mapper2);
  this->VerticalLineActor->GetProperty()->SetColor (1.0,0.0,0.0);
  mapper2->Delete();
  this->VerticalLineActor->SetVisibility (0);

  this->GetCornerAnnotation()->SetWindowLevel ( this->WindowLevelForCorner );
  
  this->SetOrientation (vtkViewImage::AXIAL_ID);

  if( vtkViewImage2D::GetViewImage2DDisplayConventions()==0 )
  {
    this->SetConventionsToRadiological();
  }
  else
  {
    this->SetConventionsToNeurological();
  }
}



vtkViewImage2D::~vtkViewImage2D()
{
  this->ImageActor->Delete();
  this->WindowLevel->Delete();
  this->WindowLevelForCorner->Delete();
  this->ImageReslice->Delete();
  this->MaskFilter->Delete();
  this->Blender->Delete();
    
  this->HorizontalLineSource->Delete();
  this->HorizontalLineActor->Delete();

  this->VerticalLineSource->Delete();
  this->VerticalLineActor->Delete();

  this->DataSetCutPlane->Delete();
  this->DataSetCutBox->Delete();
  
}




void vtkViewImage2D::Update()
{
  this->UpdateImageActor();
  this->UpdatePosition();
  this->InitializeImagePositionAndSize();
}


void vtkViewImage2D::Initialize()
{
  this->Superclass::Initialize();

  if (this->GetRenderer())
  {
    this->GetRenderer()->TwoSidedLightingOff();
  }
  

}



void vtkViewImage2D::SetVisibility (int state)
{
  this->ImageActor->SetVisibility (state);

}

int vtkViewImage2D::GetVisibility ()
{
  return this->ImageActor->GetVisibility ();
}


void vtkViewImage2D::UpdateImageActor()
{
  if( !this->GetImage() )
  {
    return;
  }

  this->ImageReslice->GetOutput()->UpdateInformation();
  this->ImageActor->GetInput()->SetUpdateExtent(this->ImageReslice->GetOutput()->GetWholeExtent());
  this->ImageActor->SetDisplayExtent(this->ImageReslice->GetOutput()->GetWholeExtent());
  
  this->FirstRender = 1;
}

  
  
void vtkViewImage2D::InitializeImagePositionAndSize()
{
  if( !this->GetImage() )
  {
    return;
  }
  
  
  if (this->FirstRender)
  {
    // make sure the input is up-to-date
    this->ImageActor->GetInput()->Update();


    this->GetRenderer()->GetActiveCamera()->OrthogonalizeViewUp();
    this->GetRenderer()->GetActiveCamera()->ParallelProjectionOn();

    // Get the bounds of the image: coordinates in the real world
    double bnds[6];
    this->GetImage()->GetBounds(bnds);
        
    // extension of the volume:
    double xs = (bnds[1] - bnds[0]);
    double ys = (bnds[3] - bnds[2]);
    double zs = (bnds[5] - bnds[4]);

    double mx = xs < ys ? ys:xs;
    mx = mx < zs ? mx:zs;
    this->InitialParallelScale = mx/2.0;
    
    /**
       Somehow, when the axes are present, they screw up the ResetZoom
       function because they are taken into account when computing the
       bounds of all actors. We need to switch them off and on after
       the call to ResetZoom().
    */
    this->VerticalLineActor->SetVisibility (0);
    this->HorizontalLineActor->SetVisibility (0);
    
    this->ResetZoom();

    this->VerticalLineActor->SetVisibility (1);
    this->HorizontalLineActor->SetVisibility (1);
    
    this->FirstRender = 0;
    
  }

}


void vtkViewImage2D::InitInteractorStyle(unsigned int p_style)
{
  
  vtkInteractorStyleImage2D *interactor = vtkInteractorStyleImage2D::New();
  interactor->SetView(this);  
  this->SetInteractionStyle(p_style);
  /// Attach the vtkCommand : basically, attach an
  /// observer for basic events (move, click, key-pressed etc)
  vtkViewImage2DCommand *cbk = vtkViewImage2DCommand::New();  
  cbk->SetView(this);
  interactor->AddObserver(vtkCommand::KeyPressEvent, cbk);
  interactor->AddObserver(vtkCommand::WindowLevelEvent, cbk);
  interactor->AddObserver(vtkCommand::StartWindowLevelEvent, cbk);
  interactor->AddObserver(vtkCommand::ResetWindowLevelEvent, cbk);
  interactor->AddObserver(vtkCommand::EndWindowLevelEvent, cbk);
  interactor->AddObserver(vtkCommand::PickEvent, cbk);
  interactor->AddObserver(vtkCommand::StartPickEvent, cbk);
  interactor->AddObserver(vtkCommand::EndPickEvent, cbk);
  interactor->AddObserver(vtkViewImage2DCommand::ResetZoomEvent, cbk);
  interactor->AddObserver(vtkViewImage2DCommand::ResetPositionEvent, cbk);
  interactor->AddObserver(vtkViewImage2DCommand::StartZSliceMoveEvent, cbk);
  interactor->AddObserver(vtkViewImage2DCommand::ZSliceMoveEvent, cbk);
  interactor->AddObserver(vtkViewImage2DCommand::EndZSliceMoveEvent, cbk);
  interactor->AddObserver(vtkViewImage2DCommand::StartMeasureEvent, cbk);
  interactor->AddObserver(vtkViewImage2DCommand::MeasureEvent, cbk);
  interactor->AddObserver(vtkViewImage2DCommand::EndMeasureEvent, cbk);
  interactor->AddObserver(vtkViewImage2DCommand::FullPageEvent, cbk);
  interactor->AddObserver(vtkViewImage2DCommand::ZoomEvent, cbk);

  this->SetInteractorStyle(interactor);

  cbk->Delete();
  interactor->Delete();
}



/*
void vtkViewImage2D::SetFullInteraction()
{
  vtkInteractorStyleImage *interactor = vtkInteractorStyleImage::New();
  /// Attach the vtkCommand : basically, attach an
  /// observer for basic events (move, click, key-pressed etc)
  vtkViewImage2DFullCommand *cbk = vtkViewImage2DFullCommand::New();  
  cbk->IV = this;  
  interactor->AddObserver(vtkCommand::WindowLevelEvent, cbk);
  interactor->AddObserver(vtkCommand::StartWindowLevelEvent, cbk);
  interactor->AddObserver(vtkCommand::ResetWindowLevelEvent, cbk);
  interactor->AddObserver(vtkCommand::EndWindowLevelEvent, cbk);
  interactor->AddObserver(vtkCommand::KeyPressEvent, cbk);
  interactor->AddObserver(vtkCommand::EnterEvent, cbk);
  interactor->AddObserver(vtkCommand::LeaveEvent, cbk);
  cbk->Delete();  
  this->SetInteractorStyle(interactor);
  interactor->Delete();
}
*/


void vtkViewImage2D::SetShowDirections(bool p_showDirections)
{
  this->ShowDirections = p_showDirections;
}


void vtkViewImage2D::SetShowCurrentPoint(bool p_showCurrentPoint)
{
  this->ShowCurrentPoint = p_showCurrentPoint;
  
  if (!this->ShowCurrentPoint)
  {
    this->HorizontalLineSource->SetPoint1(0, 0, 0.001);
    this->HorizontalLineSource->SetPoint2(0, 0, 0.001);   
    this->VerticalLineSource->SetPoint1(0, 0, 0.001);   
    this->VerticalLineSource->SetPoint2(0, 0, 0.001);
  }  
}



void vtkViewImage2D::SetShowSliceNumber(bool p_showSliceNumber)
{
  this->ShowSliceNumber = p_showSliceNumber;
  
  if (!this->ShowSliceNumber)
  {
    this->SetUpRightAnnotation("");
  }
}



int vtkViewImage2D::GetWholeZMin()
{
  return 0;  
}



int vtkViewImage2D::GetWholeZMax()
{
  if(!this->GetImage())
  {
    return 0;
  }
  
  int* ext = this->GetImage()->GetWholeExtent();
  
  assert(this->Orientation<vtkViewImage::NB_DIRECTION_IDS);  

  int axis = this->GetOrthogonalAxis(this->Orientation);
  return ext[2*axis+1];
}


int vtkViewImage2D::GetZSlice()
{
  return this->GetSlice(this->Orientation);  
}

void vtkViewImage2D::SetZSlice(int p_zslice)
{
  this->SetSlice(this->Orientation, p_zslice);
}

void vtkViewImage2D::UpdatePosition ()
{
  if( !this->GetImage() )
  {
    return;
  }
  
  double x=0;
  double y=0;
  double max_x=0;
  double max_y=0;
  double min_x=0;
  double min_y=0;
  double pos[3];

  this->GetCurrentPoint(pos);

  double *spacing = this->GetImage()->GetSpacing();
  double *origin  = this->GetImage()->GetOrigin();
  double *imBounds = this->GetImage()->GetBounds();

  // check if pos lies inside image bounds
  if( pos[0]<imBounds[0] || pos[0]>imBounds[1] ||
      pos[1]<imBounds[2] || pos[1]>imBounds[3] ||
      pos[2]<imBounds[4] || pos[2]>imBounds[5])
  {
    // we are outside image bounds
    return;
  }

  pos[0] = double(vtkrint ((pos[0]-origin[0])/spacing[0] ))*spacing[0]+origin[0];
  pos[1] = double(vtkrint ((pos[1]-origin[1])/spacing[1] ))*spacing[1]+origin[1];
  pos[2] = double(vtkrint ((pos[2]-origin[2])/spacing[2] ))*spacing[2]+origin[2];  
  
  switch (this->Orientation)
  {
      case vtkViewImage::SAGITTAL_ID:
        
        this->ImageReslice->SetResliceAxesOrigin(pos[0],0,0);
        x = (double)pos[1];
        y = (double)pos[2];
        max_x = this->GetWholeMaxPosition(1);
        max_y = this->GetWholeMaxPosition(2);
        min_x = this->GetWholeMinPosition(1);
        min_y = this->GetWholeMinPosition(2);
        break;
        
      case vtkViewImage::CORONAL_ID:

        this->ImageReslice->SetResliceAxesOrigin(0,pos[1],0);
        if( this->Conventions==RADIOLOGIC )
        {
          x = (double)pos[0];
          max_x = this->GetWholeMaxPosition(0);
          min_x = this->GetWholeMinPosition(0);
        }
        else
        {
          x = (double)pos[0]*-1.0;
          max_x = this->GetWholeMaxPosition(0)*-1.0;
          min_x = this->GetWholeMinPosition(0)*-1.0;
        }
        y = (double)pos[2];
        max_y = this->GetWholeMaxPosition(2);
        min_y = this->GetWholeMinPosition(2);
        break;

              
      case vtkViewImage::AXIAL_ID:

        this->ImageReslice->SetResliceAxesOrigin(0,0,pos[2]);

        if( this->Conventions==RADIOLOGIC )
        {
          x = (double)pos[0];
          max_x = this->GetWholeMaxPosition(0);
          min_x = this->GetWholeMinPosition(0);
        }
        else
        {
          x = (double)pos[0]*-1.0;
          max_x = this->GetWholeMaxPosition(0)*-1.0;
          min_x = this->GetWholeMinPosition(0)*-1.0;
        }
        y = (double)pos[1]*-1.0;
        max_y = this->GetWholeMaxPosition(1)*-1.0;
        min_y = this->GetWholeMinPosition(1)*-1.0;
        break;        
  }
  
  
  if(this->ShowCurrentPoint)
  {
    this->HorizontalLineSource->SetPoint1(min_x, y, 0.001);
    this->HorizontalLineSource->SetPoint2(max_x, y, 0.001);   
    this->VerticalLineSource->SetPoint1(x, min_y, 0.001);   
    this->VerticalLineSource->SetPoint2(x, max_y, 0.001);
  }

  
  this->ImageReslice->Update(); // needed to update input Extent
  
  
  if (this->GetShowAnnotations())
  {
    
    // Update annotations
    if( this->GetImage() && this->ShowSliceNumber )
    {
      int imCoor[3];
      this->GetCurrentVoxelCoordinates(imCoor);
      int dims[3];
      this->GetImage()->GetDimensions (dims);
      double mm;
      
      std::ostringstream os;
      os << "Slice: ";
      switch( this->Orientation )
      {
	  case vtkViewImage::AXIAL_ID :

            mm = ((double)imCoor[2]+origin[2]/spacing[2] )*spacing[2];
            os << imCoor[2]+1 << " / " << dims[2] << std::endl;
            os << "z: " << mm << "mm" << std::endl;
            break;
            
	    
	  case vtkViewImage::CORONAL_ID :

            mm = ((double)imCoor[1]+origin[1]/spacing[1] )*spacing[1];
            os << imCoor[1]+1 << " / " << dims[1] << std::endl;
            os << "y: " << mm << "mm" << std::endl;
            break;

            
	  case vtkViewImage::SAGITTAL_ID :

            mm = ((double)imCoor[0]+origin[0]/spacing[0] )*spacing[0];
            os << imCoor[0]+1 << " / " << dims[0] << std::endl;
            os << "x: " << mm << "mm" << std::endl;
            break;
      }
      
      os << "Value: " << this->GetCurrentPointDoubleValue() << std::endl;
//      os << "<window>\n<level>";
      this->SetUpRightAnnotation(os.str().c_str());
    }

  }
  
  this->SetDownLeftAnnotation(this->GetAboutData());


  
  //const double* c_position = this->GetCurrentPoint();
  unsigned int direction = this->GetOrthogonalAxis (this->GetOrientation());
  switch(direction)
  {
      case X_ID :
	this->DataSetCutPlane->SetOrigin (pos[0],0,0);
        this->DataSetCutPlane->SetNormal (1,0,0);
	this->DataSetCutBox->SetBounds (this->DataSetCutPlane->GetOrigin()[0],this->DataSetCutPlane->GetOrigin()[0]+this->BoxThickness,
					this->GetWholeMinPosition(1),this->GetWholeMaxPosition(1),
					this->GetWholeMinPosition(2),this->GetWholeMaxPosition(2));
	
	break;
      case Y_ID :
	this->DataSetCutPlane->SetOrigin (0,pos[1],0);
        this->DataSetCutPlane->SetNormal (0,1,0);
	this->DataSetCutBox->SetBounds (this->GetWholeMinPosition(0),this->GetWholeMaxPosition(0),
					this->DataSetCutPlane->GetOrigin()[1],this->DataSetCutPlane->GetOrigin()[1]+this->BoxThickness,
					this->GetWholeMinPosition(2),this->GetWholeMaxPosition(2));
	break;
      case Z_ID :
	this->DataSetCutPlane->SetOrigin (0,0,pos[2]);
        this->DataSetCutPlane->SetNormal (0,0,1);
	this->DataSetCutBox->SetBounds (this->GetWholeMinPosition(0),this->GetWholeMaxPosition(0),
					this->GetWholeMinPosition(1),this->GetWholeMaxPosition(1),
					this->DataSetCutPlane->GetOrigin()[2],this->DataSetCutPlane->GetOrigin()[2]+this->BoxThickness);
	break;
  }

  
  if( this->DataSetList.size() )
  {

    this->ResetAndRestablishZoomAndCamera();
    
    /*
      We need to correct for the origin of the actor. Indeed, the ImageActor
      has always position 0 in Z in axial view, in X in sagittal view and
      in Y in coronal view. The projected dataset have an origin that depends
      on the required slice and can be negative. In that case, the projected
      data are behind the image actor and thus not visible. Here, we correct
      this by translating the actor so that it becomes visible.
     */
    for( unsigned int i=0; i<this->DataSetActorList.size(); i++)
    {
      double Pos[3];
      this->DataSetActorList[i]->GetPosition (Pos);
      
      switch(direction)
      {
          case X_ID :
            Pos[0] = -1.0*pos[0] + 1.0;
            break;
            
          case Y_ID:
            Pos[1] = -1.0*pos[1] + 1.0;
            break;
            
          case Z_ID:
            Pos[2] = -1.0*pos[2] + 1.0;
            break;
      }
      
      this->DataSetActorList[i]->SetPosition (Pos);
    }
    
  }

}



void vtkViewImage2D::SetWindow (double w)
{

  if( w<0.0 )
  {
    w = 0.0;
  }

  double shiftScaleWindow = this->GetShift() + w*this->GetScale();
  
  vtkViewImage::SetWindow ( shiftScaleWindow );
  this->WindowLevelForCorner->SetWindow( shiftScaleWindow );
  
  double v_min = this->GetLevel() - 0.5*this->GetWindow();
  double v_max = this->GetLevel() + 0.5*this->GetWindow();

  if( this->GetLookupTable())
  {
  
    this->GetLookupTable()->SetRange ( (v_min-0.5*this->GetShift())/this->GetScale(),
                                       (v_max-1.5*this->GetShift())/this->GetScale());
    this->WindowLevel->GetLookupTable()->SetRange (v_min, v_max);
  }
  
}



void vtkViewImage2D::SetLevel (double l)
{

  double shiftScaleLevel = this->GetShift() + l*this->GetScale();

  vtkViewImage::SetLevel ( shiftScaleLevel );
  this->WindowLevelForCorner->SetLevel( shiftScaleLevel );
    
  double v_min = this->GetLevel() - 0.5*this->GetWindow();
  double v_max = this->GetLevel() + 0.5*this->GetWindow();
  
  if( this->GetLookupTable() )
  {
    this->GetLookupTable()->SetRange ( (v_min-0.5*this->GetShift())/this->GetScale(),
                                       (v_max-1.5*this->GetShift())/this->GetScale());
    this->WindowLevel->GetLookupTable()->SetRange (v_min, v_max);
  }
}

  

double vtkViewImage2D::GetColorWindow()
{
  return this->GetWindow();
}



double  vtkViewImage2D::GetColorLevel()
{
  return this->GetLevel();
}

  

void vtkViewImage2D::SetTransform(vtkTransform* p_transform)
{
  this->ImageReslice->SetResliceTransform(p_transform);
}


void vtkViewImage2D::SetImage(vtkImageData* image)
{
  if(!image)
  {
    return;
  }
  

  this->RegisterImage(image); /* (== m_Image=image */
  
  // check if there is a mask image. If yes, then we check
  // if the new image size and spacing agrees with the mask image.
  // If not, we remove the mask image
  if( this->GetMaskImage() )
  {
    int*    dims        = image->GetDimensions();
    double* spacing     = image->GetSpacing();
    int*    maskDims    = this->GetMaskImage()->GetDimensions();
    double* maskSpacing = this->GetMaskImage()->GetSpacing();

    if( dims[0]!=maskDims[0] || dims[1]!=maskDims[1] || dims[2]!=maskDims[2] ||
        spacing[0]!=maskSpacing[0] || spacing[1]!=maskSpacing[1] || spacing[2]!=maskSpacing[2] )
    {
      this->RemoveMaskImage();
    }
  }
  
  // should check also the overlapping image
  
  if( image->GetScalarType() == VTK_UNSIGNED_CHAR  && (image->GetNumberOfScalarComponents()==3 || image->GetNumberOfScalarComponents()==4) )
  {
    this->AuxInput = image;
  }
  else
  {
    this->AuxInput = this->WindowLevel->GetOutput();
    
    this->WindowLevel->SetInput (image);
    double range[2];
    image->GetScalarRange (range);
    if ( this->WindowLevel->GetLookupTable() )
    {
      this->WindowLevel->GetLookupTable()->SetRange (range);
    }
  }

  
  if( this->GetOverlappingImage() )
  {
    this->Blender->SetInput (0, this->AuxInput );
  }
  else
  {
    if( this->GetMaskImage() )
    {
      this->MaskFilter->SetImageInput ( this->AuxInput );
    }
    else
    {
      this->ImageReslice->SetInput ( this->AuxInput );
      this->ResliceInput = this->AuxInput;
    }
  }
  
  this->ImageActor->SetInput( this->ImageReslice->GetOutput() );
  
  this->AddActor(this->HorizontalLineActor);
  this->AddActor(this->VerticalLineActor);

  this->AddActor(this->ImageActor);


  // save the camera focal and position, and zoom, before calling Update (in SetOrientation())
  double focal[3], pos[3];
  this->GetCameraFocalAndPosition (focal, pos);
  double zoom = this->GetZoom();
  
  this->SetOrientation(this->Orientation);

  //this->Update(); // already called at the end of SetOrientation
  
  this->SetWindow( this->GetWindow() );
  this->SetLevel( this->GetLevel() );

  if( !this->FirstImage )
  {
    this->SetZoom ( zoom );
    this->SetCameraFocalAndPosition (focal, pos);    
  }

  this->FirstImage = 0;
}


void vtkViewImage2D::SetLookupTable (vtkScalarsToColors* lut)
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
                  (v_max-1.5*this->GetShift())/this->GetScale() );

  
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
  this->WindowLevel->SetLookupTable (newLut);
  newLut->Delete();
  
}



void vtkViewImage2D::SetOrientation(unsigned int p_orientation)
{
  if(p_orientation > vtkViewImage::NB_PLAN_IDS - 1)
  {
    return;
  }

  this->Orientation = p_orientation;

  
  if(!this->GetImage())
  {
    return;
  }

  this->SetupAnnotations();

  unsigned int direction = this->GetOrthogonalAxis (this->GetOrientation());
  switch(direction)
  {
      case X_ID :
	this->DataSetCutPlane->SetNormal (1,0,0);
	break;
      case Y_ID :
	this->DataSetCutPlane->SetNormal (0,1,0);
	break;
      case Z_ID :
	this->DataSetCutPlane->SetNormal (0,0,1);
	break;
  }
  
  this->ImageReslice->Modified();
  this->Update();
  
}



void vtkViewImage2D::SetInterpolationMode(int i)
{
  this->ImageActor->SetInterpolate(i);
}



int vtkViewImage2D::GetInterpolationMode(void)
{
  return this->ImageActor->GetInterpolate();
}



void vtkViewImage2D::SetMaskImage (vtkImageData* mask, vtkLookupTable* lut)
{
  
  if( !this->GetImage() || !mask || !lut)
  {
    return;
  }


  vtkViewImage::SetMaskImage (mask, lut);
  
  
  // check if the mask dimensions match the image dimensions
  int dim1[3], dim2[3];
  this->GetImage()->GetDimensions (dim1);
  mask->GetDimensions (dim2);
  if (    (dim1[0] != dim2[0]) ||
          (dim1[1] != dim2[1]) ||
          (dim1[2] != dim2[2]) )
  {
    vtkErrorMacro("Dimensions of the mask image do not match");
    return;
  }
  
  
  // check if the scalar range match the number of entries in the LUT
  double range[2];
  mask->GetScalarRange (range);
  int numLUT = lut->GetNumberOfTableValues();
  if( numLUT<(int)(range[1])+1 )
  {
    vtkErrorMacro("The number of LUT entries is less than the range of the mask.");
    return;
  }
  
  if( this->GetOverlappingImage() )
  {
    this->MaskFilter->SetImageInput ( this->Blender->GetOutput() );
  }
  else
  {
    this->MaskFilter->SetImageInput ( this->AuxInput );
  }
  this->MaskFilter->SetMaskInput (mask);
  this->MaskFilter->SetLookupTable (lut);
  this->MaskFilter->Update();
  
  
  this->ImageReslice->SetInputConnection( this->MaskFilter->GetOutputPort() );
  this->ResliceInput = this->MaskFilter->GetOutput();
  
}



void vtkViewImage2D::RemoveMaskImage()
{
  
  if( this->GetOverlappingImage() )
  {
    this->ImageReslice->SetInputConnection( this->Blender->GetOutputPort() );
    this->ResliceInput = this->Blender->GetOutput();
  }  
  else
  {
    this->ImageReslice->SetInput( this->AuxInput );
    this->ResliceInput = this->AuxInput;
  }

  vtkViewImage::SetMaskImage (0,0);
}



void vtkViewImage2D::SetOverlappingImage (vtkImageData* image)
{

  if( !this->GetImage() || !image )
  {
    return;
  }
  
  
  this->OverlappingImage = image;
  
  this->Blender->RemoveAllInputs();
  this->Blender->AddInput (this->AuxInput );
  this->Blender->AddInput (image);
  
  if( this->GetMaskImage() )
  {
    this->MaskFilter->SetInputConnection (this->Blender->GetOutputPort());
  }
  else
  {
    this->ImageReslice->SetInputConnection (this->Blender->GetOutputPort());
    this->ResliceInput = this->Blender->GetOutput();
  }
    
}



void vtkViewImage2D::RemoveOverlappingImage()
{

  if( this->GetMaskImage() )
  {
    this->MaskFilter->SetInput (this->AuxInput);
  }
  else
  {
    this->ImageReslice->SetInput (this->AuxInput);
    this->ResliceInput = this->AuxInput;
  }
  
  this->OverlappingImage = 0;
  
}

vtkActor* vtkViewImage2D::AddDataSet (vtkDataSet* dataset,  vtkProperty* property)
{

  bool doit = true;

  if (!dataset)
    doit = false;
  
  if( this->HasDataSet (dataset) )
  {
    doit = false;
  }
  vtkImageData* imagedata = NULL;
  imagedata = vtkImageData::SafeDownCast(dataset);
  
  if (imagedata)
  {
    this->SetImage(imagedata);
  }
  else
  {
    
    if ( !this->GetImage() )
    {
      doit = false;
    }
    // don't constrain the memory of input datasets anymore.
    /**
       
       if (dataset->GetActualMemorySize() > 20000)
       {
       vtkWarningMacro(<< "DataSet is to big for this projection type !\n");
       doit = false;
       }
       
    */
      
  int *extent  = this->ImageReslice->GetOutput()->GetExtent();  
  double *origin  = this->ImageReslice->GetOutput()->GetOrigin();
  double *spacing = this->ImageReslice->GetOutput()->GetSpacing();
  
    if (doit)
    {
      
      vtkMatrix4x4* matrix = vtkMatrix4x4::New();
      for (unsigned int i=0; i<3; i++)
      {
	for (unsigned int j=0; j<3; j++)
	{
	  matrix->SetElement(i,j,this->ImageReslice->GetResliceAxes()->GetElement(j,i));
	}
	matrix->SetElement(i,3,0);
      }
      matrix->SetElement(3,3,1);
      
      vtkCutter* cutter = vtkCutter::New();
      cutter->SetCutFunction (this->DataSetCutPlane);

      // Very strangely in some cases (ex : landmarks)
      // the cutter increments the RefCount of the input dataset by 2
      // making some memory leek...
      // I could not manage to know what is wrong here
      
      cutter->SetInput (dataset);
      cutter->Update();
      
      if (!cutter->GetOutput())
      {
	vtkWarningMacro(<< "Unable to cut this dataset...");
	matrix->Delete();
 	cutter->Delete();
	return NULL;
      }
      
      
      vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
      mapper->SetInput (cutter->GetOutput());
      
      vtkActor* actor = vtkActor::New();
      actor->SetUserMatrix (matrix);
      actor->SetMapper (mapper);
      if (property)
      {
	actor->SetProperty (property);
      }

      actor->PickableOff();
      
      this->AddActor (actor);
      this->DataSetList.push_back (dataset);
      this->DataSetActorList.push_back (actor);

      this->ResetAndRestablishZoomAndCamera();

      actor->Delete();
      mapper->Delete();
      matrix->Delete();
      cutter->Delete();
    }
  }

  
  return this->GetDataSetActor(dataset);
  
}


vtkActor* vtkViewImage2D::SyncAddPolyData (vtkPolyData* polydata,  vtkProperty* property, double thickness)
{

  if( this->IsLocked() )
  {
    return NULL;
  }
  

  vtkActor* actor = this->AddPolyData (polydata, property, thickness);
  
  this->Lock();
  for( unsigned int i=0; i<this->Children.size(); i++)
  {
//     vtkViewImage2D* view = dynamic_cast<vtkViewImage2D*> (this->Children[i]);
    vtkViewImage2D* view = vtkViewImage2D::SafeDownCast (this->Children[i]);
    if( view )
    {
      view->SyncAddPolyData (polydata, property, thickness);
    }
  }
  this->UnLock();

  return actor;
  
  
}


vtkActor* vtkViewImage2D::AddPolyData (vtkPolyData* polydata,  vtkProperty* property, double thickness)
{

  bool doit = true;
  if (!polydata || this->HasDataSet (polydata) || !this->GetImage())
  {
    doit = false;
  }
    

  if (doit)
  {
      
    if (thickness)
    {
      this->BoxThickness = thickness;
    }
    
    
    vtkClipDataSet* clipper = vtkClipDataSet::New();
    clipper->GenerateClippedOutputOff();
    clipper->InsideOutOn ();
    clipper->SetInput (polydata);
    
    unsigned int  direction = this->GetOrthogonalAxis (this->GetOrientation());
    switch(direction)
    {
      
	case X_ID :
	  this->DataSetCutBox->SetBounds (this->DataSetCutPlane->GetOrigin()[0]-0.5*this->BoxThickness,this->DataSetCutPlane->GetOrigin()[0]+0.5*this->BoxThickness,
					  this->GetWholeMinPosition(1),this->GetWholeMaxPosition(1),
					  this->GetWholeMinPosition(2),this->GetWholeMaxPosition(2));
	  
	  break;
	case Y_ID :
	  this->DataSetCutBox->SetBounds (this->GetWholeMinPosition(0),this->GetWholeMaxPosition(0),
					  this->DataSetCutPlane->GetOrigin()[1]-0.5*this->BoxThickness,this->DataSetCutPlane->GetOrigin()[1]+0.5*this->BoxThickness,
					  this->GetWholeMinPosition(2),this->GetWholeMaxPosition(2));
	  break;
	case Z_ID :
	  this->DataSetCutBox->SetBounds (this->GetWholeMinPosition(0),this->GetWholeMaxPosition(0),
					  this->GetWholeMinPosition(1),this->GetWholeMaxPosition(1),
					  this->DataSetCutPlane->GetOrigin()[2]-0.5*this->BoxThickness,this->DataSetCutPlane->GetOrigin()[2]+0.5*this->BoxThickness);
	  break;
    }
    
    clipper->SetClipFunction ( this->DataSetCutBox );
    clipper->Update();
    
    
    vtkMatrix4x4* matrix = vtkMatrix4x4::New();
    for (unsigned int i=0; i<3; i++)
    {
      for (unsigned int j=0; j<3; j++)
      {
	matrix->SetElement(i, j, this->ImageReslice->GetResliceAxes()->GetElement(j,i));
      }
    }
    matrix->SetElement(3,3,1);
    
    
    
    vtkDataSetMapper* mapper = vtkDataSetMapper::New();
    mapper->SetInput (clipper->GetOutput());
    
    vtkActor* actor = vtkActor::New();
    actor->SetMapper (mapper);
    actor->SetUserMatrix (matrix);  
    if (property)
    {
      actor->SetProperty (property);
    }
    
    
    this->AddActor (actor);
    this->DataSetList.push_back (polydata);
    this->DataSetActorList.push_back (actor);

    this->ResetAndRestablishZoomAndCamera();
    
    actor->Delete();
    mapper->Delete();
    matrix->Delete();
    clipper->Delete();
  }
  
  return this->GetDataSetActor(polydata);
  
}


void vtkViewImage2D::ResetZoom()
{
  vtkViewImage::ResetZoom();
  this->SetZoom ( 1.0 );
}


void vtkViewImage2D::Show2DAxis(const bool& show)
{
  this->HorizontalLineActor->SetVisibility(show);
  this->VerticalLineActor->SetVisibility(show);
}



void vtkViewImage2D::SetZoom (double factor)
{
  
  if (this->GetRenderer() == NULL)
  {
    return;
  }

  vtkViewImage::SetZoom ( factor ); //this->GetZoom() ); //*factor );

  
  vtkCamera *camera = this->GetRenderer()->GetActiveCamera();


  camera->SetParallelScale(this->InitialParallelScale / this->GetZoom() );
  //camera->GetParallelScale() / factor );

  
  if ( this->GetRenderWindowInteractor()->GetLightFollowCamera() )
  {
    this->GetRenderer()->UpdateLightsGeometryToFollowCamera();
  }
  
}



void vtkViewImage2D::SetConventionsToRadiological()
{
  this->Conventions = RADIOLOGIC;
  this->SetupAnnotations();
  this->UpdatePosition();
  this->ResetAndRestablishZoomAndCamera();
}


void vtkViewImage2D::SetConventionsToNeurological()
{
  this->Conventions = NEUROLOGIC;
  this->SetupAnnotations();
  this->UpdatePosition();
  this->ResetAndRestablishZoomAndCamera();
}

void vtkViewImage2D::SetupAnnotations()
{

  if( !this->GetImage() )
  {
    return;
  }
  
  int    *dims    = this->GetImage()->GetDimensions();
  double *spacing = this->GetImage()->GetSpacing();
  
  std::ostringstream os;
  os << "Image Size: ";
  
  switch(this->Orientation)
  {
    
      case vtkViewImage::AXIAL_ID:    // Z fixed

        if( this->Conventions == RADIOLOGIC )
        {
          
          this->ImageReslice->SetResliceAxesDirectionCosines(1, 0, 0,
                                                             0, -1, 0,
                                                             0, 0, 1);
        }
        else
        {
          this->ImageReslice->SetResliceAxesDirectionCosines(-1, 0, 0,
                                                             0, -1, 0,
                                                             0, 0, 1);
        }
        
        
        os << dims[0] << " x " << dims[1] << std::endl;
        os << "Voxel Size: " << spacing[0] << " x " << spacing[1] << " mm";
        
        if (this->ShowDirections)
        {
	  this->SetNorthAnnotation("A");
	  this->SetSouthAnnotation("P");
          if( this->Conventions == RADIOLOGIC )
          {
            this->SetEastAnnotation ("L");
            this->SetWestAnnotation("R");
          }
          else
          {
            this->SetEastAnnotation ("R");
            this->SetWestAnnotation("L");
          }
	}
        
        this->HorizontalLineActor->GetProperty()->SetColor (0.0,1.0,0.0);
        this->VerticalLineActor->GetProperty()->SetColor (0.0,0.0,1.0);
        
	break;
        
        
      case vtkViewImage::SAGITTAL_ID: // X fixed

        this->ImageReslice->SetResliceAxesDirectionCosines( 0, 1, 0,
                                                            0, 0, 1,
                                                            1, 0, 0);
        
        os << dims[1] << " x " << dims[2] << std::endl;
        os << "Voxel Size: " << spacing[1] << " x " << spacing[2] << " mm";
        
        this->HorizontalLineActor->GetProperty()->SetColor (1.0,0.0,0.0);
        this->VerticalLineActor->GetProperty()->SetColor (0.0,1.0,0.0);
        
	if (this->ShowDirections)
        {
	  this->SetNorthAnnotation("S");    
	  this->SetSouthAnnotation("I");    
	  this->SetEastAnnotation ("P");    
	  this->SetWestAnnotation("A");
	}

        break;
	
        
      case vtkViewImage::CORONAL_ID:  // Y fixed

        if( this->Conventions == RADIOLOGIC )
        { 
          this->ImageReslice->SetResliceAxesDirectionCosines(1,  0, 0,
                                                             0,  0, 1,
                                                             0,  1, 0);
        }
        else
        {
          this->ImageReslice->SetResliceAxesDirectionCosines(-1,  0, 0,
                                                             0,  0, 1,
                                                             0,  1, 0);
        }
        
          
        os << dims[0] << " x " << dims[2] << std::endl;
        os << "Voxel Size: " << spacing[0] << " x " << spacing[2] << " mm";              
          
        this->HorizontalLineActor->GetProperty()->SetColor (1.0,0.0,0.0);
        this->VerticalLineActor->GetProperty()->SetColor (0.0,0.0,1.0);
        
	if (this->ShowDirections)
        {
	  this->SetNorthAnnotation("S");
	  this->SetSouthAnnotation("I");

          if( this->Conventions == RADIOLOGIC)
          {
            this->SetEastAnnotation ("L");
            this->SetWestAnnotation("R");
          }
          else
          {
            this->SetEastAnnotation ("R");
            this->SetWestAnnotation("L");
          }
          
	}
	
	
	break;	
  }

  this->SetSizeData (os.str().c_str());
  
  if (this->GetShowAnnotations())
  {
    this->SetUpLeftAnnotation(this->GetSizeData());
  }
  
  this->SetDownLeftAnnotation(this->GetAboutData());
  
}


void vtkViewImage2D::ResetAndRestablishZoomAndCamera()
{

  if( !this->GetRenderer() )
  {
    return;
  }
  
  double zoom = this->GetZoom();
  vtkCamera *camera = this->GetRenderer()->GetActiveCamera();
  double c_position[3], focal[3];
  camera->GetPosition(c_position);
  camera->GetFocalPoint (focal);

  this->ResetZoom();
  double focal2[3], pos2[3];
  camera->GetFocalPoint (focal2);
  camera->GetPosition (pos2);
  
  //
  camera->SetFocalPoint (focal[0], focal[1], focal2[2]);
  camera->SetPosition (c_position[0], c_position[1], pos2[2]);
  
  this->SetZoom ( zoom/this->GetZoom() );

}


void vtkViewImage2D::SetCameraFocalAndPosition (double focal[3], double pos[3])
{
  if( !this->GetRenderer() )
  {
    return;
  }

  vtkCamera *camera = this->GetRenderer()->GetActiveCamera();
  double c_position[3], c_focal[3];
  camera->GetPosition(c_position);
  camera->GetFocalPoint (c_focal);

  camera->SetFocalPoint (focal[0], focal[1], c_focal[2]);
  camera->SetPosition   (pos[0], pos[1], c_position[2]);
  
}


void vtkViewImage2D::GetCameraFocalAndPosition (double focal[3], double pos[3])
{
  if( !this->GetRenderer() )
  {
    return;
  }

  vtkCamera *camera = this->GetRenderer()->GetActiveCamera();
  camera->GetPosition(pos);
  camera->GetFocalPoint (focal);
}


void vtkViewImage2D::SyncSetCameraFocalAndPosition (double focal[3], double pos[3])
{
  
  if( this->IsLocked() )
  {
    return;
  }

  if( this->GetLinkCameraFocalAndPosition() )
  {
    this->SetCameraFocalAndPosition (focal, pos);
  }
  
  this->Lock();
  for( unsigned int i=0; i<this->Children.size(); i++)
  {
//     vtkViewImage2D* view = dynamic_cast<vtkViewImage2D*> (this->Children[i]);
    vtkViewImage2D* view = vtkViewImage2D::SafeDownCast (this->Children[i]);

    if( view /*&& view->GetLinkCameraFocalAndPosition()*/ )
    {
      view->SyncSetCameraFocalAndPosition (focal, pos);
      if( !view->GetRenderWindow()->GetNeverRendered() )
      {
        view->Render();
      }
    }
  }
  this->UnLock();
  
}


void vtkViewImage2D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Orientation = " << this->Orientation << "\n";  
  double* bounds   = 0;
  double* spacing  = 0;
  double* origin   = 0;
  int*    extent   = 0;
  int*    wextent  = 0;
  int*    uextent  = 0;

  if( !this->GetImage() )
  {
    return;
  }
  
  
  this->GetImage()->Update();  
  this->ImageReslice->GetOutput()->Update();  
  this->WindowLevel->GetOutput()->Update();
  
  this->GetImage()->GetBounds();
  this->ImageReslice->GetOutput()->GetBounds();
  this->WindowLevel->GetOutput()->GetBounds();
  this->ImageActor->GetBounds();
    
  extent  = this->GetImage()->GetExtent();  
  uextent = this->GetImage()->GetUpdateExtent();  
  wextent = this->GetImage()->GetWholeExtent();  
  origin  = this->GetImage()->GetOrigin();
  spacing = this->GetImage()->GetSpacing();
  os << indent << "InputImage   - "<<this->GetImage()->GetNumberOfPoints()<<" points.\n"
     << "  extent : ("
     << extent[0] << "/" << extent[1] << " ; "
     << extent[2] << "/" << extent[3] << " ; "
     << extent[4] << "/" << extent[5] << ") \n"  
     << " uextent : ("
     << uextent[0] << "/" << uextent[1] << " ; "
     << uextent[2] << "/" << uextent[3] << " ; "
     << uextent[4] << "/" << uextent[5] << ") \n"  
     << " wextent : ("
     << wextent[0] << "/" << wextent[1] << " ; "
     << wextent[2] << "/" << wextent[3] << " ; "
     << wextent[4] << "/" << wextent[5] << ") \n"  
     << "  origin : ("
     << origin[0] << " , " << origin[1] << " , " << origin[2] << ")\n"
     << "  spacing : ("
     << spacing[0] << " , " << spacing[1] << " , " << spacing[2] << ")\n";
  
  extent  = this->ImageReslice->GetOutput()->GetExtent();  
  uextent = this->ImageReslice->GetOutput()->GetUpdateExtent();  
  wextent = this->ImageReslice->GetOutput()->GetWholeExtent();  
  origin  = this->ImageReslice->GetOutput()->GetOrigin();
  spacing = this->ImageReslice->GetOutput()->GetSpacing();
  os << indent << "Resliced Image   - "<<this->ImageReslice->GetOutput()->GetNumberOfPoints()<<" points.\n"
     << "  extent : ("
     << extent[0] << "/" << extent[1] << " ; "
     << extent[2] << "/" << extent[3] << " ; "
     << extent[4] << "/" << extent[5] << ") \n" 
     << " uextent : ("
     << uextent[0] << "/" << uextent[1] << " ; "
     << uextent[2] << "/" << uextent[3] << " ; "
     << uextent[4] << "/" << uextent[5] << ") \n"  
     << " wextent : ("
     << wextent[0] << "/" << wextent[1] << " ; "
     << wextent[2] << "/" << wextent[3] << " ; "
     << wextent[4] << "/" << wextent[5] << ") \n"  
     << "  origin : ("
     << origin[0] << " , " << origin[1] << " , " << origin[2] << ")\n"
     << "  spacing : ("
     << spacing[0] << " , " << spacing[1] << " , " << spacing[2] << ")\n";
  
  extent  = this->WindowLevel->GetOutput()->GetExtent();  
  uextent = this->WindowLevel->GetOutput()->GetUpdateExtent();  
  wextent = this->WindowLevel->GetOutput()->GetWholeExtent();  
  origin  = this->WindowLevel->GetOutput()->GetOrigin();
  spacing = this->WindowLevel->GetOutput()->GetSpacing();
  os << indent << "WindowLevel Image   - "<<this->WindowLevel->GetOutput()->GetNumberOfPoints()<<" points.\n"
     << "  extent : ("
     << extent[0] << "/" << extent[1] << " ; "
     << extent[2] << "/" << extent[3] << " ; "
     << extent[4] << "/" << extent[5] << ") \n"
     << " uextent : ("
     << uextent[0] << "/" << uextent[1] << " ; "
     << uextent[2] << "/" << uextent[3] << " ; "
     << uextent[4] << "/" << uextent[5] << ") \n"  
     << " wextent : ("
     << wextent[0] << "/" << wextent[1] << " ; "
     << wextent[2] << "/" << wextent[3] << " ; "
     << wextent[4] << "/" << wextent[5] << ") \n"  
     << "  origin : ("
     << origin[0] << " , " << origin[1] << " , " << origin[2] << ")\n"
     << "  spacing : ("
     << spacing[0] << " , " << spacing[1] << " , " << spacing[2] << ")\n";
  
  bounds = this->GetImage()->GetBounds();  
  os << indent << "InputImage \n"
     << "           bounds : ("
     << bounds[0] << "/" << bounds[1] << " ; "
     << bounds[2] << "/" << bounds[3] << " ; "
     << bounds[4] << "/" << bounds[5] << ") \n";
  bounds = this->ImageReslice->GetOutput()->GetBounds();  
  os << indent << "Resliced Image \n"
     << "           bounds : ("
     << bounds[0] << "/" << bounds[1] << " ; "
     << bounds[2] << "/" << bounds[3] << " ; "
     << bounds[4] << "/" << bounds[5] << ") \n";
  bounds = this->WindowLevel->GetOutput()->GetBounds();  
  os << indent << "WindowLevel Image \n"
     << "           bounds : ("
     << bounds[0] << "/" << bounds[1] << " ; "
     << bounds[2] << "/" << bounds[3] << " ; "
     << bounds[4] << "/" << bounds[5] << ") \n";
  bounds = this->ImageActor->GetBounds();  
  os << indent << "Image Actor  bounds : ("
     << bounds[0] << "/" << bounds[1] << " ; "
     << bounds[2] << "/" << bounds[3] << " ; "
     << bounds[4] << "/" << bounds[5] << ") \n";

}



