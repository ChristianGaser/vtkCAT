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
#include <vtkViewImage.h>

#include "vtkObjectFactory.h"

#include "vtkProp.h"
#include "vtkTextActor.h"
#include "vtkCoordinate.h"
#include "vtkProperty.h"
#include "vtkProperty2D.h"
#include "vtkTextProperty.h"
#include "vtkScalarsToColors.h"
#include "vtkImageMapToWindowLevelColors.h"
#include "vtkImageData.h"
#include "vtkActor.h"
#include "vtkRenderWindow.h"


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
#include <vtkImageAccumulate.h>
#include <vtkDoubleArray.h>
#include <vtkIntArray.h>

#ifdef vtkINRIA3D_USE_ITK
#include <itkImageToVTKImageFilter.h>
#endif

#include <assert.h>
#include <iostream>
#include <sstream>
#include <cmath>

extern int vtkrint(double a);

vtkCxxRevisionMacro(vtkViewImage, "$Revision$");
vtkStandardNewMacro(vtkViewImage);


vtkViewImage::vtkViewImage()
{
  
  this->Image       = 0;
  this->MaskImage   = 0;
  this->MaskLUT     = 0;
  this->Transform   = 0;
  this->LookupTable = 0;
  this->LinkWindowLevel = true;
  this->LinkPosition    = true;
  this->LinkZoom        = false;
  this->LinkRender      = true;
  
  this->Shift = 0.0;
  this->Scale = 1.0;
  this->Level = 128.0;
  this->Window = 255.0;
  this->Zoom   = 1.0;
  
  this->ScalarBar = vtkScalarBarActor::New();
  this->ScalarBar->GetLabelTextProperty()->SetColor (1.0,1.0,1.0);
  this->ScalarBar->GetTitleTextProperty()->SetColor (1.0,1.0,1.0);
  //this->ScalarBar->SetTextPositionToPrecedeScalarBar();
  this->ScalarBar->GetLabelTextProperty()->BoldOff();
  this->ScalarBar->GetLabelTextProperty()->ShadowOff();
  this->ScalarBar->GetLabelTextProperty()->ItalicOff();
  this->ScalarBar->SetNumberOfLabels (3);
  this->ScalarBar->GetLabelTextProperty()->SetFontSize (1);
  this->ScalarBar->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
  this->ScalarBar->SetWidth (0.1);
  this->ScalarBar->SetHeight (0.5);
  this->ScalarBar->SetPosition (0.9,0.3);
  //this->ScalarBar->GetPositionCoordinate()->SetValue ( 0.1, 0.01);
  this->ScalarBar->VisibilityOff();
    
  vtkLookupTable* bwLut = vtkLookupTable::New();
  bwLut->SetTableRange (0, 1);
  bwLut->SetSaturationRange (0, 0);
  bwLut->SetHueRange (0, 0);
  bwLut->SetValueRange (0, 1);
  bwLut->Build();
  this->SetLookupTable (bwLut);
  bwLut->Delete();

  this->CurrentPoint[0] = 0.0;
  this->CurrentPoint[1] = 0.0;
  this->CurrentPoint[2] = 0.0;
  
  this->SetAboutData ("");

  this->ResetWindowLevelMode = UserDefinedPercentage;
  this->WindowLevelPercentage = 0.1;

  // Increase polygon offsets to support some OpenGL drivers
  //vtkMapper::SetResolveCoincidentTopologyToPolygonOffset();
  //vtkMapper::SetResolveCoincidentTopologyPolygonOffsetParameters(10,10);

#ifdef vtkINRIA3D_USE_ITK
  this->ImageConverter = 0;
#endif

}



vtkViewImage::~vtkViewImage()
{
  if( this->Transform )
  {
    this->Transform->Delete();
  }
  
  if( this->Image )
  {
    this->Image->Delete();
  }


  this->ScalarBar->Delete();
  
  this->RemoveAllDataSet();

#ifdef vtkINRIA3D_USE_ITK
  this->ImageConverter = 0;
#endif
  
}



void vtkViewImage::SetImage (vtkImageData* image)
{
  vtkErrorMacro(<<"You should not use this function here\nPlease use vtkViewImage2D or vtkViewImage3D classes instead");
}



void vtkViewImage::Initialize()
{
  vtkSynchronizedView::Initialize();
  this->SetLookupTable( this->GetLookupTable() );
  this->AddActor (this->ScalarBar);
}


void vtkViewImage::Uninitialize()
{
  this->RemoveActor (this->ScalarBar);
  vtkSynchronizedView::Uninitialize();
}


void vtkViewImage::RegisterImage(vtkImageData* image)
{
  if(!image)  return;
    
  if( image != this->Image )
  {
    if( this->Image != NULL )
    {
      this->Image->UnRegister (this);
    }
    
    this->Image = image;
    this->Image->Register (this);
  }
}


