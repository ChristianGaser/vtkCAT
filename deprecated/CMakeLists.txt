cmake_minimum_required(VERSION 3.21)

PROJECT (vtkCAT)

IF(COMMAND cmake_policy)
  cmake_policy(VERSION 3.6)
  cmake_policy(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

SET(vtkCAT_BINARY_DIR .)

SET(vtkCAT_VERSION_MAJOR 0)
SET(vtkCAT_VERSION_MINOR 1)
SET(vtkCAT_VERSION_BUILD 0)
SET(vtkCAT_VERSION "${vtkCAT_VERSION_MAJOR}.${vtkCAT_VERSION_MINOR}.${vtkCAT_VERSION_BUILD}")

SET(VTK_USE_QVTK 0)

#SUBDIRS(test)

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

FIND_PACKAGE(Qt6 REQUIRED)

SET(vtkCAT_SRCS
#    vtkCAT/vtkCaretTgtSmoothPolyDataFilter.cxx
#    vtkCAT/vtkEllipsoidProjectPolyDataFilter.cxx
#    vtkCAT/vtkPolyDataCurvature.cxx
#    vtkCAT/vtkPolyDataDistortion.cxx
#    vtkCAT/vtkUtilities.cxx
#    vtkCAT/vtkPointTopologyHelper.cxx
    vtkCAT/vtkBICOBJReader.cxx
    vtkCAT/vtkSurfaceReader.cxx
    vtkCAT/vtkGiftiReader.cxx
    vtkCAT/vtkFreesurferReader.cxx
#    vtkCAT/vtkInflateSmoothPolyData.cxx
    vtkCAT/vtkInteractorStyleCAT.cxx
    vtkCAT/vtkScalarBarWidgetCAT.cxx
#    vtkCAT/vtkMINCImageReader.cxx
#    vtkCAT/vtkMINCImageAttributes.cxx
    vtkCAT/vtkMetaImageData.cxx
    vtkCAT/vtkMetaDataSet.cxx
)

SET(gifticlib_SRCS
    gifticlib/gifti_io.c
    gifticlib/gifti_tool.c
    gifticlib/gifti_xml.c
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

#INCLUDE (${CMAKE_ROOT}/Modules/FindVTK.cmake)

#--------------
# VTK
#--------------
FIND_PACKAGE(VTK REQUIRED)

IF (VTK_USE_FILE)
  ADD_LIBRARY(vtkCAT STATIC ${vtkCAT_SRCS} ${gifticlib_SRCS})
  ADD_LIBRARY(vtkRenderingAddOn STATIC ${vtkRenderingAddOn_SRCS})
  
  INCLUDE (${VTK_USE_FILE})
  
  INCLUDE_DIRECTORIES(
    ${vtkCAT_BINARY_DIR}/gifticlib
    ${vtkCAT_BINARY_DIR}/vtkCAT
    ${vtkCAT_BINARY_DIR}/vtkRenderingAddOn
  )

  LINK_LIBRARIES (
#    vtkImaging
#    vtkGraphics
#    vtkIO
#    vtkexpat
    vtklibxml2
#    vtkzlib
#    vtkFiltering
#    vtkCommon
#    vtkRendering
#    vtkNetCDF
    vtkWidgets
    vtkVolumeRendering
    vtkHybrid
    vtkCAT
    ${QT_LIBRARIES}
    expat
  )

  SET(vtkRenderingAddOn_LIBS
	vtkRenderingAddOn
  )

  SET(ITK_LIBS
	${ITK_LIBRARIES}
  )


  IF(USE_ITK)
    TARGET_LINK_LIBRARIES(vtkCAT ${ITK_LIBS} ${vtkRenderingAddOn_LIBS})
  ELSE(USE_ITK)
    TARGET_LINK_LIBRARIES(vtkCAT ${vtkRenderingAddOn_LIBS})
  ENDIF(USE_ITK)
  
#  ADD_EXECUTABLE(CAT_MarchingCubes CAT_MarchingCubes.cxx)  
#  ADD_EXECUTABLE(CAT_SurfSmooth CAT_SurfSmooth.cxx)  
#  ADD_EXECUTABLE(CAT_SphereDemons CAT_SphereDemons.cxx)  

  IF(APPLE)
    ADD_EXECUTABLE(CAT_View MACOSX_BUNDLE CAT_View.cxx)  
    ADD_EXECUTABLE(CAT_Image MACOSX_BUNDLE CAT_Image.cxx)  
    IF(VTK_USE_QVTK)
      ADD_EXECUTABLE(CAT_ViewQT MACOSX_BUNDLE CAT_ViewQT.cxx)  
    ENDIF(VTK_USE_QVTK)
    ADD_CUSTOM_COMMAND(
                      TARGET CAT_Image
                      PRE_LINK
                      COMMAND mkdir -p CAT_Image.app/Contents/MacOS 
#                      COMMAND ln -sf /Applications/CAT_Image.app/Contents/MacOS/CAT_Image /usr/local/bin/CAT_Image
                      COMMENT "Creating CAT_Image.app")
    IF(VTK_USE_QVTK)
      ADD_CUSTOM_COMMAND(
                      TARGET CAT_ViewQT
                      PRE_LINK
                      COMMAND mkdir -p CAT_ViewQT.app/Contents/MacOS
#                      COMMAND ln -sf /Applications/CAT_ViewQT.app/Contents/MacOS/CAT_ViewQT /usr/local/bin/CAT_ViewQT
                      COMMENT "Creating CAT_ViewQT.app")
    ENDIF(VTK_USE_QVTK)
    ADD_CUSTOM_COMMAND(
                      TARGET CAT_View
                      PRE_LINK
                      COMMAND mkdir -p CAT_View.app/Contents/MacOS 
#                      COMMAND ln -sf /Applications/CAT_View.app/Contents/MacOS/CAT_View /usr/local/bin/CAT_View
                      COMMENT "Creating CAT_View.app")
  ELSE(APPLE)
    ADD_EXECUTABLE(CAT_View CAT_View.cxx)  
    ADD_EXECUTABLE(CAT_Image CAT_Image.cxx)  
    IF(VTK_USE_QVTK)
      ADD_EXECUTABLE(CAT_ViewQT CAT_ViewQT.cxx)  
    ENDIF(VTK_USE_QVTK)
  ENDIF(APPLE)
  #ADD_EXECUTABLE(CAT_SurfInflate CAT_SurfInflate.cxx)  


  IF(APPLE)
    INSTALL(TARGETS CAT_View RUNTIME DESTINATION /Applications BUNDLE DESTINATION /Applications)
    INSTALL(TARGETS CAT_Image RUNTIME DESTINATION /Applications BUNDLE DESTINATION /Applications)
    IF(VTK_USE_QVTK)
      INSTALL(TARGETS CAT_ViewQT RUNTIME DESTINATION /Applications BUNDLE DESTINATION /Applications)
    ENDIF(VTK_USE_QVTK)
  ELSE(APPLE)
    INSTALL(PROGRAMS CAT_View DESTINATION bin)
    INSTALL(PROGRAMS CAT_Image DESTINATION bin)
    IF(VTK_USE_QVTK)
      INSTALL(PROGRAMS CAT_ViewQT DESTINATION bin)
    ENDIF(VTK_USE_QVTK)
  ENDIF(APPLE)
  INSTALL(PROGRAMS mgz2mnc DESTINATION bin)

  #INSTALL(TARGETS CAT_SurfInflate CAT_SurfSmooth
  #  RUNTIME DESTINATION bin
  #  LIBRARY DESTINATION lib)
    
ENDIF (VTK_USE_FILE)

#-----------------------------------------------------------------------------
# doxygen documentation

#FIND_PACKAGE(Doxygen)
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

