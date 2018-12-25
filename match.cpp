#include <pcl/io/pcd_io.h>
#include <pcl/io/ply_io.h>
#include <pcl/io/vtk_io.h>
#include <pcl/io/obj_io.h>
#include <pcl/io/io.h>
#include <pcl/point_types.h>
#include <pcl/point_cloud.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/visualization/common/common.h>
#include <pcl/ModelCoefficients.h>
#include <pcl/features/normal_3d.h>
#include <pcl/search/kdtree.h>
#include <pcl/surface/gp3.h>
#include <pcl/visualization/cloud_viewer.h> 
#include <boost/thread/thread.hpp>
#include <pcl/common/common_headers.h>
#include <pcl/features/normal_3d.h>
#include <pcl/console/parse.h>
#include <boost/make_shared.hpp>
#include <pcl/point_representation.h>
#include <pcl/filters/filter.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/registration/icp.h>
#include <pcl/registration/icp_nl.h>
#include <pcl/filters/passthrough.h>
#include <pcl/registration/transforms.h>
#include <fstream>
using namespace std;
//#include <pcl/registration/icp.h>
//�������ӻ����ڣ����������ɶ���Ϊȫ��ָ�룬��֤��ȫ��ʹ��
pcl::visualization::PCLVisualizer *viewer(new pcl::visualization::PCLVisualizer("3D Viewer"));
//���������Ӵ����бȽ�
int vp1, vp2;
//�����Ͷ���
typedef pcl::PointXYZRGB PointT;
typedef pcl::PointCloud<PointT> PointCloud;
typedef pcl::PointNormal PointNormalT;
typedef pcl::PointCloud<PointNormalT> PointCloudWithNormals;

//������ά�ؽ����̣���������Ԥ�����ָ�������񻯡�������Ⱦ

//ʲô�ṹ�壬�Ե��ƶ�����гɶԴ������
struct PCLFile
{
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud;
	std::string f_name;
	PCLFile() :cloud(new pcl::PointCloud<pcl::PointXYZRGB>){};
};

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
//����ͼ��ʾ,��ʾԴ���ƺ�Ŀ�����
void showCloudsLeft(const pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_target, const pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_source)
{
	//viewer->removePointCloud("vp1_target");
	//viewer->removePointCloud("vp1_source");
	viewer->addPointCloud(cloud_target, "vp1_target", vp1);
	viewer->addPointCloud(cloud_source, "vp1_source", vp1);
}
//����ͼ��ʾ
void showCloudsRight(const PointCloudWithNormals::Ptr cloud_target, const PointCloudWithNormals::Ptr cloud_source)
{
	//viewer->removePointCloud("source");
	//viewer->removePointCloud("target");
	pcl::visualization::PointCloudColorHandlerGenericField<PointNormalT> tgt_color_handler(cloud_target, "curvature");
	if (!tgt_color_handler.isCapable())
		PCL_WARN("Cannot create curvature color handler!");
	pcl::visualization::PointCloudColorHandlerGenericField<PointNormalT> src_color_handler(cloud_source, "curvature");
	if (!src_color_handler.isCapable())
		PCL_WARN("Cannot create curvature color handler!");
	viewer->addPointCloud(cloud_target, tgt_color_handler, "target", vp2);
	viewer->addPointCloud(cloud_source, src_color_handler, "source", vp2);
	//viewer->spinOnce();
}
////��������
//void loadData(std::vector<PCD, Eigen::aligned_allocator<PCD> > &models)
//{
//	std::string extension(".pcd");
//	//�ٶ���һ��������ʵ�ʲ���ģ��
//	for (int i = 1; i < argc; i++)
//	{
//		std::string fname = std::string(argv[i]);
//		// ������Ҫ5���ַ�������Ϊ.plot���� 5���ַ���
//		if (fname.size() <= extension.size())
//			continue;
//		std::transform(fname.begin(), fname.end(), fname.begin(), (int(*)(int))tolower);
//		//��������һ��pcd�ļ�
//		if (fname.compare(fname.size() - extension.size(), extension.size(), extension) == 0)
//		{
//			//���ص��Ʋ������������ģ���б���
//			PCD m;
//			m.f_name = argv[i];
//			pcl::io::loadPCDFile(argv[i], *m.cloud);
//			//�ӵ������Ƴ�NAN��
//			std::vector<int> indices;
//			pcl::removeNaNFromPointCloud(*m.cloud, *m.cloud, indices);
//			models.push_back(m);
//		}
//	}
//}



