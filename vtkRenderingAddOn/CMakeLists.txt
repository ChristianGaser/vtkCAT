
  SET(vtkRenderingAddOn_SRCS
    vtkBoxCutPolyLines.cxx
    vtkFillImageWithPolyData.cxx
    vtkImageBlendWithMask.cxx
    vtkOrientationAnnotation.cxx
    vtkInteractorStyleImage2D.cxx
    vtkImageOrientedPointWidget.cxx
    vtkViewImage2DCommand.cxx
    vtkViewImage2DFullCommand.cxx
    vtkDataSet3DCroppingPlaneCallback.cxx
    vtkImage3DCroppingBoxCallback.cxx
    vtkImage3DImagePlaneCallback.cxx
    vtkImageOrientedPointWidgetCallback.cxx
    vtkImageTracerWidgetCallback.cxx
    vtkSynchronizedView.cxx
    vtkViewImage.cxx
    vtkViewImage2D.cxx
    vtkViewImage3D.cxx
    vtkViewImage2DWithOrientedPoint.cxx
    vtkViewImage2DWithTracer.cxx
  )


  # C++ library for the InterfaceManagers
  ADD_LIBRARY(vtkRenderingAddOn ${vtkINRIA3D_LIBS_STYLE}
  ${vtkRenderingAddOn_SRCS}
  )
  
SET(vtkRenderingAddOn_LIBS
    vtkCommon
    vtkRendering
    vtkWidgets
    vtkVolumeRendering
    vtkHybrid
    vtkImaging
)

IF( USE_ITK )
   SET(vtkRenderingAddOn_LIBS
   ITKCommon
   ITKBasicFilters
   ${vtkRenderingAddOn_LIBS}
   )
ENDIF( USE_ITK )


  TARGET_LINK_LIBRARIES(vtkRenderingAddOn
  ${vtkRenderingAddOn_LIBS}
  )

  IF (VTK_WRAP_TCL)
    SET_SOURCE_FILES_PROPERTIES(
      vtkViewImage2DCommand.cxx
      vtkViewImage2DFullCommand.cxx
      vtkDataSet3DCroppingPlaneCallback.cxx
      vtkImage3DCroppingBoxCallback.cxx
      vtkImage3DImagePlaneCallback.cxx
      vtkImageOrientedPointWidgetCallback.cxx
      vtkImageTracerWidgetCallback.cxx
      WRAP_EXCLUDE
    )
    VTK_WRAP_TCL3(
      vtkRenderingAddOnLib
      vtkRenderingAddOnTCL_SRCS
      "${vtkRenderingAddOn_SRCS}"
      ""
      "${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}"
    )

    ADD_LIBRARY(vtkRenderingAddOnTCL ${vtkINRIA3D_LIBS_STYLE}
    ${vtkRenderingAddOnTCL_SRCS}
    )
    TARGET_LINK_LIBRARIES(vtkRenderingAddOnTCL
    vtkRenderingAddOn
    vtkCommonTCL
    vtkRenderingTCL
    vtkWidgetsTCL
    vtkVolumeRenderingTCL
    vtkHybridTCL
    vtkImagingTCL
    vtkRenderingAddOn
    )
 
    INSTALL_TARGETS(/lib vtkRenderingAddOnTCL)

  ENDIF(VTK_WRAP_TCL)


  INSTALL_TARGETS(/lib vtkRenderingAddOn)

  INSTALL_FILES(/include "(\\.h|\\.txx)$")
