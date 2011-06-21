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
#ifndef _vtkViewImage_h_
#define _vtkViewImage_h_

#include "vtkINRIA3DConfigure.h"

#include <vtkSynchronizedView.h>

#include <vtkCommand.h>
#include <vtkTransform.h>
#include <vtkScalarBarActor.h>
#include <vtkLookupTable.h>

#ifdef vtkINRIA3D_USE_ITK
#include <itkImage.h>
#endif

#include <vector>

class vtkScalarsToColors;
class vtkImageData;
class vtkImageBlend;

class vtkColorTransferFunction;

class vtkDataSet;
class vtkProperty;
class vtkMapper;
class vtkActor;
class vtkScalarBarActor;



/**
   \class vtkViewImage vtkViewImage.h "vtkViewImage2D.h"
   \brief Base class for 2D/3D image viewers, see vtkViewImage2D and vtkViewImage3D.
   \author: Pierre Fillard & Marc Traina & Nicolas Toussaint
*/


class VTK_RENDERINGADDON_EXPORT vtkViewImage : public vtkSynchronizedView
{

 public:

  static vtkViewImage* New();
  vtkTypeRevisionMacro(vtkViewImage, vtkSynchronizedView);

  //BTX
  /** Indices use in Vtk referentials corresponding to the standard x, y and z unitary vectors. */
  enum DirectionIds
  {
    X_ID,
    Y_ID,
    Z_ID,
    NB_DIRECTION_IDS  // The number of DirectionIds
  };
  //ETX
  //BTX
  /** Ids of the 2D plan displayed in the view. Each 2D plan is defined with regard to one of
      the 3 directions (x,y,z). */
  enum PlanIds
  {
    SAGITTAL_ID,
    CORONAL_ID,
    AXIAL_ID,
    NB_PLAN_IDS  // The number of PlanIds
  };
  //ETX

  //BTX
  enum EventIds
  {
    ViewImagePositionChangeEvent=(vtkCommand::UserEvent+1),
    ViewImageWindowLevelChangeEvent,
    ViewImageZoomChangeEvent
  };
  //ETX


  //BTX
  enum
  {
    FullRange,
    UserDefinedPercentage
  };
  //ETX


  // General Get Methods about the image
  
  /** Get the begining position of the first pixel in the given axis. Note : the
      position (0,0,0) is the center of the first pixel (0,0,0) so the return value can be
      negative, depending of the origin of the image. */
  double          GetWholeMinPosition(unsigned int p_axis);
  

  /** Get the end position of the last pixel in the given axis. */
  double          GetWholeMaxPosition(unsigned int p_axis);
  

  /** Get the id of the axis orthogonal to the given plan
      (x for sagittal, y for coronal and z for axial) */
  static unsigned int   GetOrthogonalAxis(unsigned int p_plan);

  
  /** Return the voxel indices of the point pos */
  void           GetVoxelCoordinates(double pos[3], int p_coordinates[3]);

  
  /** Fill the p_coodirdinate parameter with the coordinates of the voxel
      corresponding to the current point. */
  virtual void   GetCurrentVoxelCoordinates(int p_coordinates[3]);


  /** Get the value of the current point in double. */
  virtual double GetCurrentPointDoubleValue ();
  

  /* Overridden methods. */
  virtual void UpdatePosition(void) {};


  /** Get the current image. */
  vtkGetObjectMacro (Image, vtkImageData);
  
  
  /** Set the current image. */
  virtual void SetImage (vtkImageData* image);

  
#ifdef vtkINRIA3D_USE_ITK
  /**
     When ITK is set ot ON, we propose the following method to open
     directly an ITK image. Because of the templates, we have to setup
     a method for each type of image. Up to now are supported 3D images
     of types: double, float, (unsigned) int, (unsigned) short and
     (unsigned) char. This class does the ITK->VTK conversion using
     the filter itk::ImageToVTKImageFilter (taken from InsightApplications).
     The problem with this filter is that if it is not registered,
     the output is erased along with the filter (in fact the output has
     the same registered count as the input. For instance, if the input
     image comes from an ITK reader, its reference count is set to 1.
     After conversion, one would like to erase the reader. If the
     converter is not referenced, the output buffer sees its ref count
     goes to 0, which result in a godamn seg fault. To overcome this,
     we reference the current converter in the pointer ImageConverter.
   */
  //BTX
  void SetITKImage (itk::Image<double, 3>::Pointer);
  
  void SetITKImage (itk::Image<float, 3>::Pointer);
  
  void SetITKImage (itk::Image<int, 3>::Pointer);

  void SetITKImage (itk::Image<unsigned int, 3>::Pointer);

