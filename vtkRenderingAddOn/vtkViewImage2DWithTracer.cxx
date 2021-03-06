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
#include <vtkViewImage2DWithTracer.h>

#include "vtkImageActor.h"
#include <vtkFillImageWithPolyData.h>
#include "vtkGlyphSource2D.h"
#include "vtkProperty.h"
#include "vtkMapper.h"
#include "vtkInteractorObserver.h"
#include <vtkRenderWindowInteractor.h>

#include <sstream>
#include <assert.h>
#include <cmath>

#include <vtkObjectFactory.h>

vtkCxxRevisionMacro(vtkViewImage2DWithTracer, "$Revision$");
vtkStandardNewMacro(vtkViewImage2DWithTracer);


vtkViewImage2DWithTracer::vtkViewImage2DWithTracer()
{

  this->Tracing      = 0;
  this->TracerWidget = vtkImageTracerWidget::New();
  this->TracerCbk    = vtkImageTracerWidgetCallback::New();
  this->LUT          = vtkLookupTable::New();
  this->CurrentLabel = 1;

  vtkMapper::SetResolveCoincidentTopologyToDefault(); // changed in vtkImageTracerWidget::New()
  
  // Setting tracer widget properties
  //this->TracerWidget->SetDefaultRenderer ( this->GetRenderer() );
  this->TracerWidget->SetCaptureRadius(9999999.9); // this automtically closes the tracing when user release click
  this->TracerWidget->AutoCloseOn();
  this->TracerWidget->GetGlyphSource()->SetScale(2.0);
  this->TracerWidget->GetGlyphSource()->SetRotationAngle(45.0);  
  this->TracerWidget->GetGlyphSource()->Update();

  this->TracerWidget->GetHandleProperty()->SetColor (0.24,0.56,0.86);
  this->TracerWidget->GetHandleProperty()->SetLineWidth (3.0);
  this->TracerWidget->GetSelectedHandleProperty()->SetColor (1.0,0.62,0.0);
  this->TracerWidget->GetSelectedHandleProperty()->SetLineWidth (3.0);
  
  this->TracerWidget->GetLineProperty()->SetLineWidth (3.0);
  this->TracerWidget->GetLineProperty()->SetColor (0.24, 0.86, 0.41);
  this->TracerWidget->GetSelectedLineProperty()->SetLineWidth (3.0);
  
  this->TracerWidget->ProjectToPlaneOn();  
  this->TracerWidget->SetProjectionNormalToZAxes();
  // this->TracerWidget->SetProjectionPosition(0.0);
  this->TracerWidget->SetProjectionPosition(0.1);
  this->TracerWidget->SetViewProp( this->GetImageActor() );
  this->TracerWidget->SetPriority(10.0);

  this->TracerCbk->SetView (this);
  this->TracerWidget->AddObserver (vtkCommand::EndInteractionEvent, this->TracerCbk);
  this->TracerWidget->SetKeyPressActivationValue ('j'); // disable "i"

  this->LUT->SetNumberOfTableValues (256);
  this->LUT->SetTableValue (0, 0.0, 0.0, 0.0, 0.0); // erase color

  this->LUT->SetTableValue (1, 1.0, 0.0, 0.0, 0.5);
  this->LUT->SetTableValue (2, 0.0, 1.0, 0.0, 0.5);
  this->LUT->SetTableValue (3, 0.0, 0.0, 1.0, 0.5);
  this->LUT->SetTableValue (4, 1.0, 1.0, 0.0, 0.5);
  this->LUT->SetTableValue (5, 0.0, 1.0, 1.0, 0.5);
  this->LUT->SetTableValue (6, 1.0, 0.0, 1.0, 0.5);

  
  // Fill the rest of the labels with color ramps, code taken from SNAP
  for (int i = 7; i < 256; i++)
  {
    if (i < 85)
    {
      this->LUT->SetTableValue (i, ((84.0-i)/85.0 * 200.0 + 50.0)/255.0, (i/85.0 * 200.0 + 50.0)/255.0, 0, 0.5);
    }
    else if (i < 170)
    {
      this->LUT->SetTableValue (i, 0, ((169.0-i)/85.0 * 200.0 + 50)/255.0, ((i-85)/85.0 * 200.0 + 50)/255.0, 0.5);
    }
    else
    {
      this->LUT->SetTableValue (i, ((i-170)/85.0 * 200.0 + 50)/255.0, 0.0, ((255.0-i)/85.0 * 200.0 + 50)/255.0, 0.5);
    }
  }


  this->SetDownRightAnnotation("Tracer: Off");
}



void vtkViewImage2DWithTracer::Initialize ()
{
  vtkViewImage2D::Initialize();

  if( this->GetRenderWindowInteractor() )
  {
    this->GetRenderWindowInteractor()->AddObserver (vtkCommand::KeyPressEvent, this->TracerCbk, -10.0);
    this->TracerWidget->SetInteractor ( this->GetRenderWindowInteractor() );
  }
  
}


