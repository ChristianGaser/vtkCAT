/*=========================================================================

Program:   vtkINRIA3D
Module:    $Id: vtkSynchronizedView.cxx 1137 2009-04-03 15:31:45Z filus $
Language:  C++
Author:    $Author: filus $
Date:      $Date: 2009-04-03 17:31:45 +0200 (Fr, 03 Apr 2009) $
Version:   $Revision: 1137 $

Copyright (c) 2007 INRIA - Asclepios Project. All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
// version vtkRenderingAddOn
#include <vtkSynchronizedView.h>

#include "vtkObjectFactory.h"
#include "vtkInteractorObserver.h"
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
#include <vtkOrientationAnnotation.h>

#include "assert.h"
#include <algorithm>

int VTK_RENDERINGADDON_EXPORT vtkrint(double a)
{  
#ifdef WIN32
  double test = fabs ( a - double(int(a)) );
  int res = 0;
  if(a>0)
    (test>0.5)?(res=a+1):res=a;  
  else
    (test>0.5)?(res=a-1):res=a;

  return res;   
#else
  return (int)rint(a);
#endif // WIN32
}


vtkCxxRevisionMacro(vtkSynchronizedView, "$Revision: 1137 $");
vtkStandardNewMacro(vtkSynchronizedView);



vtkSynchronizedView::vtkSynchronizedView()
{
  
  this->IsProcessed     = false;
  this->InteractionOn   = true;
  this->ShowAnnotations = true;

  this->Renderer               = 0;
  this->RenderWindow           = 0;
  this->RenderWindowInteractor = 0;
  //this->InteractorStyle        = 0;

  // Initilize Annotations
  this->CornerAnnotation = vtkCornerAnnotation::New();
  this->CornerAnnotation->SetNonlinearFontScaleFactor (0.3);

  this->TextProperty = vtkTextProperty::New();
  this->CornerAnnotation->SetTextProperty ( this->TextProperty );
  
  this->OrientationAnnotation = vtkOrientationAnnotation::New();
  this->OrientationAnnotation->SetNonlinearFontScaleFactor (0.25);

  this->Parent = 0;

  this->LinkRender = true;

}



vtkSynchronizedView::~vtkSynchronizedView()
{	
  this->TextProperty->Delete();
  this->CornerAnnotation->Delete();
  this->OrientationAnnotation->Delete();

  if( this->RenderWindow )
  {
    this->RenderWindow->Delete();
	this->RenderWindow = 0;
  }


  if( this->Renderer )
  {
    this->Renderer->Delete();
	this->Renderer = 0;
  }

  if( this->RenderWindowInteractor )
  {
    this->RenderWindowInteractor->Delete();
	this->RenderWindowInteractor = 0;
  }

  /*
  if( this->InteractorStyle )
  {
    this->InteractorStyle->Delete();
	this->InteractorStyle = 0;
  }*/

  

}


//----------------------------------------------------------------------------
void vtkSynchronizedView::SetRenderWindow(vtkRenderWindow *arg)
{
  if (this->RenderWindow == arg)
  {
    return;
  }

  this->Uninitialize();
  
  if (this->RenderWindow)
  {
    this->RenderWindow->UnRegister(this);
  }
  
  this->RenderWindow = arg;
  
  if (this->RenderWindow)
  {
    this->RenderWindow->Register(this);
  }

  if (this->RenderWindow && this->RenderWindow->GetInteractor())
  {
    this->SetInteractor (this->RenderWindow->GetInteractor());
  }
  this->Initialize();
}

//----------------------------------------------------------------------------
void vtkSynchronizedView::SetRenderer(vtkRenderer *arg)
{
  if (this->Renderer == arg)
  {
    return;
  }

  this->Uninitialize();
  
  if (this->Renderer)
  {
    this->Renderer->UnRegister(this);
  }
  
  this->Renderer = arg;
  
  if (this->Renderer)
  {
    this->Renderer->Register(this);
  }

  this->Initialize();

}

//----------------------------------------------------------------------------
void vtkSynchronizedView::SetInteractor(vtkRenderWindowInteractor *arg)
{
  if (this->RenderWindowInteractor == arg)
  {
    return;
  }

  this->Uninitialize();
  
  if (this->RenderWindowInteractor)
  {
    this->RenderWindowInteractor->UnRegister(this);
  }
  
  this->RenderWindowInteractor = arg;
  
  if (this->RenderWindowInteractor)
  {
    this->RenderWindowInteractor->Register(this);
  }
  
  this->Initialize();
}

