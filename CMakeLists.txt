
cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(CAT_View)

# Specify the C++ standard
#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED True)

SET(gifticlib_SRCS
    gifticlib/gifti_io.c
    gifticlib/gifti_tool.c
    gifticlib/gifti_xml.c
)

ADD_LIBRARY(gifticlib STATIC ${gifticlib_SRCS})

SET(gifticlib_LIBS
  gifticlib
)

find_package(VTK COMPONENTS 
  CommonColor
  CommonCore
  IOLegacy
  IOImage
  InteractionStyle
  RenderingAnnotation
  RenderingContextOpenGL2
  RenderingCore
  RenderingFreeType
  RenderingGL2PSOpenGL2
  RenderingOpenGL2
)

find_package(expat REQUIRED)

TARGET_LINK_LIBRARIES(${gifticlib_LIBS})

if (NOT VTK_FOUND)
  message(FATAL_ERROR "CAT_View: Unable to find the VTK build folder.")
endif()

# Prevent a "command line is too long" failure in Windows.
set(CMAKE_NINJA_FORCE_RESPONSE_FILE "ON" CACHE BOOL "Force Ninja to use response files.")

add_executable(CAT_View MACOSX_BUNDLE CAT_VIEW.cxx vtkSurfaceReader.cxx)

# vtk_module_autoinit is needed
vtk_module_autoinit(
  TARGETS CAT_View
  MODULES ${VTK_LIBRARIES}
)

# If you have include directories for your application, specify them
target_include_directories(CAT_View PRIVATE ${VTK_INCLUDE_DIRS} gifticlib)

TARGET_LINK_LIBRARIES(CAT_View PRIVATE ${VTK_LIBRARIES} gifticlib expat)
