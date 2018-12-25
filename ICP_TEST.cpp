#include <iostream>
#include <string>
#include "PCLlibrary.h"
#include <pcl/io/ply_io.h>
#include <pcl/point_types.h>
#include <pcl/registration/icp.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/console/time.h>   // TicToc
#include <pcl/kdtree/kdtree_flann.h>
#include <fstream>
using namespace std;
typedef pcl::PointXYZRGB PointT;
typedef pcl::PointCloud<PointT> PointCloud;
typedef pcl::PointNormal PointNormalT;
typedef pcl::PointCloud<PointNormal> PointCloudWithNormals;

pcl::visualization::PCLVisualizer viewer("ICP demo");
int v1;
int v2;
bool next_iteration = true;

void
print4x4Matrix(const Eigen::Matrix4d & matrix)
{
	printf("Rotation matrix :\n");
	printf("    | %6.3f %6.3f %6.3f | \n", matrix(0, 0), matrix(0, 1), matrix(0, 2));
	printf("R = | %6.3f %6.3f %6.3f | \n", matrix(1, 0), matrix(1, 1), matrix(1, 2));
	printf("    | %6.3f %6.3f %6.3f | \n", matrix(2, 0), matrix(2, 1), matrix(2, 2));
	printf("Translation vector :\n");
	printf("t = < %6.3f, %6.3f, %6.3f >\n\n", matrix(0, 3), matrix(1, 3), matrix(2, 3));
}

void
keyboardEventOccurred(const pcl::visualization::KeyboardEvent& event,
void* nothing)
{
	if (event.getKeySym() == "space" && event.keyDown())
		next_iteration = true;
}

//��< x, y, z, curvature >��ʽ����һ���µĵ��ʾ��x��y��z�����ʣ�
class MyPointRepresentation :public pcl::PointRepresentation <PointNormalT>
{
	using pcl::PointRepresentation<PointNormalT>::nr_dimensions_;
public:
	MyPointRepresentation()
	{
		nr_dimensions_ = 4;//������ά��
	}
	//����copyToFloatArray����������ת��Ϊ4ά����
	virtual void copyToFloatArray(const PointNormalT &p, float* out) const
	{
		out[0] = p.x;
		out[1] = p.y;
		out[2] = p.z;
		out[3] = p.curvature;
	}
};

