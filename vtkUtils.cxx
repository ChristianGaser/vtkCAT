#include "vtkUtils.h"

// Linspace function similar to Matlab/Python
template<typename T>
std::vector<T> linspace(T start, T end, int num) {
    std::vector<T> result;
    if (num < 2) {
        result.push_back(start);
        return result;
    }

    T step = (end - start) / (num - 1);
    for (int i = 0; i < num; ++i) {
        result.push_back(start + i * step);
    }

    // Ensure last value is exactly 'end' due to possible floating-point arithmetic issues
    if (num > 1) {
        result.back() = end;
    }

    return result;
}

int ReadBackgroundScalars(const char* overlayFileNameBkgL, vtkSmartPointer<vtkPolyData> polyData[], vtkSmartPointer<vtkDoubleArray> scalarsBkg[], int nMeshes, bool& rhExists, string& rhSurfName) {
  bool rhExistsBkg;
  
  cout << "Read underlays: " << overlayFileNameBkgL << endl;
  
  scalarsBkg[0] = readScalars(overlayFileNameBkgL);
  
  // If defined, read right hemispheric overlay
  if ((nMeshes > 1) && rhExists) {

    // Try replacing lh/left by rh/right in the overlay filename
    string overlayFileNameR = overlayFileNameBkgL;
    if (overlayFileNameR.find("lh.") != string::npos) {
      vtksys::SystemTools::ReplaceString(overlayFileNameR,string("lh."),string("rh."));
      rhExistsBkg = vtksys::SystemTools::FileExists(overlayFileNameR);
    } else if (rhSurfName.find("left") != string::npos) {
      string overlayFileNameR = overlayFileNameBkgL;
      vtksys::SystemTools::ReplaceString(overlayFileNameR,string("left"),string("right"));
      rhExistsBkg = vtksys::SystemTools::FileExists(overlayFileNameR);
    } else rhExistsBkg = false;

    if (rhExistsBkg)
      scalarsBkg[1] = readScalars(overlayFileNameR.c_str());
    else {
      // or split the single overlay (which is merged) into left and right overlay
      try {
        scalarsBkg[1]->SetNumberOfTuples(polyData[0]->GetNumberOfPoints());
        for (auto i=0; i < polyData[0]->GetNumberOfPoints(); i++)
          scalarsBkg[1]->SetValue(i,scalarsBkg[0]->GetValue(i+polyData[1]->GetNumberOfPoints()));
      } catch (const exception& e) {
        cerr << "Error splitting values for left/right hemisphere in file " << overlayFileNameBkgL << " " << e.what() << endl;
        return EXIT_FAILURE;;
      }
    }
  }
  
  if (scalarsBkg[0] == NULL) {
    cerr << "Error reading file " << overlayFileNameBkgL << endl;
    return EXIT_FAILURE;;
  }
  return EXIT_SUCCESS;
}

