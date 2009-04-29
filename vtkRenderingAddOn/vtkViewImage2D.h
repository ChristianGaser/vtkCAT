/*=========================================================================

Program:   vtkINRIA3D
Module:    $Id: vtkViewImage2D.h 1137 2009-04-03 15:31:45Z filus $
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
#ifndef _vtkViewImage2D_h_
#define _vtkViewImage2D_h_

#include "vtkINRIA3DConfigure.h"

#include <iostream>
#include <vtkViewImage.h>

class vtkImageActor;
class vtkProp;
class vtkCutter;
class vtkActor;
class vtkLineSource;
class vtkTextMapper;
class vtkImageReslice;
class vtkImageMapToWindowLevelColors;
class vtkImageMapToColors;
class vtkTransform;
class vtkAlgorithmOutput;
class vtkImageBlendWithMask;
class vtkImageBlend;
class vtkLookupTable;
class vtkScalarsToColors;
class vtkColorTransferFunction;
class vtkInteractorStyleImage2D;
class vtkDataSet;
class vtkPolyDataMapper;
class vtkProperty;
class vtkCutter;
class vtkPlane;
class vtkBox;
class vtkBoxCutPolyLines;
class vtkDataSetMapper;
class vtkColorTransferFunction;

/**
   \class vtkViewImage2D vtkViewImage2D.h "vtkViewImage2D.h"
   \brief 2D synchronized viewer of a 3D image.
   \author Pierre Fillard & Marc Traina & Nicolas Toussaint.
   
   This class allows to view 2D orthogonal slices (namely axial, sagittal and coronal) of a 3D image.
   This class implements lots of features like: various possible interactions (positioning, zooming, 
   windowing), coloring  of the slice w.r.t. a lookup table, overlapping of a second image, 
   overlapping of a mask image (image of labels). Display conventions can be chosen between radiologic
   (the left of the patient appears on the right of the screen) and neurologic (left is left and right 
   is right).
   
   It uses a synchronization mechanisms between other views. Views are linked together using a cycle
   graph structure (implemented in the base class \class vtkSynchronizedView). All interactions are
   passed to the other Views, which in turn pass them to their Children, and so on. Thus, zooming,
   positioning, etc. are synchronized among different views (views can display other orientation of
   the image).
   
   See Examples/SynchronizedViews/SynchronizedViews for a working example of how to use this class.

*/


class VTK_RENDERINGADDON_EXPORT vtkViewImage2D : public vtkViewImage
{
 public:
  
  static vtkViewImage2D* New();
  vtkTypeRevisionMacro(vtkViewImage2D, vtkViewImage);

  
  //BTX
  enum InteractionStyleIds {
    NO_INTERACTION,
    SELECT_INTERACTION,
    WINDOW_LEVEL_INTERACTION,
    FULL_PAGE_INTERACTION,
    MEASURE_INTERACTION,
    ZOOM_INTERACTION
  };
  //ETX

  //BTX
  enum ConventionIds{
    RADIOLOGIC,
    NEUROLOGIC
  };
  //ETX


  /**
     Update the internal pipeline (slice extraction, coloring, etc.) and reset the position (place the cursor on the center pixel), 
	 zooming (zoom factor of 1.0), and windowing. It also re-computes the clipping range and other camera related parameters. 
	 It is mainly for internal used should in general not be used. See Reset() if you only want to reset position, zoom and
	 window/level.
  */
  virtual void Update();


  virtual void Uninitialize();

  /**
     This function is called right after setting both Renderer and RenderWindow.
     It allows a class to add actors for instance without knowing when the Renderer
     and RenderWindow are set. For example, vtkSynchronizedView will add the corner annotations
     during the call to the Initialize function.
  */
  virtual void Initialize();
  
  void PrintSelf(ostream& os, vtkIndent indent);

  /**
     Show or hide the direction labels (L/R/A/P/I/S).
   */
  void SetShowDirections(bool p_showDirections);
  vtkGetMacro (ShowDirections, bool);

