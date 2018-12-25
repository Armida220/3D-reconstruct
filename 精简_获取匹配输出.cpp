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
#include "GLFW/glfw3.h"
#include <time.h>
#include <omp.h>
#define SAVE_IMG
#define SAVE_Point
#define GL_DISPLAY
double yaw, pitch, lastX, lastY; int ml;
static void on_mouse_button(GLFWwindow * win, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) ml = action == GLFW_PRESS;
}
static double clamp(double val, double lo, double hi) { return val < lo ? lo : val > hi ? hi : val; }
static void on_cursor_pos(GLFWwindow * win, double x, double y)
{
	if (ml)
	{
		yaw = clamp(yaw - (x - lastX), -120, 120);
		pitch = clamp(pitch + (y - lastY), -80, 80);
	}
	lastX = x;
	lastY = y;
}


using namespace cv;
using namespace std;
using namespace pcl;

IMultiSourceFrameReader* m_pMultiFrameReader = nullptr;//������Դ���ݶ�ȡ������Kinect��ȡ�Ķ�Դ�����ɸö�ȡ����ȡ

//
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

//int savecoorandcolor(string file_name, vector<Point3DRGBAXYZ> &points, int depthheight, int depthwidth)
//{
//	ofstream corandcolor(file_name);
//	corandcolor << "ply"
//		<< "\n" << "format ascii 1.0"
//		<< "\n" << "comment PCL generated"
//		<< "\n" << "element vertex " << depthheight*depthwidth
//		<< "\n" << "property float x"
//		<< "\n" << "property float y"
//		<< "\n" << "property float z"
//		<< "\n" << "property uchar red"
//		<< "\n" << "property uchar green"
//		<< "\n" << "property uchar blue"
//		<< "\n" << "property uchar alpha"
//		<< "\n" << "element camera 1"
//		<< "\n" << "property float view_px"
//		<< "\n" << "property float view_py"
//		<< "\n" << "property float view_pz"
//		<< "\n" << "property float x_axisx"
//		<< "\n" << "property float x_axisy"
//		<< "\n" << "property float x_axisz"
//		<< "\n" << "property float y_axisx"
//		<< "\n" << "property float y_axisy"
//		<< "\n" << "property float y_axisz"
//		<< "\n" << "property float z_axisx"
//		<< "\n" << "property float z_axisy"
//		<< "\n" << "property float z_axisz"
//		<< "\n" << "property float focal"
//		<< "\n" << "property float scalex"
//		<< "\n" << "property float scaley"
//		<< "\n" << "property float centerx"
//		<< "\n" << "property float centery"
//		<< "\n" << "property int viewportx"
//		<< "\n" << "property int viewporty"
//		<< "\n" << "property float k1"
//		<< "\n" << "property float k2"
//		<< "\n" << "end_header" << endl;
//#pragma omp parallel 
//	for (int i = 0; i < depthheight*depthwidth; ++i)
//	{
//		//Mat_<float> c = structure.row(i);
//		//c /= c(3);	//������꣬��Ҫ�������һ��Ԫ�ز�������������ֵ
//		//corandcolor << Point3f(c(0), c(1), c(2)) << colors[i] << " "<< 255 << endl;
//		corandcolor << (double)points[i].x << " " << (double)points[i].y << " " << (double)points[i].z << " " <<
//			(double)points[i].r << " " << (double)points[i].g << " " << (double)points[i].b << " " << (double)points[i].a << endl;
//	}
//	corandcolor << "0 0 0 1 0 0 0 1 0 0 0 1 0 0 0 0 0 " << depthheight*depthwidth << " 1 0 0" << endl;
//	return 0;
//}


