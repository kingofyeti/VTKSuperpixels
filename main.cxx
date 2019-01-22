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

void test3DImage(std::string infile_path, std::string outfile_path, unsigned int num_superpixel = 50);
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
	std::string outfile_path = argv[2];
	unsigned int num_superpixel = 50;
	if (argc == 4)
		num_superpixel = strtol(argv[3], NULL, 10);;

	test3DImage(infile_path, outfile_path, num_superpixel);
	std::cout << float(clock() - begin_time) / CLOCKS_PER_SEC << std::endl;

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

void test3DImage(std::string infile_path, std::string outfile_path, unsigned int num_superpixel)
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
	superpixelFilter->SetOutputType(vtkSuperpixelFilter::AVGCOLOR);
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

	vtkSmartPointer<vtkNIFTIImageWriter> writer = vtkSmartPointer<vtkNIFTIImageWriter>::New();
	writer->SetInputData(superpixelFilter->GetOutput());
	writer->SetFileName(outfile_path.c_str());
	writer->Write();
}