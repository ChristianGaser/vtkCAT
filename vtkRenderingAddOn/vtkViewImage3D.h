/*=========================================================================

Program:   vtkINRIA3D
Module:    $Id: vtkViewImage3D.h 778 2008-04-09 08:00:29Z ntoussaint $
Language:  C++
Author:    $Author: ntoussaint $
Date:      $Date: 2008-04-09 10:00:29 +0200 (Mi, 09 Apr 2008) $
Version:   $Revision: 778 $

Copyright (c) 2007 INRIA - Asclepios Project. All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _vtkViewImage3D_h_
#define _vtkViewImage3D_h_

#include "vtkINRIA3DConfigure.h"

#include <vector>
#include <vtkViewImage.h>
#include <vtkRenderer.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkImageMapToColors.h>
#include <vtkBoxWidget.h>
#include <vtkPlaneWidget.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkVolumeRayCastMapper.h>
#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkVolumeRayCastMIPFunction.h>
#include <vtkVolumeRayCastIsosurfaceFunction.h>
#include <vtkVolumeProperty.h>
#include "vtkImage3DCroppingBoxCallback.h"
#include <vtkObjectFactory.h>

class vtkImageData;
class vtkVolumeTextureMapper3D;
class vtkVolumeMapper;
class vtkVolumeRayCastFunction;
class vtkPiecewiseFunction;
class vtkColorTransferFunction;
class vtkVolume;
class vtkImageActor;
class vtkAxes;
class vtkDataSet3DCroppingPlaneCallback;
class vtkTubeFilter;
class vtkAnnotatedCubeActor;
class vtkOrientationMarkerWidget;
class vtkAxesActor;
class vtkImageBlend;
class vtkViewImage2D;
class vtkScalarsToColors;
class vtkColorTransferFunction;

class vtkDataSet;
class vtkProperty;


/**
   \class vtkViewImage3D vtkViewImage3D.h "vtkViewImage3D.h"
   \brief 3D image 3D rendering viewer
   \author Pierre Fillard & Marc Traina & Nicolas Toussaint
   
   This class allows to view 3D images. Images have to be
   vtkImageData. It also handles an overlapping image.
   volume rendering and mulptiplane reconstructions are provided
   remote plan can also be used, so can be an orientation cube, ...
*/



class VTK_RENDERINGADDON_EXPORT vtkViewImage3D : public vtkViewImage
{
  
public:

  static vtkViewImage3D* New();
  vtkTypeRevisionMacro(vtkViewImage3D, vtkViewImage);

  
  // 2 rendering modes are possible
  //BTX
  enum RenderingModeIds
  {
    VOLUME_RENDERING,
    PLANAR_RENDERING
  };
  //ETX
  

  virtual void SetInteractor (vtkRenderWindowInteractor*);
  
  /** In some cases, we would like to call some cleaning
      functions before going into the unregister vtk stuff.*/
  virtual void PrepareForDelete (void);


  /** Update the display parameters based on the data information. */
  virtual void Update(){};


  virtual void PrintSelf(ostream& os, vtkIndent indent);


  /** Set the current image */
  virtual void SetImage(vtkImageData* image);

  
  /** Set/Get window/level for mapping pixels to colors. */
  virtual double GetColorWindow(void)
  { return this->GetWindow(); }

  
  /** Set/Get window/level for mapping pixels to colors. */
  virtual double GetColorLevel(void)
  { return this->GetLevel(); }

  
  /** Set/Get window/level for mapping pixels to colors. */
  virtual void SetWindow (double w);

  
  /** Set/Get window/level for mapping pixels to colors. */
  virtual void SetLevel (double l);


  /** Set the rendering mode to volume rendering (VR). */
  virtual void SetRenderingModeToVR (void)
  {
    this->SetRenderingMode (VOLUME_RENDERING);
  };

  
  /** Set the rendering mode to planar views. */
  void SetRenderingModeToPlanar (void)
  {
    this->SetRenderingMode (PLANAR_RENDERING);
  };


  /** Set the rendering mode. */
  void SetRenderingMode (int mode);

  
  /** Get the current rendering mode. */
  vtkGetMacro (RenderingMode, int);


  /** Center the view and reset the camera clipping range. */
  virtual void Center (void)
  {
    if( this->GetRenderer() )
    {
      this->GetRenderer()->ResetCameraClippingRange();
      this->GetRenderer()->ResetCamera();
    }
  }

  virtual void SetCameraPosition(double position[3]);
  virtual void SetCameraFocalPoint(double position[3]);
  virtual void SetCameraViewUp(double position[3]);
  

  
  /** Switch on/off the visibility of the image */
  void SetVisibility (bool);

  
  /** Get the visibility */
  vtkGetMacro (Visibility, bool);

  
  /** Set the box widget on */
  void BoxWidgetOn (void)
  {
    this->BoxWidgetVisibility = true;
    if (this->Visibility && this->RenderingMode == VOLUME_RENDERING)
    {
      this->BoxWidget->On();
    }
  }


