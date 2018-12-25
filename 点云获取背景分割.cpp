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
void view(shared_ptr<visualization::PCLVisualizer> viewer,PointCloud<PointXYZRGBA>::Ptr &cloud_a)
{
	//�������ӻ����ڣ����������ɶ���Ϊȫ��ָ�룬��֤��ȫ��ʹ��
	viewer->initCameraParameters();//��������������û���Ĭ�ϵĽǶȺͷ���۲����
	//������ͬviewport���Խ��жԱ�,����Ϊ��x�����Сֵ�����ֵ��y����Сֵ�����ֵ����ʶ��
	viewer->setBackgroundColor(0, 0, 0);
	//���ñ�����ɫ
	viewer->addCoordinateSystem(1.0);
	//�������ϵ,�����ᡪ��Z������Y���̣�X����
	//addPointCloud(const pcl::PointCloud<pcl::PointXYZRGBA>::ConstPtr &cloud,
	//	const std::string &id = "cloud", int viewport = 0)
	viewer->addPointCloud(cloud_a, "cloud");
	viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 1);//���õ�������
	while (!viewer->wasStopped())
	{
		viewer->spinOnce(100);
		boost::this_thread::sleep(boost::posix_time::microseconds(100000));
	}
};

//�˲�����
//ֱͨ�˲�
pcl::PointCloud<pcl::PointXYZRGBA>::Ptr  PassThroughway(pcl::PointCloud<pcl::PointXYZRGBA>::ConstPtr incloud, string A, float limitDown, float LimitUp)
//��������ֱ𣬵��ƣ��˲��ֶΣ��˲���Χ���ޣ��˲���Χ����
{
	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr outcloud(new pcl::PointCloud<pcl::PointXYZRGBA>);
	pcl::PassThrough<pcl::PointXYZRGBA> pass;
	pass.setInputCloud(incloud);
	pass.setFilterFieldName(A);//�˲��ֶΣ�ѡ��xyz
	pass.setFilterLimits(limitDown, LimitUp);
	pass.filter(*outcloud);
	return outcloud;
}

//ͳ���˲�
pcl::PointCloud<pcl::PointXYZRGBA>::Ptr StatistCloudway(pcl::PointCloud<pcl::PointXYZRGBA>::ConstPtr incloud, int searchnum, double threshold)
//���������ΪΪ�����ƣ�kmeans����������������ж���ֵ
{
	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr outcloud(new pcl::PointCloud<pcl::PointXYZRGBA>);
	// �����˲���
	pcl::StatisticalOutlierRemoval<pcl::PointXYZRGBA> sor;
	sor.setInputCloud(incloud);
	sor.setMeanK(searchnum);//������Χ�ڵ����
	sor.setStddevMulThresh(threshold);//�����ж��Ƿ�Ϊ��Ⱥ�����ֵ
	sor.filter(*outcloud);
	return outcloud;
}

pcl::PointCloud<pcl::PointXYZRGBA>::Ptr BilaCloudway(pcl::PointCloud<pcl::PointXYZRGBA>::ConstPtr incloud, float sigma_s, float sigma_r)
//���������ΪΪ�����ƣ�˫���˲����ڴ�С��
{
	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr outcloud(new pcl::PointCloud<pcl::PointXYZRGBA>);
	// �����˲���
	pcl::FastBilateralFilter<pcl::PointXYZRGBA> fbf;
	fbf.setInputCloud(incloud);
	fbf.setSigmaS(sigma_s);
	fbf.setSigmaR(sigma_r);//��˹����
	fbf.filter(*outcloud);
	return outcloud;
}

