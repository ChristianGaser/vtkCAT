#ifndef _vtk_INRIA3D_Configure_h_
#define _vtk_INRIA3D_Configure_h_

/* Whether we are building shared libraries.  */
/* #undef VTK_BUILD_SHARED_LIBS */

/* #undef wxWidgets_BUILD_SHARED_LIBS */



#if defined (WIN32) && defined (VTK_BUILD_SHARED_LIBS)

 #if defined (vtkVisuManagement_EXPORTS)
  #define VTK_VISUMANAGEMENT_EXPORT __declspec( dllexport )
 #else
  #define VTK_VISUMANAGEMENT_EXPORT __declspec( dllimport )
 #endif
 
 #if defined (vtkHWShading_EXPORTS)
  #define VTK_HWSHADING_EXPORT __declspec( dllexport )
 #else
  #define VTK_HWSHADING_EXPORT __declspec( dllimport )
 #endif
 
 #if defined (vtkHelpers_EXPORTS)
  #define VTK_HELPERS_EXPORT __declspec( dllexport )
 #else
  #define VTK_HELPERS_EXPORT __declspec( dllimport )
 #endif
 
 #if defined (vtkDataManagement_EXPORTS)
  #define VTK_DATAMANAGEMENT_EXPORT __declspec( dllexport )
 #else
  #define VTK_DATAMANAGEMENT_EXPORT __declspec( dllimport )
 #endif
 
 
 #if defined (vtkRenderingAddOn_EXPORTS)
  #define VTK_RENDERINGADDON_EXPORT __declspec( dllexport )
 #else
  #define VTK_RENDERINGADDON_EXPORT __declspec( dllimport )
 #endif

 #if defined (KWAddOn_EXPORTS)
  #define KW_ADDON_EXPORT __declspec( dllexport )
 #else
  #define KW_ADDON_EXPORT __declspec( dllimport )
 #endif

#else

  #define VTK_VISUMANAGEMENT_EXPORT
  #define VTK_HWSHADING_EXPORT
  #define VTK_HELPERS_EXPORT
  #define VTK_DATAMANAGEMENT_EXPORT
  #define VTK_RENDERINGADDON_EXPORT
  #define KW_ADDON_EXPORT

#endif




#if defined (WIN32) && defined (wxWidgets_BUILD_SHARED_LIBS)

 #if defined (wxAddOn_EXPORTS)
  #define WX_ADDON_EXPORT __declspec( dllexport )
 #else
  #define WX_ADDON_EXPORT __declspec( dllimport )
 #endif
 
 
 #if defined (wxVtk_EXPORTS)
 #    define WX_VTK_EXPORT __declspec(dllexport)
 #else
 #    define WX_VTK_EXPORT __declspec(dllimport)
 #endif
 
 
 #if defined (ImageViewer_EXPORTS)
 #    define IMAGEVIEWER_EXPORT __declspec(dllexport)
 #else
 #    define IMAGEVIEWER_EXPORT __declspec(dllimport)
 #endif
 
 
 #if defined (HelloWorldLib_EXPORTS)
 #    define HELLOWORLDLIB_EXPORT __declspec(dllexport)
 #else
 #    define HELLOWORLDLIB_EXPORT __declspec(dllimport)
 #endif

#else

 #define WX_ADDON_EXPORT
 #define WX_VTK_EXPORT
 #define IMAGEVIEWER_EXPORT
 #define HELLOWORLDLIB_EXPORT

#endif



#endif
