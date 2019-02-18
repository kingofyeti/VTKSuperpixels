#include "vtkSuperpixelFilter.h"
#include <vtkImageActor.h>
#include <vtkImageCast.h>
#include <vtkImageMagnitude.h>
#include <vtkImageShiftScale.h>
#include <vtkImageViewer2.h>
#include <vtkNIFTIImageReader.h>
#include <vtkNIFTIImageWriter.h>
#include <vtkPNGReader.h>
#include <vtkPNGWriter.h>
#include <vtkProperty2D.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSliderRepresentation2D.h>
#include <vtkSliderWidget.h>
#include <vtkSmartPointer.h>
#include <vtkTextProperty.h>
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2)

#include <stdlib.h>
#include <ctime>
#include <iostream>
#include <string>

void test3DImage(std::string infile_path, unsigned int num_superpixel = 50);
void test2DImage();

// Callback for slider, for 3d image tests
class vtkSliderCallback : public vtkCommand
{
public:
	static vtkSliderCallback* New() { return new vtkSliderCallback; }

	virtual void Execute(vtkObject* caller, unsigned long, void*)
	{
		vtkSliderWidget* sliderWidget = reinterpret_cast<vtkSliderWidget*>(caller);
		imageViewer->SetSlice(static_cast<vtkSliderRepresentation*>(sliderWidget->GetRepresentation())->GetValue());
	}
	vtkSliderCallback() : imageViewer(0) { }
	vtkImageViewer2* imageViewer;
};

int main(int argc, char* argv[])
{

	const clock_t begin_time = clock();

	//test2DImage();
	if (argc < 3){
		std::cerr << "Error: Insufficient arguments" << std::endl;
		return EXIT_FAILURE;
	}
	std::string infile_path = argv[1];
	unsigned int num_superpixel = 50;
	if (argc >= 3)
		num_superpixel = strtol(argv[2], NULL, 10);
	test3DImage(infile_path, num_superpixel);
	std::cout << "Time: " << float(clock() - begin_time) / CLOCKS_PER_SEC << std::endl;

	return EXIT_SUCCESS;
}

void test2DImage()
{
	// Read the 2d png
	vtkSmartPointer<vtkPNGReader> reader = vtkSmartPointer<vtkPNGReader>::New();
	reader->SetFileName("./lena.png");
	
	reader->Update();

	// Convert to grayscale if rgb
	vtkSmartPointer<vtkImageData> input = reader->GetOutput();
	if (input->GetNumberOfScalarComponents() > 1)
	{
		vtkSmartPointer<vtkImageMagnitude> toGrayScale = vtkSmartPointer<vtkImageMagnitude>::New();
		toGrayScale->SetInputData(reader->GetOutput());
		toGrayScale->Update();
		input = toGrayScale->GetOutput();
	}

	// Superpixel segment
	vtkSmartPointer<vtkSuperpixelFilter> superpixelFilter = vtkSmartPointer<vtkSuperpixelFilter>::New();
	superpixelFilter->SetInputData(input);
	superpixelFilter->SetNumberOfSuperpixels(1000);
	superpixelFilter->SetColorWeight(0.1);
	superpixelFilter->SetOutputType(vtkSuperpixelFilter::AVGCOLOR);
	superpixelFilter->Update();

	// Scale to uchar
	vtkSmartPointer<vtkImageShiftScale> shiftScale = vtkSmartPointer<vtkImageShiftScale>::New();
	shiftScale->SetInputData(superpixelFilter->GetOutput());
	shiftScale->SetOutputScalarTypeToUnsignedChar();
	shiftScale->SetScale(255.0f / superpixelFilter->GetOutput()->GetScalarRange()[1]);
	shiftScale->Update();

	// Visualize
	vtkSmartPointer<vtkImageViewer2> imageViewer = vtkSmartPointer<vtkImageViewer2>::New();
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	imageViewer->GetRenderWindow()->SetSize(1000, 800);
	imageViewer->SetColorLevel(255.0 / 2.0);
	imageViewer->SetColorWindow(255.0);
	imageViewer->SetInputData(shiftScale->GetOutput());
	imageViewer->GetImageActor()->InterpolateOff();
	imageViewer->SetupInteractor(renderWindowInteractor);
	//renderWindowInteractor->SetInteractorStyle(vtkSmartPointer<vtkKeyPressInteractorStyle>::New());

	// Render image
	imageViewer->Render();
	renderWindowInteractor->Start();

	// To write the image as png we cast to uchar
	vtkSmartPointer<vtkImageCast> writeCast = vtkSmartPointer<vtkImageCast>::New();
	writeCast->SetInputData(superpixelFilter->GetOutput());
	writeCast->SetOutputScalarTypeToUnsignedChar();
	writeCast->Update();
	vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();
	writer->SetInputData(writeCast->GetOutput());
	writer->SetFileName("./spdiv.png");
	writer->Write();
}