int ReadAndUpdateScalars(string overlayFileNameL, vtkSmartPointer<vtkPolyData> polyData[], vtkSmartPointer<vtkDoubleArray> scalars[], int nMeshes, bool rhExists, const string& rhSurfName, bool inverse, double clipRange[], fs::path currentPath, double overlayRange[]) {
  
  // Since the dat-file is otherwise not found, we have to go into the folder
  // of the overlay data to ensure that the gii/dat files are correctly read
  string directoryPath = vtksys::SystemTools::GetFilenamePath(overlayFileNameL);
  string baseNameL = vtksys::SystemTools::GetFilenameName(overlayFileNameL);
  
  // Change current path
  fs::current_path(directoryPath);
  
  cout << "Read overlays: " << overlayFileNameL << endl;
  
  scalars[0] = readScalars(baseNameL.c_str());
  
  // If defined, read right hemispheric overlay
  if ((nMeshes > 1) && rhExists) {

    // Try replacing lh/left by rh/right in the overlay filename
    string overlayFileNameR = baseNameL.c_str();
    if (overlayFileNameR.find("lh.") != string::npos) {
      vtksys::SystemTools::ReplaceString(overlayFileNameR,string("lh."),string("rh."));
      rhExists = vtksys::SystemTools::FileExists(overlayFileNameR);
    } else if (rhSurfName.find("left") != string::npos) {
      string overlayFileNameR = baseNameL.c_str();
      vtksys::SystemTools::ReplaceString(overlayFileNameR,string("left"),string("right"));
      rhExists = vtksys::SystemTools::FileExists(overlayFileNameR);
    } else rhExists = false;

    if (rhExists)
      scalars[1] = readScalars(overlayFileNameR.c_str());
    else {
      // or split the single overlay (which is merged) into left and right overlay
      try {
        scalars[1]->SetNumberOfTuples(polyData[0]->GetNumberOfPoints());
        for (auto i=0; i < polyData[0]->GetNumberOfPoints(); i++)
          scalars[1]->SetValue(i,scalars[0]->GetValue(i+polyData[1]->GetNumberOfPoints()));
      } catch (const exception& e) {
        cerr << "Error splitting values for left/right hemisphere in file " << overlayFileNameL << " " << e.what() << endl;
        return EXIT_FAILURE;;
      }
    }
    
    // Go back to current folder
    fs::current_path(currentPath);
  }
  
  if(inverse) {
    for (auto i = 0; i < nMeshes; i++) {
      for (auto k = 0; k < polyData[i]->GetNumberOfPoints(); k++)
        scalars[i]->SetValue(k,-(scalars[i]->GetValue(k)));
    }
  }
  
  if (scalars[0] == NULL) {
    cerr << "Error reading file " << overlayFileNameL << endl;
    return EXIT_FAILURE;;
  }

  // Clip values if defined
  if (clipRange[1] > clipRange[0]) {
    for (auto i = 0; i < nMeshes; i++) {
      for (auto k = 0; k < polyData[0]->GetNumberOfPoints(); k++) {
        double val = scalars[i]->GetValue(k);
        if (((val > clipRange[0]) && (val < clipRange[1])) || isnan(val))
          scalars[i]->SetValue(k,0.0);
      }
    }
  } else {
    for (auto i = 0; i < nMeshes; i++) {
      for (auto k = 0; k < polyData[0]->GetNumberOfPoints(); k++) {
        double val = scalars[i]->GetValue(k);
        if (isnan(val)) {
          scalars[i]->SetValue(k,0.0);
          clipRange[0] = 0;
          clipRange[1] = 1E-15;
        }
      }
    }
  }
  
  for (auto i = 0; i < nMeshes; i++) {
    polyData[i]->GetPointData()->SetScalars(scalars[i]);
    if (overlayRange[1] < overlayRange[0])
      polyData[i]->GetScalarRange( overlayRange );
  }
  return EXIT_SUCCESS;
}

