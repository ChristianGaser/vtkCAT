

#include "vtkPolyDataReader.h"
#include "vtkPolyDataWriter.h"

#include "vtkInflateSmoothPolyData.h"

static void usage(const char* const prog);

int main(int argc, char *argv[])
{
   if (argc < 3) {
      usage(argv[0]);
      exit(1);
   }
      
   double	compressStretchThreshold, inflationFactor, regularSmoothingStrength, fingerSmoothingStrength;
   int		regularSmoothingIterations, fingerSmoothingIterations, numberSmoothingCycles;
   int		outputType = 3; // 0 - inflated, 1- veryInflated, 2 - highSmooth, 3 - ellipsoid

   vtkInflateSmoothPolyData* inflateSmoothPolyData = vtkInflateSmoothPolyData::New();
   vtkPolyDataWriter* writer = vtkPolyDataWriter::New();

   int indx = -1;
   for (int j = 1; j < argc; j++) {
      if (argv[j][0] != '-') {
         indx = j;
         break;
      }
      else if (strcmp(argv[j], "-smooth_iters_reg") == 0) {
         j++;
         inflateSmoothPolyData->SetRegularSmoothingIterations(atoi(argv[j]));
      }
      else if (strcmp(argv[j], "-smooth_strength_reg") == 0) {
         j++;
         inflateSmoothPolyData->SetRegularSmoothingStrength(atof(argv[j]));
      }
      else if (strcmp(argv[j], "-num_cycles") == 0) {
         j++;
         inflateSmoothPolyData->SetNumberOfSmoothingCycles(atoi(argv[j]));
      }
      else if (strcmp(argv[j], "-inflate_factor") == 0) {
         j++;
         inflateSmoothPolyData->SetInflationFactor(atof(argv[j]));
      }
      else if (strcmp(argv[j], "-finger_threshold") == 0) {
         j++;
         inflateSmoothPolyData->SetCompressStretchThreshold(atof(argv[j]));
      }
      else if (strcmp(argv[j], "-smooth_iters_finger") == 0) {
         j++;
         inflateSmoothPolyData->SetFingerSmoothingIterations(atoi(argv[j]));
      }
      else if (strcmp(argv[j], "-smooth_strength_finger") == 0) {
         j++;
         inflateSmoothPolyData->SetFingerSmoothingStrength(atof(argv[j]));
      }
      else if (strcmp(argv[j], "-generate_compression") == 0) {
         inflateSmoothPolyData->SetGenerateAverageCompressedStretched(1);
      }
      else if (strcmp(argv[j], "-output_type") == 0) {
         j++;
         outputType = atoi(argv[j]);
      }
      else {
         cout << endl;
         cout << "ERROR: Unrecognized argument: " << argv[j] << endl; 
         cout << endl;
         exit(1);
      }
   }
   if (indx < 0) {
      usage(argv[0]);
      exit(1);
   }
   
   const int numArgs = argc - indx;
   
   if (numArgs != 2) {
      usage(argv[0]);
      exit(1);
   }
   
   char* inputFilename = argv[indx];
   char* outputFilename = argv[indx + 1];
   
   double p[7]  = {1, 0.2, 50, 1.0, 3.0, 1.0, 0};
   double p2[7] = {2, 1.0, 30, 1.4, 3.0, 1.0, 30};
   double p3[7] = {4, 1.0, 30, 1.1, 3.0, 1.0, 0};
   double p4[7] = {6, 1.0, 60, 1.6, 3.0, 1.0, 60};
   double p5[7] = {6, 1.0, 50, 1.4, 4.0, 1.0, 60};

   // read vtk surface file     
   vtkPolyDataReader *inputReader = vtkPolyDataReader::New();
   inputReader->SetFileName(inputFilename);

   inflateSmoothPolyData->SetInput(inputReader->GetOutput());
   inflateSmoothPolyData->SetNumberOfSmoothingCycles((int)p[0]);
   inflateSmoothPolyData->SetRegularSmoothingStrength(p[1]);
   inflateSmoothPolyData->SetRegularSmoothingIterations((int)p[2]);
   inflateSmoothPolyData->SetInflationFactor(p[3]);
   inflateSmoothPolyData->SetCompressStretchThreshold(p[4]);
   inflateSmoothPolyData->SetFingerSmoothingStrength(p[5]);
   inflateSmoothPolyData->SetFingerSmoothingIterations((int)p[6]);

   vtkInflateSmoothPolyData* inflated = vtkInflateSmoothPolyData::New();
   if (outputType > 0) {
     inflated->SetInput(inflateSmoothPolyData->GetOutput());
     inflated->SetNumberOfSmoothingCycles((int)p2[0]);
     inflated->SetRegularSmoothingStrength(p2[1]);
     inflated->SetRegularSmoothingIterations((int)p2[2]);
     inflated->SetInflationFactor(p2[3]);
     inflated->SetCompressStretchThreshold(p2[4]);
     inflated->SetFingerSmoothingStrength(p2[5]);
     inflated->SetFingerSmoothingIterations((int)p2[6]);   
     inflateSmoothPolyData->Delete();
   }

   vtkInflateSmoothPolyData* veryInflated = vtkInflateSmoothPolyData::New();
   if (outputType > 1) {
     veryInflated->SetInput(inflateSmoothPolyData->GetOutput());
     veryInflated->SetNumberOfSmoothingCycles((int)p3[0]);
     veryInflated->SetRegularSmoothingStrength(p3[1]);
     veryInflated->SetRegularSmoothingIterations((int)p3[2]);
     veryInflated->SetInflationFactor(p3[3]);
     veryInflated->SetCompressStretchThreshold(p3[4]);
     veryInflated->SetFingerSmoothingStrength(p3[5]);
     veryInflated->SetFingerSmoothingIterations((int)p3[6]);   
     inflated->Delete();
   }
   
   vtkInflateSmoothPolyData* highSmooth = vtkInflateSmoothPolyData::New();
   if (outputType > 2) {
     highSmooth->SetInput(inflateSmoothPolyData->GetOutput());
     highSmooth->SetNumberOfSmoothingCycles((int)p4[0]);
     highSmooth->SetRegularSmoothingStrength(p4[1]);
     highSmooth->SetRegularSmoothingIterations((int)p4[2]);
     highSmooth->SetInflationFactor(p4[3]);
     highSmooth->SetCompressStretchThreshold(p4[4]);
     highSmooth->SetFingerSmoothingStrength(p4[5]);
     highSmooth->SetFingerSmoothingIterations((int)p4[6]);   
     veryInflated->Delete();
   }
   
   vtkInflateSmoothPolyData* ellipsoid = vtkInflateSmoothPolyData::New();
   if (outputType > 3) {
     ellipsoid->SetInput(inflateSmoothPolyData->GetOutput());
     ellipsoid->SetNumberOfSmoothingCycles((int)p5[0]);
     ellipsoid->SetRegularSmoothingStrength(p5[1]);
     ellipsoid->SetRegularSmoothingIterations((int)p5[2]);
     ellipsoid->SetInflationFactor(p5[3]);
     ellipsoid->SetCompressStretchThreshold(p5[4]);
     ellipsoid->SetFingerSmoothingStrength(p5[5]);
     ellipsoid->SetFingerSmoothingIterations((int)p5[6]);   
     highSmooth->Delete();
   }
   
   // write the inflated surface
   switch (outputType) {
     case 0:
       writer->SetInput(inflateSmoothPolyData->GetOutput());
       break;
     case 1:
       writer->SetInput(inflated->GetOutput());
       break;
     case 2:
       writer->SetInput(veryInflated->GetOutput());
       break;
     case 3:
       writer->SetInput(highSmooth->GetOutput());
       break;
     case 4:
       writer->SetInput(ellipsoid->GetOutput());
       break;
   }
   writer->SetFileName(outputFilename);
   writer->SetFileTypeToASCII();
   writer->Write();
   
   ellipsoid->Delete();
   inputReader->Delete();
   writer->Delete();

   return 0;
}