double vtkViewImage::GetWholeMinPosition(unsigned int p_axis)
{
  if(!this->Image)
  {
    return -VTK_LARGE_FLOAT;
  }
    
  return this->Image->GetBounds()[p_axis * 2];// -
  //this->Image->GetSpacing()[p_axis] / 2;
}


double vtkViewImage::GetWholeMaxPosition(unsigned int p_axis)
{
  if(!this->Image)
  {
    return VTK_LARGE_FLOAT;
  }
  
  return this->Image->GetBounds()[p_axis * 2 + 1];// +
  //this->Image->GetSpacing()[p_axis] / 2;
}


unsigned int vtkViewImage::GetOrthogonalAxis(unsigned int p_plan)
{
  assert(p_plan<NB_DIRECTION_IDS);  
  
  switch(p_plan)
  {
    
      case SAGITTAL_ID:
        return X_ID;
        break;

        
      case CORONAL_ID:
        return Y_ID;
        break;

        
      case AXIAL_ID:
        
        return Z_ID;
        break;      
        
  }
  return 0;    
}



int vtkViewImage::GetSliceForPoint(const double pos[3], unsigned int p_plan)
{

  if( !this->GetImage() )
  {
    return 0;
  }

  assert(p_plan < NB_DIRECTION_IDS);

  double*       spacing = this->GetImage()->GetSpacing();
  double*       origin  = this->GetImage()->GetOrigin();
  
  int   axis     = this->GetOrthogonalAxis(p_plan);
  double  soft_pos = pos[axis];
  double  pos_max  = this->GetWholeMaxPosition(axis);
  double  pos_min  = this->GetWholeMinPosition(axis);

  // Treat extreme position at the end of the last pixel
  if ((soft_pos > pos_max-0.005) && (soft_pos < pos_max+0.005))
  {
    soft_pos = pos_max-0.005;
  }  
  if ((soft_pos > pos_min-0.005) && (soft_pos < pos_min+0.005))
  {
    soft_pos = pos_min+0.005;
  }
  return (int)vtkrint((soft_pos-origin[axis])/spacing[axis]);  
}



int vtkViewImage::GetSlice(unsigned int p_plan)
{
  const double* pos = this->GetCurrentPoint();
  return this->GetSliceForPoint (pos, p_plan);
}

void vtkViewImage::SetSlice (unsigned int p_plan, int p_zslice)
{
  /*
  if( !this->GetImage() )
  {
    return;
  }
  
  assert(p_plan<NB_DIRECTION_IDS);  
  
  unsigned int axis = this->GetOrthogonalAxis(p_plan);
  
  double* spacing = this->GetImage()->GetSpacing();
  int*    extent  = this->GetImage()->GetWholeExtent();
  double* origin  = this->GetImage()->GetOrigin();
  int     slice   = p_zslice;

  int dims[3];
  dims[0] = extent[1];
  dims[1] = extent[3];
  dims[2] = extent[5];  
  
  if(slice >= dims[axis])  slice = dims[axis];
  if(slice <  0)           slice = 0;
  
  double pos[3];
  this->GetCurrentPoint(pos);
  pos[axis] = origin[axis] + slice * spacing[axis];
  */

  double pos[3];
  this->GetPositionForSlice (p_zslice, p_plan, pos);
  
//   std::cout << "Position: " << pos[0] << " " << pos[1] << " " << pos[2] << std::endl;
  
  this->SetCurrentPoint(pos);
}



void vtkViewImage::GetPositionForSlice (int p_zslice, int orientation, double pos[3])
{

  if( !this->GetImage() )
  {
    return;
  }

  //assert(p_plan<orientation);
  unsigned int axis = this->GetOrthogonalAxis(orientation);

  double* spacing = this->GetImage()->GetSpacing();
  int*    extent  = this->GetImage()->GetWholeExtent();
  double* origin  = this->GetImage()->GetOrigin();
  int     slice   = p_zslice;

  int dims[3];
  dims[0] = extent[1];
  dims[1] = extent[3];
  dims[2] = extent[5];  
  
  if(slice >= dims[axis])  slice = dims[axis];
  if(slice <  0)           slice = 0;
  
  this->GetCurrentPoint(pos);
  pos[axis] = origin[axis] + slice * spacing[axis];
  
}


void vtkViewImage::SyncSetSlice(unsigned int p_plan, int p_zslice)
{

  if( this->IsLocked() )
  {
    return;
  }
  
  this->SetSlice(p_plan, p_zslice);
  
  this->Lock();
  for( unsigned int i=0; i<this->Children.size(); i++)
  {
    vtkViewImage* view = vtkViewImage::SafeDownCast (this->Children[i]);

    if( view && view->GetLinkPosition())
    {
      view->SyncSetSlice (p_plan, p_zslice);
      if( view->GetRenderWindow() && !view->GetRenderWindow()->GetNeverRendered() )
      {
        view->Render();
      }
    }
  }
  this->UnLock();
}


