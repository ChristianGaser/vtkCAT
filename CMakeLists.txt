PROJECT (vtkCAT)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

IF(COMMAND cmake_policy)
  cmake_policy(VERSION 2.4)
  cmake_policy(SET CMP0005 OLD)
  cmake_policy(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

SET(vtkCAT_BINARY_DIR .)

SET(vtkCAT_VERSION_MAJOR 0)
SET(vtkCAT_VERSION_MINOR 1)
SET(vtkCAT_VERSION_BUILD 0)
SET(vtkCAT_VERSION "${vtkCAT_VERSION_MAJOR}.${vtkCAT_VERSION_MINOR}.${vtkCAT_VERSION_BUILD}")

#--------------
# ITK
#--------------
OPTION(USE_ITK "If ON, Compile ITK extensions" ON)
IF(USE_ITK)
  FIND_PACKAGE(ITK)
  IF(ITK_FOUND)
   INCLUDE(${ITK_USE_FILE})		
    SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -DvtkCAT_USE_ITK -DvtkINRIA3D_USE_ITK")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DvtkCAT_USE_ITK -DvtkINRIA3D_USE_ITK")
  ELSE(ITK_FOUND)
	MESSAGE(FATAL_ERROR "ITK not found. Please set ITK_DIR.")
  ENDIF(ITK_FOUND)
  
  IF( ITK_BUILD_SHARED )
    SET( ITK_LIBS_STYLE "SHARED" )
  ELSE( ITK_BUILD_SHARED )
    SET( ITK_LIBS_STYLE "STATIC" )
  ENDIF( ITK_BUILD_SHARED )
	
ELSE(USE_ITK)
  SET(ITK_FOUND OFF)
ENDIF(USE_ITK)

SET(vtkCAT_SRCS
    vtkCAT/vtkCaretTgtSmoothPolyDataFilter.cxx
    vtkCAT/vtkEllipsoidProjectPolyDataFilter.cxx
    vtkCAT/vtkPolyDataCurvature.cxx
    vtkCAT/vtkPolyDataDistortion.cxx
    vtkCAT/vtkUtilities.cxx
    vtkCAT/vtkPointTopologyHelper.cxx
    vtkCAT/vtkBICOBJReader.cxx
    vtkCAT/vtkSurfaceReader.cxx
    vtkCAT/vtkFreesurferReader.cxx
    vtkCAT/vtkInflateSmoothPolyData.cxx
    vtkCAT/vtkInteractorStyleCAT.cxx
    vtkCAT/vtkScalarBarWidgetCAT.cxx
    vtkCAT/vtkMINCImageReader.cxx
    vtkCAT/vtkMINCImageAttributes.cxx
    vtkCAT/vtkMetaImageData.cxx
    vtkCAT/vtkMetaDataSet.cxx
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

#--------------
# VTK
#--------------
FIND_PACKAGE(VTK REQUIRED)
IF (USE_VTK_FILE)
  INCLUDE (${USE_VTK_FILE})
  
  INCLUDE_DIRECTORIES(
    ${vtkCAT_BINARY_DIR}/vtkCAT
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
    vtkVolumeRendering
    vtkHybrid
  )

  SET(vtkRenderingAddOn_LIBS
	vtkRenderingAddOn
  )

  SET(ITK_LIBS
	${ITK_LIBRARIES}
  )

  ADD_LIBRARY(vtkCAT STATIC ${vtkCAT_SRCS})
  ADD_LIBRARY(vtkRenderingAddOn STATIC ${vtkRenderingAddOn_SRCS})

  IF(USE_ITK)
    TARGET_LINK_LIBRARIES(vtkCAT ${ITK_LIBS} ${vtkRenderingAddOn_LIBS})
  ELSE(USE_ITK)
    TARGET_LINK_LIBRARIES(vtkCAT ${vtkRenderingAddOn_LIBS})
  ENDIF(USE_ITK)
  
  ADD_EXECUTABLE(CAT_MarchingCubes CAT_MarchingCubes.cxx)  
  ADD_EXECUTABLE(CAT_SurfSmooth CAT_SurfSmooth.cxx)  

  IF(APPLE)
    ADD_EXECUTABLE(CAT_View MACOSX_BUNDLE CAT_View.cxx)  
    ADD_EXECUTABLE(CAT_Image MACOSX_BUNDLE CAT_Image.cxx)  
    ADD_CUSTOM_COMMAND(
                      TARGET CAT_Image
                      PRE_LINK
                      COMMAND mkdir -p CAT_Image.app/Contents/MacOS 
                      COMMENT "Creating CAT_Image.app")
    ADD_CUSTOM_COMMAND(
                      TARGET CAT_View
                      PRE_LINK
                      COMMAND mkdir -p CAT_View.app/Contents/MacOS 
                      COMMENT "Creating CAT_View.app")
  ELSE(APPLE)
    ADD_EXECUTABLE(CAT_View CAT_View.cxx)  
    ADD_EXECUTABLE(CAT_Image CAT_Image.cxx)  
  ENDIF(APPLE)
  ADD_EXECUTABLE(CAT_SurfInflate CAT_SurfInflate.cxx)  

  IF(APPLE)
    INSTALL(TARGETS CAT_View RUNTIME DESTINATION /Applications BUNDLE DESTINATION /Applications)
    INSTALL(TARGETS CAT_Image RUNTIME DESTINATION /Applications BUNDLE DESTINATION /Applications)
  ELSE(APPLE)
    INSTALL(PROGRAMS CAT_View DESTINATION bin)
    INSTALL(PROGRAMS CAT_Image DESTINATION bin)
  ENDIF(APPLE)
  INSTALL(PROGRAMS mgz2mnc DESTINATION bin)

  INSTALL(TARGETS CAT_SurfInflate CAT_SurfSmooth CAT_MarchingCubes
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib)
    
ENDIF (USE_VTK_FILE)

#-----------------------------------------------------------------------------
# doxygen documentation

FIND_PACKAGE(Doxygen)
IF (DOXYGEN)
  SET(MY_DOXYGEN_BUILD_DIR ${PROJECT_BINARY_DIR}/Doxygen)

  CONFIGURE_FILE (
    ${PROJECT_SOURCE_DIR}/Doxygen/doxygen.config.in
    ${MY_DOXYGEN_BUILD_DIR}/doxygen.config
  )

  ADD_CUSTOM_TARGET(doc
    ${DOXYGEN}
    ${MY_DOXYGEN_BUILD_DIR}/doxygen.config
  )
ENDIF(DOXYGEN) 

