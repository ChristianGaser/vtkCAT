
cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(vtkCAT)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

ADD_DEFINITIONS(-DHAVE_ZLIB)

SET(gifticlib_SRCS
    gifticlib/gifti_io.c
    gifticlib/gifti_tool.c
    gifticlib/gifti_xml.c
)

SET(zlib_SRCS
    zlib/adler32.c
    zlib/crc32.c
    zlib/gzio.c
    zlib/inffast.c
    zlib/inftrees.c
    zlib/trees.c
    zlib/zutil.c
    zlib/compress.c
    zlib/deflate.c
    zlib/infback.c
    zlib/inflate.c
    zlib/uncompr.c
)
ADD_LIBRARY(gifticlib STATIC ${gifticlib_SRCS})
ADD_LIBRARY(zlib STATIC ${zlib_SRCS})

SET(gifticlib_LIBS
  gifticlib
)

SET(zlib_LIBS
  zlib
)

find_package(VTK REQUIRED COMPONENTS
  InteractionStyle
  RenderingAnnotation
  RenderingContextOpenGL2
  RenderingGL2PSOpenGL2
  RenderingOpenGL2
)

TARGET_LINK_LIBRARIES(${gifticlib_LIBS} ${zlib_LIBS})

if (NOT VTK_FOUND)
  message(FATAL_ERROR "vtkCAT: Unable to find the VTK build folder.")
endif()

# Prevent a "command line is too long" failure in Windows.
set(CMAKE_NINJA_FORCE_RESPONSE_FILE "ON" CACHE BOOL "Force Ninja to use response files.")

add_executable(CAT_ViewSurf CAT_ViewSurf.cxx vtkSurfaceReader.cxx vtkStat.cxx vtkUtils.cxx)
add_executable(CAT_ViewImage CAT_ViewImage.cxx vtkSurfaceReader.cxx)

# vtk_module_autoinit is needed
vtk_module_autoinit(
  TARGETS CAT_ViewSurf CAT_ViewImage
  MODULES ${VTK_LIBRARIES}
)

# If you have include directories for your application, specify them
target_include_directories(CAT_ViewSurf PRIVATE ${VTK_INCLUDE_DIRS} gifticlib zlib)
target_include_directories(CAT_ViewImage PRIVATE ${VTK_INCLUDE_DIRS} gifticlib zlib)

TARGET_LINK_LIBRARIES(CAT_ViewSurf PRIVATE ${VTK_LIBRARIES} gifticlib expat zlib)
TARGET_LINK_LIBRARIES(CAT_ViewImage PRIVATE ${VTK_LIBRARIES} gifticlib expat zlib)