static void
usage(const char* const prog)
{
   cout << endl
   << "NAME " << endl
   << "     " << prog << " - inflate surface and smooth fingers"<< endl
   << "" << endl
   << "SYNOPSIS" << endl
   << "     " << prog << " [options] <inputFiducial.vtk> <outputSphere.vtk>" << endl
   << "" << endl
   << "DESCRIPTION" << endl
   << "     This program will peform the following to the <input.vtk> surface" << endl
   << "     for the specified number of cycles:" << endl 
   << "        1) Smooth the entire surface." << endl
   << "        2) Inflate the surface so that it is similar in shape " << endl
   << "           to an ellipsoid with an aspect ratio of the " << endl
   << "           <fiducial.vtk> surface." << endl
   << "        3) Calculate error metrics on the surface." << endl
   << "        4) Perform targeted smoothing on nodes in the surface" << endl
   << "           having high distortion." << endl
   << endl
   << "OPTIONS" << endl
   << "   -smooth_iters_reg" << endl
   << "      Number of iteration entire surface is smoothed each cycle." << endl
   << "   -smooth_strength_reg" << endl
   << "      Smoothing strength when smoothing entire surface each cycle" << endl
   << "   -num_cycles" << endl
   << "      Number of cycles." << endl
   << "   -inflate_factor" << endl
   << "      Inflation factor used when inflating surface." << endl
   << "   -finger_threshold" << endl
   << "      Nodes with average compressed/stretched value larger than this" << endl
   << "      value will be finger smoothed." << endl
   << "   -smooth_iters_finger" << endl
   << "      Number of iteration when finger smoothing." << endl
   << "   -smooth_strength_finger" << endl
   << "      Smoothing strength when smoothing fingers." << endl
   << "   -generate_compression" << endl
   << "      Generate average compression/strechted values embedded as scalars." << endl
   << "   -output_type" << endl
   << "      0 - inflated." << endl
   << "      1 - veryInflated." << endl
   << "      2 - highSmooth." << endl
   << "      3 - ellipsoid." << endl
   << endl
   << "REQUIRED PARAMETERS" << endl
   << "     <inputFiducial.vtk> " << endl
   << "     <outputSphere.vtk> " << endl
   << "" << endl
   << "EXAMPLE" << endl
   << "     " << prog << " -smooth_iters_reg 75 input.vtk output.vtk" << endl
   << endl
   << endl;
}
