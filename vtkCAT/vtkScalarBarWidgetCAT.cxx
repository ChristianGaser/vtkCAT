/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkScalarBarWidgetCAT.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkScalarBarWidgetCAT.h"
#include "vtkScalarBarActor.h"
#include "vtkCallbackCommand.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkCoordinate.h"
#include "vtkLookupTable.h"
#include "vtkMapper.h"

vtkCxxRevisionMacro(vtkScalarBarWidgetCAT, "$Revision: 1.7 $");
vtkStandardNewMacro(vtkScalarBarWidgetCAT);
vtkCxxSetObjectMacro(vtkScalarBarWidgetCAT, ScalarBarActor, vtkScalarBarActor);

//-------------------------------------------------------------------------
vtkScalarBarWidgetCAT::vtkScalarBarWidgetCAT()
{
  this->ScalarBarActor = vtkScalarBarActor::New();
  this->EventCallbackCommand->SetCallback(vtkScalarBarWidgetCAT::ProcessEvents);
  this->State = vtkScalarBarWidgetCAT::Outside;
  this->LeftButtonDown = 0;
  this->RightButtonDown = 0;
  this->Priority = 0.55;
}

//-------------------------------------------------------------------------
vtkScalarBarWidgetCAT::~vtkScalarBarWidgetCAT()
{
  this->SetScalarBarActor(0);
}

//-------------------------------------------------------------------------
void vtkScalarBarWidgetCAT::SetEnabled(int enabling)
{
  if ( ! this->Interactor )
    {
    vtkErrorMacro(<<"The interactor must be set prior to enabling/disabling widget");
    return;
    }
  
  if ( enabling ) 
    {
    vtkDebugMacro(<<"Enabling line widget");
    if ( this->Enabled ) //already enabled, just return
      {
      return;
      }
    
    if ( ! this->CurrentRenderer )
      {
      this->SetCurrentRenderer(this->Interactor->FindPokedRenderer(
        this->Interactor->GetLastEventPosition()[0],
        this->Interactor->GetLastEventPosition()[1]));
      if (this->CurrentRenderer == NULL)
        {
        return;
        }
      }

    this->Enabled = 1;
    
    // listen for the following events
    vtkRenderWindowInteractor *i = this->Interactor;
    i->AddObserver(vtkCommand::MouseMoveEvent, 
                   this->EventCallbackCommand, this->Priority);
    i->AddObserver(vtkCommand::LeftButtonPressEvent, 
                   this->EventCallbackCommand, this->Priority);
    i->AddObserver(vtkCommand::LeftButtonReleaseEvent, 
                   this->EventCallbackCommand, this->Priority);
    i->AddObserver(vtkCommand::RightButtonPressEvent, 
                   this->EventCallbackCommand, this->Priority);
    i->AddObserver(vtkCommand::RightButtonReleaseEvent, 
                   this->EventCallbackCommand, this->Priority);

    // Add the scalar bar
    this->CurrentRenderer->AddViewProp(this->ScalarBarActor);
    this->InvokeEvent(vtkCommand::EnableEvent,NULL);
    }
  else //disabling------------------------------------------
    {
    vtkDebugMacro(<<"Disabling line widget");
    if ( ! this->Enabled ) //already disabled, just return
      {
      return;
      }
    this->Enabled = 0;

    // don't listen for events any more
    this->Interactor->RemoveObserver(this->EventCallbackCommand);

    // turn off the line
    this->CurrentRenderer->RemoveActor(this->ScalarBarActor);
    this->InvokeEvent(vtkCommand::DisableEvent,NULL);
    this->SetCurrentRenderer(NULL);
    }
  
  this->Interactor->Render();
}

//-------------------------------------------------------------------------
void vtkScalarBarWidgetCAT::ProcessEvents(vtkObject* vtkNotUsed(object), 
                                       unsigned long event,
                                       void* clientdata, 
                                       void* vtkNotUsed(calldata))
{
  vtkScalarBarWidgetCAT* self = reinterpret_cast<vtkScalarBarWidgetCAT *>( clientdata );
  
  //okay, let's do the right thing
  switch(event)
    {
    case vtkCommand::LeftButtonPressEvent:
      self->OnLeftButtonDown();
      break;
    case vtkCommand::LeftButtonReleaseEvent:
      self->OnLeftButtonUp();
      break;
    case vtkCommand::RightButtonPressEvent:
      self->OnRightButtonDown();
      break;
    case vtkCommand::RightButtonReleaseEvent:
      self->OnRightButtonUp();
      break;
    case vtkCommand::MouseMoveEvent:
      self->OnMouseMove();
      break;
    }
}

//-------------------------------------------------------------------------
int vtkScalarBarWidgetCAT::ComputeStateBasedOnPosition(int X, int Y, 
                                                    int *pos1, int *pos2)
{
  int Result;
  
  // assume we are moving
  Result = vtkScalarBarWidgetCAT::Moving;

  return Result;
}

//-------------------------------------------------------------------------
void vtkScalarBarWidgetCAT::OnLeftButtonDown()
{
  // We're only here is we are enabled
  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  // are we over the widget?
  //this->Interactor->FindPokedRenderer(X,Y);
  int *pos1 = this->ScalarBarActor->GetPositionCoordinate()
    ->GetComputedDisplayValue(this->CurrentRenderer);
  int *pos2 = this->ScalarBarActor->GetPosition2Coordinate()
    ->GetComputedDisplayValue(this->CurrentRenderer);

  // are we not over the scalar bar, ignore
  if (X < pos1[0] || X > pos2[0] || Y < pos1[1] || Y > pos2[1])
    {
    return;
    }
  
  // start a drag, store the normalized view coords
  double X2 = X;
  double Y2 = Y;
  // convert to normalized viewport coordinates
  this->CurrentRenderer->DisplayToNormalizedDisplay(X2,Y2);
  this->CurrentRenderer->NormalizedDisplayToViewport(X2,Y2);
  this->CurrentRenderer->ViewportToNormalizedViewport(X2,Y2);
  this->StartPosition[0] = X2;
  this->StartPosition[1] = Y2;

  this->State = this->ComputeStateBasedOnPosition(X, Y, pos1, pos2);
  
  this->EventCallbackCommand->SetAbortFlag(1);
  this->StartInteraction();
  this->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  this->LeftButtonDown = 1;
}