int main_old()
{
	// The point clouds we will be using
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_in(new pcl::PointCloud<pcl::PointXYZRGB>);// Original point cloud
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_tr(new pcl::PointCloud<pcl::PointXYZRGB>);  // Transformed point cloud
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_icp(new pcl::PointCloud<pcl::PointXYZRGB>);  // ICP output point cloud

	int iterations = 30;  // Default number of ICP iterations
	string inname = "F:/DesktopFile_Pointget/1-DATA/2017-10-18/2017-1-after/A2017-1After_2017-1.pcd";
	string trname = "F:/DesktopFile_Pointget/1-DATA/2017-10-18/2017-1-after/A2017-1After_2017-7.pcd";
	pcl::console::TicToc time;
	time.tic();
	if (pcl::io::loadPCDFile(inname, *cloud_in) < 0)
	{
		PCL_ERROR("Error loading cloud %s.\n", inname);
		return (-1);
	}
	std::cout << "\nLoaded file " << inname << " (" << cloud_in->size() << " points) in " << time.toc() << " ms\n" << std::endl;
	time.tic();
	if (pcl::io::loadPCDFile(trname, *cloud_tr) < 0)
	{
		PCL_ERROR("Error loading cloud %s.\n", inname);
		return (-1);
	}
	std::cout << "\nLoaded file " << trname << " (" << cloud_tr->size() << " points) in " << time.toc() << " ms\n" << std::endl;


	// Defining a rotation matrix and translation vector
	Eigen::Matrix4d transformation_matrix = Eigen::Matrix4d::Identity();

	//// A rotation matrix (see https://en.wikipedia.org/wiki/Rotation_matrix)
	
	time.tic();
	pcl::IterativeClosestPoint<pcl::PointXYZRGB, pcl::PointXYZRGB> icp;
	icp.setMaximumIterations(iterations);
	icp.setInputSource(cloud_in);
	icp.setInputTarget(cloud_tr);
	icp.align(*cloud_icp);
	icp.setMaximumIterations(1);  // We set this variable to 1 for the next time we will call .align () function
	std::cout << "Applied " << iterations << " ICP iteration(s) in " << time.toc() << " ms" << std::endl;

	if (icp.hasConverged())
	{
		std::cout << "\nICP has converged, score is " << icp.getFitnessScore() << std::endl;
		std::cout << "\nICP transformation " << iterations << " : cloud_icp -> cloud_in" << std::endl;
		transformation_matrix = icp.getFinalTransformation().cast<double>();
		print4x4Matrix(transformation_matrix);
	}
	else
	{
		PCL_ERROR("\nICP has not converged.\n");
		return (-1);
	}

	// Visualization
	pcl::visualization::PCLVisualizer viewer("ICP demo");
	// Create two verticaly separated viewports
	int v1(0);
	int v2(1);
	viewer.createViewPort(0.0, 0.0, 0.5, 1.0, v1);
	viewer.createViewPort(0.5, 0.0, 1.0, 1.0, v2);

	// The color we will be using
	float bckgr_gray_level = 0.0;  // Black
	float txt_gray_lvl = 1.0 - bckgr_gray_level;

	// Original point cloud is white
	pcl::visualization::PointCloudColorHandlerCustom< pcl::PointXYZRGB> cloud_in_color_h(cloud_in, (int)255 * txt_gray_lvl, (int)255 * txt_gray_lvl,
		(int)255 * txt_gray_lvl);
	viewer.addPointCloud(cloud_in, cloud_in_color_h, "cloud_in_v1", v1);
	viewer.addPointCloud(cloud_in, cloud_in_color_h, "cloud_in_v2", v2);

	// Transformed point cloud is green
	pcl::visualization::PointCloudColorHandlerCustom< pcl::PointXYZRGB> cloud_tr_color_h(cloud_tr, 20, 180, 20);
	viewer.addPointCloud(cloud_tr, cloud_tr_color_h, "cloud_tr_v1", v1);

	// ICP aligned point cloud is red
	pcl::visualization::PointCloudColorHandlerCustom< pcl::PointXYZRGB> cloud_icp_color_h(cloud_icp, 180, 20, 20);
	viewer.addPointCloud(cloud_icp, cloud_icp_color_h, "cloud_icp_v2", v2);

	// Adding text descriptions in each viewport
	viewer.addText("White: Original point cloud\nGreen: Matrix transformed point cloud", 10, 15, 16, txt_gray_lvl, txt_gray_lvl, txt_gray_lvl, "icp_info_1", v1);
	viewer.addText("White: Original point cloud\nRed: ICP aligned point cloud", 10, 15, 16, txt_gray_lvl, txt_gray_lvl, txt_gray_lvl, "icp_info_2", v2);

	std::stringstream ss;
	ss << iterations;
	std::string iterations_cnt = "ICP iterations = " + ss.str();
	viewer.addText(iterations_cnt, 10, 60, 16, txt_gray_lvl, txt_gray_lvl, txt_gray_lvl, "iterations_cnt", v2);

	// Set background color
	viewer.setBackgroundColor(bckgr_gray_level, bckgr_gray_level, bckgr_gray_level, v1);
	viewer.setBackgroundColor(bckgr_gray_level, bckgr_gray_level, bckgr_gray_level, v2);

	// Set camera position and orientation
	viewer.setCameraPosition(-3.68332, 2.94092, 5.71266, 0.289847, 0.921947, -0.256907, 0);
	viewer.setSize(1280, 1024);  // Visualiser window size

	// Register keyboard callback :
	viewer.registerKeyboardCallback(&keyboardEventOccurred, (void*)NULL);

	// Display the visualiser
	while (!viewer.wasStopped())
	{
		viewer.spinOnce();

		// The user pressed "space" :
		if (next_iteration)
		{
			// The Iterative Closest Point algorithm
			time.tic();
			icp.align(*cloud_icp);
			std::cout << "Applied 1 ICP iteration in " << time.toc() << " ms" << std::endl;

			if (icp.hasConverged())
			{
				printf("\033[11A");  // Go up 11 lines in terminal output.
				printf("\nICP has converged, score is %+.0e\n", icp.getFitnessScore());
				std::cout << "\nICP transformation " << ++iterations << " : cloud_icp -> cloud_in" << std::endl;
				transformation_matrix *= icp.getFinalTransformation().cast<double>();  // WARNING /!\ This is not accurate! For "educational" purpose only!
				print4x4Matrix(transformation_matrix);  // Print the transformation between original pose and current pose

				ss.str("");
				ss << iterations;
				std::string iterations_cnt = "ICP iterations = " + ss.str();
				viewer.updateText(iterations_cnt, 10, 60, 16, txt_gray_lvl, txt_gray_lvl, txt_gray_lvl, "iterations_cnt");
				viewer.updatePointCloud(cloud_icp, cloud_icp_color_h, "cloud_icp_v2");
			}
			else
			{
				PCL_ERROR("\nICP has not converged.\n");
				return (-1);
			}
		}
		next_iteration = false;
	}
	return (0);
}