/*
void vtkSynchronizedView::SetInteractorStyle(vtkInteractorStyle* style)
{
 
  if (this->InteractorStyle == style)
  {
    return;
  }

  this->Uninitialize();

  if (this->InteractorStyle)
  {
    this->InteractorStyle->UnRegister(this);
  }
  
  this->InteractorStyle = style;
  
  
  if (this->InteractorStyle)
  {
    this->InteractorStyle->Register(this);
  }

  this->Initialize();

	this->Uninitialize();
	this->InteractorStyle = style;
	this->Initialize();

  //this->SetInteraction();
  //this->Modified();
}
*/


//----------------------------------------------------------------------------
void vtkSynchronizedView::Initialize()
{
  if (this->Renderer)
  {
    this->Renderer->SetBackground(0.9,0.9,0.9);
	this->Renderer->AddViewProp ( this->CornerAnnotation );
	this->Renderer->AddViewProp ( this->OrientationAnnotation );
  }
  
  if (this->RenderWindow && this->Renderer)
  {
    this->RenderWindow->AddRenderer(this->Renderer);  
  }

  if (this->RenderWindowInteractor )
  {/*
	  if ( this->InteractorStyle )
	  {
		this->RenderWindowInteractor->SetInteractorStyle( this->InteractorStyle );
		this->SetInteraction();
	  }
   */
    this->RenderWindowInteractor->SetRenderWindow(this->RenderWindow);
  }
}

//----------------------------------------------------------------------------
void vtkSynchronizedView::Uninitialize()
{
  if (this->Renderer)
  {
    this->Renderer->RemoveViewProp( this->CornerAnnotation );
	this->Renderer->RemoveViewProp( this->OrientationAnnotation );
  }
  
  if (this->RenderWindow && this->Renderer)
  {
    this->RenderWindow->RemoveRenderer(this->Renderer);
  }
  /*
  if (this->RenderWindowInteractor)
  {
    //this->RenderWindowInteractor->SetInteractorStyle(NULL);
    //this->RenderWindowInteractor->SetRenderWindow(NULL); // never!
  }*/
}


std::vector < vtkSynchronizedView* > vtkSynchronizedView::GetViewToObserve(void) const
{ 
  return this->GetChildren(); 
}

void 
vtkSynchronizedView::AddActor (vtkProp* actor)
{
  if (this->GetRenderer())
  {
    this->GetRenderer()->AddActor(actor);
  }
}


void 
vtkSynchronizedView::RemoveActor (vtkProp* actor)
{
  if (this->GetRenderer())
  {
    this->GetRenderer()->RemoveActor(actor);
  }
}

void
vtkSynchronizedView::SetInteractionOff()
{
  this->InteractionOn = false;
  this->SetInteraction();  
}

void
vtkSynchronizedView::SetInteractionOn()
{
  this->InteractionOn = true;
  this->SetInteraction();  
}

void
vtkSynchronizedView::SetInteraction()
{
	if ( this->GetRenderWindowInteractor() && this->GetRenderWindowInteractor()->GetInteractorStyle() )
  {
	  if ( !this->GetInteractionOn() )
	  {
		  this->GetRenderWindowInteractor()->GetInteractorStyle()->SetEnabled(0);
	  }
	  else
	  {
	      this->GetRenderWindowInteractor()->GetInteractorStyle()->SetEnabled(1);
	  }
  }
}


void
vtkSynchronizedView::SetUpLeftAnnotation(const char *p_annotation)
{
  this->upLeftAnnotation = p_annotation;  
  this->UpdateAnnotations();  
}
const char *
vtkSynchronizedView::GetUpLeftAnnotation()
{
  return this->upLeftAnnotation.c_str();  
}
void
vtkSynchronizedView::SetUpRightAnnotation(const char *p_annotation)
{
  this->upRightAnnotation = p_annotation;  
  this->UpdateAnnotations();  
}
const char *
vtkSynchronizedView::GetUpRightAnnotation()
{
  return this->upRightAnnotation.c_str();  
}
void
vtkSynchronizedView::SetDownLeftAnnotation(const char *p_annotation)
{
  this->downLeftAnnotation = p_annotation;  
  this->UpdateAnnotations();  
}
const char *
vtkSynchronizedView::GetDownLeftAnnotation()
{
  return this->downLeftAnnotation.c_str();  
}
void
vtkSynchronizedView::SetDownRightAnnotation(const char *p_annotation)
{
  this->downRightAnnotation = p_annotation;  
  this->UpdateAnnotations();  
}
const char *
vtkSynchronizedView::GetDownRightAnnotation()
{
  return this->downRightAnnotation.c_str();  
}
void
vtkSynchronizedView::SetNorthAnnotation(const char *p_annotation)
{
  this->northAnnotation = p_annotation;  
  this->UpdateAnnotations();  
}
const char *
vtkSynchronizedView::GetNorthAnnotation()
{
  return this->northAnnotation.c_str();  
}
void
vtkSynchronizedView::SetSouthAnnotation(const char *p_annotation)
{
  this->southAnnotation = p_annotation;  
  this->UpdateAnnotations();  
}
const char *
vtkSynchronizedView::GetSouthAnnotation()
{
  return this->southAnnotation.c_str();  
}
void
vtkSynchronizedView::SetEastAnnotation(const char *p_annotation)
{
  this->eastAnnotation = p_annotation;  
  this->UpdateAnnotations();  
}
const char *
vtkSynchronizedView::GetEastAnnotation()
{
  return this->eastAnnotation.c_str();  
}
void
vtkSynchronizedView::SetWestAnnotation(const char *p_annotation)
{
  this->ouestAnnotation = p_annotation;
  this->UpdateAnnotations();  
}
const char *
vtkSynchronizedView::GetWestAnnotation()
{
  return this->ouestAnnotation.c_str();  
}



