PROJECT (vtkCAT)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

IF(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

SET(vtkCAT_BINARY_DIR .)

SET(vtkCAT_VERSION_MAJOR 0)
SET(vtkCAT_VERSION_MINOR 1)
SET(vtkCAT_VERSION_BUILD 0)
SET(vtkCAT_VERSION "${vtkCAT_VERSION_MAJOR}.${vtkCAT_VERSION_MINOR}.${vtkCAT_VERSION_BUILD}")

#SUBDIRS(vtkRenderingAddOn)

SET(vtkCAT_SRCS
    lib/vtkMINCImageAttributes.cxx
    lib/vtkMINCImageReader.cxx
    lib/vtkMINCImageWriter.cxx
    lib/vtkCaretTgtSmoothPolyDataFilter.cxx
    lib/vtkEllipsoidProjectPolyDataFilter.cxx
    lib/vtkPolyDataCurvature.cxx
    lib/vtkPolyDataDistortion.cxx
    lib/vtkUtilities.cxx
    lib/vtkPointTopologyHelper.cxx
    lib/vtkAnalyzeRawReader.cxx
    lib/vtkAnalyzeImageReader.cxx
    lib/vtkBICOBJReader.cxx
    lib/vtkSurfaceReader.cxx
    lib/vtkFreesurferReader.cxx
    lib/vtkInflateSmoothPolyData.cxx
    lib/vtkInteractorStyleCAT.cxx
    lib/vtkScalarBarWidgetCAT.cxx
)

SET(vtkRenderingAddOn_SRCS
    vtkRenderingAddOn/vtkBoxCutPolyLines.cxx
    vtkRenderingAddOn/vtkFillImageWithPolyData.cxx
    vtkRenderingAddOn/vtkImageBlendWithMask.cxx
    vtkRenderingAddOn/vtkOrientationAnnotation.cxx
    vtkRenderingAddOn/vtkInteractorStyleImage2D.cxx
    vtkRenderingAddOn/vtkImageOrientedPointWidget.cxx
    vtkRenderingAddOn/vtkViewImage2DCommand.cxx
    vtkRenderingAddOn/vtkViewImage2DFullCommand.cxx
    vtkRenderingAddOn/vtkDataSet3DCroppingPlaneCallback.cxx
    vtkRenderingAddOn/vtkImage3DCroppingBoxCallback.cxx
    vtkRenderingAddOn/vtkImage3DImagePlaneCallback.cxx
    vtkRenderingAddOn/vtkImageOrientedPointWidgetCallback.cxx
    vtkRenderingAddOn/vtkImageTracerWidgetCallback.cxx
    vtkRenderingAddOn/vtkSynchronizedView.cxx
    vtkRenderingAddOn/vtkViewImage.cxx
    vtkRenderingAddOn/vtkViewImage2D.cxx
    vtkRenderingAddOn/vtkViewImage3D.cxx
    vtkRenderingAddOn/vtkViewImage2DWithOrientedPoint.cxx
    vtkRenderingAddOn/vtkViewImage2DWithTracer.cxx
)

ADD_DEFINITIONS(-DUSE_NAMESPACE_STD)

INCLUDE (${CMAKE_ROOT}/Modules/FindVTK.cmake)

FIND_PACKAGE(VTK REQUIRED)
IF (USE_VTK_FILE)
  INCLUDE (${USE_VTK_FILE})
  
  INCLUDE_DIRECTORIES(
    ${vtkCAT_BINARY_DIR}/lib
    ${vtkCAT_BINARY_DIR}/vtkRenderingAddOn
  )

  LINK_LIBRARIES (
    vtkImaging
    vtkGraphics
    vtkIO
    vtkFiltering
    vtkCommon
    vtkRendering
    vtkNetCDF
    vtkWidgets
    vtkCAT
    )

SET(vtkRenderingAddOn_LIBS
    vtkCommon
    vtkRendering
    vtkWidgets
    vtkVolumeRendering
    vtkHybrid
    vtkImaging
	vtkRenderingAddOn
)

  ADD_LIBRARY(vtkCAT STATIC ${vtkCAT_SRCS})
  ADD_LIBRARY(vtkRenderingAddOn STATIC ${vtkRenderingAddOn_SRCS})

  TARGET_LINK_LIBRARIES(vtkCAT ${vtkRenderingAddOn_LIBS})

  ADD_EXECUTABLE(CAT_MarchingCubes CAT_MarchingCubes.cxx)  
  ADD_EXECUTABLE(CAT_SurfSmooth CAT_SurfSmooth.cxx)  
  IF(APPLE)
    ADD_EXECUTABLE(CAT_View MACOSX_BUNDLE CAT_View.cxx)  
    ADD_EXECUTABLE(CAT_Image MACOSX_BUNDLE CAT_Image.cxx)  
    ADD_CUSTOM_COMMAND(
                      TARGET CAT_View
                      PRE_LINK
                      COMMAND mkdir -p CAT_View.app/Contents/MacOS 
                      COMMENT "Creating CAT_View.app")
    ADD_CUSTOM_COMMAND(
                      TARGET CAT_Image
                      PRE_LINK
                      COMMAND mkdir -p CAT_Image.app/Contents/MacOS 
                      COMMENT "Creating CAT_Image.app")
  ELSE(APPLE)
    ADD_EXECUTABLE(CAT_View CAT_View.cxx)  
    ADD_EXECUTABLE(CAT_Image CAT_Image.cxx)  
  ENDIF(APPLE)
  ADD_EXECUTABLE(CAT_SurfInflate CAT_SurfInflate.cxx)  

  IF(APPLE)
    INSTALL(TARGETS CAT_View RUNTIME DESTINATION /Applications BUNDLE DESTINATION /Applications)
    INSTALL(TARGETS CAT_Image RUNTIME DESTINATION /Applications BUNDLE DESTINATION /Applications)
  ENDIF(APPLE)

  INSTALL(PROGRAMS CAT_View DESTINATION bin)
  INSTALL(PROGRAMS CAT_Image DESTINATION bin)
  INSTALL(PROGRAMS mgz2mnc DESTINATION bin)

  INSTALL(TARGETS CAT_SurfInflate CAT_SurfSmooth CAT_MarchingCubes
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib)
    
ENDIF (USE_VTK_FILE)
