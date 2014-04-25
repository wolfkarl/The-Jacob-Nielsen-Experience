////////////////////////////////////////////////////////////////////////////////
//
// Class grabbing frames from the depth camera
//
// Authors: Stefan Richter (2011) and Patrick Lühne (2012)
//
////////////////////////////////////////////////////////////////////////////////

#include "DepthCamera.h"

#include "DepthCameraException.h"

////////////////////////////////////////////////////////////////////////////////
//
// DepthCamera
//
////////////////////////////////////////////////////////////////////////////////

const std::string DepthCamera::s_sampleXMLPath = "SamplesConfig.xml";

////////////////////////////////////////////////////////////////////////////////

DepthCamera::DepthCamera()
{
	xn::EnumerationErrors errors;
	XnStatus status = XN_STATUS_OK;

	// Initialize the camera settings
	status = m_context.InitFromXmlFile(s_sampleXMLPath.c_str(), &errors);

	if (status == XN_STATUS_NO_NODE_PRESENT)
	{
		XnChar errorString[1024];
		errors.ToString(errorString, 1024);

		throw DepthCameraException(errorString);
	}

	if (status != XN_STATUS_OK)
		throwException("Could not open file", status);

	// Find the depth node
	status = m_context.FindExistingNode(XN_NODE_TYPE_DEPTH, m_depthGenerator);

	if (status != XN_STATUS_OK)
		throwException("Finding the depth generator failed", status);

	// Find the RGB node
	status = m_context.FindExistingNode(XN_NODE_TYPE_IMAGE, m_imageGenerator);

	if (status != XN_STATUS_OK)
		throwException("Finding the image generator failed", status);

	// Start generating frames
	status = m_context.StartGeneratingAll();

	if (status != XN_STATUS_OK)
		throwException("Starting generating failed", status);

	m_depthGenerator.GetAlternativeViewPointCap().SetViewPoint(m_imageGenerator);
}

////////////////////////////////////////////////////////////////////////////////

DepthCamera::~DepthCamera()
{
	m_context.Shutdown();
}

////////////////////////////////////////////////////////////////////////////////

void DepthCamera::throwException(std::string description, XnStatus status)
{
	throw DepthCameraException(description + ": " + xnGetStatusString(status));
}

////////////////////////////////////////////////////////////////////////////////

bool DepthCamera::loadVideo(const std::string &rgbFile,
							const std::string &depthFile)
{
	if (rgbFile.length() <= 1 || depthFile.length() <= 1)
		return false;

	m_rgbReader = cv::VideoCapture(rgbFile);
	m_depthReader = cv::VideoCapture(depthFile);

	return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DepthCamera::frameFromVideo(cv::Mat &rgbImage, cv::Mat &depthImage)
{
	if (m_rgbReader.isOpened() && !m_rgbReader.read(rgbImage))
		return false;

	if (m_depthReader.isOpened() && !m_depthReader.read(depthImage))
		return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////////

int DepthCamera::frameFromCamera(cv::Mat &rgbImage, cv::Mat &depthImage,
								 int depthType)
{
	// Read next available data
	XnStatus status = m_context.WaitAndUpdateAll();

	if (status != XN_STATUS_OK)
		return status;

	// Process the data
	m_depthGenerator.GetMetaData(m_depthMetaData);
	m_imageGenerator.GetMetaData(m_imageMetaData);

	// convert depth
	if (depthType == CV_16UC1)
		convertDepthToMat_16UC1(m_depthMetaData, depthImage);
	else
		convertDepthToMat_8UC3(m_depthMetaData, depthImage);

	// convert rgb
	convertRGBToMat(m_imageMetaData, rgbImage);

	return XN_STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////

bool DepthCamera::frameFromFile(std::string rgbFile, cv::Mat &rgbImage,
							   std::string depthFile, cv::Mat &depthImage)
{
	rgbImage = cv::imread(rgbFile);

	if (rgbImage.rows <= 0 || rgbImage.cols <= 0)
		return false;

	depthImage = cv::imread(depthFile);

	if (depthImage.rows <= 0 || depthImage.cols <= 0)
		return false;

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

void DepthCamera::convertDepth_8UC3_to_16UC1(const cv::Mat &depth8,
											 cv::Mat &depth16)
{
	depth16 = cv::Mat(depth8.rows, depth16.cols, CV_16UC1);
	ushort *out = (ushort*)depth16.data;

	for (int row = 0; row < depth8.rows; row++)
		for (int col = 0; col < depth8.cols; col++)
		{
			out[row * depth8.cols + col]
				= depth8.data[(row * depth8.cols + col) * 3 + 0]
				| (depth8.data[(row * depth8.cols + col) * 3 + 1] << 8);
		}
}

////////////////////////////////////////////////////////////////////////////////

void DepthCamera::convertDepthToMat_16UC1(
	const xn::DepthMetaData &depthMetaData, cv::Mat &kinectDepthOut)
{
	XnUInt16 depthWidth = depthMetaData.XRes();
	XnUInt16 depthHeight = depthMetaData.YRes();

	const XnDepthPixel *depth = depthMetaData.Data();
	ushort *out = (ushort*)kinectDepthOut.data;

	for (unsigned int x = 0, y = 0; y < depthHeight; ++y)
		for (x = 0; x < depthWidth; ++x, ++depth, ++out)
			*out = *depth;
}

////////////////////////////////////////////////////////////////////////////////

void DepthCamera::convertDepthToMat_8UC3(
	const xn::DepthMetaData &depthMetaData, cv::Mat &depthImage)
{
	XnUInt16 depthWidth = depthMetaData.XRes();
	XnUInt16 depthHeight = depthMetaData.YRes();

	const XnDepthPixel* depth = depthMetaData.Data();
	uchar *out = (uchar*)depthImage.data;

	for (unsigned int x = 0, y = 0; y < depthHeight; ++y)
		for (x = 0; x < depthWidth; ++x, ++depth, out += 3)
		{
			out[0] = *depth & 0x00FF;
			out[1] = *depth >> 8;
			out[2] = 0;
		}
}

////////////////////////////////////////////////////////////////////////////////

void DepthCamera::convertRGBToMat(const xn::ImageMetaData &imageMetaData,
								  cv::Mat &rgbImage)
{
	XnUInt16 width = imageMetaData.XRes();
	XnUInt16 height = imageMetaData.YRes();

	const XnRGB24Pixel *rgb = imageMetaData.RGB24Data();
	uchar *out = (uchar*)rgbImage.data;

	for (unsigned int x = 0, y = 0; y < height; ++y)
		for (x = 0; x < width; ++x, ++rgb, out += 3)
		{
			out[2] = rgb->nRed;
			out[1] = rgb->nGreen;
			out[0] = rgb->nBlue;
		}
}
