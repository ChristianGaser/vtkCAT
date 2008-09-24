/*=========================================================================

Program:   vtkINRIA3D
Module:    $Id: vtkBoxCutPolyLines.h 608 2008-01-14 08:21:23Z filus $
Language:  C++
Author:    $Author: filus $
Date:      $Date: 2008-01-14 09:21:23 +0100 (Mo, 14 Jan 2008) $
Version:   $Revision: 608 $

Copyright (c) 2007 INRIA - Asclepios Project. All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _vtk_BoxCutPolyLines_h_
#define _vtk_BoxCutPolyLines_h_

#include "vtkINRIA3DConfigure.h"

#include "vtkPolyDataAlgorithm.h"
#include <vector>

class VTK_RENDERINGADDON_EXPORT vtkBoxCutPolyLines: public vtkPolyDataAlgorithm
{

 public:
  static vtkBoxCutPolyLines *New();
  vtkTypeRevisionMacro(vtkBoxCutPolyLines, vtkPolyDataAlgorithm);
  //vtkStandardNewMacro(vtkBoxCutPolyLines);
  //{ return new vtkBoxCutPolyLines; }

  void SetBoxClip(double xmin,double xmax,
		  double ymin,double ymax,
		  double zmin,double zmax);
  
  void PrintSelf (ostream& os, vtkIndent indent){};
  
 protected:
  vtkBoxCutPolyLines();
  ~vtkBoxCutPolyLines(){};
  
  // Usual data generation method
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  

 private:
  vtkBoxCutPolyLines (const vtkBoxCutPolyLines&);
  void operator=(const vtkBoxCutPolyLines&);

  double XMin;
  double XMax;
  double YMin;
  double YMax;
  double ZMin;
  double ZMax;
  
  
};



#endif