  /**
     Show or hide the indication label about the current selected position.
   */
  void SetShowCurrentPoint(bool p_showCurrentPoint);
  vtkGetMacro (ShowCurrentPoint, bool);

  /**
     Show or hide the indication label about the displayed slice.
   */
  void SetShowSliceNumber(bool p_showSliceNumber);
  vtkGetMacro (ShowSliceNumber, bool);

  /**
     Show or hide the 2D axes.
   */
  void Show2DAxis(const bool&);
  
  /** Set the image to display. */
  virtual void SetImage(vtkImageData* image);

  
  /** Return the vtkImageActor of the view. */
  vtkGetObjectMacro (ImageActor, vtkImageActor);

  
  /** Set a transform to apply to visualize the image. */
  void SetTransform(vtkTransform* p_transform);  


  /** Set the orientation of the image.
      @param orientation 0 is XY (axial), 1 is YZ (sagittal), 2 is XZ (coronal). */
  void SetOrientation(unsigned int orientation);


  /** Get the orientation of the view. */
  vtkGetMacro (Orientation, unsigned int);
  
  
  /** Get the id of the first slice available relative to the current orientation. */
  int GetWholeZMin();
  /** Get the id of the last slice available relative to the current orientation. */
  int GetWholeZMax();
 
   /** Get the current slice. Z is relative to the view orientation. */
  int GetZSlice();  

  /**  Set the current slice. Z is relative to the view orientation. */
  void SetZSlice(int p_slice);
  
  /** Set/Get window/level for mapping pixels to colors. */
  virtual double GetColorWindow();
  
  /** Set/Get window/level for mapping pixels to colors. */
  virtual double GetColorLevel();

  /** Set/Get window/level for mapping pixels to colors. */
  virtual void SetWindow (double);

  /** Set/Get window/level for mapping pixels to colors. */
  virtual void SetLevel (double);
  
  /** Return a pointer to the instance of vtkImageMapToColors of the View. */
  vtkGetObjectMacro (WindowLevel, vtkImageMapToColors);

  /** Return a pointer to the instance of vtkImageReslice of the View. */
  vtkGetObjectMacro (ImageReslice, vtkImageReslice);
  
  /** Set/Get the interpolation method (1: linear, 0: nearest neighbor). */
  void SetInterpolationMode(int i);
  
  /** Set/Get the interpolation method (1: linear, 0: nearest neighbor). */
  int GetInterpolationMode(void);

  /** Set the type of user interaction type for the left button of the mouse. */
  vtkSetMacro (LeftButtonInteractionStyle, unsigned int);
  vtkGetMacro (LeftButtonInteractionStyle, unsigned int);

  /** Set the type of user interaction type for the right button of the mouse. */
  vtkSetMacro (RightButtonInteractionStyle, unsigned int);
  vtkGetMacro (RightButtonInteractionStyle, unsigned int);

  /** Set the type of user interaction type for the middle button of the mouse. */
  vtkSetMacro (MiddleButtonInteractionStyle, unsigned int);
  vtkGetMacro (MiddleButtonInteractionStyle, unsigned int);

  /** Set the type of user interaction type for the wheel of the mouse. */
  vtkSetMacro (WheelInteractionStyle, unsigned int);
  vtkGetMacro (WheelInteractionStyle, unsigned int);

  /** Set the type of user interaction to all mouse buttons at once. */
  void SetInteractionStyle (const unsigned int& style)
  {
    if (this->GetLeftButtonInteractionStyle() == this->InteractionStyle)
      this->SetLeftButtonInteractionStyle   (style);
    if (this->GetMiddleButtonInteractionStyle() == this->InteractionStyle)
      this->SetMiddleButtonInteractionStyle (style);
    if (this->GetRightButtonInteractionStyle() == this->InteractionStyle)
      this->SetRightButtonInteractionStyle  (style);
    if (this->GetWheelInteractionStyle() == this->InteractionStyle)
      this->SetWheelInteractionStyle        (style);
    this->InteractionStyle = style;
  }
  
