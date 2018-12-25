#include "PCLlibrary.h"
#include <iostream>
#include <string>
#include <pcl/console/time.h>   // TicToc
#include <io.h>  
#include <direct.h> 
#include <stdio.h>
#include <Kinect.h>
#include <Windows.h>
#include <highgui.h>
#include <opencv2/core/core.hpp>  
#include <opencv2/opencv.hpp>
#include <cv.h>
#include <iostream>
#include <GL/glut.h>
#include "GLFW/glfw3.h"
#include <time.h>
#include <thread>




using namespace cv;
using namespace std;
using namespace pcl;

typedef pcl::PointCloud<pcl::PointXYZRGB>::Ptr PointXYZRGBPtr;
////��һ���ַ���������Ӣ�������֣��������ܻ������ģ���ȡָ��λ��������
//int getNumberInString(string istring, bool &hasnumbr)
//{
//	int number = 0;
//	string filterstring;
//	for (int i = istring.size(); i > istring.size() - 10 && istring[i] != '_'; i--)
//	{
//		filterstring += istring[i];
//	}
//	for (int i = filterstring.size(); i >0; i--)
//	{
//		if (filterstring[i] >= '0'&&filterstring[i] <= '9')
//		{
//			number = number * 10 + filterstring[i] - '0';
//		}
//	}
//	if (number == 0)
//		hasnumbr = false;
//	return number;
//}
//
////�����ļ�������ָ�����ִ�С��С��������
//template <class T>
//int reRangeFileName(vector<T>& files, vector<int> &SerialNumber)
//{
//	if (files.size() != SerialNumber.size())
//	{
//		cout << "The number of Files and Serial Number is wrong" << endl;
//		return -1;
//	}
//	for (int i = 0; i<files.size(); i++)
//	{
//		for (int j = i; j < files.size(); j++)
//		{
//			if (SerialNumber[i] >= SerialNumber[j])
//			{
//				//swap(SerialNumber[i], SerialNumber[j]);
//
//				int tmp;
//				tmp = SerialNumber[j];
//				SerialNumber[j] = SerialNumber[i];
//				SerialNumber[i] = tmp;
//				//swap(files[i], files[j]);
//				T tmpname;
//				tmpname = files[j];
//				files[j] = files[i];
//				files[i] = tmpname;
//			}
//		}
//
//	}
//	return 0;
//}
//
////��ȡָ���ļ������ض���ʽ���ļ�
//int GetAllFiles_CertainFormat(string path, vector<string>& files, string format)
//{
//	intptr_t hFile = 0;//_findnext��������Ϊintprt_t,����long���ͣ���intptr_tת����long���Ͷ�ʧ����
//	struct  _finddata_t fileinfo;
//	vector<int> SerialNumber;//���ڴ洢���
//	string serialnumber;//��������
//	bool hasnumber = true;//�����ж��ļ������Ƿ�������
//	int nonum = 0;//������û������ʱ����
//	string p;
//	if ((hFile = _findfirst(p.assign(path).append("/*" + format).c_str(), &fileinfo)) != -1)
//	{
//		do
//		{
//			if ((fileinfo.attrib & _A_SUBDIR))//�ж��Ƿ�Ϊ�ļ���
//			{
//				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, ".") != 0)
//				{
//					GetAllFiles_CertainFormat((p.assign(path).append("/")).append(fileinfo.name), files, format);
//
//				}
//			}
//			else
//			{
//				serialnumber.append(fileinfo.name);
//				int num = getNumberInString(serialnumber, hasnumber);
//				//cout << "number:" << num << endl;
//				if (hasnumber == true)
//					SerialNumber.push_back(num);
//				else
//				{
//					SerialNumber.push_back(nonum);
//				}
//				p.assign(path).append("/").append(fileinfo.name);
//				files.push_back(p);
//			}
//			nonum++;
//			serialnumber.clear();
//
//		} while (_findnext(hFile, &fileinfo) == 0);
//		_findclose(hFile);
//	}
//	reRangeFileName(files, SerialNumber);
//	cout << "-------------------------------------------------" << endl;
//	return 0;
//}
//
////class P_gray
////{
////public:
////	int x;
////	int y;
////	int depth;
////};