void UpdateScalarBarAndLookupTable(int n1, int n2, vtkSmartPointer<vtkDoubleArray> scalars[], vtkLookupTable* lookupTable, vtkLookupTable* lookupTableColorBar, const double overlayRange[], const double clipRange[], int colorbar, bool bkgWhite, int fontSize, bool logColorbar, bool printStats, vtkRenderer* renderer, const char* Title, double alpha) {

  double white[3] = {1.0, 1.0, 1.0};
  double black[3] = {0.0, 0.0, 0.0};

  vtkSmartPointer<vtkDoubleArray> scalarLR = vtkSmartPointer<vtkDoubleArray>::New();
  int nValuesLR = n1 + n2;
  scalarLR->SetNumberOfTuples(nValuesLR);

  for (auto i=0; i < n1; i++)
    scalarLR->SetValue(i,scalars[0]->GetValue(i));
  for (auto i=0; i < n2; i++)
    scalarLR->SetValue(i+n2,scalars[1]->GetValue(i));

  for (auto i = 0; i < 256; i++) {
    double rgb[4];
    lookupTable->GetTableValue(i, rgb);
    lookupTableColorBar->SetTableValue(i, rgb[0], rgb[1], rgb[2], alpha);
  }

  if (clipRange[1] > clipRange[0]) {
    int start = 0, end = 256;
    
    if (clipRange[0] > overlayRange[0])
      start = round(256*(clipRange[0] - overlayRange[0])/(overlayRange[1] - overlayRange[0]));
      
    if (clipRange[1] > overlayRange[0])
      end = round(256*(clipRange[1] - overlayRange[0])/(overlayRange[1] - overlayRange[0]));

    for (auto i = start; i < end; i++)
      lookupTableColorBar->SetTableValue(i, 0.5, 0.5, 0.5, alpha);
  }

  // Use discrete colorbar with gaps
  if (colorbar == 2) {
    int steps = 8;
    for (auto i = 0; i < 256; i++) {        
      if (!(i % steps)) {
        if (bkgWhite) lookupTableColorBar->SetTableValue(i, 1.0, 1.0, 1.0, 0);
        else lookupTableColorBar->SetTableValue(i, 0.0, 0.0, 0.0, 0);
      }
    }
  }

  lookupTableColorBar->SetTableRange( overlayRange );
  lookupTableColorBar->SetAlphaRange( alpha, alpha );

  vtkSmartPointer<vtkTextProperty> textProperty = vtkSmartPointer<vtkTextProperty>::New();
  if (bkgWhite) textProperty->SetColor(black);
  else textProperty->SetColor(white);
  if (fontSize) textProperty->SetFontSize(fontSize);

  vtkNew<vtkScalarBarActor> scalarBar;
  scalarBar->SetOrientationToHorizontal();
  scalarBar->SetLookupTable(lookupTableColorBar);
  scalarBar->SetWidth(0.3);
  scalarBar->SetHeight(0.05);
  scalarBar->SetPosition(0.35, 0.05);

  // Replace label values by log-scaled values
  if (logColorbar) {
    scalarBar->SetNumberOfLabels(0); // Suppress labels
    
    // Set color of annotation
    scalarBar->SetAnnotationTextProperty(textProperty);      

    for (int i = floor(overlayRange[0]); i < ceil(overlayRange[1]) + 1; ++i) {
      double value = (double) i;

      array<char, 64> buffer;
      
      if (logColorbar) {
        if (value > 0)
          snprintf(buffer.data(), buffer.size(), "%g", 1.0/pow(10,value));  
        else if (value < 0)
          snprintf(buffer.data(), buffer.size(), "%g", -1.0/pow(10,-value));
        else
          snprintf(buffer.data(), buffer.size(), "%g", 0.0);
      } else snprintf(buffer.data(), buffer.size(), "%g", value);  

      scalarBar->GetLookupTable()->SetAnnotation(value, buffer.data());
      scalarBar->GetLookupTable()->SetAnnotation(value, buffer.data());
    }
  } else if (fontSize) {
    scalarBar->SetNumberOfLabels(0); // Suppress labels
    
    // Set color of annotation
    scalarBar->SetAnnotationTextProperty(textProperty);

    auto vec = linspace(overlayRange[0], overlayRange[1], 5);

    for (auto value : vec) {
      array<char, 64> buffer;
      snprintf(buffer.data(), buffer.size(), "%g", value);  
        
      scalarBar->GetLookupTable()->SetAnnotation(value, buffer.data());
    }      
  }
  
  scalarBar->SetTitleTextProperty(textProperty);
  scalarBar->SetLabelTextProperty(textProperty);

  scalarBar->GetTitleTextProperty()->SetLineOffset(-10); // Apply additional specific settings after copying
  scalarBar->GetTitleTextProperty()->BoldOn();

  char info[150];
  snprintf(info, sizeof(info), "Mean=%.3f Median=%.3f SD=%.3f", get_mean(scalarLR), get_median(scalarLR), get_std(scalarLR));
  if (printStats) scalarBar->SetTitle(info);
  else scalarBar->SetTitle(Title);
  
  renderer->AddActor2D(scalarBar);

}