void vtkViewImage::SyncSetZSlice(int p_zslice)
{
  if( this->IsLocked() )
  {
    return;
  }
  
  this->SetZSlice(p_zslice);
  
  this->Lock();
  for( unsigned int i=0; i<this->Children.size(); i++)
  {
    vtkViewImage* view = vtkViewImage::SafeDownCast (this->Children[i]);

    if( view && view->GetLinkPosition())
    {
      view->SyncSetZSlice (p_zslice);
      if( view->GetRenderWindow() && !view->GetRenderWindow()->GetNeverRendered() )
      {
        view->Render();
      }
    }
  }
  this->UnLock();
}


void vtkViewImage::GetVoxelCoordinates(double pos[3], int p_coordinates[3])
{
  if(!this->Image)
  {
    return;
  }
  
  p_coordinates[this->GetOrthogonalAxis(SAGITTAL_ID)] = this->GetSliceForPoint(pos, SAGITTAL_ID);
  p_coordinates[this->GetOrthogonalAxis(CORONAL_ID)]  = this->GetSliceForPoint(pos, CORONAL_ID);  
  p_coordinates[this->GetOrthogonalAxis(AXIAL_ID)]    = this->GetSliceForPoint(pos, AXIAL_ID);
}


void vtkViewImage::GetCurrentVoxelCoordinates(int p_coordinates[3])
{
  p_coordinates[this->GetOrthogonalAxis(SAGITTAL_ID)] = this->GetSlice(SAGITTAL_ID);
  p_coordinates[this->GetOrthogonalAxis(CORONAL_ID)]  = this->GetSlice(CORONAL_ID);  
  p_coordinates[this->GetOrthogonalAxis(AXIAL_ID)]    = this->GetSlice(AXIAL_ID);
}






void vtkViewImage::SyncSetCurrentPoint(const double p_point[3])
{

  if( this->IsLocked() )
  {
    return;
  }

  this->SetCurrentPoint (p_point);
  
  // this boolean is used so that the other observe won't call
  // SetCurrentPoint again and again and again...
  this->Lock();
  for( unsigned int i=0; i<this->Children.size(); i++)
  {
//     vtkViewImage* view = reinterpret_cast<vtkViewImage*> ( this->Children[i] );
    vtkViewImage* view = vtkViewImage::SafeDownCast( this->Children[i] );
    if( view && view->GetLinkPosition())
    {
      view->SyncSetCurrentPoint (p_point);
      if( view->GetRenderWindow() && !view->GetRenderWindow()->GetNeverRendered() )
      {
        view->Render();
      }
    }
  }
  
  this->UnLock();
    
}



double vtkViewImage::GetCurrentPointDoubleValue () 
{
  if (!this->Image)
  {
    return -1;
  }

  
  int coordinates[3];
  this->GetCurrentVoxelCoordinates(coordinates);

  void *scalar = this->GetImage()->GetScalarPointer(coordinates);

  if( !scalar ) return -1.0;
  
  switch (this->GetImage()->GetScalarType()) {
  case VTK_DOUBLE:
    return (*(double*)scalar);
  case VTK_FLOAT:
    return (double) (*(float*)scalar);
  case VTK_UNSIGNED_LONG:
    return (double) (*(unsigned long*)scalar);
  case VTK_LONG:
    return (double) (*(long*)scalar);
  case VTK_UNSIGNED_INT:
    return (double) (*(unsigned int*)scalar);
  case VTK_INT:
    return (double) (*(int*)scalar);
  case VTK_UNSIGNED_SHORT:
    return (double) (*(unsigned short*)scalar);
  case VTK_SHORT:
    return (double) (*(short*)scalar);
  case VTK_UNSIGNED_CHAR:
    return (double) (*(unsigned char*)scalar);
  case VTK_CHAR:
    return (double) (*(char*)scalar);
  }    
  return -1;        
}


void vtkViewImage::SyncResetCurrentPoint()
{
  if( this->IsLocked() )
  {
    return;
  }


  this->ResetCurrentPoint();
  
  this->Lock();
  for( unsigned int i=0; i<this->Children.size(); i++)
  {
//     vtkViewImage* view = reinterpret_cast<vtkViewImage*> (this->Children[i]);
    vtkViewImage* view = vtkViewImage::SafeDownCast( this->Children[i] );
    if( view && view->GetLinkPosition() )
    {
      view->SyncResetCurrentPoint ();
      if( view->GetRenderWindow() && !view->GetRenderWindow()->GetNeverRendered() )
      {
        view->Render();
      }          
    }
  }
  this->UnLock();
  
}



