/*=========================================================================

Program:   vtkINRIA3D
Module:    $Id: vtkSynchronizedView.h 1137 2009-04-03 15:31:45Z filus $
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
#ifndef _vtkSynchronizedView_h_
#define _vtkSynchronizedView_h_

#include "vtkINRIA3DConfigure.h"

#include <vector>
#include <string>

#include "vtkObject.h"
#include "vtkTextProperty.h"
#include <vtkCornerAnnotation.h>




class vtkProp;
class vtkRenderer;
class vtkRenderWindow;
class vtkRenderWindowInteractor;

//class vtkInteractorStyle;
class vtkOrientationAnnotation;

class vtkImageData;
class vtkLookupTable;



/**
   \class vtkSynchronizedView vtkSynchronizedView.h "vtkSynchronizedView.h"
   \brief Base class for vtkViewImage
   \author Pierre Fillard & Marc Traina & Nicolas Toussaint
   
   This class is a vtkObject that includes a renderer, a renderwindow and
   a renderwindowinteractor for visualization purposes. You can can manually add actors
   or use vtkViewImage class for more convenience.

   It uses a tree-like structure to propagate functions call in order to synchronize
   several views together, like changing the window/level, the current position, etc.
   Each view has one parent and several children. When a children is added, its parent
   is automatically set. Once a view is removed from the tree using function Detach(),
   its children are automatically appended to its parent's children, so that the function
   propagation mechanism is not broken.

   The tree structure should cycled, i.e. the last added children should point to
   the first view. In that case, and in order to prevent infinite loops when propagating
   functions, the functions Lock() and UnLock() must be used. When Lock() is called, a view
   does not transmit anything. UnLock() permits to go back to normal function propagation.
   
*/


class VTK_RENDERINGADDON_EXPORT vtkSynchronizedView : public vtkObject
{
  
 public:
  
  static vtkSynchronizedView* New();
  vtkTypeRevisionMacro(vtkSynchronizedView, vtkObject);
  

  /** Set the RenderWindow */
  virtual void SetRenderWindow(vtkRenderWindow *arg);
  

  /** Set the Renderer **/
  virtual void SetRenderer(vtkRenderer *arg);


  /** Attach an interactor to the internal RenderWindow. **/
  virtual void SetInteractor(vtkRenderWindowInteractor*);

  
  /** Get the vtkRenderWindow associated */
  vtkGetObjectMacro (RenderWindow, vtkRenderWindow);
  

  /** Get the vtkRenderWindow associated */
  vtkGetObjectMacro (Renderer, vtkRenderer);
  

  /** Get the vtkRenderWindow associated */
  vtkGetObjectMacro (RenderWindowInteractor, vtkRenderWindowInteractor);

  
  /** Add the actor to the first renderer of the render window if exist. 
      Do nothing otherwise.*/
  virtual void AddActor(vtkProp* actor);

  
  /** remove the actor from the first renderer of the render window if exist. 
      Do nothing otherwise.*/
  virtual void RemoveActor(vtkProp* actor);

  
  /** Enable or Disable interaction on the view. The Interaction mode is store
      internaly and set at each time the widget is showed. The interaction
      mode cannot be set before the vtkRenderWindowInteractor is initialized. */
  void SetInteractionOff( void );
  void SetInteractionOn( void );
  
  vtkGetMacro (InteractionOn, bool);
  

  /** Specify the interactor style */
  //void SetInteractorStyle(vtkInteractorStyle*);

  
  /** Get the interactor style */
  //vtkGetObjectMacro (InteractorStyle, vtkInteractorStyle);

  
  /** Set the background color. Format is RGB, 0 <= R,G,B <=1
      Example: SetBackgroundColor(0.9,0.9,0.9) for grey-white. */
  virtual void SetBackgroundColor(double r, double g, double b);

  
  /** Set/Get annotations methods*/
  void         SetUpLeftAnnotation (const char *p_annotation);  
  const char * GetUpLeftAnnotation ();  
  void         SetUpRightAnnotation (const char *p_annotation);  
  const char * GetUpRightAnnotation ();
  void         SetDownLeftAnnotation (const char *p_annotation);  
  const char * GetDownLeftAnnotation ();  
  void         SetDownRightAnnotation (const char *p_annotation);  
  const char * GetDownRightAnnotation ();  
  void         SetNorthAnnotation (const char *p_annotation);  
  const char * GetNorthAnnotation ();  
  void         SetSouthAnnotation (const char *p_annotation);  
  const char * GetSouthAnnotation ();  
  void         SetEastAnnotation  (const char *p_annotation);
  const char * GetEastAnnotation  ();  
  void         SetWestAnnotation (const char *p_annotation);  
  const char * GetWestAnnotation ();


  /** Get/Set lower left annotation */ 
  virtual const char* GetAboutData(void) const
  { return this->AboutData_str.c_str(); }

  
  /** Get/Set lower left annotation */
  virtual void SetAboutData (const char* str)
  {
    AboutData_str = str;
    this->SetDownLeftAnnotation( AboutData_str.c_str() );
  }