pcl::PointCloud<pcl::PointXYZRGBA>::Ptr SegCloudway(pcl::PointCloud<pcl::PointXYZRGBA>::ConstPtr incloud, double threshold, int max_iterations, double probability, 
	int modeType = SACMODEL_PLANE, int methodType = SAC_RANSAC)
{
	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr outcloud(new pcl::PointCloud<pcl::PointXYZRGBA>);

	pcl::ModelCoefficients::Ptr coefficients(new pcl::ModelCoefficients);
	pcl::PointIndices::Ptr inliers(new pcl::PointIndices);
	// ����ƽ��ָ����
	pcl::SACSegmentation<pcl::PointXYZRGBA> seg;
	// Optional
	seg.setOptimizeCoefficients(true);
	// Mandatory
	seg.setModelType(pcl::SACMODEL_PLANE);//��ȡĿ��ģ������
	seg.setMethodType(pcl::SAC_RANSAC);//������������Ransac��Lmeds
	seg.setDistanceThreshold(threshold);//��ѡ�ĵ�Ŀ��ģ�͵ľ��룬�����ڸ���ֵ������Ŀ��ģ���ϣ�Ĭ��ֵΪ0
	seg.setMaxIterations(threshold);//������������Ĭ��ֵΪ50
	seg.setProbability(0.9);//����һ��������������Ⱥ��ĸ��ʣ�Ĭ��ֵΪ0.99
	seg.setInputCloud(incloud);
	seg.segment(*inliers, *coefficients);//�����ȡ���������Ŀ��ģ�͵Ĳ���
	if (inliers->indices.size() == 0)
	{
		PCL_ERROR("Could not estimate a planar model for the given dataset.");
	}
	//��ȡ�ض�����
	
	pcl::ExtractIndices<pcl::PointXYZRGBA> Ei;
	Ei.setIndices(inliers);
	Ei.setInputCloud(incloud);
	Ei.filter(*outcloud);
	return outcloud;
}

int main()
{
	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr Incloud(new pcl::PointCloud<pcl::PointXYZRGBA>);
	//int errors = pcl::io::loadPCDFile<pcl::PointXYZRGBA>("F:/DesktopFile_Pointget/1-DATA/2017-10-18/2017-1/2017-1.pcd", *Incloud);
	int errors = pcl::io::loadPCDFile<pcl::PointXYZRGBA>("H:/����/2017.11.11/0_5/2017_11_113.pcd", *Incloud);
	if (errors == -1)
	{
		cout << "Can not find the file!" << endl;
		return -1;
	}
	cout << "Loaded" << endl;
	shared_ptr<visualization::PCLVisualizer> viewer(new visualization::PCLVisualizer("ԭʼ��������"));
	view(viewer,Incloud);
	//�˲�����
	//ֱͨ�˲��ֱ���xxyz������й��ˣ��ֱܴ��ֲڵĹ��˴�������
	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr PassCloud(new pcl::PointCloud<pcl::PointXYZRGBA>);
	//�������ϵ,�����ᡪ��Z������Y���̣�X����
	PassCloud = PassThroughway(Incloud, "x", -0.5, 0.5);
	PassCloud = PassThroughway(PassCloud, "y", -0.27, 0.5);
	PassCloud = PassThroughway(PassCloud, "z", -0.5, 1);
	shared_ptr<visualization::PCLVisualizer> viewer2(new visualization::PCLVisualizer("ֱͨ�˲��������-��С��Χ"));
	view(viewer2, PassCloud);

	//ͳ���˲�����������Ⱥ��
	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr StatistCloud(new pcl::PointCloud<pcl::PointXYZRGBA>);
	StatistCloud = StatistCloudway(PassCloud,20,15);
	shared_ptr<visualization::PCLVisualizer> viewer3(new visualization::PCLVisualizer("ͳ���˲��������-�޳���Ⱥ��"));
	view(viewer3, StatistCloud);


	////˫���˲�����
	//pcl::PointCloud<pcl::PointXYZRGBA>::Ptr BilaCloud(new pcl::PointCloud<pcl::PointXYZRGBA>);
	//BilaCloud = StatistCloudway(PassCloud,20,0.2);
	//shared_ptr<visualization::PCLVisualizer> viewer4(new visualization::PCLVisualizer("˫���˲�-���룬��Ե����"));
	//view(viewer4, BilaCloud);
	//pcl::PointCloud<pcl::PointXYZRGBA>::Ptr SegCloud(new pcl::PointCloud<pcl::PointXYZRGBA>);
	//SegCloud = SegCloudway(SegCloud,0.01,50,0.9);
	//shared_ptr<visualization::PCLVisualizer> viewer5(new visualization::PCLVisualizer("�ָ����"));
	//view(viewer5,SegCloud);

	return 0;
}