void vtkViewImage::ResetCurrentPoint()
{
  
  if (!this->Image)
  {
    return;
  }
  

  double* bounds  = this->GetImage()->GetBounds();
  /*
    double* spacing = this->GetImage()->GetSpacing();
    int*    ext     = this->GetImage()->GetWholeExtent();
    double* origin  = this->GetImage()->GetOrigin();
  */
    
  double  pos[3] = 
    {
      /*
        origin[0] + ext[1] / 2.0 * spacing[0],
        origin[1] + ext[3] / 2.0 * spacing[1],
        origin[2] + ext[5] / 2.0 * spacing[2]
      */
      (bounds[0]+bounds[1])/2.0,
      (bounds[2]+bounds[3])/2.0,
      (bounds[4]+bounds[5])/2.0
    };
  
  this->SetCurrentPoint(pos);  
}



void vtkViewImage::SyncSetWindow (double w)
{
  if( this->IsLocked() )
  {
    return;
  }

  
  this->SetWindow (w);

  
  // this boolean is used so that the other observe won't call
  // SetCurrentPoint again and again and again...
  this->Lock();
  for( unsigned int i=0; i<this->Children.size(); i++)
  {
    
    vtkViewImage* view = vtkViewImage::SafeDownCast (this->Children[i]);
    
    if( view && view->GetLinkWindowLevel() )
    {
      view->SyncSetWindow (w);
      if( view->GetRenderWindow() && !view->GetRenderWindow()->GetNeverRendered() )
      {
        view->Render();
      }          
    }
  }
  
  this->UnLock();
}



void vtkViewImage::SyncSetLevel (double l)
{

  if( this->IsLocked() )
  {
    return;
  }
  

  this->SetLevel (l);
  
  // this boolean is used so that the other observe won't call
  // SetCurrentPoint again and again and again...
  this->Lock();
  for( unsigned int i=0; i<this->Children.size(); i++)
  {
    vtkViewImage* view = vtkViewImage::SafeDownCast (this->Children[i]);

    if( view && view->GetLinkWindowLevel() )
    {
      view->SyncSetLevel (l);
      if( view->GetRenderWindow() && !view->GetRenderWindow()->GetNeverRendered() )
      {
        view->Render();
      }
    }
  }
  
  this->UnLock();
  
}


void vtkViewImage::SetWindowLevelFrom(vtkViewImage* p_view)
{
  if (p_view)
  {
    this->SetWindow( p_view->GetColorWindow() );
    this->SetLevel( p_view->GetColorLevel() );
  }
}



void vtkViewImage::SyncResetWindowLevel()
{

  if( this->IsLocked() )
  {
    return;
  }
  
  
  this->ResetWindowLevel();
  
  this->Lock();
  for( unsigned int i=0; i<this->Children.size(); i++)
  {
//     vtkViewImage* view = reinterpret_cast<vtkViewImage*> (this->Children[i]);
    vtkViewImage* view = vtkViewImage::SafeDownCast(this->Children[i]);
    if( view && view->GetLinkWindowLevel() )
    {
      view->SyncResetWindowLevel ();
      if( view->GetRenderWindow() && !view->GetRenderWindow()->GetNeverRendered() )
      {
        view->Render();
      }          
    }
  }
  this->UnLock();
  
}