int main()
{
	//��ʾӳ��ͼ��
#ifdef GL_DISPLAY
	/// [gl display]
	// Open a GLFW window to display our output
	int a = glfwInit();
	GLFWwindow * win = glfwCreateWindow(1024, 768, "gl_win", nullptr, nullptr);
	glfwSetCursorPosCallback(win, on_cursor_pos);
	glfwSetMouseButtonCallback(win, on_mouse_button);
	glfwMakeContextCurrent(win);
#endif

	//��ȡKinect�豸
	IKinectSensor* m_pKinectSensor;
	HRESULT hr;
	hr = GetDefaultKinectSensor(&m_pKinectSensor);
	//IMultiSourceFrameReader* m_pMultiFrameReader = nullptr;//������Դ���ݶ�ȡ������Kinect��ȡ�Ķ�Դ�����ɸö�ȡ����ȡ
	//����Kinect2.0
	ConnectionKin(m_pKinectSensor, hr);

	//�Ӷ�ȡ���зֱ��ȡ2�ָ�ʽ������
	IColorFrameReference* m_pColorFrameReference = nullptr;
	IDepthFrameReference* m_pDepthFrameReference = nullptr;  //2�ָ�ʽ����Դ֡����
	IColorFrame* m_pColorFrame = nullptr;
	IDepthFrame* m_pDepthFrame = nullptr;  //2�ָ�ʽ����Դ֡

	//Ԥ��2�ָ�ʽͼƬ		PS���������Ϊ4ͨ����ͼ��Kinect������ֻ����BGRA�ĸ�ʽ���
	Mat i_rgb(1080, 1920, CV_8UC4);
	Mat i_depth(424, 512, CV_8UC1);
	//��ʼ����Դ����֡
	UINT16 *depthData = new UINT16[424 * 512];
	IMultiSourceFrame* m_pMultiFrame = nullptr;

	int fps = 0;
	int saveline = 0;
	while (true)
	{
		clock_t timeStart;
		timeStart = clock();
		cout << timeStart << endl;

		//��ȡ�µĶ�Դ����֡
		hr = m_pMultiFrameReader->AcquireLatestFrame(&m_pMultiFrame);//����ȡַ,Ŀ��ʹ����m_pMultiFrame�ĵ�ַΪ��֡�ĵ�ַ��
		if (FAILED(hr) || !m_pMultiFrame)
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

		clock_t timegainRGB = clock();
		cout << "��ȡRGB��ʱ" << timegainRGB - timeStart << endl;
		cout << "����ȡRGB�ܺ�ʱ" << timegainRGB - timeStart << endl;

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

		clock_t timegainDepth = clock();
		cout << "��ȡDepth��ʱ" << timegainDepth - timegainRGB << endl;
		cout << "����ȡDepth�ܺ�ʱ" << timegainDepth - timeStart << endl;

		//��ʾ
		namedWindow("RGBpic", CV_WINDOW_NORMAL);
		imshow("RGBpic", i_rgb);
		namedWindow("Depthpic", CV_WINDOW_NORMAL);
		imshow("Depthpic", i_depth);
		if (waitKey(1) == VK_ESCAPE)
			break;

		clock_t timeshow = clock();
		cout << "ͼ����ʾ��ʱ" << timeshow - timegainDepth << endl;
		cout << "����ʾͼ���ܺ�ʱ" << timeshow - timeStart << endl;

		//���ͼӳ�䵽��ɫͼ
		//��ȡ����ӳ����
		ICoordinateMapper* m_pCoordinateMapper;
		ColorSpacePoint* m_pColorCoordinates = new ColorSpacePoint[512 * 424];//������ɫ�ռ�����飬����֮���ӳ�䣻
		CameraSpacePoint* m_pCameraCoordinates = new CameraSpacePoint[512 * 424];
		hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
		//����ӳ����ȵ㵽��ɫ����ϵ�£�m_pColorCoordinates��ÿ�����ɫ��������µĵ㺬�������Ϣ
		hr = m_pCoordinateMapper->MapDepthFrameToColorSpace(512 * 424, depthData, 512 * 424, m_pColorCoordinates);
		Mat i_DepthToRgb(424, 512, CV_8UC4);
		//pcl::PointXYZRGBA *pclpoint;
		const int depthheight = 424, depthwidth = 512;
		const int sum = depthheight*depthwidth;
		vector<Point3DRGBAXYZ> points(sum);
		//ע�⣺424Ϊ�У�512Ϊ�У����ϡ�,���ָ�ǡ���,�С����������һ������ôд��
		clock_t timepremap = clock();
		cout << "premap��ʱ" << timepremap - timeshow << endl;
		cout << "��premap�ܺ�ʱ" << timepremap - timeStart << endl;

		if (SUCCEEDED(hr))
		{
			for (int i = 0; i < 424 * 512; i++)
			{
				ColorSpacePoint p = m_pColorCoordinates[i];//��������ǻ�ȡ��Щ�ڲ�ɫ�������ϵ����ȵ������X��Y
				if (p.X != -numeric_limits<float>::infinity() && p.Y != -numeric_limits<float>::infinity())
				{
					int colorX = static_cast<int>(p.X + 0.5f);//������������
					int colorY = static_cast<int>(p.Y + 0.5f);
					//static_cast<int>ǿ��ת������
					if ((colorX >= 0 && colorX < 1920) && (colorY >= 0 && colorY < 1080))
					{

						i_DepthToRgb.data[i * 4] = i_rgb.data[(colorY * 1920 + colorX) * 4];
						i_DepthToRgb.data[i * 4 + 1] = i_rgb.data[(colorY * 1920 + colorX) * 4 + 1];
						i_DepthToRgb.data[i * 4 + 2] = i_rgb.data[(colorY * 1920 + colorX) * 4 + 2];
						//�������߷ֱ�ΪRGB����ͨ����������BGRͨ��������Ϊ͸����
						i_DepthToRgb.data[i * 4 + 3] = i_rgb.data[(colorY * 1920 + colorX) * 4 + 3];
						points[i].r = i_DepthToRgb.data[i * 4];
						points[i].g = i_DepthToRgb.data[i * 4 + 1];
						points[i].b = i_DepthToRgb.data[i * 4 + 2];
						points[i].a = i_DepthToRgb.data[i * 4 + 3];
					
					}
				}
			}
		}
		
		clock_t timecoloremap = clock();
		cout << "coloremap��ʱ" << timecoloremap - timepremap << endl;
		cout << "��coloremap�ܺ�ʱ" << timecoloremap - timeStart << endl;
		
		hr = m_pCoordinateMapper->MapDepthFrameToCameraSpace(512 * 424, depthData, 512 * 424, m_pCameraCoordinates);
		//ӳ����ȵ㵽�������ϵ��m_pCameraCoordinatesΪ�������ϵ�µĵ㣬���Ҿ��������Ϣ
		if (SUCCEEDED(hr))
		{
			for (int i = 0; i < 512 * 424; i++)
			{
				CameraSpacePoint p2 = m_pCameraCoordinates[i];
				if (p2.X != -std::numeric_limits<float>::infinity() && p2.Y != -std::numeric_limits<float>::infinity() && p2.Z != -std::numeric_limits<float>::infinity())
				{
					points[i].x = static_cast<float>(p2.X);
					points[i].y = static_cast<float>(p2.Y);
					points[i].z = static_cast<float>(p2.Z);
				}
#ifdef GL_DISPLAY
				//cout << "x: " << cameraX << "y: " << cameraY << "z: " << cameraZ << endl;
				GLubyte *rgb = new GLubyte();
				rgb[2] = i_DepthToRgb.data[i * 4 + 0];
				rgb[1] = i_DepthToRgb.data[i * 4 + 1];
				rgb[0] = i_DepthToRgb.data[i * 4 + 2];
				// ��ʾ��
				glColor3ubv(rgb);
				glVertex3f(points[i].x, points[i].y, points[i].z);
#endif
			}
		}

		clock_t timecameraemap = clock();
		cout << "cameraemap��ʱ" << timecameraemap - timecoloremap << endl;
		cout << "��cameraemap�ܺ�ʱ" << timecameraemap - timeStart << endl;
		//cout << "<<------------------------------------------------->>" << endl;
		//cout << endl;

//#ifdef SAVE_Point
//		fps++;
//		if (fps % 1 == 0)
//		{
//			saveline++;
//			string PCLsaveadd = "C:/Users/Zhihong MA/Desktop/KinectPointdata/PLY/Omp";
//			ostringstream PCLsaveAdd;
//			PCLsaveAdd << PCLsaveadd << "PCLpoint" << saveline << ".ply";
//			savecoorandcolor(PCLsaveAdd.str(), points, depthheight, depthwidth);
//		}
//
//#endif
		clock_t timesave = clock();
		cout << "save��ʱ" << timesave - timecameraemap << endl;
		cout << "��save�ܺ�ʱ" << timesave - timeStart << endl;
		cout << "<<------------------------------------------------->>" << endl;
		cout << endl;

		imshow("DepthToRgbpic", i_DepthToRgb);
		if (waitKey(1) == VK_ESCAPE)
			break;


#ifdef GL_DISPLAY		
		glfwPollEvents();
		// Set up a perspective transform in a space that we can rotate by clicking and dragging the mouse
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60, (float)1280 / 960, 0.01f, 20.0f);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(0, 0, 0, 0, 0, 1, 0, -1, 0);
		glTranslatef(0, 0, +0.5f);
		glRotated(pitch, 1, 0, 0);
		glRotated(yaw, 0, 1, 0);
		glTranslatef(0, 0, -0.5f);

		// We will render our depth data as a set of points in 3D space
		glPointSize(2);
		glEnable(GL_DEPTH_TEST);
		glBegin(GL_POINTS);
#endif

#ifdef GL_DISPLAY
		glEnd();
		glfwSwapBuffers(win);
#endif

		// �ͷ���Դ
		SafeRelease(m_pColorFrame);
		SafeRelease(m_pDepthFrame);
		SafeRelease(m_pColorFrameReference);
		SafeRelease(m_pDepthFrameReference);
		SafeRelease(m_pMultiFrame);
		//SafeRelease(m_pCoordinateMapper);
	}

	//�ر��豸�ʹ���	
	destroyAllWindows();
	m_pKinectSensor->Close();
	return 0;
}



