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


using namespace std;
using namespace pcl;
//using namespace pcl::io;
using namespace io;
//���ӻ�
void view(shared_ptr<visualization::PCLVisualizer> viewer, PointCloud<PointXYZRGBA>::Ptr &cloud_a)
{
	//�������ӻ����ڣ����������ɶ���Ϊȫ��ָ�룬��֤��ȫ��ʹ��
	viewer->initCameraParameters();//��������������û���Ĭ�ϵĽǶȺͷ���۲����
	//������ͬviewport���Խ��жԱ�,����Ϊ��x�����Сֵ�����ֵ��y����Сֵ�����ֵ����ʶ��
	viewer->setBackgroundColor(0, 0, 0);
	//���ñ�����ɫ
	viewer->addCoordinateSystem(1.0);
	//�������ϵ,�����ᡪ��Z������Y���̣�X����
	//addPointCloud(const pcl::PointCloud<pcl::PointXYZRGBARGBA>::ConstPtr &cloud,
	//	const std::string &id = "cloud", int viewport = 0)
	viewer->addPointCloud(cloud_a, "cloud");
	viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 1);//���õ�������
	while (!viewer->wasStopped())
	{
		viewer->spinOnce(100);
		boost::this_thread::sleep(boost::posix_time::microseconds(100000));
	}
};

