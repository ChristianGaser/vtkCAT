PROJECT (vtkCAT)

SET(vtkCAT_BINARY_DIR .)

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
lib/vtkVolumeRawReader.cxx
lib/vtkVolumeAnalyzeReader.cxx
lib/vtkBICOBJReader.cxx
lib/vtkFreesurferReader.cxx
lib/vtkInflateSmoothPolyData.cxx
lib/vtkCorrectTopologyPolyData.cxx
lib/vtkInteractorStyleCAT.cxx
)

SET(vtkTopology_SRCS
lib/topologysimplification/Face.cpp
lib/topologysimplification/Loop.cpp
lib/topologysimplification/PatchDisk.cpp
lib/topologysimplification/SurfaceVtk.cpp
lib/topologysimplification/FastLoop.cpp
lib/topologysimplification/Segment.cpp
lib/topologysimplification/Vertex.cpp
lib/topologysimplification/utils.cpp
)

ADD_DEFINITIONS(-DUSE_NAMESPACE_STD)

INCLUDE (${CMAKE_ROOT}/Modules/FindVTK.cmake)

FIND_PACKAGE(VTK REQUIRED)
IF (USE_VTK_FILE)
  INCLUDE (${USE_VTK_FILE})
  INCLUDE_DIRECTORIES(${vtkCAT_BINARY_DIR}/lib)

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

  ADD_LIBRARY(vtkCAT STATIC ${vtkCAT_SRCS} ${vtkTopology_SRCS})
  TARGET_LINK_LIBRARIES(vtkCAT)

  ADD_EXECUTABLE(CAT_MarchingCubes CAT_MarchingCubes.cxx)  
  ADD_EXECUTABLE(CAT_SurfSmooth CAT_SurfSmooth.cxx)  
  IF(APPLE)
    ADD_EXECUTABLE(CAT_View MACOSX_BUNDLE CAT_View.cxx)  
    ADD_CUSTOM_COMMAND(
                      TARGET CAT_View
                      PRE_LINK
                      COMMAND mkdir -p CAT_View.app/Contents/MacOS 
                      COMMENT "Creating CAT_View.app")
  ELSE(APPLE)
    ADD_EXECUTABLE(CAT_View CAT_View.cxx)  
  ENDIF(APPLE)
  ADD_EXECUTABLE(CAT_SurfInflate CAT_SurfInflate.cxx)  
  ADD_EXECUTABLE(CAT_CorrectTopology CAT_CorrectTopology.cxx)  

  IF(APPLE)
    INSTALL(TARGETS CAT_View RUNTIME DESTINATION /Applications)
  ENDIF(APPLE)

  INSTALL(PROGRAMS CAT_View DESTINATION bin)
  INSTALL(TARGETS CAT_SurfInflate CAT_SurfSmooth CAT_MarchingCubes CAT_CorrectTopology
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib)
    
ENDIF (USE_VTK_FILE)