// Fill the lookup table using the color transfer function
void fillLookupTable(vtkLookupTableWithEnabling* lookupTable, vtkColorTransferFunction* colorTransferFunction, double alpha) {

  for (int i = 0; i < 256; i++) {
      double* rgb;
      double value = (static_cast<double>(i) / 255.0) * 100.0;
      rgb = colorTransferFunction->GetColor(value);
      lookupTable->SetTableValue(i, rgb[0], rgb[1], rgb[2], alpha); // Set RGBA, with full opacity
  }
}

// Obtain LookUpTable for defined colormap
vtkSmartPointer<vtkLookupTableWithEnabling> getLookupTable(int colormap, double alpha, double overlayRange[], double clipRange[], int clipFullColors)
{
  double c1, c2;
  vtkSmartPointer<vtkLookupTableWithEnabling> lookupTable = vtkSmartPointer<vtkLookupTableWithEnabling>::New();
  vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();

  if (clipFullColors) {
    c1 = 100.0*(clipRange[0] - overlayRange[0])/(overlayRange[1] - overlayRange[0]);
    c2 = 100.0*(clipRange[1] - overlayRange[0])/(overlayRange[1] - overlayRange[0]);
  } else {
    c1 = c2 = 50.0;
  }
  
  switch(colormap) {
  case C1:
    // Add RGB points to the function
    colorTransferFunction->AddRGBPoint(0.0,  50/255.0,136/255.0,189/255.0);
    colorTransferFunction->AddRGBPoint(c1*1/4, 102/255.0,194/255.0,165/255.0);
    colorTransferFunction->AddRGBPoint(c1*2/4, 171/255.0,221/255.0,164/255.0);
    colorTransferFunction->AddRGBPoint(c1*3/4, 230/255.0,245/255.0,152/255.0);
    colorTransferFunction->AddRGBPoint(c1, 255/255.0,255/255.0,191/255.0);
    
    colorTransferFunction->AddRGBPoint(c2, 255/255.0,255/255.0,191/255.0);
    colorTransferFunction->AddRGBPoint(c2*5/4, 254/255.0,224/255.0,139/255.0);
    colorTransferFunction->AddRGBPoint(c2*6/4, 253/255.0,174/255.0,97/255.0); 
    colorTransferFunction->AddRGBPoint(c2*7/4, 244/255.0,109/255.0,67/255.0); 
    colorTransferFunction->AddRGBPoint(100.0,213/255.0,62/255.0,79/255.0); 

    fillLookupTable(lookupTable,colorTransferFunction,alpha);
    break;
  case C2:
    // Add RGB points to the function
    colorTransferFunction->AddRGBPoint(0.0, 0.0, 0.6, 1.0); 
    colorTransferFunction->AddRGBPoint(c1*1/2, 0.5, 1.0, 0.5); 
    colorTransferFunction->AddRGBPoint(c1, 1.0, 1.0, 0.5); 
    colorTransferFunction->AddRGBPoint(c2, 1.0, 1.0, 0.5); 
    colorTransferFunction->AddRGBPoint(c2*3/2, 1.0, 0.75, 0.5); 
    colorTransferFunction->AddRGBPoint(100.0, 1.0, 0.5, 0.5);

    fillLookupTable(lookupTable,colorTransferFunction,alpha);
    break;
  case C3:
    // Add RGB points to the function
    colorTransferFunction->AddRGBPoint(0.0,  0/255.0,143/255.0,213/255.0); 
    colorTransferFunction->AddRGBPoint(c1*1/2, 111/255.0,190/255.0,70/255.0); 
    colorTransferFunction->AddRGBPoint(c1, 255/255.0,220/255.0,45/255.0); 
    colorTransferFunction->AddRGBPoint(c2, 255/255.0,220/255.0,45/255.0); 
    colorTransferFunction->AddRGBPoint(c2*3/2, 252/255.0,171/255.0,23/255.0); 
    colorTransferFunction->AddRGBPoint(100.0,238/255.0,28/255.0,58/255.0);

    fillLookupTable(lookupTable,colorTransferFunction,alpha);
    break;
  case JET:
    // Add RGB points to the function
    colorTransferFunction->AddRGBPoint(0.0, 0.0, 0.0, 0.5625); 
    colorTransferFunction->AddRGBPoint(c1*1/3, 0.0, 0.0, 1.0); 
    colorTransferFunction->AddRGBPoint(c1*2/3, 0.0, 1.0, 1.0); 
    colorTransferFunction->AddRGBPoint(c1, 0.5, 1.0, 0.5); 
    colorTransferFunction->AddRGBPoint(c2, 0.5, 1.0, 0.5); 
    colorTransferFunction->AddRGBPoint(c2*4/3, 1.0, 1.0, 0.0);
    colorTransferFunction->AddRGBPoint(c2*5/3, 1.0, 0.0, 0.0);
    colorTransferFunction->AddRGBPoint(100.0, 0.5, 0.0, 0.0);

    fillLookupTable(lookupTable,colorTransferFunction,alpha);
    break;
  case FIRE:
    // Add RGB points to the function
    colorTransferFunction->AddRGBPoint(0.0, 0.0, 0.0, 0.0); 
    colorTransferFunction->AddRGBPoint(c1*1/2, 0.5, 0.0, 0.0); 
    colorTransferFunction->AddRGBPoint(c1, 1.0, 0.0, 0.0); 
    colorTransferFunction->AddRGBPoint(c2, 1.0, 0.0, 0.0); 
    colorTransferFunction->AddRGBPoint(c2*3/2, 1.0, 0.5, 0.0); 
    colorTransferFunction->AddRGBPoint(100.0, 1.0, 1.0, 0.0);

    fillLookupTable(lookupTable,colorTransferFunction,alpha);
    break;
  case BIPOLAR:
    // Add RGB points to the function
    colorTransferFunction->AddRGBPoint(0.0, 0.0, 1.0, 1.0); 
    colorTransferFunction->AddRGBPoint(c1*1/2, 0.0, 0.0, 1.0); 
    colorTransferFunction->AddRGBPoint(c1, 0.1, 0.1, 0.1); 
    colorTransferFunction->AddRGBPoint(c2, 0.1, 0.1, 0.1); 
    colorTransferFunction->AddRGBPoint(c2*5/4, 0.5, 0.0, 0.0); 
    colorTransferFunction->AddRGBPoint(c2*6/4, 1.0, 0.0, 0.0); 
    colorTransferFunction->AddRGBPoint(c2*7/4, 1.0, 0.5, 0.0); 
    colorTransferFunction->AddRGBPoint(100.0, 1.0, 1.0, 0.0);

    fillLookupTable(lookupTable,colorTransferFunction,alpha);
    break;
  case GRAY:
    lookupTable->SetHueRange( 0.0, 0.0 );
    lookupTable->SetSaturationRange( 0.0, 0.0 );
    lookupTable->SetValueRange( 0.0, 1.0 );
    break;
  }
  return lookupTable;
}