  /** Set the box widget on */
  void BoxWidgetOff (void)
  {
    this->BoxWidgetVisibility = false;
    this->BoxWidget->Off();
  }

  
  /** Set the box widget visibility */
  void SetBoxWidget (bool a)
  {
    if(a)
    {
      this->BoxWidgetOn();
    }
    else
    {
      this->BoxWidgetOff();
    }
  }

  
  /** Get the box widget visibility */
  vtkGetMacro (BoxWidgetVisibility, bool);

  
  /** Set the plane widget on */
  void PlaneWidgetOn (void)
  {
    this->PlaneWidget->On();
  }

  
  /** Set the plane widget on */
  void PlaneWidgetOff (void)
  {
    this->PlaneWidget->Off();
  }

  
  /** Set the plane widget on */
  void SetPlaneWidget (bool a)
  {
    if(a)
    {
      this->PlaneWidget->On();
    }
    else
    {
      this->PlaneWidget->Off();
    }
  }

  /** Get the dataset callback */
  vtkGetObjectMacro (DataSetCallback, vtkDataSet3DCroppingPlaneCallback);

  /*
  void SetViewForPlaneWidget (vtkViewImage2D* view)
  {
    this->PlaneCallback->SetViewImage2D (view);
    }*/
  
  
  /** Set the visibility of the 3D cursor */
  void Set3DCursorVisibilityOn (void)
  {
    this->AxesVisibility = true;
    this->AxesActor->SetVisibility (1);
  }

  
  /** Set the visibility of the 3D cursor */
  void Set3DCursorVisibilityOff (void)
  {
    this->AxesVisibility = false;
    this->AxesActor->SetVisibility (0);
  }


  /** Set the visibility of the 3D cursor */
  void Set3DCursorVisibility (bool a)
  {
    this->AxesVisibility = a;
    this->AxesActor->SetVisibility (a);
  }


  void SetCubeVisibilityOn (void)
  {
    this->Marker->SetEnabled (1);
    this->Marker->InteractiveOff();
  }
  
  
  void SetCubeVisibilityOff (void)
  {
    this->Marker->SetEnabled (0);
  }

  
  void SetCubeVisibility (bool a)
  {
    this->Marker->SetEnabled (a);
    if( a )
    {
      this->Marker->InteractiveOff();
    }
  }


  void SetVolumeMapperToTexture (void)
  {
    this->VolumeActor->SetMapper ( this->VolumeMapper3D );
    this->Callback->SetVolumeMapper ( this->VolumeMapper3D );
  }
  
  
  void SetVolumeMapperToRayCast (void)
  {
    this->VolumeActor->SetMapper ( this->VolumeRayCastMapper );
    this->Callback->SetVolumeMapper ( this->VolumeRayCastMapper );
  }
  

  void SetVolumeRayCastFunctionToComposite (void)
  {
    this->VolumeRayCastMapper->SetVolumeRayCastFunction ( this->VolumeRayCastCompositeFunction );
  }

  
  void SetVolumeRayCastFunctionToMIP (void)
  {
    this->VolumeRayCastMapper->SetVolumeRayCastFunction ( this->VolumeRayCastMIPFunction );
  }
  

  void SetVolumeRayCastFunctionToIsosurface (void)
  {
    this->VolumeRayCastMapper->SetVolumeRayCastFunction ( this->VolumeRayCastIsosurfaceFunction );
  }


  void ShadeOn (void)
  {
    this->VolumeProperty->ShadeOn();
  }

  
  void ShadeOff (void)
  {
    this->VolumeProperty->ShadeOff();
  }

  bool GetShade (void)
  {
    return this->VolumeProperty->GetShade();
  }
  
  

  /** Set an overlapping image */
  virtual void SetOverlappingImage (vtkImageData*);


  /** Get the overlapping image */
  vtkGetObjectMacro (OverlappingImage, vtkImageData);
  

  /** Remove the overlapping image */
  virtual void RemoveOverlappingImage (void);

  
  /** Set a user-defined lookup table */
  virtual void SetLookupTable (vtkScalarsToColors* lut);

  vtkScalarsToColors* GetColorFunction(void) const;
  
  /** Get volume actor */
  vtkGetObjectMacro (VolumeActor, vtkVolume);

  
  /** Get each plane's actor */
  vtkGetObjectMacro (ActorAxial, vtkImageActor);

  
  /** Get each plane's actor */
  vtkGetObjectMacro (ActorSagittal, vtkImageActor);


