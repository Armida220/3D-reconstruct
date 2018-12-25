#include <stdio.h>
#include <Kinect.h>
#include <Windows.h>
#include <highgui.h>
#include <opencv2/core/core.hpp>  
#include <opencv2/opencv.hpp>
#include <cv.h>
#include <iostream>
#include "PCllibrary.h"
//#include <GL/glut.h>
#include "GLFW/glfw3.h"
#include <time.h>
#include <vector>
#define SAVE_IMG
#define SAVE_Point
#define GL_DISPLAY
double yaw, pitch, lastX, lastY; int ml;


using namespace cv;
using namespace std;

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

void saveRGBDfile(string &file_name, vector<Point3DRGBAXYZ> points, int &depthheight, int &depthwidth)
{
	ofstream corandcolor;
	corandcolor.open(file_name, std::ios::out | std::ios::app);
	if (corandcolor.is_open())
	{
		for (auto c : points)
		{
			ostringstream outformate; 
			outformate << (double)c.r << " " << (double)c.g << " " << (double)c.b << " " << (double)c.depth << endl;
			string outstr= outformate.str();
			char outchar[100];
			strcpy(outchar,outstr.c_str());
			corandcolor.write(outchar,  strlen(outchar));
		}
		corandcolor.close();
	}
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
		int depthheight = 424, depthwidth = 512;
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
						points[i].depth = depthData[i];
					}
				}
			}
			clock_t timecoloremap = clock();
			cout << "coloremap��ʱ" << timecoloremap - timepremap << endl;
			cout << "��coloremap�ܺ�ʱ" << timecoloremap - timeStart << endl;
#ifdef SAVE_Point
		fps++;
		if (fps % 1 == 0)
		{
			saveline++;
			string RGBDsaveadd = "C:/Users/zhihong/Desktop/2/radius_test/5/";
			ostringstream RGBDsaveAdd;
			RGBDsaveAdd << RGBDsaveadd << "RGBDpoint" << saveline << ".rgbd";
			saveRGBDfile(RGBDsaveAdd.str(), points, depthheight, depthwidth);
		}
		clock_t timesave = clock();
		cout << "save��ʱ" << timesave-timecoloremap << endl;
		cout << "��save�ܺ�ʱ" << timesave - timeStart << endl;
		//cout << "<<------------------------------------------------->>" << endl;
		//cout << endl;
#endif
		}
		// �ͷ���Դ
		SafeRelease(m_pColorFrame);
		SafeRelease(m_pDepthFrame);
		SafeRelease(m_pColorFrameReference);
		SafeRelease(m_pDepthFrameReference);
		SafeRelease(m_pMultiFrame);
		//SafeRelease(m_pCoordinateMapper);
		clock_t timeend;
		timeend = clock();
		cout << "�ܺ�ʱ" << timeend - timeStart << endl;
		cout << "<<------------------------------------------------->>" << endl;
		cout << endl;
	}
	//�ر��豸�ʹ���	
	destroyAllWindows();
	m_pKinectSensor->Close();
	return 0;
}



