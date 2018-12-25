#include <stdio.h>
#include <Kinect.h>
#include <Windows.h>
#include <highgui.h>
#include <opencv2/core/core.hpp>  
#include <opencv2/opencv.hpp>
#include <pcl/filters/impl/bilateral.hpp>
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
void view(shared_ptr<visualization::PCLVisualizer> viewer, PointCloud<PointXYZRGB>::Ptr &cloud_a)
{
	//�������ӻ����ڣ����������ɶ���Ϊȫ��ָ�룬��֤��ȫ��ʹ��
	viewer->initCameraParameters();//��������������û���Ĭ�ϵĽǶȺͷ���۲����
	//������ͬviewport���Խ��жԱ�,����Ϊ��x�����Сֵ�����ֵ��y����Сֵ�����ֵ����ʶ��
	viewer->setBackgroundColor(0, 0, 0);
	//���ñ�����ɫ
	viewer->addCoordinateSystem(1.0);
	//�������ϵ,�����ᡪ��X������Y���̣�Z����
	//addPointCloud(const pcl::PointCloud<pcl::PointXYZRGB>::ConstPtr &cloud,
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
pcl::PointCloud<pcl::PointXYZRGB>::Ptr  PassThroughway(pcl::PointCloud<pcl::PointXYZRGB>::ConstPtr incloud, string A, float limitDown, float LimitUp)
//��������ֱ𣬵��ƣ��˲��ֶΣ��˲���Χ���ޣ��˲���Χ����
{
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr outcloud(new pcl::PointCloud<pcl::PointXYZRGB>);
	pcl::PassThrough<pcl::PointXYZRGB> pass;
	pass.setInputCloud(incloud);
	pass.setFilterFieldName(A);//�˲��ֶΣ�ѡ��xyz
	pass.setFilterLimits(limitDown, LimitUp);
	pass.filter(*outcloud);
	cout << "PassThrough Done��" << endl;
	return outcloud;
}
//

pcl::PointCloud<pcl::PointXYZRGB>::Ptr StatistCloudway(pcl::PointCloud<pcl::PointXYZRGB>::ConstPtr incloud, int searchnum, double threshold)
//���������ΪΪ�����ƣ�kmeans����������������ж���ֵ
{
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr outcloud(new pcl::PointCloud<pcl::PointXYZRGB>);
	// �����˲���
	pcl::StatisticalOutlierRemoval<pcl::PointXYZRGB> sor;
	sor.setInputCloud(incloud);
	sor.setMeanK(searchnum);//������Χ�ڵ����
	sor.setStddevMulThresh(threshold);//�����ж��Ƿ�Ϊ��Ⱥ�����ֵ
	sor.filter(*outcloud);
	cout << "StatisticalOutlierRemoval Done��" << endl;
	return outcloud;
}

//BilateralFilterֻ�ܶ�PointXYZI���͵ĵ������ݴ���FastBilateralFilter��ֻ�ܴ�������ĵ��ƣ���ʵ����������޷�ʹ��
pcl::PointCloud<pcl::PointXYZRGB>::Ptr FastBilaCloudway(pcl::PointCloud<pcl::PointXYZRGB>::ConstPtr incloud, float sigma_s, float sigma_r)
//���������ΪΪ�����ƣ����ڿռ�����/����ʹ�õĸ�˹��׼ƫ��ռ䣩,���ڿ������������½����ٵĸ�˹�ı�׼ƫ��(���)
{
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr outcloud(new pcl::PointCloud<pcl::PointXYZRGB>);
	// �����˲���
	/*pcl::BilateralFilter<pcl::PointXYZRGB> bf;*/
	pcl::FastBilateralFilter<pcl::PointXYZRGB> fbf;
	//pcl::search::Search<pcl::PointXYZRGB>::Ptr kdtree;
	//����kd��������˫���˲�ֻ�ܶ�������ƽ��д���kd����Դ����������
	//pcl::FastBilateralFilter<pcl::PointXYZRGB> fbf;
	fbf.setInputCloud(incloud);
	//bf.setSearchMethod(kdtree);
	fbf.setSigmaS(sigma_s);
	fbf.setSigmaR(sigma_r);//��˹����
	//fbf.filter(*outcloud);
	fbf.applyFilter(*outcloud);
	return outcloud;
}

////ƽ��ģ�ͷָ�
//pcl::PointCloud<pcl::PointXYZRGB>::Ptr SegCloudway(pcl::PointCloud<pcl::PointXYZRGB>::ConstPtr incloud, double threshold, int max_iterations, double probability,
//	int modeType = SACMODEL_PLANE, int methodType = SAC_RANSAC)
//{
//	pcl::PointCloud<pcl::PointXYZRGB>::Ptr outcloud(new pcl::PointCloud<pcl::PointXYZRGB>);
//
//	pcl::ModelCoefficients::Ptr coefficients(new pcl::ModelCoefficients);
//	pcl::PointIndices::Ptr inliers(new pcl::PointIndices);
//	// ����ƽ��ָ����
//	pcl::SACSegmentation<pcl::PointXYZRGB> seg;
//	// Optional
//	seg.setOptimizeCoefficients(true);
//	// Mandatory
//	seg.setModelType(pcl::SACMODEL_PLANE);//��ȡĿ��ģ������
//	seg.setMethodType(pcl::SAC_RANSAC);//������������Ransac��Lmeds
//	seg.setDistanceThreshold(threshold);//��ѡ�ĵ�Ŀ��ģ�͵ľ��룬�����ڸ���ֵ������Ŀ��ģ���ϣ�Ĭ��ֵΪ0
//	seg.setMaxIterations(threshold);//������������Ĭ��ֵΪ50
//	seg.setProbability(probability);//����һ��������������Ⱥ��ĸ��ʣ�Ĭ��ֵΪ0.99
//	seg.setInputCloud(incloud);
//	seg.segment(*inliers, *coefficients);//�����ȡ���������Ŀ��ģ�͵Ĳ���
//	if (inliers->indices.size() == 0)
//	{
//		PCL_ERROR("Could not estimate a planar model for the given dataset.");
//	}
//	//��ȡ�ض�����
//
//	pcl::ExtractIndices<pcl::PointXYZRGB> Ei;
//	Ei.setIndices(inliers);
//	Ei.setInputCloud(incloud);
//	Ei.filter(*outcloud);
//	return outcloud;
//}
//
////�ֲ������ָ�
//void RegionGrowingSeg(pcl::PointCloud<pcl::PointXYZRGB>::ConstPtr incloud, pcl::PointCloud<pcl::PointXYZRGB>::ConstPtr outcloud)
//{
//	pcl::search::Search<pcl::PointXYZRGB>::Ptr tree = boost::shared_ptr<pcl::search::Search<pcl::PointXYZRGB> >(new pcl::search::KdTree<pcl::PointXYZRGB>);
//	pcl::PointCloud <pcl::Normal>::Ptr normals(new pcl::PointCloud <pcl::Normal>);
//	pcl::NormalEstimation<pcl::PointXYZRGB, pcl::Normal> normal_estimator;
//	normal_estimator.setSearchMethod(tree);
//	normal_estimator.setInputCloud(incloud);
//	normal_estimator.setKSearch(50);
//	normal_estimator.compute(*normals);
//
//	//pcl::IndicesPtr indices(new std::vector <int>);
//	//pcl::PassThrough<pcl::PointXYZRGB> pass;
//	//pass.setInputCloud(incloud);
//	//pass.setFilterFieldName("z");
//	//pass.setFilterLimits(0.0, 1.0);
//	//pass.filter(*indices);
//
//	pcl::RegionGrowing<pcl::PointXYZRGB, pcl::Normal> reg;
//	reg.setMinClusterSize(50);
//	reg.setMaxClusterSize(1000000);
//	reg.setSearchMethod(tree);
//	reg.setNumberOfNeighbours(30);
//	reg.setInputCloud(incloud);
//	//reg.setIndices (indices);
//	reg.setInputNormals(normals);
//	reg.setSmoothnessThreshold(3.0 / 180.0 * M_PI);
//	reg.setCurvatureThreshold(1.0);
//
//	std::vector <pcl::PointIndices> clusters;
//	reg.extract(clusters);
//	outcloud = reg.getColoredCloudRGBA();
//	//outcloud = reg.getColoredCloud();
//
//	std::cout << "Number of clusters is equal to " << clusters.size() << std::endl;
//	std::cout << "First cluster has " << clusters[0].indices.size() << " points." << endl;
//	std::cout << "These are the indices of the points of the initial" <<
//		std::endl << "cloud that belong to the first cluster:" << std::endl;
//	int counter = 0;
//	while (counter < clusters[0].indices.size())
//	{
//		std::cout << clusters[0].indices[counter] << ", ";
//		counter++;
//		if (counter % 10 == 0)
//			std::cout << std::endl;
//	}
//	cout << clusters.size() << endl;
//	std::cout << "RegionGrowingSegment Done��"  << std::endl;
//}

int main()
{
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr Incloud(new pcl::PointCloud<pcl::PointXYZRGB>);
	int errors = pcl::io::loadPCDFile<pcl::PointXYZRGB>("C:\\Users\\Zhihong MA\\Desktop\\KinectPointdata\\xlr\\3.pcd", *Incloud);
	if (errors == -1)
	{
		cout << "Can not find the file!" << endl;
		return -1;
	}
	cout << "Loaded" << endl;
	shared_ptr<visualization::PCLVisualizer> viewer(new visualization::PCLVisualizer("ԭʼ��������"));
	view(viewer, Incloud);
	//�˲�����
	//ֱͨ�˲��ֱ���xxyz������й��ˣ��ֱܴ��ֲڵĹ��˴�������
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr PassCloud(new pcl::PointCloud<pcl::PointXYZRGB>);
	PassCloud = PassThroughway(PassCloud, "x", -0.8, 1.2);
	PassCloud = PassThroughway(PassCloud, "y", -1.3, 1.3);
	PassCloud = PassThroughway(Incloud, "z", -0.5, 1.5);
	shared_ptr<visualization::PCLVisualizer> viewer2(new visualization::PCLVisualizer("ֱͨ�˲��������-��С��Χ"));
	view(viewer2, PassCloud);

	//ͳ���˲�����������Ⱥ��
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr StatistCloud(new pcl::PointCloud<pcl::PointXYZRGB>);
	StatistCloud = StatistCloudway(PassCloud, 20, 2);
	shared_ptr<visualization::PCLVisualizer> viewer3(new visualization::PCLVisualizer("ͳ���˲��������-�޳���Ⱥ��"));
	view(viewer3, StatistCloud);

	////˫���˲�����
	//pcl::PointCloud<pcl::PointXYZRGB>::Ptr BilaCloud(new pcl::PointCloud<pcl::PointXYZRGB>);
	//BilaCloud = BilaCloudway(StatistCloud, 5, 0.03);
	//shared_ptr<visualization::PCLVisualizer> viewer4(new visualization::PCLVisualizer("˫���˲�-���룬��Ե����"));
	//view(viewer4, BilaCloud);
	////ƽ��ָ�
	//pcl::PointCloud<pcl::PointXYZRGB>::Ptr SegCloud(new pcl::PointCloud<pcl::PointXYZRGB>);
	//SegCloud = SegCloudway(SegCloud, 0.01, 25, 0.9);
	//shared_ptr<visualization::PCLVisualizer> viewer5(new visualization::PCLVisualizer("�ָ����"));
	//view(viewer5, SegCloud);

	////�ֲ������ָ�
	//pcl::PointCloud<pcl::PointXYZRGB>::Ptr ReGrowSegCloud(new pcl::PointCloud<pcl::PointXYZRGB>);
	//RegionGrowingSeg(StatistCloud, ReGrowSegCloud);
	//shared_ptr<visualization::PCLVisualizer> viewer5(new visualization::PCLVisualizer("�ֲ������ָ�����-�޳���Ⱥ��"));
	//view(viewer5, ReGrowSegCloud);

	return 0;
}