int main()
{
	// The point clouds we will be using
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_in(new pcl::PointCloud<pcl::PointXYZRGB>);// Original point cloud
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_tr(new pcl::PointCloud<pcl::PointXYZRGB>);  // Transformed point cloud
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_icp(new pcl::PointCloud<pcl::PointXYZRGB>);  // ICP output point cloud

	int iterations = 30;  // Default number of ICP iterations
	string inname = "C:/Users/Zhihong MA/Desktop/data/side_1.pcd";
	string trname = "C:/Users/Zhihong MA/Desktop/data/side_20.pcd";
//	string inname = "F:/DesktopFile_Pointget/1-DATA/2017-10-18/2017-1-after/A2017-1After_2017-7.pcd";
//	string trname = "F:/DesktopFile_Pointget/1-DATA/2017-10-18/2017-1-after/A2017-1After_2017-8.pcd";
	pcl::console::TicToc time;
	time.tic();
	if (pcl::io::loadPCDFile(inname, *cloud_in) < 0)
	{
		PCL_ERROR("Error loading cloud %s.\n", inname);
		return (-1);
	}
	std::cout << "\nLoaded file " << inname << " (" << cloud_in->size() << " points) in " << time.toc() << " ms\n" << std::endl;
	time.tic();
	if (pcl::io::loadPCDFile(trname, *cloud_tr) < 0)
	{
		PCL_ERROR("Error loading cloud %s.\n", trname);
		return (-1);
	}
	std::cout << "\nLoaded file " << trname << " (" << cloud_tr->size() << " points) in " << time.toc() << " ms\n" << std::endl;

	pcl::VoxelGrid<PointT> grid; //��һ�������ĵ��ƣ��ۼ���һ���ֲ���3D������, ���²������˲���������
	//�²���
	grid.setLeafSize(0.001, 0.001, 0.001);
	grid.setInputCloud(cloud_in);
	grid.filter(*cloud_in);
	grid.setInputCloud(cloud_tr);
	grid.filter(*cloud_tr); 
	std::cout << "1 " << endl;
	// Defining a rotation matrix and translation vector
	Eigen::Matrix4d transformation_matrix = Eigen::Matrix4d::Identity();

	//�������淨�ߺ�����
	PointCloudWithNormals::Ptr point_with_normal1(new PointCloudWithNormals);
	PointCloudWithNormals::Ptr point_with_normal2(new PointCloudWithNormals);
	pcl::NormalEstimation<PointT, PointNormalT> norm_est;//�ö������ڼ��㷨����
	pcl::search::KdTree<pcl::PointXYZRGB>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZRGB>()); //����kd�������ڼ��㷨��������������
	norm_est.setSearchMethod(tree);//������������
	norm_est.setKSearch(30); // ��������ڵ�����
	norm_est.setInputCloud(cloud_in);//����������
	norm_est.compute(*point_with_normal1); //���㷨���������洢��points_with_normals_src
	pcl::copyPointCloud(*cloud_in, *point_with_normal1); //���Ƶ��ƣ����꣩��points_with_normals_src����������ͷ�������
	norm_est.setInputCloud(cloud_tr);//��3�м���Ŀ����Ƶķ�������ͬ��
	norm_est.compute(*point_with_normal2);
	pcl::copyPointCloud(*cloud_tr, *point_with_normal2);

	std::cout << "2 " << endl;

	MyPointRepresentation point_representation;
	//��Ȩ����ά�ȣ��Ժ�����xyz����ƽ��
	float alpha[4] = { 1.0, 1.0, 1.0, 1.0 };
	point_representation.setRescaleValues(alpha);//��������ֵ����������ʱʹ�ã�

	//����������ICP���� �����ò���
	pcl::IterativeClosestPointNonLinear<PointNormalT, PointNormalT> reg;//����������ICP����ICP���壬ʹ��Levenberg-Marquardt���Ż���
	reg.setTransformationEpsilon(1e-6);//���������������������Ż���

	reg.setMaxCorrespondenceDistance(0.0001);//���ö�Ӧ��֮��������루0.01m��,����׼�����У����Դ��ڸ���ֵ�ĵ�
	reg.setPointRepresentation(boost::make_shared<const MyPointRepresentation>(point_representation));
	//����Դ���ƺ�Ŀ�����
	reg.setInputSource(point_with_normal1); // ����������ƣ����任�ĵ��ƣ�
	reg.setInputTarget(point_with_normal2); // ����Ŀ�����
	reg.setMaximumIterations(30); //�����ڲ��Ż��ĵ�������

	//��һ��ѭ����������ͬ�����Ż�����ʹ������ӻ�
	Eigen::Matrix4f Ti = Eigen::Matrix4f::Identity(), prev, targetToSource;
	PointCloudWithNormals::Ptr reg_result = point_with_normal1; //���ڴ洢���������+��������
	reg.setMaximumIterations(30);

	std::cout << "3 " << endl;
	////// A rotation matrix (see https://en.wikipedia.org/wiki/Rotation_matrix )
	//time.tic();
	//pcl::IterativeClosestPoint<pcl::PointXYZRGB, pcl::PointXYZRGB> icp;
	//icp.setMaximumIterations(iterations);
	//icp.setInputSource(cloud_in);
	//icp.setInputTarget(cloud_tr);
	//icp.align(*cloud_icp);
	//icp.setMaximumIterations(30);  // We set this variable to 1 for the next time we will call .align () function
	//std::cout << "Applied " << iterations << " ICP iteration(s) in " << time.toc() << " ms" << std::endl;


	//if (icp.hasConverged())
	//{
	//	std::cout << "\nICP has converged, score is " << icp.getFitnessScore() << std::endl;
	//	std::cout << "\nICP transformation " << iterations << " : cloud_icp -> cloud_in" << std::endl;
	//	transformation_matrix = icp.getFinalTransformation().cast<double>();
	//	print4x4Matrix(transformation_matrix);
	//}
	//else
	//{
	//	PCL_ERROR("\nICP has not converged.\n");
	//	return (-1);
	//}

	//pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_trans(new pcl::PointCloud<pcl::PointXYZRGB>);  // trans output point cloud
	//pcl::transformPointCloud(*cloud_tr, *cloud_trans, transformation_matrix);

	// Visualization
	
	// Create two verticaly separated viewports

	viewer.createViewPort(0.0, 0.0, 0.5, 1.0, v1);
	viewer.createViewPort(0.5, 0.0, 1.0, 1.0, v2);

	// The color we will be using
	float bckgr_gray_level = 0.0;  // Black
	float txt_gray_lvl = 1.0 - bckgr_gray_level;

	// Original point cloud is white
	pcl::visualization::PointCloudColorHandlerCustom< pcl::PointXYZRGB> cloud_in_color_h(cloud_in, (int)255 * txt_gray_lvl, (int)255 * txt_gray_lvl,
		(int)255 * txt_gray_lvl);
	//viewer.addPointCloud(cloud_in, cloud_in_color_h, "cloud_in_v1", v1);
	//viewer.addPointCloud(cloud_in, cloud_in_color_h, "cloud_in_v2", v2);
	viewer.addPointCloud(cloud_in, "cloud_in_v1", v1);
	viewer.addPointCloud(cloud_tr, "cloud_in_v2", v1);
	// Transformed point cloud is green
	pcl::visualization::PointCloudColorHandlerCustom< pcl::PointXYZRGB> cloud_tr_color_h(cloud_tr, 20, 180, 20);
	//viewer.addPointCloud(cloud_icp, cloud_icp_color_h, "cloud_icp_v2", v2);
	//viewer.addPointCloud(cloud_icp, "cloud_icp_v1", v2);
	// ICP aligned point cloud is red
	pcl::visualization::PointCloudColorHandlerCustom< pcl::PointXYZRGB> cloud_icp_color_h(cloud_icp, 180, 20, 20);
	//viewer.addPointCloud(cloud_icp, cloud_icp_color_h, "cloud_icp_v2", v2);
	viewer.addPointCloud(cloud_icp, "cloud_icp_v2", v2);
	// Adding text descriptions in each viewport
	viewer.addText("White: Original point cloud\nGreen: Matrix transformed point cloud", 10, 15, 16, txt_gray_lvl, txt_gray_lvl, txt_gray_lvl, "icp_info_1", v1);
	viewer.addText("White: Original point cloud\nRed: ICP aligned point cloud", 10, 15, 16, txt_gray_lvl, txt_gray_lvl, txt_gray_lvl, "icp_info_2", v2);

	std::stringstream ss;
	ss << iterations;
	std::string iterations_cnt = "ICP iterations = " + ss.str();
	viewer.addText(iterations_cnt, 10, 60, 16, txt_gray_lvl, txt_gray_lvl, txt_gray_lvl, "iterations_cnt", v2);

	// Set background color
	viewer.setBackgroundColor(bckgr_gray_level, bckgr_gray_level, bckgr_gray_level, v1);
	viewer.setBackgroundColor(bckgr_gray_level, bckgr_gray_level, bckgr_gray_level, v2);

	// Set camera position and orientation
	viewer.setCameraPosition(-3.68332, 2.94092, 5.71266, 0.289847, 0.921947, -0.256907, 0);
	viewer.setSize(1280, 1024);  // Visualiser window size

	// Register keyboard callback :
	viewer.registerKeyboardCallback(&keyboardEventOccurred, (void*)NULL);
	std::cout << "4 " << endl;
	// Display the visualiser
	while (!viewer.wasStopped())
	{
		viewer.spinOnce();

		// The user pressed "space" :
		if (next_iteration)
		{
			// The Iterative Closest Point algorithm
			time.tic();
			reg.align(*reg_result);
			//icp.align(*cloud_icp);
			std::cout << "Applied 1 ICP iteration in " << time.toc() << " ms" << std::endl;
			if (reg.hasConverged())
			//if (icp.hasConverged())
			{
				printf("\033[11A");  // Go up 11 lines in terminal output.
				printf("\nICP has converged, score is %+.0e\n", reg.getFitnessScore());
				//printf("\nICP has converged, score is %+.0e\n", icp.getFitnessScore());
				std::cout << "\nICP transformation " << ++iterations << " : cloud_icp -> cloud_in" << std::endl;
				transformation_matrix *= reg.getFinalTransformation().cast<double>();  // WARNING /!\ This is not accurate! For "educational" purpose only!
				//transformation_matrix *= icp.getFinalTransformation().cast<double>();  // WARNING /!\ This is not accurate! For "educational" purpose only!
				print4x4Matrix(transformation_matrix);  // Print the transformation between original pose and current pose

				ss.str("");
				ss << iterations;
				std::string iterations_cnt = "ICP iterations = " + ss.str();
				viewer.updateText(iterations_cnt, 10, 60, 16, txt_gray_lvl, txt_gray_lvl, txt_gray_lvl, "iterations_cnt");
				//viewer.updatePointCloud(cloud_icp, cloud_icp_color_h, "cloud_icp_v2");
				viewer.updatePointCloud(cloud_icp,"cloud_icp_v2");
			}
			else
			{
				PCL_ERROR("\nICP has not converged.\n");
				return (-1);
			}
		}
		else
		{
			ofstream fout("C:/Users/Zhihong MA/Desktop/data/transfomation_matrix.txt");
			fout << transformation_matrix << endl;
			next_iteration = false;
		}
		
	}
	return (0);
}