  void SetITKImage (itk::Image<short, 3>::Pointer);

  void SetITKImage (itk::Image<unsigned short, 3>::Pointer);

  void SetITKImage (itk::Image<long, 3>::Pointer);

  void SetITKImage (itk::Image<unsigned long, 3>::Pointer);

  void SetITKImage (itk::Image<char, 3>::Pointer);

  void SetITKImage (itk::Image<unsigned char, 3>::Pointer);
  //ETX
#endif

  
  /** Update the the display parameters based on the data information. */
  virtual void Update(){};
  

  /** Get/Set the transformation to be applied to the image. */
  vtkSetObjectMacro (Transform, vtkTransform);
  vtkGetObjectMacro (Transform, vtkTransform);

  /** Get the ScalarBarActor */
  vtkGetObjectMacro (ScalarBar, vtkScalarBarActor);

  /** Set/Get the scalar bar visibility */
  void ScalarBarVisibilityOn (void)
  { this->ScalarBar->VisibilityOn(); }

  /** Set/Get the scalar bar visibility */
  void ScalarBarVisibilityOff (void)
  { this->ScalarBar->VisibilityOff(); }

  /** Set/Get the scalar bar visibility */
  void SetScalarBarVisibility (const int &v)
  { this->ScalarBar->SetVisibility(v); }

  /** Set/Get the scalar bar visibility */
  int GetScalarBarVisibility (void) const
  { return this->ScalarBar->GetVisibility(); }

  
  /**
     Set/Get a user-defined lookup table. This method is synchronized.
  */
  void SyncSetLookupTable (vtkScalarsToColors* lut);
  virtual void SetLookupTable (vtkScalarsToColors* lut)
  {
    this->LookupTable = lut;
    this->ScalarBar->SetLookupTable ( lut );
  }
  virtual vtkScalarsToColors* GetLookupTable (void) const
  { return this->LookupTable; }


  /**
     Set/Get the window/level/zoom parameter. Method to be overriden
     by subclasses.
  */
  //vtkSetMacro (Window, double);
  virtual void SetWindow (double win)
  {
    this->Window = win;
    this->InvokeEvent (vtkViewImage::ViewImageWindowLevelChangeEvent, NULL);
  }  
  vtkGetMacro (Window, double);
  
  //vtkSetMacro (Level, double);
  virtual void SetLevel (double lev)
  {
    this->Level = lev;
    this->InvokeEvent (vtkViewImage::ViewImageWindowLevelChangeEvent, NULL);
  }
  vtkGetMacro (Level, double);

  //vtkSetMacro (Zoom, double);
  virtual void SetZoom (double zoom)
  {
    this->Zoom = zoom;
    this->InvokeEvent (vtkViewImage::ViewImageZoomChangeEvent, NULL);
  }  
  vtkGetMacro (Zoom, double);
  

  /** */
  int  GetSliceForPoint(const double pos[3], unsigned int p_orientation);

  /** Get the slice number within the current point is located.
      @param p_orientation : the desired slice type AXIAL, SAGITTAL, CORONAL. */
  int  GetSlice(unsigned int p_orientation);

  /** Set the slice number within the current point is
      @param p_slice : the desired slice
      @param p_orientation : the desired slice type AXIAL, SAGITTAL, CORONAL. */
  void SyncSetSlice (unsigned int p_orientation, int p_slice);
  void SetSlice (unsigned int p_orientation, int p_slice);

  
  /** Set the Slice to display. Z is relative to the displayed plan. */
  void SyncSetZSlice (int p_slice);
  virtual void SetZSlice(int p_slice) {
      (void) p_slice;
  }


  /** */
  void GetPositionForSlice ( int slice, int orientation, double pos[3]);
  
  
  /**
     Change the current position of the image. This method should be
     overriden by subsclasses.
     This method is synchronized.
  */
  void SyncSetCurrentPoint(const double p_point[3]);

  virtual void SetCurrentPoint (const double p_point[3])
  {
    this->CurrentPoint[0] = p_point[0];
    this->CurrentPoint[1] = p_point[1];
    this->CurrentPoint[2] = p_point[2];
    this->UpdatePosition();

    this->InvokeEvent (vtkViewImage::ViewImagePositionChangeEvent, NULL);
  }
  