//����ƥ��
void pairAlign(const pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_src, const pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_tgt, pcl::PointCloud<pcl::PointXYZRGB>::Ptr output, Eigen::Matrix4f &final_transform, bool downsample = false)
{
	//Ϊ��һ���Ժ͸��ٵ��²���
	//ע�⣺Ϊ�˴����ݼ���Ҫ��������
	PointCloud::Ptr src(new PointCloud);
	PointCloud::Ptr tgt(new PointCloud);
	pcl::VoxelGrid<PointT> grid; //��һ�������ĵ��ƣ��ۼ���һ���ֲ���3D������, ���²������˲���������
	//�²���
	if (downsample)
	{
		grid.setLeafSize(0.05, 0.05, 0.05);
		grid.setInputCloud(cloud_src);
		grid.filter(*src);
		grid.setInputCloud(cloud_tgt);
		grid.filter(*tgt);
	}
	else
	{
		src = cloud_src;
		tgt = cloud_tgt;
	}

	//�������淨�ߺ�����
	PointCloudWithNormals::Ptr points_with_normals_src(new PointCloudWithNormals);//����Դ����ָ�루ע�������Ͱ�������ͷ�������
	PointCloudWithNormals::Ptr points_with_normals_tgt(new PointCloudWithNormals);//����Ŀ�����ָ�루ע�������Ͱ�������ͷ�������
	pcl::NormalEstimation<PointT, PointNormalT> norm_est;//�ö������ڼ��㷨����
	pcl::search::KdTree<pcl::PointXYZRGB>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZRGB>()); //����kd�������ڼ��㷨��������������
	norm_est.setSearchMethod(tree);//������������
	norm_est.setKSearch(30); // ��������ڵ�����
	norm_est.setInputCloud(src);//����������
	norm_est.compute(*points_with_normals_src); //���㷨���������洢��points_with_normals_src
	pcl::copyPointCloud(*src, *points_with_normals_src); //���Ƶ��ƣ����꣩��points_with_normals_src����������ͷ�������
	norm_est.setInputCloud(tgt);//��3�м���Ŀ����Ƶķ�������ͬ��
	norm_est.compute(*points_with_normals_tgt);
	pcl::copyPointCloud(*tgt, *points_with_normals_tgt);


	//����һ���Զ����ı�﷽ʽ ʵ��
	MyPointRepresentation point_representation;
	//��Ȩ����ά�ȣ��Ժ�����xyz����ƽ��
	float alpha[4] = { 1.0, 1.0, 1.0, 1.0 };
	point_representation.setRescaleValues(alpha);//��������ֵ����������ʱʹ�ã�

	//����������ICP���� �����ò���
	pcl::IterativeClosestPointNonLinear<PointNormalT, PointNormalT> reg;//����������ICP����ICP���壬ʹ��Levenberg-Marquardt���Ż���
	reg.setTransformationEpsilon(1e-6);//���������������������Ż���
	//***** ע�⣺�����Լ����ݿ�Ĵ�С���ڸò���

	reg.setMaxCorrespondenceDistance(0.0001);//���ö�Ӧ��֮��������루0.01m��,����׼�����У����Դ��ڸ���ֵ�ĵ�
	//���õ��ʾ
	reg.setPointRepresentation(boost::make_shared<const MyPointRepresentation>(point_representation));
	//����Դ���ƺ�Ŀ�����
	reg.setInputSource(points_with_normals_src); // ����������ƣ����任�ĵ��ƣ�
	reg.setInputTarget(points_with_normals_tgt); // ����Ŀ�����
	reg.setMaximumIterations(30); //�����ڲ��Ż��ĵ�������

	//��һ��ѭ����������ͬ�����Ż�����ʹ������ӻ�
	Eigen::Matrix4f Ti = Eigen::Matrix4f::Identity(), prev, targetToSource;
	PointCloudWithNormals::Ptr reg_result = points_with_normals_src; //���ڴ洢���������+��������
	reg.setMaximumIterations(30);
	for (int i = 0; i < 30; ++i)//����
	{
		PCL_INFO("Iteration Nr. %d.\n", i); //��������ʾ�����Ĵ���
		//������ƣ����ڿ��ӻ�
		points_with_normals_src = reg_result;
		//����
		reg.setInputSource(points_with_normals_src); //��������������ƣ����任�ĵ��ƣ�����Ϊ������һ�ε������Ѿ������任��
		reg.align(*reg_result);//���루��׼����������

		Ti = reg.getFinalTransformation() * Ti; //�ۻ���ÿ�ε����ģ��任����
		//������ת����֮ǰת��֮��Ĳ���С����ֵ
		//��ͨ����С����Ӧ���������Ƴ���
		if (fabs((reg.getLastIncrementalTransformation() - prev).sum()) < reg.getTransformationEpsilon())
			reg.setMaxCorrespondenceDistance(reg.getMaxCorrespondenceDistance() - 0.00001);//��С��Ӧ��֮��������루�������ù���
		prev = reg.getLastIncrementalTransformation();//��һ�α任�����
		//��ʾ��ǰ��׼״̬���ڴ��ڵ����������򵥵���ʾԴ���ƺ�Ŀ�����
		//showCloudsRight(points_with_normals_tgt, points_with_normals_src);
	}
	//
	//�����Ŀ����Ƶ�Դ���Ƶı任����
	targetToSource = Ti.inverse();
	//
	//��Ŀ����� �任�ص� Դ����֡
	pcl::transformPointCloud(*cloud_tgt, *output, targetToSource);

	viewer->addPointCloud(output, "target", vp2);
	viewer->addPointCloud(cloud_src, "source", vp2);

	PCL_INFO("Press q to continue the registration.\n");

	//���Դ���Ƶ�ת��Ŀ��
	*output += *cloud_src; // ƴ�ӵ���ͼ���ĵ㣩������Ŀ���������Ƶĵ�����
	final_transform = targetToSource; //���յı任��Ŀ����Ƶ�Դ���Ƶı任����

}