  /** Get the type of interaction (when set globally to all buttons & wheel). */
  vtkGetMacro (InteractionStyle, unsigned int);

    
  /**
     Set an overlapping mask image with its corresponding LookupTable. A label image MUST
	 be of type unsigned char. Labels are mapped through colors using the vtkLookupTable.
	 For instance, label 1 (i.e. pixels whose value is 1) will be mapped throught index 1
	 (table value 1) of the vtkLookupTable.
  */
  void SetMaskImage (vtkImageData* mask, vtkLookupTable* lut);

  /** Remove the mask image (if any). */
  void RemoveMaskImage (void);

  /** Set an overlapping second image, which will appear by transparency on top of the 
      initial image. */
  void SetOverlappingImage (vtkImageData* image);

  /**  Get the overlapping image (if any). */
  vtkGetObjectMacro (OverlappingImage, vtkImageData);

  /** Remove the overlapping image (if any). */
  void RemoveOverlappingImage (void);
  
  /** Set/Get a user-defined lookup table to color the image with. */
  virtual void SetLookupTable (vtkScalarsToColors* lut);

  /* Updates the position of the image. */
  virtual void UpdatePosition ();

  /** Show/hide the image. */
  virtual void SetVisibility (int state);
  virtual int  GetVisibility (void);

  /** Set the camera focal and position link ON or OFF. */
  vtkSetMacro (LinkCameraFocalAndPosition, bool);
  
  /** Get the camera focal and position link value. */
  vtkGetMacro (LinkCameraFocalAndPosition, bool);

  /**
    Add a dataset to the view.
    The dataset will be display as an intersection of it with the displayed image slice,
    i.e., tetrahedron will be displayed as triangles, triangles as lines, lines as points.
    A vtkProperty can be specified to specify the freshly generated actor properties.
  */
  virtual vtkActor* AddDataSet (vtkDataSet* dataset, vtkProperty* property = NULL);

  /**
    This method is similar to AddDataSet(), except it only works with vtkPolyData.
    It cuts the polydata using 2 planes defined by the current image slice being
    visualized and a user-provided thickness (generally the spacing between 2 consecutive
	slices). For instance, it works nicely with lines: AddDataSet() turns lines into points
    while AddPolyData() displays lines as short segments.
  */
  virtual vtkActor* SyncAddPolyData (vtkPolyData* polydata,  vtkProperty* property = NULL, double thickness = 0.2);
  virtual vtkActor* AddPolyData (vtkPolyData* polydata,  vtkProperty* property = NULL, double thickness = 0.2);

  /** Set the actual zoom factor of the view. */
  virtual void SetZoom (double factor);

  /** Reset the zoom factor of the view to its initial value and center the camera. */
  void ResetZoom (void);

  /**
     Switch between radiological (left is right and right is left) and
     neurological (left is left and right is right) conventions.
  */
  virtual void SetConventionsToRadiological (void);

  /**
     Switch between radiological (left is right and right is left) and
     neurological (left is left and right is right) conventions.
  */
  virtual void SetConventionsToNeurological (void);

  /**
     Switch between radiological (left is right and right is left) and
     neurological (left is left and right is right) conventions.
  */
  vtkGetMacro (Conventions, unsigned int);


  /** 
     Global functions to Set/Get the display conventions. Useful to change the display conventions of all view
	 by calling only once vtkViewImage2D::SetViewImage2DDisplayConventions(0/1). 
  */
  static void SetViewImage2DDisplayConventions(int val);
  static void SetViewImage2DDisplayConventionsToRadiologic() {vtkViewImage2D::SetViewImage2DDisplayConventions(0);};
  static void SetViewImage2DDisplayConventionsToNeurologic() {vtkViewImage2D::SetViewImage2DDisplayConventions(1);};
  static int  GetViewImage2DDisplayConventions();


  /** Manually set the camera focal and position. Used to set the translation. */
  void SetCameraFocalAndPosition (double focal[3], double pos[3]);

