unix{
  QMAKE_CXXFLAGS_WARN_ON += -Wno-deprecated
  QMAKE_CFLAGS_DEBUG = -g -O0
}

LANGUAGE = C++
TEMPLATE = app
CONFIG += release \
	warn_on \

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

LIBS += -L${VTK_LIB_DIR} \
	 -lQVTK \
	 -lvtkWidgets \
	 -lvtkRendering \
	 -lvtkGraphics \
	 -lvtkImaging \
	 -lvtkIO \
	 -lvtkFiltering \
	 -lvtkCommon \
	 -lvtkpng \
	 -lvtktiff \
	 -lvtkzlib \
	 -lvtkjpeg \
	 -lvtkexpat \
	 -lvtkftgl \
	 -lvtkfreetype \
	 -lvtksys \
	 -ldl \
	 -framework OpenGL \
	 -framework AGL \
	 -lm \
	 -L../ \
     -lvtkCAT
	 
INCLUDEPATH += ${VTK_INC_DIR} \
			../vtkCAT

unix:target.path = /usr/local/bin 
macx:target.path = /Applications
 
INSTALLS += target

SOURCES = qt_SurfView.cxx