  /**
     Synonym to SyncSetCurrentPoint().
   */
  void SyncSetPosition (const double p_point[3])
  {
    this->SyncSetCurrentPoint (p_point);
  }
  

  
  /** Return the current point.*/
  virtual const double* GetCurrentPoint() const
  { return CurrentPoint; }

  
  /** Return the current point.*/
  void GetCurrentPoint(double p_point[3]) const
  {
    p_point[0] = CurrentPoint[0];
    p_point[1] = CurrentPoint[1];
    p_point[2] = CurrentPoint[2];
  }
  
  
  /** Set/Get window/level for mapping pixels to colors. */
  virtual double GetColorWindow(){return -1.0;}

  
  /** Set/Get window/level for mapping pixels to colors. */
  virtual double GetColorLevel(){return -1.0;}

  
  /**
     Set/Get window/level for mapping pixels to colors.
     This method is synchronized , except it LinkWindowLevel if set to 0.
  */
  void SyncSetWindow(double w);

  
  /**
     Set/Get window/level for mapping pixels to colors.
     This method is synchronized , except it LinkWindowLevel if set to 0.
  */
  void SyncSetLevel(double l);


  /**
     Set the zoom factor. This method is synchronized unless SetLinkZoom
     is set to 0.
   */
  void SyncSetZoom (double factor);

  
  /** Set (copy) the windows level from given view window level.*/
  virtual void SetWindowLevelFrom(vtkViewImage* p_view);
  

  /**
     Reset window level to calculated default value.
     This method is synchronized , except it LinkWindowLevel if set to 0.
  */
  void SyncResetWindowLevel();

  virtual void ResetWindowLevel();

  
  /**
     Reset the current point to the center of the image in the 3 axes.
     This method is synchronized.
  */
  void SyncResetCurrentPoint();

  virtual void ResetCurrentPoint();

  /**
     Synonym to SyncResetCurrentPoint().
  */
  void SyncResetPosition()
  {
    this->SyncResetCurrentPoint();
  }


  void SyncResetZoom();
  virtual void ResetZoom();

  
  /** Reset Window-Level, current-point and zoom. */
  void SyncReset();
  virtual void Reset (void)
  {
    this->ResetWindowLevel();
    this->ResetCurrentPoint();
    this->ResetZoom();
  }
  
  
  /** Set the color window/level link ON or OFF */
  vtkSetMacro (LinkWindowLevel, bool);
  vtkGetMacro (LinkWindowLevel, bool);


  /** Set the position link ON or OFF */
  vtkSetMacro (LinkPosition, bool);
  vtkGetMacro (LinkPosition, bool);

  /** Set the zoom link ON or OFF */
  vtkSetMacro (LinkZoom, bool);
  vtkGetMacro (LinkZoom, bool);

  /** Set the render link ON or OFF */
  vtkSetMacro (LinkRender, bool);
  vtkGetMacro (LinkRender, bool);


  /** Shift/Scale are used to get the true image intensity if the image
      was scaled before being inputed to the view.*/
  vtkSetMacro (Shift, double);
  vtkGetMacro (Shift, double);

  /** Shift/Scale are used to get the true image intensity if the image
      was scaled before being inputed to the view.*/
  vtkSetMacro (Scale, double);
  vtkGetMacro (Scale, double);
  
  
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  
  /** Get/Set Method for lower left annotation */ 
  virtual const char* GetSizeData(void) const
  { return SizeData_str.c_str(); }

  
  virtual void SetSizeData (const char* str)
  { SizeData_str = str; }


  /**
     Set a mask image and its corresponding LookupTable. The mask image will
     be overlapped to the current image, and the lookup table is used to assess
     the color of the label: label 0 will have color given by entry 0 of the LUT, etc.
     The image has to be of type unsigned char.
     This method is synchronized.
  */
  void SyncSetMaskImage (vtkImageData* mask, vtkLookupTable* lut);
  virtual void SetMaskImage (vtkImageData* mask, vtkLookupTable* lut)
  {
    this->MaskImage = mask;
    this->MaskLUT   = lut;
  }
  /** Get the mask image (if any). */
  vtkGetObjectMacro (MaskImage, vtkImageData);

  
  /**
     Remove the mask image (if any)
  */
  void SyncRemoveMaskImage ();
  virtual void RemoveMaskImage (void){};
  
  /**
     Set an overlapping second image. It uses an internal LUT to assess the color.
     It does not need to be of type unsigned char.
     This method is synchronized.
  */
  void SyncSetOverlappingImage (vtkImageData* image);
  virtual void SetOverlappingImage (vtkImageData* image) {
      (void) image;
  }

  /**
     Remove the overlapping image (if any)
  */
  void SyncRemoveOverlappingImage ();
  void RemoveOverlappingImage (void){};

