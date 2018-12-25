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
#include <thread>

#define SAVE_IMG
#define SAVE_Point
#define GL_DISPLAY


using namespace cv;
using namespace std;
using namespace pcl;

IMultiSourceFrameReader* m_pMultiFrameReader = nullptr;//������Դ���ݶ�ȡ������Kinect��ȡ�Ķ�Դ�����ɸö�ȡ����ȡ												  
template<class Interface> // ��ȫ�ͷ�ָ��
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

		Mat i_DepthToRgb(424, 512, CV_8UC4);
		//ע�⣺424Ϊ�У�512Ϊ�У����ϡ�,���ָ�ǡ���,�С����������һ������ôд��

		pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloud_outall(new pcl::PointCloud<pcl::PointXYZRGBA>);
		pcl::PointXYZRGBA pclpoint;
		clock_t timepremap = clock();
		cout << "premap��ʱ" << timepremap - timeshow << endl;
		cout << "��premap�ܺ�ʱ" << timepremap - timeStart << endl;

		hr = m_pCoordinateMapper->MapDepthFrameToColorSpace(512 * 424, depthData, 512 * 424, m_pColorCoordinates);
		if (SUCCEEDED(hr))
			hr = m_pCoordinateMapper->MapDepthFrameToCameraSpace(512 * 424, depthData, 512 * 424, m_pCameraCoordinates);
		//ӳ����ȵ㵽�������ϵ��m_pCameraCoordinatesΪ�������ϵ�µĵ㣬���Ҿ��������Ϣ
		
		if (SUCCEEDED(hr))
		{
			for (int i = 0; i < 424 * 512; i++)
			{
				ColorSpacePoint p = m_pColorCoordinates[i];//��������ǻ�ȡ��Щ�ڲ�ɫ�������ϵ����ȵ������X��Y
				CameraSpacePoint p2 = m_pCameraCoordinates[i];
				if (p.X != -numeric_limits<float>::infinity() && p.Y != -numeric_limits<float>::infinity()
					&& p2.X != -std::numeric_limits<float>::infinity() && p2.Y != -std::numeric_limits<float>::infinity() && p2.Z != -std::numeric_limits<float>::infinity())
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
						pclpoint.r = i_DepthToRgb.data[i * 4];
						pclpoint.g = i_DepthToRgb.data[i * 4 + 1];
						pclpoint.b = i_DepthToRgb.data[i * 4 + 2];
						pclpoint.a = i_DepthToRgb.data[i * 4 + 3];
						pclpoint.x = static_cast<float>(p2.X);
						pclpoint.y = static_cast<float>(p2.Y);
						pclpoint.z = static_cast<float>(p2.Z);
					}
				}
				cloud_outall->push_back(pclpoint);
			}
		}

		clock_t timemap = clock();
		cout << "map��ʱ" << timemap - timepremap << endl;
		cout << "��map�ܺ�ʱ" << timemap - timeStart << endl;

//#ifdef SAVE_Point
//		fps++;
//		if (fps % 1 == 0)
//		{
//			saveline++;
//			string PCLsaveadd = "C:/Users/Zhihong MA/Desktop/KinectPointdata/PLY/";
//			ostringstream PCLsaveAdd;
//			PCLsaveAdd << PCLsaveadd << "xPCLpoint" << saveline << ".pcd";
//			////savecoorandcolor(PCLsaveAdd.str(), points, depthheight, depthwidth);
//			pcl::io::savePCDFileBinary(PCLsaveAdd.str(), *cloud_outall);//����Ϊ�˱�֤�ٶȣ�ѡ������Ʊ������һ��Ҫ��PCD��ply���pcd���ܶ�
//		}
//
//#endif
		clock_t timesave = clock();
		cout << "save��ʱ" << timesave - timemap << endl;
		cout << "��save�ܺ�ʱ" << timesave - timeStart << endl;
		cout << "<<------------------------------------------------->>" << endl;

		imshow("DepthToRgbpic", i_DepthToRgb);
		if (waitKey(1) == VK_ESCAPE)
			break;

		// �ͷ���Դ
		SafeRelease(m_pColorFrame);
		SafeRelease(m_pDepthFrame);
		SafeRelease(m_pColorFrameReference);
		SafeRelease(m_pDepthFrameReference);
		SafeRelease(m_pMultiFrame);
		//SafeRelease(m_pCoordinateMapper);

		clock_t timeEnd = clock();
		cout << "Save��End��ʱ" << timeEnd - timesave<< endl;
		cout << "ѭ��һ���ܺ�ʱ" << timeEnd - timeStart << endl;
		cout << "<<------------------------------------------------->>" << endl;
		cout << endl;
	}

	//�ر��豸�ʹ���	
	destroyAllWindows();
	m_pKinectSensor->Close();
	return 0;
}