void test3DImage(std::string infile_path, unsigned int num_superpixel)
{
	// Read 2d or 3d meta image (mhd)
	vtkSmartPointer<vtkNIFTIImageReader> reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
	reader->SetFileName(infile_path.c_str());
	reader->Update();

	// Superpixel segment
	vtkSmartPointer<vtkSuperpixelFilter> superpixelFilter = vtkSmartPointer<vtkSuperpixelFilter>::New();
	superpixelFilter->SetInputData(reader->GetOutput());
	superpixelFilter->SetNumberOfSuperpixels(num_superpixel);
	superpixelFilter->SetSwapIterations(0);
	// vtkSuperpixelFilter::OutputType outtype = vtkSuperpixelFilter::AVGCOLOR;
	// if (output_type == "LABEL"){
	// 	outtype = vtkSuperpixelFilter::LABEL;
	// }
	// if (output_type == "RANDRGB"){
	// 	outtype = vtkSuperpixelFilter::RANDRGB;
	// }
	// if (output_type == "AVGCOLOR"){
	// 	outtype = vtkSuperpixelFilter::AVGCOLOR;
	// }
	// if (output_type == "MAXCOLOR"){
	// 	outtype = vtkSuperpixelFilter::MAXCOLOR;
	// }
	// if (output_type == "MINCOLOR"){
	// 	outtype = vtkSuperpixelFilter::MINCOLOR;
	// }
	// superpixelFilter->SetOutputType(outtype);
	superpixelFilter->Update();

	// vtkSmartPointer<vtkImageShiftScale> imageCast = vtkSmartPointer<vtkImageShiftScale>::New();
	// imageCast->SetInputData(superpixelFilter->GetOutput());
	// imageCast->SetOutputScalarTypeToUnsignedChar();
	// imageCast->SetScale(255.0f / superpixelFilter->GetOutput()->GetScalarRange()[1]);
	// imageCast->Update();

// 	// Visualize
// 	vtkSmartPointer<vtkImageViewer2> imageViewer = vtkSmartPointer<vtkImageViewer2>::New();
// 	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
// 	imageViewer->GetRenderWindow()->SetSize(1000, 800);
// 	imageViewer->SetInputData(imageCast->GetOutput());
// 	imageViewer->SetSlice((imageViewer->GetSliceMax() + imageViewer->GetSliceMin()) / 2); // Set to middle slice
// 	imageViewer->GetImageActor()->InterpolateOff();
// 	imageViewer->SetupInteractor(renderWindowInteractor);

// #pragma region Setup Slider
// 	vtkSmartPointer<vtkSliderRepresentation2D> sliderRep = vtkSmartPointer<vtkSliderRepresentation2D>::New();
// 	sliderRep->SetMinimumValue(0);
// 	sliderRep->SetMaximumValue(superpixelFilter->GetOutput()->GetDimensions()[2]);
// 	sliderRep->SetValue(90);
// 	sliderRep->GetSliderProperty()->SetColor(1, 0, 0);
// 	sliderRep->GetLabelProperty()->SetColor(1, 0, 0);
// 	sliderRep->GetSelectedProperty()->SetColor(0, 1, 0);
// 	sliderRep->GetTubeProperty()->SetColor(1, 1, 0);
// 	sliderRep->GetCapProperty()->SetColor(1, 1, 0);
// 	sliderRep->GetPoint1Coordinate()->SetCoordinateSystemToDisplay();
// 	sliderRep->GetPoint1Coordinate()->SetValue(40, 40);
// 	sliderRep->GetPoint2Coordinate()->SetCoordinateSystemToDisplay();
// 	sliderRep->GetPoint2Coordinate()->SetValue(200, 40);
// 	vtkSmartPointer<vtkSliderWidget> sliderWidget = vtkSmartPointer<vtkSliderWidget>::New();
// 	sliderWidget->SetInteractor(renderWindowInteractor);
// 	sliderWidget->SetRepresentation(sliderRep);
// 	sliderWidget->SetAnimationModeToAnimate();
// 	sliderWidget->EnabledOn();

// 	vtkSmartPointer<vtkSliderCallback> callback = vtkSmartPointer<vtkSliderCallback>::New();
// 	callback->imageViewer = imageViewer;
// 	sliderWidget->AddObserver(vtkCommand::InteractionEvent, callback);
// #pragma endregion

// 	// Render image
// 	imageViewer->Render();
// 	/*imageViewer->GetRenderer()->ResetCamera();
// 	imageViewer->Render();*/
// 	renderWindowInteractor->Start();
	
	// Get 3 Output image
	// 0: Label, 1: RandomRGB, 2:AVGColor

	std::string file_prefix;

	if (infile_path.substr(infile_path.length() - 7) == ".nii.gz")
	{
		file_prefix = infile_path.substr(0, infile_path.length() - 7);
	}else if (infile_path.substr(infile_path.length() - 4) == ".nii")
	{
		file_prefix = infile_path.substr(0, infile_path.length() - 4);
	}else{
		std::cout << "Incorrect input file type\n";
		return;
	}

	std::vector<vtkImageData*> outputData(3);
	for (int i =0;i<3;i++){
		vtkSmartPointer<vtkNIFTIImageWriter> writer = vtkSmartPointer<vtkNIFTIImageWriter>::New();
		outputData[i] = superpixelFilter->GetOutput(i);
		writer->SetInputData(outputData[i]);

		std::string outfile_path;
		if (i == 0) outfile_path = file_prefix + "_label.nii.gz";
		if (i == 1) outfile_path = file_prefix + "_randrgb.nii.gz";
		if (i == 2) outfile_path = file_prefix +  "_avgcolor.nii.gz";

		writer->SetFileName(outfile_path.c_str());
		writer->Write();
	}
}