//-------------------------------------------------------------------------
void vtkScalarBarWidgetCAT::OnMouseMove()
{
  // compute some info we need for all cases
  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  double *range;
  vtkScalarsToColors* lut = this->ScalarBarActor->GetLookupTable();
  vtkLookupTable* scalarBar = vtkLookupTable::New();
  range = lut->GetRange();

   // compute the display bounds of the scalar bar if we are inside or outside
  int *pos1, *pos2;
  if (this->State == vtkScalarBarWidgetCAT::Outside ||
      this->State == vtkScalarBarWidgetCAT::Inside)
    {
    pos1 = this->ScalarBarActor->GetPositionCoordinate()
      ->GetComputedDisplayValue(this->CurrentRenderer);
    pos2 = this->ScalarBarActor->GetPosition2Coordinate()
      ->GetComputedDisplayValue(this->CurrentRenderer);
  
    if (this->State == vtkScalarBarWidgetCAT::Outside)
      {
      // if we are not over the scalar bar, ignore
      if (X < pos1[0] || X > pos2[0] ||
          Y < pos1[1] || Y > pos2[1])
        {
        return;
        }
      // otherwise change our state to inside
      this->State = vtkScalarBarWidgetCAT::Inside;
      }
  
    // if inside, set the cursor to the correct shape
    if (this->State == vtkScalarBarWidgetCAT::Inside)
      {
      // if we have left then change cursor back to default
      if (X < pos1[0] || X > pos2[0] ||
          Y < pos1[1] || Y > pos2[1])
        {
        this->State = vtkScalarBarWidgetCAT::Outside;
        return;
        }
      return;
      }
    }
  
  double XF = X;
  double YF = Y;
  // convert to normalized viewport coordinates
  this->CurrentRenderer->DisplayToNormalizedDisplay(XF,YF);
  this->CurrentRenderer->NormalizedDisplayToViewport(XF,YF);
  this->CurrentRenderer->ViewportToNormalizedViewport(XF,YF);
  
  // there are four parameters that can be adjusted
  double *fpos1 = this->ScalarBarActor->GetPositionCoordinate()->GetValue();
  double *fpos2 = this->ScalarBarActor->GetPosition2Coordinate()->GetValue();
  
  double rdiff = range[1] - range[0];
  
  // check whether the left or right part of the scalebar is selected 
  if (XF < (fpos1[0] + fpos2[0]/2.0))
    range[0] += rdiff*(XF - this->StartPosition[0])/fpos2[0]*2.0;
  else 
    range[1] += rdiff*(XF - this->StartPosition[0])/fpos2[0]*2.0;

  // ensure that range[0] is always smaller than range[1]
  if (range[0] >= range[1])
    range[0] = range[1] - 1e-3;

  scalarBar->SetTableRange(range);
//  scalarBar->SetHueRange( 0.667, 0.0 );
//  scalarBar->SetSaturationRange( 1, 1 );
//  scalarBar->SetValueRange( 1, 1 );
  scalarBar->Build();
  this->ScalarBarActor->SetLookupTable(scalarBar);

  // start a drag
  this->EventCallbackCommand->SetAbortFlag(1);
  this->InvokeEvent(vtkCommand::InteractionEvent,NULL);
  this->Interactor->Render();

  // update range of scalar mapper
  vtkActorCollection* actorCollection = this->Interactor->FindPokedRenderer(0.75,0.75)->GetActors();
  actorCollection->GetLastActor()->GetMapper()->SetScalarRange(range);
}

//-------------------------------------------------------------------------
void vtkScalarBarWidgetCAT::OnLeftButtonUp()
{
  if (this->State == vtkScalarBarWidgetCAT::Outside || this->LeftButtonDown == 0)
    {
    return;
    }

  // stop adjusting
  this->State = vtkScalarBarWidgetCAT::Outside;
  this->EventCallbackCommand->SetAbortFlag(1);
  this->EndInteraction();
  this->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  this->LeftButtonDown = 0;
}

//-------------------------------------------------------------------------
void vtkScalarBarWidgetCAT::OnRightButtonDown()
{

  // are we not over the scalar bar, ignore
  if (this->State == vtkScalarBarWidgetCAT::Outside)
    {
    return;
    }

  if (this->HasObserver(vtkCommand::RightButtonPressEvent) ) 
    {
    this->EventCallbackCommand->SetAbortFlag(1);
    this->InvokeEvent(vtkCommand::RightButtonPressEvent,NULL);
    }
  RightButtonDown = 1;
}

//-------------------------------------------------------------------------
void vtkScalarBarWidgetCAT::OnRightButtonUp()
{
  if ( this->RightButtonDown == 0 ) 
    {
    return;
    }

  if (this->HasObserver(vtkCommand::RightButtonReleaseEvent)) 
    {
    this->EventCallbackCommand->SetAbortFlag(1);
    this->InvokeEvent(vtkCommand::RightButtonReleaseEvent,NULL);
    }
  this->RightButtonDown = 0;
}

//-------------------------------------------------------------------------
void vtkScalarBarWidgetCAT::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  os << indent << "ScalarBarActor: " << this->ScalarBarActor << "\n";
}