void vtkViewImage2DWithTracer::Uninitialize ()
{

  if( this->GetRenderWindowInteractor() )
  {
    this->GetRenderWindowInteractor()->RemoveObserver ( this->TracerCbk );
    this->TracerWidget->SetInteractor ( NULL );
  }
 
  vtkViewImage2D::Uninitialize();
}



void vtkViewImage2DWithTracer::SetImage (vtkImageData* image)
{
  vtkViewImage2D::SetImage (image);

  if ( this->GetImage() )
  {
    this->TracerWidget->SetInput ( this->GetImage() );
    this->TracerWidget->PlaceWidget();
    this->TracerWidget->Off();
  }
}

  

vtkViewImage2DWithTracer::~vtkViewImage2DWithTracer()
{
  this->TracerCbk->Delete();
  this->TracerWidget->Delete();
  this->LUT->Delete();
}



void vtkViewImage2DWithTracer::PrepareForDelete()
{
  this->TracerWidget->Off();
  this->RemoveObserver (this->TracerCbk);
}



void vtkViewImage2DWithTracer::SetLUTValue (const int& index, const double& R,
                                            const double& G,
                                            const double& B,
                                            const double& A)
{
  if( index<0 || index>255 )
  {
    std::cerr << "Index for LUT is too large (min is 0, max is 255)!" << std::endl;
    return;
  }

  this->LUT->SetTableValue (index, R, G, B, A);
  
}



void vtkViewImage2DWithTracer::ValidateTracing()
{

  if( !this->GetImage() )
  {
    return;
  }
  

  vtkPolyData* tracing = this->TracerCbk->GetPolyData();
  if( !tracing || tracing->GetNumberOfPoints()<3 )
  {
    return;
  }

  double val = this->CurrentLabel;
  
  vtkFillImageWithPolyData* filler = vtkFillImageWithPolyData::New();
  filler->SetInput ( this->GetImage() );


        
  switch( this->GetOrientation() )
  {          
      case vtkViewImage2D::AXIAL_ID:
        filler->SetExtractionDirection (2);
        break;
        
      case vtkViewImage2D::CORONAL_ID:
        filler->SetExtractionDirection (1);        
        break;
        
      case vtkViewImage2D::SAGITTAL_ID:
        filler->SetExtractionDirection (0);
        break;
  }
  
  filler->SetPolyData (tracing);
  filler->SetInsidePixelValue (val);
  filler->Update();

  this->Tracing = this->GetMaskImage();
  
  // must check if the dimensions of the image match the dimensions of
  // the tracing
  if( this->Tracing )
  {
    int* dims1 = this->Tracing->GetDimensions();
    int* dims2 = this->GetImage()->GetDimensions();
    if( (dims1[0]!=dims2[0]) || (dims1[1]!=dims2[1]) || (dims1[2]!=dims2[2]) )
    {
      // I set to 0 the previous tracing
      this->Tracing = 0;
    }
  }
    
  
  // the output of the filler is merged with the previous tracing, if any
  if( this->Tracing )
  {   
    // do the merging myself
    int numPoints = this->Tracing->GetNumberOfPoints();
    unsigned char* buff1 = (unsigned char*)(this->Tracing->GetScalarPointer());
    unsigned char* buff2 = (unsigned char*)(filler->GetOutput()->GetScalarPointer());

    for( int i=0; i<numPoints; i++)
    {
      int val = (int)(*buff2);
      if ( val==255 ) // erase label in fact
        *buff1 = (unsigned char)(0);
      else if( val>0 )
        *buff1 = (unsigned char)(val);
      
      ++buff1;
      ++buff2;
    }

    this->Tracing->Modified();
    
    this->SyncSetMaskImage (this->Tracing, this->LUT);
    
  }
  else
  {
    this->Tracing = vtkImageData::New();
    this->Tracing->DeepCopy ( filler->GetOutput() );
    this->Tracing->SetOrigin( this->GetImage()->GetOrigin() );

    this->SyncSetMaskImage (this->Tracing, this->LUT);
    
    this->Tracing->Delete();
  }

  filler->Delete();

}


void vtkViewImage2DWithTracer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf (os, indent);
}

void vtkViewImage2DWithTracer::SetConventionsToRadiological()
{
  int tracerOn = this->TracerWidget->GetEnabled();
  this->SetManualTracingVisibility(0);
  vtkViewImage2D::SetConventionsToRadiological();
  if( tracerOn )
  {
    this->SetManualTracingVisibility(1);
  }
  
}

void vtkViewImage2DWithTracer::SetConventionsToNeurological()
{
  int tracerOn = this->TracerWidget->GetEnabled();
  this->SetManualTracingVisibility(0);
  vtkViewImage2D::SetConventionsToNeurological();
  if( tracerOn )
  {
    this->SetManualTracingVisibility(1);
  }
  
}
