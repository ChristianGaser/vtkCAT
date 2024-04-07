#include "vtkSurfaceReader.h"

extern "C" {
  #include "gifticlib/gifti_io.h"
}

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

int FreadInt(FILE *fp)
{
    unsigned char b1,b2,b3,b4;
    int count;
    
    count = fread(&b1, 1, 1, fp);
    count = fread(&b2, 1, 1, fp);
    count = fread(&b3, 1, 1, fp);
    count = fread(&b4, 1, 1, fp);
    return((b2 << 16) + (b3 << 8) + b4) ;

}

float FreadFloat(FILE *fp)
{
    unsigned char b1,b2,b3,b4;
    int count;
    
    count = fread(&b1, 1, 1, fp);
    count = fread(&b2, 1, 1, fp);
    count = fread(&b3, 1, 1, fp);
    count = fread(&b4, 1, 1, fp);
    long temp = (long)(b1 << 24 | b2 << 16 | b3 << 8 | b4);
    return *((float *) &temp);    // convert long into a float

}

int Fread3(FILE *fp)
{
    unsigned char b1,b2,b3;
    int count;
    
    count = fread(&b1, 1, 1, fp);
    count = fread(&b2, 1, 1, fp);
    count = fread(&b3, 1, 1, fp);
    return((b1 << 16) + (b2 << 8) + b3) ;

}


vtkSmartPointer<vtkPolyData> ConvertGIFTIToVTK(gifti_image* image)
{
  char line[1024];
  float xyz[3];
  int num_vertices, num_faces, i, j, index, numDA;

    vtkSmartPointer<vtkPolyData> vtkData = vtkSmartPointer<vtkPolyData>::New();
    // Conversion logic here
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();
  vtkProperty *property = vtkProperty::New();

  property->SetAmbient(0.3);
  property->SetDiffuse(0.3);
  property->SetSpecular(0.4);
  property->SetSpecularPower(10);
  property->SetOpacity(1);
  property->SetColor(1,1,1);

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
      throw std::runtime_error("input_gifti: malformed coords data array");
    }

    long long num_faces = 0;
    num_cols = 0;
    if (faces->ind_ord == GIFTI_IND_ORD_ROW_MAJOR) // RowMajorOrder
      gifti_DA_rows_cols (faces, &num_faces, &num_cols);
    else // ColumnMajorOrder
      gifti_DA_rows_cols (faces, &num_cols, &num_faces);

    if (num_faces <= 0 || num_cols != 3) {
      throw std::runtime_error("input_gifti: malformed faces data array");
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
    throw std::runtime_error("input_gifti: File does not contain vertices and faces!");
  }

  for (numDA = 0; numDA < image->numDA; numDA++) {
    giiDataArray* darray = image->darray[numDA];
    
    /* did these already */
    if ((darray->intent == NIFTI_INTENT_POINTSET) ||
        (darray->intent == NIFTI_INTENT_TRIANGLE)) continue;

    if (darray->intent == NIFTI_INTENT_SHAPE) 
      throw std::runtime_error("input_gifti: Reading of shape data not yet implemented.\n");
  }

  vtkData->SetPoints(points.GetPointer());
  vtkData->SetPolys(polys.GetPointer());
    
/*  points->Delete();
  polys->Delete();
  property->Delete();
*/

    return vtkData;
}

vtkSmartPointer<vtkPolyData> ReadGIFTIFile(const char* filename)
{
    gifti_image* gimage = gifti_read_image(filename, 1);
    if (!gimage) {
        throw std::runtime_error("Failed to read GIFTI file.");
    }

  int valid = gifti_valid_gifti_image (gimage, 1);
  if (valid == 0) {
    throw std::runtime_error("Failed to read GIFTI file.");
  }

    auto vtkData = ConvertGIFTIToVTK(gimage);
    gifti_free_image(gimage);
    return vtkData;
}
