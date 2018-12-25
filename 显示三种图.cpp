#include <stdio.h>
#include <Kinect.h>
#include <Windows.h>
#include <highgui.h>
#include <opencv2/core/core.hpp>  
#include <opencv2/opencv.hpp>
#include <cv.h>
#include <iostream>
#include "PCllibrary.h"
#include <GL/glut.h>
//#include <GLFW/glfw3.h>
#define GL_DISPLAY

using namespace cv;
using namespace std;

IMultiSourceFrameReader* m_pMultiFrameReader = nullptr;//������Դ���ݶ�ȡ������Kinect��ȡ�Ķ�Դ�����ɸö�ȡ����ȡ


// ��ȫ�ͷ�ָ��
template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
	if (pInterfaceToRelease != NULL)
	{
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}

//��ȡ����Kinect2.0
int ConnectionKin(IKinectSensor* m_pKinectSensor, HRESULT hr)
{
	if (FAILED(hr))
	{
		cout << "Can't find the Kinect" << endl;
		return hr;
	}
	if (m_pKinectSensor)
	{
		hr = m_pKinectSensor->Open();
		if (SUCCEEDED(hr))
		{
			//��ȡ����ȡ��Դ��RGB�������⣬��ȣ�����,
			hr = m_pKinectSensor->OpenMultiSourceFrameReader(
				FrameSourceTypes::FrameSourceTypes_Color |
				FrameSourceTypes::FrameSourceTypes_Infrared |
				FrameSourceTypes::FrameSourceTypes_Depth,
				&m_pMultiFrameReader);
		}
	}
	if (!m_pKinectSensor || FAILED(hr))
	{
		cout << "Warning: No Connection!/NO Source" << endl;
		return E_FAIL;
	}
	return 0;
}