//Kinect2.0����Ա궨
int main()
{
	Mat i_rgb(1080, 1920, CV_8UC3);
	Mat i_depth(424, 512, CV_16UC1);
	/*i_rgb = imread("F:/1-DesktopFile_Pointget/1-DATA/2017.11.16/50_2/2016.11.16_2.jpg");
	i_depth = imread("F:/1-DesktopFile_Pointget/1-DATA/2017.11.16/200_2/2016.11.16_2.png", IMREAD_ANYDEPTH);*/
	i_rgb = imread("F:/1-DesktopFile_Pointget/1-DATA/2017.12.25/619_C_1/side_1.jpg");
	i_depth = imread("F:/1-DesktopFile_Pointget/1-DATA/2017.12.25/619_C_1/side_1.png", IMREAD_ANYDEPTH);
	
	imshow("RGB", i_rgb);
	imshow("Gray", i_depth);

	//�����ת���󣺽���������任��RGB���
	Eigen::Matrix3f RGB_inmatrix;
	//RGB����ڲξ���
	RGB_inmatrix<< 1032.709231,0.000000,981.428774,
		0.000000, 1030.974347, 538.918598,
		0.000000, 0.000000, 1.000000;
	//RGB�������ϵ��
	Eigen::Matrix3f distortion_RGB;
	distortion_RGB << 0.054988, -0.059714, -0.003015, -0.002555;

	//����������ڲξ���
	Eigen::Matrix3f Infrared_inmatrix;
	Infrared_inmatrix << 363.160085, 0.000000, 258.046828,
		0.000000, 363.064039, 209.335001,
		0.000000, 0.000000, 1.000000;
	//�������������ϵ��
	Eigen::Matrix3f distortion_Infrared;
	distortion_Infrared << 0.100285, -0.240013, -0.004493, -0.003895;

	//��ת�����ƽ������
	Eigen::Matrix3f R;
	Eigen::Vector3f T;
	R << 0.999993921369000, -0.00246100194499998, -0.00230707843499998,
		0.00246283082899992, 0.999996741455000, 0.000762005131000021,
		0.00230520838700001, -0.000767345323000019, 0.999996531067000;
	T << -40.3062703803215,
		27.7074518129964,
		-6.52067730622377;

	Eigen::Matrix3f Rcal;
	Rcal << RGB_inmatrix*R*Infrared_inmatrix.inverse();
	Eigen::Vector3f Tcal;
	Tcal << RGB_inmatrix*T;
	Tcal[0]=Tcal[0]*1.5;
	Tcal[1] = Tcal[1] * 0.1;
	cout << "��ת����\n" << Rcal << endl;
	cout << "ƽ��������\n" << Tcal << endl;

	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloud_outall(new pcl::PointCloud<pcl::PointXYZRGBA>);
	pcl::PointXYZRGBA pclpoint;
	

	Mat i_DepthToRgb(424, 512, CV_8UC4);//(y,x)

	for (int row = 0; row < 424; ++row)
	{
		for (int col = 0; col < 512; ++col)
		{
			/*unsigned short depthValue = p[row * 512 + col];*/
			unsigned short* p = (unsigned short*)i_depth.data;
			//unsigned short depthValue = i_depth.at<unsigned short>(Point(col, row));
			unsigned short depthValue = p[row * 512 + col];
			//cout << "depthValue:       " << depthValue << endl;
			if (depthValue != -std::numeric_limits<unsigned short>::infinity() && depthValue != 0 && depthValue != 65535 )
			{
				// ͶӰ����ɫͼ�ϵ�����
				Eigen::Vector3f uv_depth;
				uv_depth<<col, row,	1.0f;                            // !!!p_ir
				//Eigen::Vector3f uv_color = depthValue / 1000.f*R*uv_depth + T / 1000;   // !!!Z_rgb*p_rgb=R*Z_ir*p_ir+T; (����1000����Ϊ�˴Ӻ��ױ���)
				Eigen::Vector3f uv_color = Rcal*depthValue/1000.f*uv_depth - Tcal/1000.f;

				int X = static_cast<int>(uv_color[0] / uv_color[2] + 0.5f);             // !!!Z_rgb*p_rgb -> p_rgb,�˴��õ�x
				int Y = static_cast<int>(uv_color[1] / uv_color[2] + 0.5f);             // !!!Z_rgb*p_rgb -> p_rgb,�˴��õ�y
				float Z = static_cast<float>(uv_color[2] );
				//int Z = static_cast<int>(uv_color[2] + 0.5f);                           // !!!Z_rgb*p_rgb -> p_rgb,�˴��õ�z
			
				//cout << "X:       " << X << "     Y:      " << Y << "     Z:      " << Z << endl;

				if ((X >= 0 && X < 1920) && (Y >= 0 && Y < 1080))
				{
					//cout << "X:       " << X << "     Y:      " << Y << endl;
					/*i_DepthToRgb.data[i * 3] = i_rgb.data[3 * (Y * 1920 + X)];
					i_DepthToRgb.data[i * 3 + 1] = i_rgb.data[3 * (Y * 1920 + X) + 1];
					i_DepthToRgb.data[i * 3 + 2] = i_rgb.data[3 * (Y * 1920 + X) + 2];*/

					////���ַ����õ������ص�ֻ��ʾ�Ҷȣ�Ϊɶ����
					//i_DepthToRgb.at<Vec3b>(Point(col, row))[0] = i_rgb.at<Vec3b>(Point(X, Y))[0];
					//i_DepthToRgb.at<Vec3b>(Point(col, row))[1] = i_rgb.at<Vec3b>(Point(X, Y))[1];
					//i_DepthToRgb.at<Vec3b>(Point(col, row))[2] = i_rgb.at<Vec3b>(Point(X, Y))[2];
					//i_DepthToRgb.at<Vec3b>(Point(col, row))[3] = i_rgb.at<Vec3b>(Point(X, Y))[3];

					i_DepthToRgb.data[(row * 512 + col) * 4] = i_rgb.data[3 * (Y * 1920 + X)];
					i_DepthToRgb.data[(row * 512 + col) * 4 + 1] = i_rgb.data[3 * (Y * 1920 + X) + 1];
					i_DepthToRgb.data[(row * 512 + col) * 4 + 2] = i_rgb.data[3 * (Y * 1920 + X) + 2];

					//�������߷ֱ�ΪRGB����ͨ����������BGRͨ��������Ϊ͸����
					//i_DepthToRgb.data[i * 4 + 3] = i_rgb.data[(Y * 1920 + X) * 4 + 3];
					/*pclpoint.r = i_DepthToRgb.at<Vec3b>(col, row)[0];
					pclpoint.g = i_DepthToRgb.at<Vec3b>(col, row)[1];
					pclpoint.b = i_DepthToRgb.at<Vec3b>(col, row)[2];*/
					
					/*pclpoint.r = i_DepthToRgb.data[i * 3];
					pclpoint.g = i_DepthToRgb.data[i * 3 + 1];
					pclpoint.b = i_DepthToRgb.data[i * 3 + 2];*/
					//pclpoint.a = i_DepthToRgb.data[i * 4 + 3];
					/*pclpoint.x = static_cast<float>(X);
					pclpoint.y = static_cast<float>(Y);*/
					//pclpoint.z = static_cast<float>(Z);
				}

			}
			cloud_outall->push_back(pclpoint);
			//cout <<"�ڼ����㣺 " <<i << endl;			
		}
	}
	namedWindow("Result",CV_WINDOW_NORMAL);
	imshow("Result", i_DepthToRgb);

	//imwrite("C:\\Users\\Zhihong MA\\Desktop\\1.jpg", i_DepthToRgb);

	//for (int i = 0; i < 424 * 512; i++)
	//{
	//	ColorSpacePoint p = m_pColorCoordinates[i];//��������ǻ�ȡ��Щ�ڲ�ɫ�������ϵ����ȵ������X��Y
	//	CameraSpacePoint p2 = m_pCameraCoordinates[i];
	//	if (p.X != -numeric_limits<float>::infinity() && p.Y != -numeric_limits<float>::infinity()
	//		&& p2.X != -std::numeric_limits<float>::infinity() && p2.Y != -std::numeric_limits<float>::infinity() && p2.Z != -std::numeric_limits<float>::infinity())
	//	{
	//		int colorX = static_cast<int>(p.X + 0.5f);//������������
	//		int colorY = static_cast<int>(p.Y + 0.5f);
	//		//static_cast<int>ǿ��ת������
	//		if ((colorX >= 0 && colorX < 1920) && (colorY >= 0 && colorY < 1080))
	//		{

	//			i_DepthToRgb.data[i * 4] = i_rgb.data[(colorY * 1920 + colorX) * 4];
	//			i_DepthToRgb.data[i * 4 + 1] = i_rgb.data[(colorY * 1920 + colorX) * 4 + 1];
	//			i_DepthToRgb.data[i * 4 + 2] = i_rgb.data[(colorY * 1920 + colorX) * 4 + 2];
	//			//�������߷ֱ�ΪRGB����ͨ����������BGRͨ��������Ϊ͸����
	//			i_DepthToRgb.data[i * 4 + 3] = i_rgb.data[(colorY * 1920 + colorX) * 4 + 444444444446464446446444464444 
	//			pclpoint.g = i_DepthToRgb.data[i * 4 + 1];
	//			pclpoint.b = i_DepthToRgb.data[i * 4 + 2];
	//			pclpoint.a = i_DepthToRgb.data[i * 4 + 3];
	//			pclpoint.x = static_cast<float>(p2.X);
	//			pclpoint.y = static_cast<float>(p2.Y);
	//			pclpoint.z = static_cast<float>(p2.Z);
	//		}
	//	}
	//	cloud_outall->push_back(pclpoint);
	//}


	waitKey(0);
	getchar();
}