void usage(const char* const prog)
{
  cout << endl
  << "NAME " << endl
  << "    " << prog << " - render left and right surfaces"<< endl
  << "" << endl
  << "SYNOPSIS" << endl
  << "    " << prog << " [options] <left_surface>" << endl
  << "" << endl
  << "DESCRIPTION" << endl
  << "    This program will render the left and right surfaces. Only the left hemipshere should be defined while the right is automatically selected." << endl
  << endl
  << "OPTIONS" << endl
  << " Overlay:" << endl
  << "  -scalar scalarInput (deprecated) " << endl
  << "     File with scalar values (gifit, ascii or Freesurfer format), either for the left or merged hemispheres." << endl
  << endl
  << "  -overlay scalarInput  " << endl
  << "  -ov scalarInput  " << endl
  << "     File with scalar values (gifit, ascii or Freesurfer format), either for the left or merged hemispheres." << endl
  << endl
  << "  -range lower upper  " << endl
  << "  -r lower upper  " << endl
  << "     Range of scalar values." << endl
  << "     Default value: " << defaultScalarRange[0] << " " << defaultScalarRange[1] << endl
  << endl
  << "  -clip lower upper  " << endl
  << "  -cl lower upper  " << endl
  << "     Clip scalar values. These values will be not displayed." << endl
  << "     Default value: " << defaultClipRange[0] << " " << defaultClipRange[1] << endl
  << endl
  << "  -clip2 lower upper  " << endl
  << "  -cl2 lower upper  " << endl
  << "     Clip scalar values. These values will be not displayed, but here the full range of the colors is shown and not clipped." << endl
  << "     Default value: " << defaultClipRange[0] << " " << defaultClipRange[1] << endl
  << endl
  << "  -bkg scalarInputBkg  " << endl
  << "     File with scalar values for background surface (gifit, ascii or Freesurfer format), either for the left or merged hemispheres." << endl
  << endl
  << "  -range-bkg lower upper  " << endl
  << "  -rb lower upper  " << endl
  << "     Range of background scalar values." << endl
  << "     Default value: " << defaultScalarRangeBkg[0] << " " << defaultScalarRangeBkg[1] << endl
  << endl
  << "  -colorbar  " << endl
  << "  -cb  " << endl
  << "     Show colorbar (default no)." << endl
  << endl
  << "  -colorbar2  " << endl
  << "  -cb2  " << endl
  << "     Show discrete colorbar with gaps (default no)." << endl
  << endl
  << "  -title  " << endl
  << "     Set name for colorbar (default scalar-file)." << endl
  << endl
  << "  -fontsize  " << endl
  << "  -fs  " << endl
  << "     Set font size for colorbar." << endl
  << endl
  << "  -log  " << endl
  << "     Use log-scaled colorbar (overlay values should be log-scaled)." << endl
  << endl
  << "  -inverse  " << endl
  << "     Invert input values." << endl
  << endl
  << "  -stats  " << endl
  << "     Output mean/median/SD in colorbar." << endl
  << endl
  << " Colors:" << endl
  << "  -opacity value  " << endl
  << "  -op value  " << endl
  << "     Value for opacity of overlay." << endl
  << "     Default value: " << defaultAlpha << endl
  << endl
  << "  -white  " << endl
  << "     Use white background" << endl
  << endl
  << "  -fire  " << endl
  << "     Use custom fire colorbar 1 (default jet)." << endl
  << "  -bipolar  " << endl
  << "     Use custom bipolar colorbar 1 (default jet)." << endl
  << "  -c1  " << endl
  << "     Use custom rainbow colorbar 1 (default jet)." << endl
  << "  -c2  " << endl
  << "     Use custom rainbow colorbar 2 (default jet)." << endl
  << "  -c3  " << endl
  << "     Use custom rainbow colorbar 3 (default jet)." << endl
  << endl
  << " Output:" << endl
  << "  -size xsize ysize  " << endl
  << "  -sz xsize ysize  " << endl
  << "     Window size." << endl
  << "     Default value: " << defaultWindowSize[0] << " " << defaultWindowSize[1] << endl
  << endl
  << "  -output output.png  " << endl
  << "  -save output.png  " << endl
  << "     Save as png-file or skip extension to save image with the overlay or mesh name as png." << endl
  << endl
  << "KEYBOARD INTERACTIONS" << endl
  << "      Use Shift-key for small rotations and Ctrl/Cmd-key for flipping." << endl
  << endl
  << "  l L Rotate left." << endl
  << endl
  << "  r R Rotate right." << endl
  << endl
  << "  u U Rotate up." << endl
  << endl
  << "  d D Rotate down." << endl
  << endl
  << "  w   Show wireframe." << endl
  << endl
  << "  s   Show shaded." << endl
  << endl
  << "  3   Stereo view." << endl
  << endl
  << "  g   Save image png-file." << endl
  << endl
  << "  f   Zoom to selected point." << endl
  << endl
  << "  q e Quit." << endl
  << endl
  << "REQUIRED PARAMETERS" << endl
  << "    <input.gii> " << endl
  << "" << endl
  << "EXAMPLE" << endl
  << "    " << prog << " -range -1 1 -overlay overlayInput.txt lh.central.freesurfer.gii" << endl
  << endl
  << endl;
}