  /** Get the camera focal and position. */
  void GetCameraFocalAndPosition (double focal[3], double pos[3]);

  /** Synchronized version of SetCameraFocalAndPosition. */
  void SyncSetCameraFocalAndPosition (double focal[3], double pos[3]);

  /** Add an imagebackground image. You can apply to the image actor a transformation transform, and choose a slice if it is a volume. */
  void SetBG(vtkImageData* image, int slice = 0, vtkTransform* transform = NULL);

  /** Remove the background image (if any). */
  void RemoveBGImage (void);

  /** Set the opacity of the background Actor */
  void SetBGOpacity(double opacity);

  /** Set the opacity of the principale ImageActor */
   void SetOpacity(double opacity);

  /** Return the vtkImageActor of the background image. */
  vtkGetObjectMacro (BGActor, vtkImageActor);

  /** Return the vtkImageData of the background image. */
  vtkGetObjectMacro (BGImage, vtkImageData);

/** A complete reset: a classical reset and also cancel all the transformations*/
  void Clear(void);

  vtkGetMacro (FirstImage, int);
  
 protected:
  vtkViewImage2D();
  ~vtkViewImage2D();

  
  /**
     Initialize rendering parameters only once. m_FirstRender controls if the
     initialization has been done or not.
  */
  void InitializeImagePositionAndSize(void);

  /** Draw the annotations. For internal use only. */
  void SetupAnnotations (void);

  /**
     Call ResetZoom() and restablish the zoom factor and the camera position to the current state.
     The call to ResetZoom() is sometimes usefull to force VTK to update the bounding box of all
     objects in the scene so that everything becomes visible. Internal use only.
   */
  void ResetAndRestablishZoomAndCamera (void);



  
 private:
  
  void           InitInteractorStyle(unsigned int p_style);
  // Update data information and set the update and display extents.
  // Reset the camera position.
  void           UpdateImageActor();
  

  /// internal use: image to input to the mask filter or image blender
  vtkImageData*                   AuxInput;

  /// internal use: image to input to the reslicer
  vtkImageData*                   ResliceInput;

  /// overlapping image
  vtkImageData*                   OverlappingImage;

  /// to map the image to the current window level
  vtkImageMapToWindowLevelColors* WindowLevelForCorner;
  vtkImageMapToColors*            WindowLevel;
    
  /// vtkImageReslice is the swiss-army-knife of image geometry filters: It can
  /// permute, rotate, flip, scale, resample, deform, and pad image data in any
  /// combination with reasonably high efficiency.
  vtkImageReslice*                ImageReslice;

  // mask filter
  vtkImageBlendWithMask*          MaskFilter;

  // image blender
  vtkImageBlend*                  Blender;
  
  /// actors and mappers:
  vtkImageActor*                  ImageActor; 
  vtkActor*                       HorizontalLineActor;
  vtkLineSource*                  HorizontalLineSource;  
  vtkActor*                       VerticalLineActor;
  vtkLineSource*                  VerticalLineSource;
    
  int                             FirstRender;
  bool                            ShowCurrentPoint;
  bool                            ShowDirections;  
  bool                            ShowSliceNumber;

  bool                            LinkCameraFocalAndPosition;
  
  /// Orientation: 0 is XY, 1 is ZY, 2 is XZ
  unsigned int                    Orientation;

  unsigned int                    InteractionStyle;

  
  vtkPlane*                     DataSetCutPlane;
  vtkBox*                       DataSetCutBox;
  double                        BoxThickness;
  
  unsigned int LeftButtonInteractionStyle;
  unsigned int RightButtonInteractionStyle;
  unsigned int MiddleButtonInteractionStyle;
  unsigned int WheelInteractionStyle;


  double       InitialParallelScale;  

  unsigned int Conventions;

  int          FirstImage;

/// background image 
  vtkImageData*                  BGImage;
  vtkImageActor *                BGActor;
  vtkImageMapToColors*           BGWindowLevel;
  
};

#endif /* _vtkViewImage2D_h_ */