void
vtkSynchronizedView::UpdateAnnotations()
{
  if ( this->GetShowAnnotations() )
  {
    this->CornerAnnotation->SetText (1, this->GetDownRightAnnotation() );
    this->CornerAnnotation->SetText (2, this->GetUpLeftAnnotation() );
    this->CornerAnnotation->SetText (3, this->GetUpRightAnnotation() );
  }
  else
  {
    this->CornerAnnotation->SetText (1, "" );
    this->CornerAnnotation->SetText (2, "" );
    this->CornerAnnotation->SetText (3, "" );
  }
  //always show about data...
  this->CornerAnnotation->SetText (0, this->GetDownLeftAnnotation() );
  
  this->OrientationAnnotation->SetText (0, this->GetEastAnnotation());
  this->OrientationAnnotation->SetText (1, this->GetNorthAnnotation());
  this->OrientationAnnotation->SetText (2, this->GetWestAnnotation());
  this->OrientationAnnotation->SetText (3, this->GetSouthAnnotation());
}


void
vtkSynchronizedView::SetBackgroundColor(double r, double g, double b)
{
  if (this->Renderer)
  {
    this->Renderer->SetBackground(r,g,b);
  }
}


// void
// vtkSynchronizedView::Display (bool a)
// {
//   if( !this->GetIsProcessed() )
//   {
//     this->Show (a);
//     this->Lock();
//     for( unsigned int i=0; i<this->ViewToObserve.size(); i++)
//       this->ViewToObserve[i]->Display (a);
//     this->UnLock();
//   }
  
// }


// void
// vtkSynchronizedView::DisplayInFullPage(bool a)
// {
//   if( !this->GetIsProcessed() )
//   {
//     this->SetIsFullScreen ( a );
//     this->Show (true);

//     this->Lock();
//     for( unsigned int i=0; i<this->ViewToObserve.size(); i++)
//         this->ViewToObserve[i]->Display ( !a );
//     this->UnLock();
    
//   }


//   // force the call to the Layout() method: if the Parent
//   // is set, then I get its sizer and call Layout(). Implies
//   // that all linked vtkSynchronizedView have the same paremt, that may
//   // not be true all the time.
//   if( this->GetParent()!=NULL )
//   {
//     if( this->GetParent()->GetSizer() != NULL )
//       this->GetParent()->GetSizer()->Layout();
//   }
  
    
// }


bool vtkSynchronizedView::HasChild (vtkSynchronizedView* view) const
{
  if( !view )
  {
    return false;
  }
  
  
  for( unsigned int i=0; i<this->Children.size(); i++)
  {
    if ( this->Children[i]==view )
    {
      return true;
    }
  }

  return false;
  
}


bool vtkSynchronizedView::HasView (vtkSynchronizedView* view) const
{
  // @deprecated HasView is deprecated, use HasChild instead.
  return this->HasChild (view);  
}



void vtkSynchronizedView::AddViewToObserve (vtkSynchronizedView* view)
{

  // @deprecated AddViewToObserve is deprecated, use AddChild instead.
  this->AddChild (view);
  /*
  if( this->HasView(view) )
  {
    return;
  }
  
  if( view )
  {
    view->Register(this);
    this->ViewToObserve.push_back (view);
    }*/
  
}


void vtkSynchronizedView::RemoveViewToObserve(vtkSynchronizedView* view)
{

  // @deprecated RemoveViewToObserve is deprecated, use RemoveChild instead.
  this->RemoveChild (view);
  
  /*
  if( !view )
  {
    return;
  }

  vtkstd::vector< vtkSynchronizedView* > newViewsToObserve;
  
  for( unsigned int i=0; i<this->ViewToObserve.size(); i++)
  {
    if( this->ViewToObserve[i]==view )
    {
      this->ViewToObserve[i]->UnRegister (this);
    }
    else
    {
      newViewsToObserve.push_back(this->ViewToObserve[i]);
    }
  }
  
  this->ViewToObserve = newViewsToObserve;
  */
}



