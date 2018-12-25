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
//������ά�ؽ����̣���������Ԥ�����ָ�������񻯡�������Ⱦ
//���ӻ�
//�����ᡪ��X������Y���̣�Z����
using namespace pcl;
using namespace pcl::io;
using namespace io;
using namespace std;



int main()
{
	bool simple(false), rgb(false), custom_c(false), normals(false),
		shapes(false), viewports(false), interaction_customization(false);

	//�������
	PointCloud<PointXYZ>::Ptr cloud_a(new PointCloud<PointXYZ>);
	//PointCloud<PointXYZRGBA>::Ptr cloud_b(new PointCloud<PointXYZRGBA>);
	//string filepath1 = "F:/1-DesktopFile_Pointget/1-DATA/2017.11.11/50_2After/After_2017_11_1154.pcd";
	//string filepath2 = "F:/1-DesktopFile_Pointget/1-DATA/2017.11.11/50_2After/After_2017_11_1153.pcd";
	//string filepath1 = "F:/1-DesktopFile_Pointget/1-DATA/2017.11.11/50_5_2After_2/After_2017.11.11_33.pcd";
	//string filepath1 = "F:/1-DesktopFile_Pointget/1-DATA/2017.11.11/50_5_2/2017.11.11_1.pcd";

	//string filepath1= "C:\\Users\\Zhihong MA\\Desktop\\box.pcd";
	string filepath1 = "F:\\1-DesktopFile_Pointget\\2_2018_0918ֲ����ά�ؽ�����\\���ӽ�ͼ��\\1.obj";
	/*cout << filepath1 << endl;*/
	/*int errors = pcl::io::loadPCDFile(filepath1, *cloud_a);*/
	int errors = pcl::io::loadOBJFile(filepath1, *cloud_a);
	//int errors = pcl::io::loadPLYFile(filepath1, *cloud_a);
	//int errors2 = pcl::io::loadPCDFile(filepath2, *cloud_b);


	//int errors = loadPCDFile("C:/Users/Zhihong MA/Desktop/test2017.9.22/AfterFilter/Test1.pcd_Filter.pcd", *cloud_a);
	if (errors == -1)
	{
		cout << "Can not find the file" << endl;
		return -1;
	}
	cout << "loaded" << endl;
	//getchar();

	cout << "Set Visualization" << endl;
	//�������ӻ����ڣ����������ɶ���Ϊȫ��ָ�룬��֤��ȫ��ʹ��
	shared_ptr<visualization::PCLVisualizer> viewer(new visualization::PCLVisualizer("3D Viewer2"));
	viewer->initCameraParameters();//��������������û���Ĭ�ϵĽǶȺͷ���۲����
	//������ͬviewport���Խ��жԱ�,����Ϊ��x�����Сֵ�����ֵ��y����Сֵ�����ֵ����ʶ��
	//viewer->setBackgroundColor(0, 0, 0);

	//���ñ�����ɫ
	viewer->setBackgroundColor(0, 0, 0);
	viewer->addPointCloud(cloud_a, "cloud1");
	//viewer->addPointCloud(cloud_b, "cloud2");
	//pcl::visualization::PointCloudColorHandlerRGBField<PointXYZRGBA>  rgbx(cloud_a);//��ʾɫ����Ϣ
	//viewer->addPointCloud<PointXYZRGBA>(cloud_a, rgbx, "cloud1");
	//viewer->addPointCloud<PointXYZRGB>(cloud_a, "cloud12");
	//��ӵ��ƣ�����������ʶ�������������ֺ͵��ã�
	//��������һ���Ѿ���ʾ�ĵ��ƣ��û������ȵ���removePointCloud()�����ṩ��Ҫ���µĵ��Ƶ�ID�š�
	//PCL1.1�汾���Ͽ�����updatePointCloud()ֱ��ʵ�ֵ��Ƶĸ���
	//viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 2, "cloud2");//���õ�������,�˴�Ϊsize
	viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 1, "cloud1");//���õ�������
	//viewer->addCoordinateSystem(1.0);//�������ϵ

	//pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZRGBA> colorxx(cloud_a, 0, 255, 0);
	////����һ���Զ�����ɫ������PointCloudColorHandlerCustom���󣬲�������ɫΪĳ����ɫ
	//viewer->updatePointCloud(cloud_a, colorxx, "cloud12");
	//viewer->addPointCloud(cloud_a, "cloud12");
	//���ַ�ʽ����ʹ��cloud_a�е��Ƶĵ���ɫ��Ϊ��ɫ������ǰ�����½�һ��id����������ԭid�����޸�,������ٵ���cloud1��ʱ��ͻ�����ɫ��
	//�����������id cloud12���ٵ���cloud1ʱ������ԭ������ɫ
	//�������߶������cloud_a���޸�


	//����spinonce�������Ӵ�����ʱ���ʱ�䣬���������̵Ƚ�������
	while (!viewer->wasStopped())
	{
		viewer->spinOnce(100);
		boost::this_thread::sleep(boost::posix_time::microseconds(100000));
	}
}