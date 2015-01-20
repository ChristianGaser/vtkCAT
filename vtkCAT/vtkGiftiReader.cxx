/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSthis->FileName: vtkGiftiReader.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkGiftiReader.h"

#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkProperty.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkByteSwap.h"
#include "vtkUnsignedCharArray.h"

extern "C" {
  #include "../gifticlib/gifti_io.h"
}

vtkCxxRevisionMacro(vtkGiftiReader, "$Revision: 36 $");
vtkStandardNewMacro(vtkGiftiReader);

static giiDataArray* 
gifti_alloc_and_add_darray (gifti_image* image)
{
        if (!image) {
                fprintf (stderr,"** gifti_alloc_and_add_darray: NULL image\n");
                return NULL;
        }

        /* Try to add an empty array. */
        if (gifti_add_empty_darray(image,1)) {
                fprintf (stderr,"** gifti_alloc_and_add_darray: gifti_add_empty_darray "
                                                 "failed\n");
                return NULL;
        }

        /* Return the array we just allocated. */
        return image->darray[image->numDA-1];
}

static double 
gifti_get_DA_value_2D (giiDataArray* da, int row, int col)
{
        int dim0_index, dim1_index;
        int dims_0=0, dims_1=0;

        if (!da || !da->data) {
                fprintf (stderr,"** gifti_get_DA_value_2D, invalid params: data=%p\n", da);
                exit(1);
        }

        if (da->num_dim == 1) {
                // support for using this routine to read 1D data, under one condition...
                if (col != 0)
                {
                        fprintf (stderr,"** gifti_get_DA_value_2D, array dim is 1 "
                                "but trying to access 2D data element (col=%d)\n",col);
                        exit(1);
                }
                dims_0 = da->dims[0];
                dims_1 = 1; // 1D data
        }
        else if (da->num_dim != 2) {
                fprintf (stderr,"** gifti_get_DA_value_2D, array dim is %d\n", da->num_dim);
                exit(1);
        }
        else {
                dims_0 = da->dims[0];
                dims_1 = da->dims[1];
        }

        /* Get the dim0 and dims[1] indices based on our order. */
        if (GIFTI_IND_ORD_ROW_MAJOR == da->ind_ord) {
                dim0_index = row;
                dim1_index = col;
        }
        else if (GIFTI_IND_ORD_COL_MAJOR == da->ind_ord) {
                // NJS NOTE: notice that order is treated as row/col, so that the
                // calling sequence can just assume row major
                dim0_index = row;//col;
                dim1_index = col;//row;
        }
        else {
                fprintf (stderr,"** gifti_get_DA_value_2D, unknown ind_ord: %d\n", da->ind_ord);
                exit(1);
        }
        if (da->num_dim == 1) /* support for using this routine to read 1D data */ {
                dim0_index = row;
                dim1_index = col;
        }

        /* Check the indices. */
        if (dim0_index < 0 || dim0_index >= dims_0 || dim1_index < 0 || dim1_index >= dims_1) {
                fprintf(stderr,"** gifti_get_DA_value_2D, invalid params: "
                        "dim0_index=%d (max=%d), dim1_index=%d (max=%d)\n",
                        dim0_index, dims_0, dim1_index, dims_1);
                exit(1);
        }

        /* Switch on the data type and return the appropriate
                 element. Indexing depends on the data order. */
        switch (da->datatype) {
        default :
                fprintf(stderr,"** gifti_get_DA_value_2D, unsupported type %d-"
                        "unknown, or can't convert to double\n",da->datatype);
                exit(1);
        case NIFTI_TYPE_UINT8: {
                if ( GIFTI_IND_ORD_ROW_MAJOR == da->ind_ord )
                        return (double)*((unsigned char*) (da->data) + (dim0_index*dims_1) + dim1_index);
                else
                        return (double)*((unsigned char*)
                        (da->data) + dim0_index + (dim1_index*dims_0));
                break;
        }
        case NIFTI_TYPE_INT16: {
                if ( GIFTI_IND_ORD_ROW_MAJOR == da->ind_ord )
                        return (double)*((short*) (da->data) + (dim0_index*dims_1) + dim1_index);
                else
                        return (double)*((short*) (da->data) + dim0_index + (dim1_index*dims_0));
                break;
        }
        case NIFTI_TYPE_INT32: {
                if ( GIFTI_IND_ORD_ROW_MAJOR == da->ind_ord )
                        return (double)*((int*) (da->data) + (dim0_index*dims_1) + dim1_index);
                else
                        return (double)*((int*) (da->data) + dim0_index + (dim1_index*dims_0));
                break;
        }
        case NIFTI_TYPE_FLOAT32: {
                if ( GIFTI_IND_ORD_ROW_MAJOR == da->ind_ord )
                        return (double)*((float*) (da->data) + (dim0_index*dims_1) + dim1_index);
                else
                        return (double)*((float*) (da->data) + dim0_index + (dim1_index*dims_0));
                break;
        }
        case NIFTI_TYPE_INT8: {
                if ( GIFTI_IND_ORD_ROW_MAJOR == da->ind_ord )
                        return (double)*((char*) (da->data) + (dim0_index*dims_1) + dim1_index);
                else
                        return (double)*((char*) (da->data) + dim0_index + (dim1_index*dims_0));
                break;
        }
        case NIFTI_TYPE_UINT16: {
                if ( GIFTI_IND_ORD_ROW_MAJOR == da->ind_ord )
                        return (double)*((unsigned short*) (da->data) + (dim0_index*dims_1) + dim1_index);
                else
                        return (double)*((unsigned short*) (da->data) + dim0_index + (dim1_index*dims_0));
                break;
        }
        case NIFTI_TYPE_UINT32: {
                if ( GIFTI_IND_ORD_ROW_MAJOR == da->ind_ord )
                        return (double)*((unsigned int*) (da->data) + (dim0_index*dims_1) + dim1_index);
                else
                        return (double)*((unsigned int*) (da->data) + dim0_index + (dim1_index*dims_0));
                break;
        }
        }

        exit(1);
}


