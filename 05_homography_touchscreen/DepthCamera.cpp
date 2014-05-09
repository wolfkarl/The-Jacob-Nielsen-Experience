////////////////////////////////////////////////////////////////////////////////
//
// Class grabbing frames from the depth camera
//
// Authors: Stephan Richter (2011) and Patrick Lühne (2012)
//
////////////////////////////////////////////////////////////////////////////////

#include "DepthCamera.h"

#include <iostream>

#include "DepthCameraException.h"

////////////////////////////////////////////////////////////////////////////////
//
// Forward declarations
//
////////////////////////////////////////////////////////////////////////////////

void XN_CALLBACK_TYPE Forward_User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie);
void XN_CALLBACK_TYPE Forward_User_LostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie);
void XN_CALLBACK_TYPE Forward_UserPose_PoseDetected(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId, void* pCookie);
void XN_CALLBACK_TYPE Forward_UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie);
void XN_CALLBACK_TYPE Forward_UserCalibration_CalibrationComplete(xn::SkeletonCapability& capability, XnUserID nId, XnCalibrationStatus calibrationError, void* pCookie);

////////////////////////////////////////////////////////////////////////////////
//
// DepthCamera
//
////////////////////////////////////////////////////////////////////////////////

const std::string DepthCamera::s_sampleXMLPath = "SamplesConfig.xml";

////////////////////////////////////////////////////////////////////////////////

DepthCamera *DepthCamera::s_instance = NULL;

////////////////////////////////////////////////////////////////////////////////

DepthCamera *DepthCamera::instance()
{
	if (!s_instance)
		s_instance = new DepthCamera;

	return s_instance;
}

////////////////////////////////////////////////////////////////////////////////

bool DepthCamera::hasInstance()
{
	return s_instance;
}

////////////////////////////////////////////////////////////////////////////////

void DepthCamera::deleteInstance()
{
	if (!s_instance)
		return;

	delete s_instance;
	s_instance = NULL;
}

////////////////////////////////////////////////////////////////////////////////

DepthCamera::DepthCamera()
{
	xn::EnumerationErrors errors;
	XnStatus status = XN_STATUS_OK;

	// Initialize the camera settings
	status = m_context.InitFromXmlFile(s_sampleXMLPath.c_str(), m_scriptNode,
									   &errors);

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

	// Find the user generator node providing the skeleton
	status = m_context.FindExistingNode(XN_NODE_TYPE_USER, m_userGenerator);

	if (status != XN_STATUS_OK)
		status = m_userGenerator.Create(m_context);

	XnCallbackHandle hUserCallbacks, hCalibrationCallbacks, hPoseCallbacks;
	m_userGenerator.RegisterUserCallbacks(Forward_User_NewUser, Forward_User_LostUser, NULL, hUserCallbacks);
	m_userGenerator.GetSkeletonCap().RegisterToCalibrationStart(Forward_UserCalibration_CalibrationStart, NULL, hCalibrationCallbacks);
	m_userGenerator.GetSkeletonCap().RegisterToCalibrationComplete(Forward_UserCalibration_CalibrationComplete, NULL, hCalibrationCallbacks);
	m_userGenerator.GetPoseDetectionCap().RegisterToPoseDetected(Forward_UserPose_PoseDetected, NULL, hPoseCallbacks);
	m_userGenerator.GetSkeletonCap().GetCalibrationPose(m_calibrationPose);
	m_userGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_UPPER);

	if (status != XN_STATUS_OK)
		throwException("Finding the user generator failed", status);

	// Start generating frames
	status = m_context.StartGeneratingAll();

	if (status != XN_STATUS_OK)
		throwException("Starting generating failed", status);

	m_depthGenerator.GetAlternativeViewPointCap().SetViewPoint(m_imageGenerator);
}

////////////////////////////////////////////////////////////////////////////////