  /** Show/Hide the annotations. Call UpdateAnnotations after this function. */
  vtkSetMacro (ShowAnnotations, bool);
  vtkGetMacro (ShowAnnotations, bool);
    

  /** Get the corner annotation. */
  vtkGetObjectMacro (CornerAnnotation, vtkCornerAnnotation);

  
  /** Get the corner annotation. */
  vtkGetObjectMacro (OrientationAnnotation, vtkOrientationAnnotation);

  
  /** Update the annotations. */
  virtual void UpdateAnnotations( void );

  
  virtual void PrintSelf(ostream& os, vtkIndent indent);
  

  /** Call the RenderWindow's Render() method. */
  virtual void Render (void);
  virtual void SyncRender (void);
  
  

  /** Reset the camera */
  virtual void ResetCamera();


  // @deprecated AddViewToObserve is deprecated, use AddChild instead.
  VTK_LEGACY (void AddViewToObserve (vtkSynchronizedView* p_view));


  vtkSynchronizedView* GetParent (void) const;

  /**
     Add a child to the list of children. Check if the child is already
     in the list firt.
   */
  void AddChild (vtkSynchronizedView* p_view);

  //BTX
  void AddChildren (std::vector<vtkSynchronizedView*> p_viewlist);
  //ETX
  
  /**
     Remove a child form the list of children.
  */
  void RemoveChild (vtkSynchronizedView* view);

  void RemoveAllChildren (void);

  /**
     Detach the view, i.e. add its own children (if any) to its parent's children (if any).
  */
  void Detach (void);
  
  
  // @deprecated RemoveViewToObserve is deprecated, use RemoveChild instead.
  VTK_LEGACY (void RemoveViewToObserve (vtkSynchronizedView* p_view));

  
  /** Clear all the views to observe */
  // @deprecated RemoveAllViewToObserve is deprecated, use Detach instead.
  VTK_LEGACY (void RemoveAllViewToObserve());


  /**
     This function is called right after setting both Renderer and RenderWindow.
     It allows a class to add actors for instance without knowing when the Renderer
     and RenderWindow are set. For example, vtkSynchronizedView will add the corner annotations
     during the call to the Initialize function.
  */
  virtual void Initialize(void);
    
  //BTX
  std::vector < vtkSynchronizedView* > GetChildren(void) const
  { return this->Children;}
  VTK_LEGACY(std::vector < vtkSynchronizedView* > GetViewToObserve(void) const);
  //ETX
  
  void SetTextColor (double color[3])
  {
    this->TextProperty->SetColor (color[0], color[1], color[2]);
    this->CornerAnnotation->Modified();
  }
  
  virtual void DrawOn();
  virtual void DrawOff();


  vtkSetMacro (IsProcessed, bool);
  vtkGetMacro (IsProcessed, bool);

  bool IsLocked (void)
  { return this->GetIsProcessed(); }

  /**
     Part of the function propagation mechanism, when the function Lock() is
     called, the view does not transmit the function to its children (and does
     not do anything in fact).
   */
  void Lock (void);

  /**
     A call to UnLock() permits to transmit function calls to the view's children.
   */
  void UnLock (void);


  // @deprecated HasView is deprecated, use HasChild instead.
  VTK_LEGACY (bool HasView (vtkSynchronizedView*) const);
  
  /**
     Returns true if the view has this child in its list.
  */
  bool HasChild (vtkSynchronizedView*) const;

  /** Set the render link ON or OFF */
  vtkSetMacro (LinkRender, bool);
  vtkGetMacro (LinkRender, bool);
  
 protected:
  vtkSynchronizedView();
  ~vtkSynchronizedView();

  //BTX
  vtkSynchronizedView*                 Parent;
  std::vector < vtkSynchronizedView* > Children;
  //ETX
  
  virtual void Uninitialize();

  virtual void SetInteraction();
  
  /**
     Set the parent for this view. Internal use only.
   */
  void SetParent (vtkSynchronizedView* view);
    
 private:

  /// VTK Renderer:
  vtkRenderWindow*                RenderWindow;
  vtkRenderer*                    Renderer;
  vtkRenderWindowInteractor*      RenderWindowInteractor;
  
  //vtkInteractorStyle*             InteractorStyle;
  vtkCornerAnnotation*            CornerAnnotation;
  vtkOrientationAnnotation*       OrientationAnnotation;

  vtkTextProperty*                TextProperty;
  
  //BTX
  std::string             upLeftAnnotation;
  std::string             upRightAnnotation;
  std::string             downLeftAnnotation;
  std::string             downRightAnnotation;
  std::string             northAnnotation;
  std::string             southAnnotation;
  std::string             eastAnnotation;
  std::string             ouestAnnotation;
  std::string             AboutData_str;
  //ETX
  bool                    ShowAnnotations;
  
  bool                    IsProcessed;
  bool                    InteractionOn;
  bool                    LinkRender;
  
};

#endif /* _vtkSynchronizedView_h_ */