int main_source()
{
	// The point clouds we will be using
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_in(new pcl::PointCloud<pcl::PointXYZRGB>);// Original point cloud
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_tr(new pcl::PointCloud<pcl::PointXYZRGB>);  // Transformed point cloud
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_icp(new pcl::PointCloud<pcl::PointXYZRGB>);  // ICP output point cloud

	int iterations = 30;  // Default number of ICP iterations
	string inname = "F:/DesktopFile_Pointget/1-DATA/2017-10-18/2017-1-after/A2017-1After_2017-1.pcd";
	string trname = "F:/DesktopFile_Pointget/1-DATA/2017-10-18/2017-1-after/A2017-1After_2017-7.pcd";
	pcl::console::TicToc time;
	time.tic();
	if (pcl::io::loadPCDFile(inname, *cloud_in) < 0)
	{
		PCL_ERROR("Error loading cloud %s.\n", inname);
		return (-1);
	}
	std::cout << "\nLoaded file " << inname << " (" << cloud_in->size() << " points) in " << time.toc() << " ms\n" << std::endl;

	// Defining a rotation matrix and translation vector
	Eigen::Matrix4d transformation_matrix = Eigen::Matrix4d::Identity();

	// A rotation matrix (see https://en.wikipedia.org/wiki/Rotation_matrix)
	double theta = M_PI / 8;  // The angle of rotation in radians
	transformation_matrix(0, 0) = cos(theta);
	transformation_matrix(0, 1) = -sin(theta);
	transformation_matrix(1, 0) = sin(theta);
	transformation_matrix(1, 1) = cos(theta);

	// A translation on Z axis (0.4 meters)
	transformation_matrix(2, 3) = 0.1;

	// Display in terminal the transformation matrix
	std::cout << "Applying this rigid transformation to: cloud_in -> cloud_icp" << std::endl;
	print4x4Matrix(transformation_matrix);

	// Executing the transformation
	pcl::transformPointCloud(*cloud_in, *cloud_icp, transformation_matrix);
	*cloud_tr = *cloud_icp;  // We backup cloud_icp into cloud_tr for later use

	// The Iterative Closest Point algorithm
	time.tic();
	pcl::IterativeClosestPoint<pcl::PointXYZRGB, pcl::PointXYZRGB> icp;
	icp.setMaximumIterations(iterations);
	icp.setInputSource(cloud_icp);
	icp.setInputTarget(cloud_in);
	icp.align(*cloud_icp);
	icp.setMaximumIterations(1);  // We set this variable to 1 for the next time we will call .align () function
	std::cout << "Applied " << iterations << " ICP iteration(s) in " << time.toc() << " ms" << std::endl;

	if (icp.hasConverged())
	{
		std::cout << "\nICP has converged, score is " << icp.getFitnessScore() << std::endl;
		std::cout << "\nICP transformation " << iterations << " : cloud_icp -> cloud_in" << std::endl;
		transformation_matrix = icp.getFinalTransformation().cast<double>();
		print4x4Matrix(transformation_matrix);
	}
	else
	{
		PCL_ERROR("\nICP has not converged.\n");
		return (-1);
	}

	// Visualization
	pcl::visualization::PCLVisualizer viewer("ICP demo");
	// Create two verticaly separated viewports
	int v1(0);
	int v2(1);
	viewer.createViewPort(0.0, 0.0, 0.5, 1.0, v1);
	viewer.createViewPort(0.5, 0.0, 1.0, 1.0, v2);

	// The color we will be using
	float bckgr_gray_level = 0.0;  // Black
	float txt_gray_lvl = 1.0 - bckgr_gray_level;

	// Original point cloud is white
	pcl::visualization::PointCloudColorHandlerCustom< pcl::PointXYZRGB> cloud_in_color_h(cloud_in, (int)255 * txt_gray_lvl, (int)255 * txt_gray_lvl,
		(int)255 * txt_gray_lvl);
	viewer.addPointCloud(cloud_in, cloud_in_color_h, "cloud_in_v1", v1);
	viewer.addPointCloud(cloud_in, cloud_in_color_h, "cloud_in_v2", v2);

	// Transformed point cloud is green
	pcl::visualization::PointCloudColorHandlerCustom< pcl::PointXYZRGB> cloud_tr_color_h(cloud_tr, 20, 180, 20);
	viewer.addPointCloud(cloud_tr, cloud_tr_color_h, "cloud_tr_v1", v1);

	// ICP aligned point cloud is red
	pcl::visualization::PointCloudColorHandlerCustom< pcl::PointXYZRGB> cloud_icp_color_h(cloud_icp, 180, 20, 20);
	viewer.addPointCloud(cloud_icp, cloud_icp_color_h, "cloud_icp_v2", v2);

	// Adding text descriptions in each viewport
	viewer.addText("White: Original point cloud\nGreen: Matrix transformed point cloud", 10, 15, 16, txt_gray_lvl, txt_gray_lvl, txt_gray_lvl, "icp_info_1", v1);
	viewer.addText("White: Original point cloud\nRed: ICP aligned point cloud", 10, 15, 16, txt_gray_lvl, txt_gray_lvl, txt_gray_lvl, "icp_info_2", v2);

	std::stringstream ss;
	ss << iterations;
	std::string iterations_cnt = "ICP iterations = " + ss.str();
	viewer.addText(iterations_cnt, 10, 60, 16, txt_gray_lvl, txt_gray_lvl, txt_gray_lvl, "iterations_cnt", v2);

	// Set background color
	viewer.setBackgroundColor(bckgr_gray_level, bckgr_gray_level, bckgr_gray_level, v1);
	viewer.setBackgroundColor(bckgr_gray_level, bckgr_gray_level, bckgr_gray_level, v2);

	// Set camera position and orientation
	viewer.setCameraPosition(-3.68332, 2.94092, 5.71266, 0.289847, 0.921947, -0.256907, 0);
	viewer.setSize(1280, 1024);  // Visualiser window size

	// Register keyboard callback :
	viewer.registerKeyboardCallback(&keyboardEventOccurred, (void*)NULL);

	// Display the visualiser
	while (!viewer.wasStopped())
	{
		viewer.spinOnce();

		// The user pressed "space" :
		if (next_iteration)
		{
			// The Iterative Closest Point algorithm
			time.tic();
			icp.align(*cloud_icp);
			std::cout << "Applied 1 ICP iteration in " << time.toc() << " ms" << std::endl;

			if (icp.hasConverged())
			{
				printf("\033[11A");  // Go up 11 lines in terminal output.
				printf("\nICP has converged, score is %+.0e\n", icp.getFitnessScore());
				std::cout << "\nICP transformation " << ++iterations << " : cloud_icp -> cloud_in" << std::endl;
				transformation_matrix *= icp.getFinalTransformation().cast<double>();  // WARNING /!\ This is not accurate! For "educational" purpose only!
				print4x4Matrix(transformation_matrix);  // Print the transformation between original pose and current pose

				ss.str("");
				ss << iterations;
				std::string iterations_cnt = "ICP iterations = " + ss.str();
				viewer.updateText(iterations_cnt, 10, 60, 16, txt_gray_lvl, txt_gray_lvl, txt_gray_lvl, "iterations_cnt");
				viewer.updatePointCloud(cloud_icp, cloud_icp_color_h, "cloud_icp_v2");
			}
			else
			{
				PCL_ERROR("\nICP has not converged.\n");
				return (-1);
			}
		}
		next_iteration = false;
	}
	return (0);
}