DepthCamera::~DepthCamera()
{
	m_context.Release();
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
	XnUserID userIDs[15];
	XnUInt16 numberOfUsers = 15;

	if (status != XN_STATUS_OK)
		return status;

	// Process the data
	m_depthGenerator.GetMetaData(m_depthMetaData);
	m_imageGenerator.GetMetaData(m_imageMetaData);
	m_userGenerator.GetUsers(userIDs, numberOfUsers);

	// convert depth
	if (depthType == CV_16UC1)
		convertDepthToMat_16UC1(m_depthMetaData, depthImage);
	else
		convertDepthToMat_8UC3(m_depthMetaData, depthImage);

	// convert rgb
	convertRGBToMat(m_imageMetaData, rgbImage);

	if (numberOfUsers > 0)
		for (XnUInt16 i = 0; i < numberOfUsers; i++)
		{
			if (m_userGenerator.GetSkeletonCap().IsTracking(userIDs[i]))
			{
				showSkeleton(rgbImage, userIDs[i]);

				onSkeletonTracked(userIDs[i]);
			}
			else if (m_userGenerator.GetSkeletonCap().IsCalibrating(userIDs[i]))
				std::cout << "[Info] Calibrating." << std::endl;
		}

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

////////////////////////////////////////////////////////////////////////////////

void DepthCamera::showSkeleton(cv::Mat &rgbImage, XnUserID userID)
{
	drawLimb(rgbImage, userID, XN_SKEL_HEAD, XN_SKEL_NECK);
	drawLimb(rgbImage, userID, XN_SKEL_NECK, XN_SKEL_LEFT_SHOULDER);
	drawLimb(rgbImage, userID, XN_SKEL_LEFT_SHOULDER, XN_SKEL_LEFT_ELBOW);
	drawLimb(rgbImage, userID, XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND);

	drawLimb(rgbImage, userID, XN_SKEL_NECK, XN_SKEL_RIGHT_SHOULDER);
	drawLimb(rgbImage, userID, XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_ELBOW);
	drawLimb(rgbImage, userID, XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND);

	drawLimb(rgbImage, userID, XN_SKEL_LEFT_SHOULDER, XN_SKEL_TORSO);
	drawLimb(rgbImage, userID, XN_SKEL_RIGHT_SHOULDER, XN_SKEL_TORSO);
}

////////////////////////////////////////////////////////////////////////////////

void DepthCamera::drawLimb(cv::Mat &rgbImage, XnUserID userID,
						   XnSkeletonJoint eJoint1, XnSkeletonJoint eJoint2)
{
	XnSkeletonJointPosition joint1, joint2;
	m_userGenerator.GetSkeletonCap().GetSkeletonJointPosition(userID, eJoint1, joint1);
	m_userGenerator.GetSkeletonCap().GetSkeletonJointPosition(userID, eJoint2, joint2);

	if (joint1.fConfidence < 0.5 && joint2.fConfidence < 0.5)
		return;

	// convert coordinates
	XnPoint3D point[2];
	point[0] = joint1.position;
	point[1] = joint2.position;

	m_depthGenerator.ConvertRealWorldToProjective(2, point, point);

	// circles for joints
	cv::circle(rgbImage, cv::Point(point[0].X, point[0].Y), 5, cv::Scalar::all(255));
	cv::circle(rgbImage, cv::Point(point[1].X, point[1].Y), 5, cv::Scalar::all(255));
	cv::line(rgbImage, cv::Point(point[0].X, point[0].Y), cv::Point(point[1].X, point[1].Y), cv::Scalar::all(255));
}

////////////////////////////////////////////////////////////////////////////////

xn::DepthGenerator &DepthCamera::depthGenerator()
{
	return m_depthGenerator;
}

////////////////////////////////////////////////////////////////////////////////

xn::ImageGenerator &DepthCamera::imageGenerator()
{
	return m_imageGenerator;
}

////////////////////////////////////////////////////////////////////////////////

xn::UserGenerator &DepthCamera::userGenerator()
{
	return m_userGenerator;
}

////////////////////////////////////////////////////////////////////////////////

void XN_CALLBACK_TYPE Forward_User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
	DepthCamera::instance()->User_NewUser(generator, nId, pCookie);
}

////////////////////////////////////////////////////////////////////////////////

void DepthCamera::User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
	// New user found
	std::cout << "[Info] New user " << nId << " tracked." << std::endl;

	m_userGenerator.GetPoseDetectionCap().StartPoseDetection(m_calibrationPose, nId);
}

////////////////////////////////////////////////////////////////////////////////

// Callback: An existing user was lost
void XN_CALLBACK_TYPE Forward_User_LostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
	DepthCamera::instance()->User_LostUser(generator, nId, pCookie);
}

////////////////////////////////////////////////////////////////////////////////

void DepthCamera::User_LostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
	std::cout << "[Info] Lost user " << nId << "." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void XN_CALLBACK_TYPE Forward_UserPose_PoseDetected(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId, void* pCookie)
{
	DepthCamera::instance()->UserPose_PoseDetected(capability, strPose, nId, pCookie);
}

////////////////////////////////////////////////////////////////////////////////

void DepthCamera::UserPose_PoseDetected(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId, void* pCookie)
{
	std::cout << "[Info] Pose " << strPose << " detected for user " << nId
		<< std::endl;

	m_userGenerator.GetPoseDetectionCap().StopPoseDetection(nId);
	m_userGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}

////////////////////////////////////////////////////////////////////////////////

// Callback: Started calibration
void XN_CALLBACK_TYPE Forward_UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie)
{
	DepthCamera::instance()->UserCalibration_CalibrationStart(capability, nId, pCookie);
}

////////////////////////////////////////////////////////////////////////////////

void DepthCamera::UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie)
{
	std::cout << "[Info] Calibration started for user " << nId << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

// Callback: Finished calibration
void XN_CALLBACK_TYPE Forward_UserCalibration_CalibrationComplete(xn::SkeletonCapability& capability, XnUserID nId, XnCalibrationStatus calibrationError, void* pCookie)
{
	DepthCamera::instance()->UserCalibration_CalibrationComplete(capability, nId, calibrationError, pCookie);
}

////////////////////////////////////////////////////////////////////////////////

void DepthCamera::UserCalibration_CalibrationComplete(xn::SkeletonCapability& capability, XnUserID nId, XnCalibrationStatus calibrationError, void* pCookie)
{
	if (calibrationError == XN_CALIBRATION_STATUS_OK)
	{
		// Calibration succeeded
		std::cout << "[Info] Calibration completed for user " << nId
			<< std::endl;

		m_userGenerator.GetSkeletonCap().StartTracking(nId);
	}
	else
	{
		// Calibration failed
		std::cout << "[Info] Calibration failed for user " << nId << std::endl;

		m_userGenerator.GetPoseDetectionCap().StartPoseDetection(m_calibrationPose, nId);
	}
}