// Description:
// Instantiate object with NULL this->FileNamename.
vtkGiftiReader::vtkGiftiReader()
{
  this->FileName = NULL;
  this->SetNumberOfInputPorts(0);
}

vtkGiftiReader::~vtkGiftiReader()
{
  if (this->FileName)
    {
    delete [] this->FileName;
    this->FileName = NULL;
    }
}

int vtkGiftiReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{

  char line[1024];
  float xyz[3];
  int num_vertices, num_faces, i, j, index, numDA;

  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (!this->FileName) 
    {
    vtkErrorMacro(<< "A FileName must be specified.");
    return 1;
    }
    
  vtkDebugMacro(<<"Reading this->FileName");

  // intialise some structures to store the this->FileName contents in
  vtkPoints *points = vtkPoints::New(); 
  vtkCellArray *polys = vtkCellArray::New();
  vtkProperty *property = vtkProperty::New();

  property->SetAmbient(0.3);
  property->SetDiffuse(0.3);
  property->SetSpecular(0.4);
  property->SetSpecularPower(10);
  property->SetOpacity(1);
  property->SetColor(1,1,1);

  gifti_image* image = gifti_read_image (this->FileName, 1);
  if (NULL == image) {
    fprintf (stderr,"input_gifti: cannot read image\n");
    return(-1);
  }

  int valid = gifti_valid_gifti_image (image, 1);
  if (valid == 0) {
    fprintf (stderr,"input_gifti: GIFTI this->FileName %s is invalid!\n", this->FileName);
    gifti_free_image (image);
    return(-1);
  }

  giiDataArray* coords = NULL;
  giiDataArray* faces  = NULL;

  for (numDA = 0; numDA < image->numDA; numDA++) {
    if (image->darray[numDA]->intent == NIFTI_INTENT_POINTSET) {
      coords = image->darray[numDA];
    }
    else if (image->darray[numDA]->intent == NIFTI_INTENT_TRIANGLE) {
      faces = image->darray[numDA];
    }
  }

  if (coords && faces) {
  
    /* Check the number of vertices and faces. */
    long long num_vertices = 0;
    long long num_cols = 0;
    if (coords->ind_ord == GIFTI_IND_ORD_ROW_MAJOR) // RowMajorOrder
      gifti_DA_rows_cols (coords, &num_vertices, &num_cols);
    else // ColumnMajorOrder
      gifti_DA_rows_cols (coords, &num_cols, &num_vertices);

    if (num_vertices <= 0 || num_cols != 3) {
      fprintf (stderr,"input_gifti: malformed coords data array in this->FileName "
        "%s: num_vertices=%d num_cols=%d\n",
        this->FileName, (int)num_vertices, (int)num_cols);
      gifti_free_image (image);
      return(-1);
    }

    long long num_faces = 0;
    num_cols = 0;
    if (faces->ind_ord == GIFTI_IND_ORD_ROW_MAJOR) // RowMajorOrder
      gifti_DA_rows_cols (faces, &num_faces, &num_cols);
    else // ColumnMajorOrder
      gifti_DA_rows_cols (faces, &num_cols, &num_faces);

    if (num_faces <= 0 || num_cols != 3) {
      fprintf (stderr,"mrisReadGIFTIthis->FileName: malformed faces data array in this->FileName "
        "%s: num_faces=%d num_cols=%d\n",
        this->FileName, (int)num_faces, (int)num_cols);
      gifti_free_image (image);
      return(-1);
    }
    
    int vertex_index;
    for (vertex_index = 0; vertex_index < num_vertices; vertex_index++) {
          for (int j=0; j<3; ++j) {
        xyz[j] = (float) gifti_get_DA_value_2D (coords, vertex_index, j);
      }
          points->InsertNextPoint(xyz);
    }

    int face_index;
    for (face_index = 0; face_index < num_faces; face_index++) {
        polys->InsertNextCell(3);

      for (j = 0; j < 3; j++) {
        index = gifti_get_DA_value_2D (faces, face_index, j);
        polys->InsertCellPoint(index);
      }
    }
  } else {
    fprintf (stderr,"input_gifti: GIFTI this->FileName %s does not contain vertices and faces!\n", this->FileName);
    gifti_free_image (image);
    return(-1);
  }

  for (numDA = 0; numDA < image->numDA; numDA++) {
    giiDataArray* darray = image->darray[numDA];
    
    /* did these already */
    if ((darray->intent == NIFTI_INTENT_POINTSET) ||
        (darray->intent == NIFTI_INTENT_TRIANGLE)) continue;

    if (darray->intent == NIFTI_INTENT_SHAPE) 
      fprintf(stderr, "input_gifti: Reading of shape data not yet implemented.\n");
  }

	output->SetPoints(points);
	output->SetPolys(polys);
  
  points->Delete();
  polys->Delete();
  property->Delete();

  return 1;
}

void vtkGiftiReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "File Name: " 
     << (this->FileName ? this->FileName : "(none)") << "\n";

}