  /**
    Add a dataset to the view (polydata or grid).
    The dataset will be cut by planes defining the current slice displayed.
    This results in a loss of dimensionality, i.e. tetrahedron will be displayed
    as triangles, triangles as lines, lines as points.
    A vtkProperty of the dataset can be specified.
    This method is synchronized.
  */
  vtkActor* SyncAddDataSet (vtkDataSet* dataset, vtkProperty* property = NULL);
  virtual vtkActor* AddDataSet (vtkDataSet* dataset, vtkProperty* property = NULL);

  
  /**
    This method allows you to remove a previously added dataset off the view.
    It simply removes the actor from the renderer.
    This method is synchronized.
  */
  void SyncRemoveDataSet (vtkDataSet* dataset);
  virtual void RemoveDataSet (vtkDataSet* dataset);
  
  void SyncRemoveAllDataSet ();
  virtual void RemoveAllDataSet ();

  /**
     Test if the dataset was already passed to the view.
  */
  virtual bool HasDataSet (vtkDataSet* dataset);


  vtkDataSet* GetDataSet (unsigned int i);
  vtkDataSet* GetDataSetFromActor (vtkActor* actor);
  vtkActor* GetDataSetActor (unsigned int i);
  vtkActor* GetDataSetActor (vtkDataSet* dataset);
  
  
  /**
     This method colorizes a given dataset (previously added with AddDataSet()) by one of its arrays
     It doesn't handle colorization by a specific component yet. If the array contains multi-component scalars,
     it takes the norm of the vector.
     This method is synchronized.
  */
  void SyncColorDataSetByArray(vtkDataSet* dataset, const char* arrayname, vtkColorTransferFunction* transfer);

  virtual void ColorDataSetByArray(vtkDataSet* dataset, const char* arrayname, vtkColorTransferFunction* transfer);

  /**
     This method changes the actors associated with a given dataset (previously added with AddDataSet()) 
     to switch between the use of cell array or point data array. Used for switch between color and direction based
     colors of fiber bundles projections.
     This method is synchronized.
  */
  void SyncModifyActors(vtkDataSet* dataset, bool cellColors);

  virtual void ModifyActors(vtkDataSet* dataset, bool cellColors);


  // derived from vtkSynchronizedView
  virtual void Initialize(void);
  virtual void Uninitialize(void);


  /**
     Specify how the ResetWindowLevel() method behaves. If set to FullRange,
     ResetWindowLevel() sets the contrast to match the full range of the image
     i.e: 0:range[0] and 255: range[1].
     If set to UserDefinedPercentage, the X% highest and lowest voxels are
     removed to calculate the range. It is more robust to outliers that have
     a very high and low intensity compared to the main element of the image.
     The percentage is set with SetWindowLevelPercentage() (default: 0.1).
   */
  void SetResetWindowLevelMode(const int& mode)
  {
    this->ResetWindowLevelMode = mode;
  }
  
  void SetResetWindowLevelModeToFullRange( void )
  {
    this->ResetWindowLevelMode = FullRange;
  }
  
  void SetResetWindowLevelModeToUserDefinedPercentage( void )
  {
    this->ResetWindowLevelMode = UserDefinedPercentage;
  }
    

  vtkSetMacro (WindowLevelPercentage, double);
  vtkGetMacro (WindowLevelPercentage, double);
  
  
 protected:
  vtkViewImage();
  ~vtkViewImage();


  /** Register the image. Internal Use Only. */
  virtual void RegisterImage (vtkImageData* image);

  //BTX
  std::vector<vtkDataSet*>      DataSetList;
  std::vector<vtkActor*>        DataSetActorList;
  //ETX
  
  
 private:

  /// VTK image
  vtkImageData*   Image;

  vtkImageData*   MaskImage;
  vtkLookupTable* MaskLUT;
  
  double          CurrentPoint[3];

  vtkTransform*   Transform;
  vtkScalarsToColors* LookupTable;

  vtkScalarBarActor*  ScalarBar;

  bool            LinkWindowLevel;
  bool            LinkPosition;
  bool            LinkZoom;
  bool            LinkRender;
  
  double          Scale;
  double          Shift;

  double          Window;
  double          Level;
  double          Zoom;

#ifdef vtkINRIA3D_USE_ITK
  /**
     This pointer is used to store internally a reference to the
     current ITK->VTK converter, in order to prevent the image buffer
     to be deleted unexpectdely. See the SetITKImageInXXX for more
     information.
   */
  //BTX
  itk::ProcessObject::Pointer ImageConverter;
  //ETX
#endif

  //BTX
  std::string     SizeData_str;
  //ETX

  int           ResetWindowLevelMode;
  double        WindowLevelPercentage;
  
};

#endif /* _vtkViewImage_h_ */