int main()
{

	//��ȡKinect�豸
	IKinectSensor* m_pKinectSensor;
	HRESULT hr;
	hr = GetDefaultKinectSensor(&m_pKinectSensor);
	//IMultiSourceFrameReader* m_pMultiFrameReader = nullptr;//������Դ���ݶ�ȡ������Kinect��ȡ�Ķ�Դ�����ɸö�ȡ����ȡ
	//����Kinect2.0
	ConnectionKin(m_pKinectSensor, hr);

	//�Ӷ�ȡ���зֱ��ȡ���ָ�ʽ������
	IColorFrameReference* m_pColorFrameReference = nullptr;
	IInfraredFrameReference* m_pInfraredFrameReference = nullptr;
	IDepthFrameReference* m_pDepthFrameReference = nullptr;  //���ָ�ʽ����Դ֡����
	IColorFrame* m_pColorFrame = nullptr;
	IInfraredFrame* m_pInfraredFrame = nullptr;
	IDepthFrame* m_pDepthFrame = nullptr;  //���ָ�ʽ����Դ֡

	//Ԥ�����ָ�ʽͼƬ		PS���������Ϊ4ͨ����ͼ��Kinect������ֻ����BGRA�ĸ�ʽ���
	Mat i_rgb(1080, 1920, CV_8UC4);
	Mat i_depth(424, 512, CV_8UC1);
	Mat i_infrared(424, 512, CV_16UC1);

	//��ʼ����Դ����֡
	UINT16 *depthData = new UINT16[424 * 512];
	IMultiSourceFrame* m_pMultiFrame = nullptr;
	
	while (true)
	{
		//��ȡ�µĶ�Դ����֡
		hr = m_pMultiFrameReader->AcquireLatestFrame(&m_pMultiFrame);//����ȡַ,Ŀ��ʹ����m_pMultiFrame�ĵ�ַΪ��֡�ĵ�ַ��
		if (FAILED(hr)||!m_pMultiFrame)
		{
			//cout << "Can't get the latest frame!" << endl;
			continue;
		}
		//��ȡRGB����Դ��������ͼ��
		if (SUCCEEDED(hr))
		{
			hr = m_pMultiFrame->get_ColorFrameReference(&m_pColorFrameReference);
			if (SUCCEEDED(hr))
			{
				//����RGB���ݵ�ͼƬ
				hr = m_pColorFrameReference->AcquireFrame(&m_pColorFrame);
				UINT nColorBufferSize = 1920 * 1080 * 4; //���û����С
				if (SUCCEEDED(hr))
				{
					hr = m_pColorFrame->CopyConvertedFrameDataToArray(nColorBufferSize,
						reinterpret_cast<BYTE*>(i_rgb.data), ColorImageFormat::ColorImageFormat_Bgra);//CopyConvertedFrameDataToArray(�������������ʽ)
				}
			}
		}



		//��ȡ���Դ��������ͼ��
		if (SUCCEEDED(hr))
		{
			hr = m_pMultiFrame->get_DepthFrameReference(&m_pDepthFrameReference);
			if (SUCCEEDED(hr))
			{
				hr = m_pDepthFrameReference->AcquireFrame(&m_pDepthFrame);
				if (SUCCEEDED(hr))
				{
					hr = m_pDepthFrame->CopyFrameDataToArray(424 * 512, depthData);
					for (int i = 0; i < 512 * 424; ++i)
					{
						//0-255���ͼ��Ϊ����ʾ������ֻȡ������ݵĵ�8λ��
						BYTE intensity = static_cast<BYTE>(depthData[i] % 256);
						reinterpret_cast<BYTE*>(i_depth.data)[i] = intensity;
					}
					//hr = m_pDepthFrame->CopyFrameDataToArray(424 * 512, reinterpret_cast<UINT16*>(i_depth.data));
				}
			}
		}


		//��ȡ������Դ
		if (SUCCEEDED(hr))
		{
			hr = m_pMultiFrame->get_InfraredFrameReference(&m_pInfraredFrameReference);
			if (SUCCEEDED(hr))
			{
				hr = m_pInfraredFrameReference->AcquireFrame(&m_pInfraredFrame);
				if (SUCCEEDED(hr))
				{
					hr = m_pInfraredFrame->CopyFrameDataToArray(424 * 512, reinterpret_cast<UINT16*>(i_infrared.data));
				}
			}

		}
		
		//��ʾ
		namedWindow("RGBpic", CV_WINDOW_NORMAL);
		imshow("RGBpic", i_rgb);

		namedWindow("Depthpic", CV_WINDOW_NORMAL);
		imshow("Depthpic", i_depth);
		
		namedWindow("Infraredpic", CV_WINDOW_NORMAL);
		imshow("Infraredpic", i_infrared);
		if (waitKey(1) == VK_ESCAPE)
			break;


		//���ͼӳ�䵽��ɫͼ
		//��ȡ����ӳ����
		ICoordinateMapper* m_pCoordinateMapper;
		ColorSpacePoint* m_pColorCoordinates = new ColorSpacePoint[512 * 424];//������ɫ�ռ�����飬����֮���ӳ�䣻
		CameraSpacePoint* m_pCameraCoordinates = new CameraSpacePoint[512 * 424];
		hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
		//����ӳ��
		hr = m_pCoordinateMapper->MapDepthFrameToColorSpace(512 * 424, depthData, 512 * 424, m_pColorCoordinates);
		Mat i_DepthToRgb(424, 512, CV_8UC4);
		//ע�⣺424λ�У�512Ϊ�У����������ϡ�*�����Ŷ����С��У���������ֻȡ��˺�Ľ���������ϡ�,���ָ�ǡ���,�С����������һ������ôд��
		if (SUCCEEDED(hr))
		{
			for (int i = 0; i < 424 * 512; i++)
			{
				ColorSpacePoint p = m_pColorCoordinates[i];
				if (p.X != -numeric_limits<float>::infinity() && p.Y != -numeric_limits<float>::infinity())
				{
					int colorX = static_cast<int>(p.X + 0.5f);
					int colorY = static_cast<int>(p.Y + 0.5f);
					//static_cast<int>ǿ��ת������
					if ((colorX >= 0 && colorX < 1920) && (colorY >= 0 && colorY < 1080))
					{
						i_DepthToRgb.data[i * 4] = i_rgb.data[(colorY * 1920 + colorX) * 4];
						i_DepthToRgb.data[i * 4+1] = i_rgb.data[(colorY * 1920 + colorX) * 4+1];
						i_DepthToRgb.data[i * 4+2] = i_rgb.data[(colorY * 1920 + colorX) * 4+2];
						i_DepthToRgb.data[i * 4+3] = i_rgb.data[(colorY * 1920 + colorX) * 4+3];
					}
				}
			}
		}
		imshow("DepthToRgbpic", i_DepthToRgb);
		if (waitKey(1) == VK_ESCAPE)
			break;

//#ifdef GL_DISPLAY
//		/// [gl display]
//		// Open a GLFW window to display our output
//		int a = glfwInit();
//		GLFWwindow * win = glfwCreateWindow(1024, 768, "gl_win", nullptr, nullptr);
//		glfwSetCursorPosCallback(win, on_cursor_pos);
//		glfwSetMouseButtonCallback(win, on_mouse_button);
//		glfwMakeContextCurrent(win);
//#endif
//
//#ifdef GL_DISPLAY
//		glfwPollEvents();
//		// Set up a perspective transform in a space that we can rotate by clicking and dragging the mouse
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//		glMatrixMode(GL_PROJECTION);
//		glLoadIdentity();
//		gluPerspective(60, (float)1280 / 960, 0.01f, 20.0f);
//		glMatrixMode(GL_MODELVIEW);
//		glLoadIdentity();
//		gluLookAt(0, 0, 0, 0, 0, 1, 0, -1, 0);
//		glTranslatef(0, 0, +0.5f);
//		glRotated(pitch, 1, 0, 0);
//		glRotated(yaw, 0, 1, 0);
//		glTranslatef(0, 0, -0.5f);
//
//		// We will render our depth data as a set of points in 3D space
//		glPointSize(2);
//		glEnable(GL_DEPTH_TEST);
//		glBegin(GL_POINTS);
//#endif

		//���ͼ��ӳ�䵽����ռ�
		if (SUCCEEDED(hr))
		{
			hr = m_pCoordinateMapper->MapDepthFrameToCameraSpace(512 * 424, depthData, 512 * 424, m_pCameraCoordinates);
			if (SUCCEEDED(hr))
			{
				for (int i = 0; i < 512 * 424; i++)
				{
					CameraSpacePoint p = m_pCameraCoordinates[i];
					if (p.X != -std::numeric_limits<float>::infinity() && p.Y != -std::numeric_limits<float>::infinity() && p.Z != -std::numeric_limits<float>::infinity())
					{
						float cameraX = static_cast<float>(p.X);
						float cameraY = static_cast<float>(p.Y);
						float cameraZ = static_cast<float>(p.Z);

						//cout << "x: " << cameraX << "y: " << cameraY << "z: " << cameraZ << endl;
						GLubyte *rgb = new GLubyte();
						rgb[2] = i_DepthToRgb.data[i * 4 + 0];
						rgb[1] = i_DepthToRgb.data[i * 4 + 1];
						rgb[0] = i_DepthToRgb.data[i * 4 + 2];
						// ��ʾ��
						glColor3ubv(rgb);
						glVertex3f(cameraX, -cameraY, cameraZ);
					}
				}
			}
		}
		


		//namedWindow("ӳ�����", CV_WINDOW_NORMAL);
		//imshow("ӳ�����", rgb);
		//if (waitKey(1) == VK_ESCAPE)
		//	break;


		// �ͷ���Դ
		SafeRelease(m_pColorFrame);
		SafeRelease(m_pDepthFrame);
		SafeRelease(m_pInfraredFrame);
		SafeRelease(m_pColorFrameReference);
		SafeRelease(m_pDepthFrameReference);
		SafeRelease(m_pInfraredFrameReference);
		SafeRelease(m_pMultiFrame);
		SafeRelease(m_pCoordinateMapper);
	}

	//�ر��豸�ʹ���	
	destroyAllWindows();
	m_pKinectSensor->Close();
	return 0;
}
