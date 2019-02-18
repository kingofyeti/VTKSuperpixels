#pragma once
#include <vector>
#include <iostream>
#include <vtkImageAlgorithm.h>
#include <vtkImageData.h>

class Cluster;
class ClusterPair;
class PixelNode;
class MxHeap;

class vtkSuperpixelFilter : public vtkImageAlgorithm
{
public:
	// Color labels: Sequential unique grayscale for every cluster (ie: 0, 1, 2, ...)
	// Random rgb: Random rgb value for every cluster
	// Average color: Averages the grayscale output
	// Max: Takes the maximum of the cluster
	// Min: Takes the minimum of the cluster
	enum OutputType
	{
		LABEL,
		RANDRGB,
		AVGCOLOR,
		MAXCOLOR,
		MINCOLOR
	};

public:
	static vtkSuperpixelFilter* New();
	vtkTypeMacro(vtkSuperpixelFilter, vtkImageAlgorithm);

	vtkSuperpixelFilter();
	~vtkSuperpixelFilter();

	void SetOutputType(OutputType outputType) { vtkSuperpixelFilter::outputType = outputType; this->Modified(); }
	vtkSetMacro(NumberOfSuperpixels, unsigned int);
	vtkGetMacro(NumberOfSuperpixels, unsigned int);
	vtkSetMacro(SwapIterations, unsigned int);
	vtkGetMacro(SwapIterations, unsigned int);
	// The color is scaled by this when added to the heap
	vtkSetMacro(ColorWeight, double);
	vtkGetMacro(ColorWeight, double);
	std::vector<Cluster*> getClusters() { return outputClusters; }
	std::vector<ClusterPair*> getClusterPairs() { return outputPairs; }
	PixelNode* getPixels() { return px; }
	int getPixelCount() { return NumberOfPixels; }

protected:
	int RequestInformation(vtkInformation* request, vtkInformationVector** inputVec, vtkInformationVector* outputVec) VTK_OVERRIDE;
	int RequestData(vtkInformation* request, vtkInformationVector** inputVec, vtkInformationVector* outputVec) VTK_OVERRIDE;

private:
	vtkSuperpixelFilter(const vtkSuperpixelFilter&);
	void operator=(const vtkSuperpixelFilter&);

	void initClusters(vtkImageData* input);
	MxHeap* createHeap(vtkImageData* input);
	void removeEdges(MxHeap* minHeap, ClusterPair* pair);

	void calcHeap(vtkImageData* input);

	void calcColorLabels(vtkImageData* output);
	void calcRandRgb(vtkImageData* output);
	void calcAvgColors(vtkImageData* output);
	void calcMaxColors(vtkImageData* output);
	void calcMinColors(vtkImageData* output);

	// Returns the largest change in energy of the swaps made
	void computeSwap(int width, int height, int depth);

private:
	// Clusters array containing every cluster made
	Cluster* clusters = nullptr;
	PixelNode* px = nullptr;
	// Points to all the resulting clusters (subset of clusters)
	std::vector<Cluster*> outputClusters;
	std::vector<ClusterPair*> outputPairs;

	MxHeap* minHeap = nullptr;

	OutputType outputType = LABEL;
	unsigned int NumberOfSuperpixels = 0;
	unsigned int NumberOfPixels = 0;
	double ColorWeight = 1.0;
	unsigned int SwapIterations = 0;
};