  /** Get each plane's actor */
  vtkGetObjectMacro (ActorCoronal, vtkImageActor);


  vtkGetObjectMacro (PlaneWidget, vtkPlaneWidget);

  
  vtkGetObjectMacro (VolumeProperty, vtkVolumeProperty);

  vtkGetObjectMacro (BoxWidget, vtkBoxWidget);
  


  virtual void UpdatePosition ();

  /** Get/Set Each plane visibility flag */
  vtkGetMacro (ShowAxial, int);
  void SetShowAxial (int val);
  vtkBooleanMacro (ShowAxial, int);
  
  /** Get/Set Each plane visibility flag */
  vtkGetMacro (ShowCoronal, int);
  void SetShowCoronal (int val);
  vtkBooleanMacro (ShowCoronal, int);
  
  /** Get/Set Each plane visibility flag */
  vtkGetMacro (ShowSagittal, int);
  void SetShowSagittal (int val);
  vtkBooleanMacro (ShowSagittal, int);

  /**
    Add a dataset to the view (polydata or grid)
    the dataset will be display as its intersection with the displayed image slice
    i.e. tetrahedron will be displayed as triangles, triangles as lines, lines as points
    you can associate a property to be plugged into the dataset actor
  */
  virtual vtkActor* AddDataSet (vtkDataSet* dataset, vtkProperty* property = NULL);

  /**
     This method colorizes a given dataset (previously added with AddDataSet()) by one of its arrays
     It doesn't handle a specific component colorization yet. If the array contains multi component scalars,
     It takes the norm of the vector.
  */
  virtual void ColorDataSetByArray(vtkDataSet* dataset, const char* arrayname, vtkColorTransferFunction* transfer);

  virtual void UpdateDataSetPipeline (vtkDataSet* dataset);

  
  
  
protected:

  vtkViewImage3D();
  ~vtkViewImage3D();
  
  
private:

  /// overlap image
  vtkImageData*                   OverlappingImage;
  
  // texture mapper in 3D
  vtkVolumeMapper*                VolumeMapper3D;

  // volume ray cast mapper
  vtkVolumeRayCastMapper*         VolumeRayCastMapper;

  // ray cast function
  vtkVolumeRayCastMIPFunction*        VolumeRayCastMIPFunction;

  // ray cast function
  vtkVolumeRayCastCompositeFunction*  VolumeRayCastCompositeFunction;

  // ray cast function
  vtkVolumeRayCastIsosurfaceFunction* VolumeRayCastIsosurfaceFunction;
  
  // volume property
  vtkVolumeProperty*              VolumeProperty;

  // volume actor
  vtkVolume*                      VolumeActor;

  // opacity transfer function
  vtkPiecewiseFunction*           OpacityFunction;

  // color transfer function
  vtkColorTransferFunction*       ColorFunction;

  // axes
  vtkAxes*                        Axes;
  bool                            AxesVisibility;
  
  
  // turn axes into tubes
  vtkTubeFilter*                  TubeFilter;
  
  // axes actor
  vtkActor*                       AxesActor;

  // box widget
  vtkBoxWidget*                   BoxWidget;
  bool                            BoxWidgetVisibility;

  // vtkImagePlane widget
  vtkPlaneWidget*                 PlaneWidget;
  
  // image 3D cropping box callback
  vtkImage3DCroppingBoxCallback*  Callback;
  vtkDataSet3DCroppingPlaneCallback* DataSetCallback;
  
  //vtkImage3DImagePlaneCallback*   PlaneCallback;

  // 4 image blenders
  vtkImageBlend*                  AxialBlender;
  vtkImageBlend*                  SagittalBlender;
  vtkImageBlend*                  CoronalBlender;
  vtkImageBlend*                  Blender;
  
  // Image color mapper
  vtkImageMapToColors* AxialColorMapper;
  vtkImageMapToColors* SagittalColorMapper;
  vtkImageMapToColors* CoronalColorMapper;
  
  // actors
  vtkImageActor*                  ActorSagittal;
  vtkImageActor*                  ActorCoronal;
  vtkImageActor*                  ActorAxial;

  
  // rendering mode
  RenderingModeIds                RenderingMode;
  bool                            Visibility;

  
  // annotated cube actor
  vtkAnnotatedCubeActor*          Cube;
  vtkAxesActor*                   CubeAxes;
  vtkOrientationMarkerWidget*     Marker;


  // Flags for showing the planes
  int                            ShowAxial;
  int                            ShowCoronal;
  int                            ShowSagittal;

  int                            FirstImage;

/*   //BTX */
/*   std::vector<vtkDataSet*>      DataSetList; */
/*   std::vector<vtkActor*>        DataSetActorList; */
/*   //ETX */

  
};
  

#endif