void vtkViewImage::ResetWindowLevel()
{

  if (this->IsLocked())
  {
    return;
  }
   
  if (!this->Image)
  {
    return;
  }
  

  this->Image->UpdateInformation();
  this->Image->SetUpdateExtent( this->Image->GetWholeExtent() );
  this->Image->Update();


  if( this->Image->GetScalarType() == VTK_UNSIGNED_CHAR  && (this->Image->GetNumberOfScalarComponents()==3 || this->Image->GetNumberOfScalarComponents()==4) )
  {
    return;
  }
  

  double* range = this->Image->GetScalarRange();

  double window = range[1]-range[0];
  double level = 0.5*(range[1]+range[0]);
  
  if( this->ResetWindowLevelMode ==  FullRange )
  {
    // nothing to do
  }
  else if( this->ResetWindowLevelMode ==  UserDefinedPercentage )
  {
    
    vtkImageAccumulate* histogram = vtkImageAccumulate::New();
    histogram->SetInput ( this->Image );
    
    int extent[6] = {0, 1000, 0, 0, 0, 0};
    histogram->SetComponentExtent (extent);
    
    double spacing[3] = { (range[1]-range[0])/1000.0, 0.0, 0.0};
    histogram->SetComponentSpacing (spacing);
    
    double origin[3] = {range[0], 0.0, 0.0};
    histogram->SetComponentOrigin (origin);
    histogram->Update();
    
    
    vtkImageData* output = histogram->GetOutput();  
    vtkIntArray*  ptData = vtkIntArray::SafeDownCast (output->GetPointData()->GetScalars());
    
    if( !ptData)
    {
      vtkErrorMacro( << "Error: Cannot cast point data to integers.");
      return;
    }
    
    double numVox = histogram->GetVoxelCount();
    double onePercent = numVox/100.0;
    
    int start=1;
    double currentPercent = 0.0;
    while( currentPercent<this->UserDefinedPercentage && start<1000-this->UserDefinedPercentage)
    {
      double tuple;
      ptData->GetTuple (start, &tuple);
      currentPercent += tuple/onePercent;    
      start++;
    }
    
    currentPercent = 0.0;
    int end = 999;
    while( currentPercent<this->UserDefinedPercentage && end>0 )
    {
      double tuple;
      ptData->GetTuple (end, &tuple);
      currentPercent += tuple/onePercent;    
      end--;
    }
    
    window = (end-start)*(range[1]-range[0])/1000.0;
    level = 0.5*(start + end)*(range[1]-range[0])/1000.0;
    
    histogram->Delete();
  }
  
  
  window = (window-this->GetShift())/this->GetScale();
  level = (level-this->GetShift())/this->GetScale();
  
  
  this->SetWindow ( window );
  this->SetLevel ( level );
  
}



bool vtkViewImage::HasDataSet (vtkDataSet* dataset)
{
  if( !dataset )
  {
    return false;
  }
  
  bool res = false;
  for( unsigned int i=0; i<this->DataSetList.size(); i++)
  {
    if( dataset == this->DataSetList[i] )
    {
      res = true;
      break;
    }
  }

  return res;
}



vtkActor* vtkViewImage::SyncAddDataSet (vtkDataSet* dataset,  vtkProperty* property)
{

  if( this->IsLocked() )
  {
    return NULL;
  }


  vtkActor* actor = this->AddDataSet (dataset, property);
  
  this->Lock();
  for( unsigned int i=0; i<this->Children.size(); i++)
  {
//     vtkViewImage* view = dynamic_cast<vtkViewImage*> (this->Children[i]);
    vtkViewImage* view = vtkViewImage::SafeDownCast (this->Children[i]);

    if( view )
    {
      view->SyncAddDataSet (dataset, property);
    }
  }
  this->UnLock();

  return actor;
  
}




vtkActor* vtkViewImage::AddDataSet (vtkDataSet* dataset,  vtkProperty* property)
{
  vtkErrorMacro(<<"You should not use this function here\nPlease use vtkViewImage2D or vtkViewImage3D classes instead (or any derived class).");
  return NULL;  
}



void vtkViewImage::SyncRemoveDataSet (vtkDataSet* dataset)
{

  if( this->IsLocked() )
  {
    return;
  }
  
  
  this->RemoveDataSet (dataset);
  
  this->Lock();
  for( unsigned int i=0; i<this->Children.size(); i++)
  {
//     vtkViewImage* view = dynamic_cast<vtkViewImage*> (this->Children[i]);
    vtkViewImage* view = vtkViewImage::SafeDownCast (this->Children[i]);

    if( view )
    {
      view->SyncRemoveDataSet (dataset);
    }
  }
  this->UnLock();

}



void vtkViewImage::RemoveDataSet (vtkDataSet* dataset)
{

  if (!dataset)
  {
    return;
  }
  
  std::vector<vtkActor*>   t_actorlist = this->DataSetActorList;
  std::vector<vtkDataSet*> t_datasetlist = this->DataSetList;
  this->DataSetActorList.clear();
  this->DataSetList.clear();
  
  for (unsigned int i=0; i<t_datasetlist.size(); i++)
  {
    if (dataset != t_datasetlist[i])
    {
      this->DataSetList.push_back (t_datasetlist[i]);
      this->DataSetActorList.push_back (t_actorlist[i]);
      }
    else
    {  
      this->RemoveActor (t_actorlist[i]);
    }
  }
}



void vtkViewImage::SyncRemoveAllDataSet ()
{
  
  if( this->IsLocked() )
  {
    return;
  }
  
  
  this->RemoveAllDataSet ();
  
  this->Lock();
  for( unsigned int i=0; i<this->Children.size(); i++)
  {
//     vtkViewImage* view = dynamic_cast<vtkViewImage*> (this->Children[i]);
    vtkViewImage* view = vtkViewImage::SafeDownCast (this->Children[i]);

    if( view )
    {
      view->SyncRemoveAllDataSet ();
    }
  }
  this->UnLock();
}