void vtkSynchronizedView::RemoveAllViewToObserve()
{

  // @deprecated RemoveAllViewToObserve is deprecated, use Detach instead.
  this->Detach();
  
  /*
  for( unsigned int i=0; i<this->ViewToObserve.size(); i++)
  {
    if( this->ViewToObserve[i] )
    {
      this->ViewToObserve[i]->UnRegister (this);
    }
  }
  
  this->ViewToObserve.clear();
  */
}


void
vtkSynchronizedView::ResetCamera()
{
  if (this->Renderer)
  {
    this->Renderer->ResetCamera();
  }
}


void vtkSynchronizedView::Render()
{
  if (this->RenderWindow && !this->RenderWindow->GetNeverRendered() )
  {
    this->RenderWindow->Render();
  }
}


void vtkSynchronizedView::SyncRender()
{
  
  if( this->IsLocked() )
  {
    return;
  }
  
  this->Render ();
  
  // this boolean is used so that the other observe won't call
  // Render() again and again and again...
  this->Lock();
  for( unsigned int i=0; i<this->Children.size(); i++)
  {
//     vtkSynchronizedView* view = reinterpret_cast<vtkSynchronizedView*> ( this->Children[i] );
    vtkSynchronizedView* view = this->Children[i];
    if( view && view->GetLinkRender())
    {
      view->SyncRender ();
    }
  }
  
  this->UnLock();
}



void vtkSynchronizedView::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Interaction-State = " << this->GetInteractionOn() << "\n";
  os << indent << "Render Window = " << this->RenderWindow << "\n";
  if (this->GetRenderWindow())
    this->GetRenderWindow()->PrintSelf (os, indent);
  
}

void vtkSynchronizedView::DrawOn()
{
  if (this->Renderer)
  {
//     this->Renderer->DrawOn();
  }
}

void vtkSynchronizedView::DrawOff()
{
  if (this->Renderer)
  {
//     this->Renderer->DrawOff();
  }
}


void vtkSynchronizedView::SetParent (vtkSynchronizedView* parent)
{
  this->Parent = parent;
}

vtkSynchronizedView* vtkSynchronizedView::GetParent() const
{
  return this->Parent;
}


void vtkSynchronizedView::AddChild (vtkSynchronizedView* child)
{

  if( this->HasChild(child) ) // nothing to do
  {
    return;
  }

  if ( child==this ) // someone wants to add itself as a child to itself
  {
    // remove the child from its parent as apparently it doesn't want it anymore
    vtkSynchronizedView* parent = child->GetParent();
    if( parent )
    {
      parent->RemoveChild (child);
    }
    // but do nothing more
    return;
  }

  
  if( child )
  {
    /**
       We temporary store the child's parent
     */
    vtkSynchronizedView* parent = child->GetParent();
    child->Register  (this);

    /**
       Now that the child has changed its parent, we remove
       the child from its previous parent's children list.
       If we have done that earlier, this could result in
       a call to Delete(), since the RemoveChild function
       unregsiter the object.
     */
    if( parent )
    {
      parent->RemoveChild (child);
    }

    
    child->SetParent (this);
    this->Children.push_back (child);
  }
  
}


void vtkSynchronizedView::AddChildren (std::vector<vtkSynchronizedView*> children)
{

  for( unsigned int i=0; i<children.size(); i++)
  {
    this->AddChild (children[i]);
  }
  
}


void vtkSynchronizedView::RemoveChild (vtkSynchronizedView* view)
{
  
  if( !view || this->Children.size()==0 )
  {
    return;
  }

  std::vector<vtkSynchronizedView*>::iterator it =
    std::find (this->Children.begin(), this->Children.end(), view);

  if( it==this->Children.end() ) // view is not in the list
  {
    return;
  }
  
  (*it)->SetParent (0);
  (*it)->UnRegister (this);
  this->Children.erase (it);  
}


void vtkSynchronizedView::RemoveAllChildren()
{

  for( unsigned int i=0; i<this->Children.size(); i++)
  {
	this->Children[i]->SetParent( 0 );
    this->Children[i]->UnRegister (this);
  }

  this->Children.clear();
  
}


void vtkSynchronizedView::Detach (void)
{
  
  vtkSynchronizedView* parent = this->GetParent();
  if( parent )
  {
    parent->AddChildren ( this->Children );
    parent->RemoveChild ( this );
    
    /**
       Handle the case where the parent's parent of the view is the view itself.
       Tell the it that it no longer has a parent, life is sad...
    */
    ;
    if( parent->GetParent()==this )
    {
      parent->SetParent(0);
      parent->UnRegister(this);
    }
    
  }
  else
  {
    this->RemoveAllChildren(); 
  }
  
}


void vtkSynchronizedView::Lock()
{
  this->SetIsProcessed (true);
}


void vtkSynchronizedView::UnLock()
{
  this->SetIsProcessed (false);
}