int Saveplyfile(pcl::PointCloud<pcl::PointXYZRGB>::Ptr result_cloud, std::string pathname)
{
	ofstream corandcolor(pathname);
	corandcolor << "ply"
		<< "\n" << "format ascii 1.0"
		<< "\n" << "comment PCL generated"
		<< "\n" << "element vertex " << result_cloud->size()
		<< "\n" << "property float x"
		<< "\n" << "property float y"
		<< "\n" << "property float z"
		<< "\n" << "property uchar red"
		<< "\n" << "property uchar green"
		<< "\n" << "property uchar blue"
		<< "\n" << "property uchar alpha"
		<< "\n" << "element camera 1"
		<< "\n" << "property float view_px"
		<< "\n" << "property float view_py"
		<< "\n" << "property float view_pz"
		<< "\n" << "property float x_axisx"
		<< "\n" << "property float x_axisy"
		<< "\n" << "property float x_axisz"
		<< "\n" << "property float y_axisx"
		<< "\n" << "property float y_axisy"
		<< "\n" << "property float y_axisz"
		<< "\n" << "property float z_axisx"
		<< "\n" << "property float z_axisy"
		<< "\n" << "property float z_axisz"
		<< "\n" << "property float focal"
		<< "\n" << "property float scalex"
		<< "\n" << "property float scaley"
		<< "\n" << "property float centerx"
		<< "\n" << "property float centery"
		<< "\n" << "property int viewportx"
		<< "\n" << "property int viewporty"
		<< "\n" << "property float k1"
		<< "\n" << "property float k2"
		<< "\n" << "end_header" << endl;
	for (size_t i = 0; i < result_cloud->size(); ++i)
	{
		corandcolor << result_cloud->points[i].x << " " << result_cloud->points[i].y << " " << result_cloud->points[i].z << " " <<
			(int)result_cloud->points[i].r << " " << (int)result_cloud->points[i].g << " " << (int)result_cloud->points[i].b << " " << 255 << endl;
	}
	corandcolor << "0 0 0 1 0 0 0 1 0 0 0 1 0 0 0 0 0 " << result_cloud->size() << " 1 0 0" << endl;
	return 0;
}

int main()
{
	//�������
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_a(new pcl::PointCloud<pcl::PointXYZRGB>);
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_b(new pcl::PointCloud<pcl::PointXYZRGB>);
	int errors = pcl::io::loadPCDFile("F:/DesktopFile_Pointget/1-DATA/2017-10-18/2017-1-after/A2017-1After_2017-7.pcd", *cloud_a);
	int errors2 = pcl::io::loadPCDFile("F:/DesktopFile_Pointget/1-DATA/2017-10-18/2017-1-after/A2017-1After_2017-8.pcd", *cloud_b);
	if (errors == -1)
	{
		cout << "Can not find the file" << endl;
		return -1;
	}
	cout << "loaded" << endl;

	cout << "Set Visualization" << endl;

	////���������Ӵ����бȽ�
	viewer->createViewPort(0.0, 0, 0.5, 1.0, vp1);
	viewer->createViewPort(0.5, 0, 1.0, 1.0, vp2);
	viewer->initCameraParameters();//��������������û���Ĭ�ϵĽǶȺͷ���۲����
	viewer->setBackgroundColor(0, 0, 0);
	//int vp1,vp2;
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr result_cloud(new pcl::PointCloud<pcl::PointXYZRGB>);
	Eigen::Matrix4f GlobalTransform = Eigen::Matrix4f::Identity(), pairTransform;
	showCloudsLeft(cloud_b, cloud_a);
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr temp(new pcl::PointCloud<pcl::PointXYZRGB>);
	//�������ת��

	pairAlign(cloud_a, cloud_b, temp, pairTransform, true);
	//����ȫ�ֱ任
	pcl::transformPointCloud(*temp, *result_cloud, GlobalTransform);
	//showCloudsRight(cloud_b, result_cloud);

	GlobalTransform = pairTransform * GlobalTransform;
	//������׼�ԣ�ת������һ�����ƿ����
	//std::stringstream pathname;
	//pathname <<  "C:/Users/user/Desktop/test/object2/result2x.ply";
	////pcl::io::savePLYFile(ss.str(), *result_cloud, true);
	//Saveplyfile(result_cloud,pathname.str());
	cout << "result_cloud->size()" << result_cloud->size() << endl;

	//����spinonce�������Ӵ�����ʱ���ʱ�䣬���������̵Ƚ�������
	while (!viewer->wasStopped())
	{
		viewer->spinOnce(100);
		boost::this_thread::sleep(boost::posix_time::microseconds(100000));
	}
}