void vtkViewImage::RemoveAllDataSet ()
{
  for (unsigned int i=0; i<this->DataSetActorList.size(); i++)
  {
    this->RemoveActor (this->DataSetActorList[i]);
  }
  this->DataSetActorList.clear();
  this->DataSetList.clear();
  
}



void vtkViewImage::SyncModifyActors(vtkDataSet* dataset, bool cellColors)
{

  if( this->IsLocked() )
  {
    return;
  }
  
  
  this->ModifyActors (dataset, cellColors);

  
  this->Lock();
  for( unsigned int i=0; i<this->Children.size(); i++)
  {
//     vtkViewImage* view = dynamic_cast<vtkViewImage*> (this->Children[i]);
    vtkViewImage* view = vtkViewImage::SafeDownCast (this->Children[i]);

    if( view )
    {
      view->SyncModifyActors (dataset, cellColors);
    }
  }
  this->UnLock();
  
}



void vtkViewImage::ModifyActors(vtkDataSet* dataset, bool cellColors)
{
  bool doit = true;
  
  
  if (!dataset)
  {
    doit = false;
  }
  
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
    
    if ( cellColors )
      mapper->SetScalarModeToUseCellData();
    else
      mapper->SetScalarModeToUsePointData();
  }
}



void vtkViewImage::SyncColorDataSetByArray(vtkDataSet* dataset, const char* arrayname, vtkColorTransferFunction* transfer)
{

  if( this->IsLocked() )
  {
    return;
  }
  
  
  this->ColorDataSetByArray (dataset, arrayname, transfer);

  
  this->Lock();
  for( unsigned int i=0; i<this->Children.size(); i++)
  {
//     vtkViewImage* view = dynamic_cast<vtkViewImage*> (this->Children[i]);
    vtkViewImage* view = vtkViewImage::SafeDownCast (this->Children[i]);

    if( view )
    {
      view->SyncColorDataSetByArray (dataset, arrayname, transfer);
    }
  }
  this->UnLock();
  
}


void vtkViewImage::ColorDataSetByArray(vtkDataSet* dataset, const char* arrayname, vtkColorTransferFunction* transfer)
{

  bool doit = true;
  
  
  if (!dataset)
  {
    doit = false;
  }
  
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
        mapper->SetScalarModeToUseCellFieldData();
      }
    }
    
    if (!array && dataset->GetPointData())
    {
      array = dataset->GetPointData()->GetArray (arrayname);
      if (array)
      {
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



vtkDataSet* vtkViewImage::GetDataSet (unsigned int i)
{
  if (i<0 || i>this->DataSetList.size())
    return NULL;

  return this->DataSetList[i];
}

vtkDataSet* vtkViewImage::GetDataSetFromActor (vtkActor* actor)
{
  for (unsigned int i=0; i<this->DataSetActorList.size(); i++)
  {
    if (actor == this->DataSetActorList[i])
      return this->DataSetList[i];
  }
  return NULL;
}

  



vtkActor* vtkViewImage::GetDataSetActor (unsigned int i)
{
  if (i<0 || i>=this->DataSetActorList.size())
    return NULL;

  return this->DataSetActorList[i];  
}



vtkActor* vtkViewImage::GetDataSetActor (vtkDataSet* dataset)
{
  if (!dataset)
    return NULL;
  
  for (unsigned int i=0; i<this->DataSetList.size(); i++)
  {
    if (dataset == this->DataSetList[i])
      return this->DataSetActorList[i];
  }
  return NULL;
  
}


void vtkViewImage::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}



void vtkViewImage::SyncSetZoom (double factor)
{

  if( this->IsLocked() )
  {
    return;
  }
  
  this->SetZoom (factor);
  
  this->Lock();
  for( unsigned int i=0; i<this->Children.size(); i++)
  {
//     vtkViewImage* view =  dynamic_cast<vtkViewImage*>(this->Children[i]);
    vtkViewImage* view = vtkViewImage::SafeDownCast (this->Children[i]);

    if( view && view->GetLinkZoom() )
    {
        view->SyncSetZoom (factor);
        if( view->GetRenderWindow() && !view->GetRenderWindow()->GetNeverRendered() )
        {
          view->Render();
        }
    }
  }

  this->UnLock();
}



void vtkViewImage::SyncSetLookupTable (vtkScalarsToColors* lut)
{

  if( this->IsLocked() )
  {
    return;
  }
  
  this->SetLookupTable (lut);

  this->Lock();
  for( unsigned int i=0; i<this->Children.size(); i++)
  {
//     vtkViewImage* view =  dynamic_cast<vtkViewImage*>(this->Children[i]);
    vtkViewImage* view = vtkViewImage::SafeDownCast (this->Children[i]);

    if( view )
    {
      view->SyncSetLookupTable (lut);
      if( view->GetRenderWindow() && !view->GetRenderWindow()->GetNeverRendered() )
      {
        view->Render();
      }
    }
  }

  this->UnLock();
}



void vtkViewImage::SyncResetZoom ()
{

  if( this->IsLocked() )
  {
    return;
  }
  
  this->ResetZoom ();

  this->Lock();
  for( unsigned int i=0; i<this->Children.size(); i++)
  {
//     vtkViewImage* view =  dynamic_cast<vtkViewImage*>(this->Children[i]);
    vtkViewImage* view = vtkViewImage::SafeDownCast (this->Children[i]);

    if( view )
    {
      if( view->GetLinkZoom() )
      {
        view->SyncResetZoom ();
        if( !view->GetRenderWindow()->GetNeverRendered() )
        {
          view->Render();
        }
      }
    }
  }

  this->UnLock();
}



void vtkViewImage::ResetZoom()
{
  this->ResetCamera();
  this->Zoom = 1.0;
}



void vtkViewImage::SyncReset ()
{

  if( this->IsLocked() )
  {
    return;
  }
  
  this->Reset ();

  this->Lock();
  for( unsigned int i=0; i<this->Children.size(); i++)
  {
//     vtkViewImage* view =  dynamic_cast<vtkViewImage*>(this->Children[i]);
    vtkViewImage* view = vtkViewImage::SafeDownCast (this->Children[i]);

    if( view )
    {
      view->SyncReset ();
      if( !view->GetRenderWindow()->GetNeverRendered() )
      {
        view->Render();
      }
    }
  }

  this->UnLock();
}




void vtkViewImage::SyncSetMaskImage (vtkImageData* mask, vtkLookupTable* lut)
{

  if( this->IsLocked() )
  {
    return;
  }
  
  this->SetMaskImage (mask, lut);

  this->Lock();
  for( unsigned int i=0; i<this->Children.size(); i++)
  {
//     vtkViewImage* view =  dynamic_cast<vtkViewImage*>(this->Children[i]);
    vtkViewImage* view = vtkViewImage::SafeDownCast (this->Children[i]);

    if( view )
    {
      view->SyncSetMaskImage (mask, lut);
      if( view->GetRenderWindow() && !view->GetRenderWindow()->GetNeverRendered() )
      {
        view->Render();
      }
    }
  }

  this->UnLock();
}


void vtkViewImage::SyncSetOverlappingImage (vtkImageData* image)
{

  if( this->IsLocked() )
  {
    return;
  }
  
  this->SetOverlappingImage (image);

  this->Lock();
  for( unsigned int i=0; i<this->Children.size(); i++)
  {
//     vtkViewImage* view =  dynamic_cast<vtkViewImage*>(this->Children[i]);
    vtkViewImage* view = vtkViewImage::SafeDownCast (this->Children[i]);

    if( view )
    {
      view->SyncSetOverlappingImage (image);
      if( view->GetRenderWindow() && !view->GetRenderWindow()->GetNeverRendered() )
      {
        view->Render();
      }
    }
  }

  this->UnLock();
}



void vtkViewImage::SyncRemoveMaskImage ()
{

  if( this->IsLocked() )
  {
    return;
  }
  
  this->RemoveMaskImage ();

  this->Lock();
  for( unsigned int i=0; i<this->Children.size(); i++)
  {
//     vtkViewImage* view =  dynamic_cast<vtkViewImage*>(this->Children[i]);
    vtkViewImage* view = vtkViewImage::SafeDownCast (this->Children[i]);

    if( view )
    {
      view->SyncRemoveMaskImage ();
      if( !view->GetRenderWindow()->GetNeverRendered() )
      {
        view->Render();
      }
    }
  }

  this->UnLock();
}





void vtkViewImage::SyncRemoveOverlappingImage ()
{

  if( this->IsLocked() )
  {
    return;
  }
  
  this->RemoveOverlappingImage ();

  this->Lock();
  for( unsigned int i=0; i<this->Children.size(); i++)
  {
//     vtkViewImage* view =  dynamic_cast<vtkViewImage*>(this->Children[i]);
    vtkViewImage* view = vtkViewImage::SafeDownCast (this->Children[i]);

    if( view )
    {
      view->SyncRemoveOverlappingImage ();
      if( !view->GetRenderWindow()->GetNeverRendered() )
      {
        view->Render();
      }
    }
  }

  this->UnLock();
}


#ifdef vtkINRIA3D_USE_ITK

void vtkViewImage::SetITKImage (itk::Image<double, 3>::Pointer itkImage)
{
  if( itkImage.IsNull() )
  {
    return;
  }
  
  typedef itk::ImageToVTKImageFilter< itk::Image<double, 3> > ConverterType;
  ConverterType::Pointer myConverter = ConverterType::New();
  myConverter->SetInput ( itkImage );
  myConverter->Update();

  this->SetImage ( myConverter->GetOutput() );

  this->ImageConverter = myConverter;  
}


void vtkViewImage::SetITKImage (itk::Image<float, 3>::Pointer itkImage)
{
  if( itkImage.IsNull() )
  {
    return;
  }
  
  typedef itk::ImageToVTKImageFilter< itk::Image<float, 3> > ConverterType;
  ConverterType::Pointer myConverter = ConverterType::New();
  myConverter->SetInput ( itkImage );
  myConverter->Update();
  
  this->SetImage ( myConverter->GetOutput() );

  this->ImageConverter = myConverter;  
}

void vtkViewImage::SetITKImage (itk::Image<int, 3>::Pointer itkImage)
{
  if( itkImage.IsNull() )
  {
    return;
  }
  
  typedef itk::ImageToVTKImageFilter< itk::Image<int, 3> > ConverterType;
  ConverterType::Pointer myConverter = ConverterType::New();
  myConverter->SetInput ( itkImage );
  myConverter->Update();

  this->SetImage ( myConverter->GetOutput() );

  this->ImageConverter = myConverter;  
}


void vtkViewImage::SetITKImage (itk::Image<unsigned int, 3>::Pointer itkImage)
{
  if( itkImage.IsNull() )
  {
    return;
  }
  
  typedef itk::ImageToVTKImageFilter< itk::Image<unsigned int, 3> > ConverterType;
  ConverterType::Pointer myConverter = ConverterType::New();
  myConverter->SetInput ( itkImage );
  myConverter->Update();

  this->SetImage ( myConverter->GetOutput() );

  this->ImageConverter = myConverter;  
}


void vtkViewImage::SetITKImage (itk::Image<short, 3>::Pointer itkImage)
{
  if( itkImage.IsNull() )
  {
    return;
  }
  
  typedef itk::ImageToVTKImageFilter< itk::Image<short, 3> > ConverterType;
  ConverterType::Pointer myConverter = ConverterType::New();
  myConverter->SetInput ( itkImage );
  myConverter->Update();

  this->SetImage ( myConverter->GetOutput() );

  this->ImageConverter = myConverter;  
}

void vtkViewImage::SetITKImage (itk::Image<unsigned short, 3>::Pointer itkImage)
{
  if( itkImage.IsNull() )
  {
    return;
  }
  
  typedef itk::ImageToVTKImageFilter< itk::Image<unsigned short, 3> > ConverterType;
  ConverterType::Pointer myConverter = ConverterType::New();
  myConverter->SetInput ( itkImage );
  myConverter->Update();

  this->SetImage ( myConverter->GetOutput() );

  this->ImageConverter = myConverter;  
}

void vtkViewImage::SetITKImage (itk::Image<char, 3>::Pointer itkImage)
{
  if( itkImage.IsNull() )
  {
    return;
  }
  
  typedef itk::ImageToVTKImageFilter< itk::Image<char, 3> > ConverterType;
  ConverterType::Pointer myConverter = ConverterType::New();
  myConverter->SetInput ( itkImage );
  myConverter->Update();

  this->SetImage ( myConverter->GetOutput() );

  this->ImageConverter = myConverter;  
}

void vtkViewImage::SetITKImage (itk::Image<unsigned char, 3>::Pointer itkImage)
{
  if( itkImage.IsNull() )
  {
    return;
  }
  
  typedef itk::ImageToVTKImageFilter< itk::Image<unsigned char, 3> > ConverterType;
  ConverterType::Pointer myConverter = ConverterType::New();
  myConverter->SetInput ( itkImage );
  myConverter->Update();

  this->SetImage ( myConverter->GetOutput() );

  this->ImageConverter = myConverter;  
}

void vtkViewImage::SetITKImage (itk::Image<long, 3>::Pointer itkImage)
{
  if( itkImage.IsNull() )
  {
    return;
  }
  
  typedef itk::ImageToVTKImageFilter< itk::Image<long, 3> > ConverterType;
  ConverterType::Pointer myConverter = ConverterType::New();
  myConverter->SetInput ( itkImage );
  myConverter->Update();

  this->SetImage ( myConverter->GetOutput() );

  this->ImageConverter = myConverter;  
}

void vtkViewImage::SetITKImage (itk::Image<unsigned long, 3>::Pointer itkImage)
{
  if( itkImage.IsNull() )
  {
    return;
  }
  
  typedef itk::ImageToVTKImageFilter< itk::Image<unsigned long, 3> > ConverterType;
  ConverterType::Pointer myConverter = ConverterType::New();
  myConverter->SetInput ( itkImage );
  myConverter->Update();

  this->SetImage ( myConverter->GetOutput() );

  this->ImageConverter = myConverter;  
}


#endif
