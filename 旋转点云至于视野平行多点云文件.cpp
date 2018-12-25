#include "PCLlibrary.h"
#include <iostream>
#include <string>
#include <pcl/console/time.h>   // TicToc
#include <time.h>
#include <pcl/search/kdtree.h>
#include <direct.h> 
#include <omp.h>
#include"getfile.h"
#include <time.h>

using namespace std;

#define Pi 3.141592657;
typedef pcl::PointXYZRGB PointT;
typedef pcl::PointCloud<PointT> PointCloud;
typedef pcl::PointNormal PointNormalT;
typedef pcl::PointCloud<PointNormalT> PointCloudWithNormals;
typedef pcl::PointCloud<pcl::PointXYZRGB>::Ptr PointXYZRGBPtr;
typedef pcl::PointCloud<pcl::PointXYZ>::Ptr PointXYZPtr;


//boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer(new pcl::visualization::PCLVisualizer("3D Viewer"));

//��ȡ�����ļ�
void ConfigFileRead(map<string, string>& m_mapConfigInfo)
{
	ifstream configFile;
	string path = "./��ת.ini";
	configFile.open(path.c_str());
	string str_line;
	if (configFile.is_open())
	{
		while (!configFile.eof())
		{
			getline(configFile, str_line);
			if (str_line.find('#') == 0) //���˵�ע����Ϣ��������׸��ַ�Ϊ#�͹��˵���һ��
			{
				continue;
			}
			size_t pos = str_line.find('=');
			string str_key = str_line.substr(0, pos);
			string str_value = str_line.substr(pos + 1);
			m_mapConfigInfo.insert(pair<string, string>(str_key, str_value));
		}
	}
	else
	{
		cout << "Cannot open config file setting.ini, path: ";
		exit(-1);
	}
}

//The Bilateral Filter for Point Clouds��˫���˲�
PointXYZRGB Bilateral_Filters(pcl::PointCloud<PointXYZRGB>::Ptr cloudin, pcl::PointCloud<pcl::Normal>::Ptr normalsin, vector<float> kdpointDis,
	float sigmac, float sigmas)
{
	PointXYZRGB oricloud = cloudin->at(0);//�����
	Normal orinormal = normalsin->at(0);//����㷨����
	double norx = orinormal.normal_x;//���ߵ�x
	double nory = orinormal.normal_y;//���ߵ�y
	double norz = orinormal.normal_z;//���ߵ�z

	float sigmac2 = pow(sigmac, 2);//sigamcƽ��
	float sigmas2 = pow(sigmas, 2);//sigamsƽ��
	double up = 0, down = 0;
	double dis2 = (oricloud.x - cloudin->at(1).x)*(oricloud.x - cloudin->at(1).x) +
		(oricloud.y - cloudin->at(1).y)*(oricloud.y - cloudin->at(1).y) +
		(oricloud.z - cloudin->at(1).z)*(oricloud.z - cloudin->at(1).z);
	double dis = sqrt(dis2);
	for (int i = 1; i < cloudin->size(); i++)
	{
		Normal Jnormal = normalsin->at(i);
		double pi_jx = (oricloud.x - cloudin->at(i).x);//pi-pj��x
		double pi_jy = (oricloud.y - cloudin->at(i).y);//pi-pj��y
		double pi_jz = (oricloud.z - cloudin->at(i).z);//pi-pj��z

		double dd = kdpointDis.at(i);
		double dd2 = pow(kdpointDis.at(i), 2);//ŷ�Ͼ���ƽ��
		double dn = (pi_jx * norx + pi_jy * nory + pi_jz * norz);// (pi_jz * norz);
		double dn2 = pow((pi_jx * norx + pi_jy * nory + pi_jz * norz), 2);//�����ڻ���ƽ��
		double Wc = exp(-dd2 / (2 * sigmac2));//Wc,��˳�˲�Ȩ�غ�����
		double Ws = exp(-dn2 / (2 * sigmas2));//Ws,��������Ȩ�غ�����
		up += Wc * Ws * dn;
		down += Wc * Ws;
	}
	double a = up / down;
	a = -a;
	PointXYZRGB newpoint;
	newpoint = oricloud;
	newpoint.x = oricloud.x + a * orinormal.normal_x;
	newpoint.y = oricloud.y + a * orinormal.normal_y;
	newpoint.z = oricloud.z + a * orinormal.normal_z;
	return newpoint;
}

void getbox(const pcl::PointCloud<PointXYZRGB>::Ptr &cloudin, double &xmax, double &xmin,
	double &ymax, double &ymin, double &zmax, double &zmin)
{
	int temxmax = 0, temymax = 0, temzmax = 0, temxmin = INT_MAX, temymin = INT_MAX, temzmin = INT_MAX;
	omp_set_num_threads(8);
#pragma omp parallel for
	for (int i = 0; i < cloudin->size(); i++) {
		auto point = cloudin->at(i);
		if (point.x <= temxmin)
			temxmin = point.x;
		if (point.x >= temxmax)
			temxmax = point.x;
		if (point.y <= temymin)
			temymin = point.y;
		if (point.y >= temymax)
			temymax = point.y;
		if (point.z <= temzmin)
			temzmin = point.z;
		if (point.z >= temzmax)
			temzmax = point.z;
	}
	xmax = temxmax + 0.001; xmin = temxmin + 0.001;
	ymax = temymax + 0.001; ymin = temymin + 0.001;
	zmax = temzmax + 0.001; zmin = temzmin + 0.001;
}

int main()
{
	cout << "Ready.....";
	getchar();
	cout << "Start working" << endl;
	time_t t1 = GetTickCount();
	map<string, string> param;
	ConfigFileRead(param);
	string format = param["format"];
	string filepath = param["filepath"];
	string filepathCircle = param["filepath_Circle"];
	string outfilepath = param["outfilepath"];
	string outfilepathCircle = param["outfilepath_Circle"];
	string kdtreeRadiusOrNum = param["kdtreeRadiusOrNum"];
	istringstream sigmacstr(param["sigmac"]), sigmasstr(param["sigmas"]), neipointnum(param["knum"]), kdtreeradiusstr(param["kdtreeradius"]);
	float sigmac, sigmas, kdtreeradius;
	sigmacstr >> sigmac;
	sigmasstr >> sigmas;
	kdtreeradiusstr >> kdtreeradius;
	int k;
	neipointnum >> k;
	cout << "Input Filepath: " << filepathCircle << endl;
	cout << "Output FIlepath: " << outfilepathCircle << endl;

	//ѭ���汾
	vector<string> Allname;
	vector< pcl::PointCloud<PointXYZRGB>::Ptr> AllCloud;
	GetAllFiles_CertainFormat(filepathCircle, Allname, format);
	cout << "File numbers: " << Allname.size() << endl;
	for (int i = 0; i < Allname.size(); i++)
	{
		pcl::PointCloud<PointXYZRGB>::Ptr cloudtemp(new pcl::PointCloud<PointXYZRGB>);
		pcl::io::loadPCDFile(Allname.at(i), *cloudtemp);
		AllCloud.push_back(cloudtemp);
	}


	//�����ļ��汾
	//pcl::PointCloud<PointXYZRGB>::Ptr cloudin(new pcl::PointCloud<PointXYZRGB>);
	////pcl::io::loadPCDFile("C:\\Users\\zhihong\\Desktop\\2\\2\\A619-T-3_side29pcd.pcd", *cloud);
	//pcl::io::loadPCDFile(filepath, *cloudin);
	//pcl::PointCloud<PointXYZRGB>::Ptr cloudout(new pcl::PointCloud<PointXYZRGB>);
	omp_set_num_threads(8);
#pragma omp parallel for
	//boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer(new pcl::visualization::PCLVisualizer("3D Viewer"));
	for (int id = 0; id < AllCloud.size(); id++)//
	{
		pcl::PointCloud<PointXYZRGB>::Ptr cloudin(new pcl::PointCloud<PointXYZRGB>);
		pcl::PointCloud<PointXYZRGB>::Ptr cloudout(new pcl::PointCloud<PointXYZRGB>);
		cloudin = AllCloud.at(id);
		
		double xmax, xmin, ymax, ymin, zmax, zmin;
		getbox(cloudin, xmax, xmin, ymax, ymin, zmax, zmin);


		//vector<int>kdpointID(k + 1);
		//vector<float>kdpointDis(k + 1);
		//// ����kdtree  
		//pcl::search::KdTree<PointXYZRGB>::Ptr kdtree1(new pcl::search::KdTree<PointXYZRGB>);
		//pcl::NormalEstimation<pcl::PointXYZRGB, pcl::Normal> n;
		//pcl::PointCloud<pcl::Normal>::Ptr normals(new pcl::PointCloud<pcl::Normal>);
		////pcl::KdTreeFLANN<PointXYZ>::Ptr tree1(new pcl::KdTreeFLANN<PointXYZ>);
		//if (kdtreeRadiusOrNum == "Num")
		//{
		//	kdtree1->setInputCloud(cloudin);
		//	//����ÿ���㷨����
		//	n.setInputCloud(cloudin);
		//	//���Ʒ������ʱ����Ҫ���ѵĽ��ڵ��С
		//	n.setKSearch(k);
		//	//��ʼ���з������
		//	n.compute(*normals);
		//}
		//else if (kdtreeRadiusOrNum == "Radius")
		//{
		//	kdtree1->setInputCloud(cloudin);
		//	n.setInputCloud(cloudin);
		//	n.setRadiusSearch(kdtreeradius);
		//	n.compute(*normals);
		//}
		//else
		//{
		//	cout << "The Kdtree method of " << id << " is not 'Num' or 'Radius' " << endl;
		//	continue;
		//}
		//int cloudinsize = cloudin->size();
		//for (int i = 0; i < cloudinsize; i++) {//cloudin->size()
		//	pcl::PointCloud<PointXYZRGB>::Ptr tempkdcloud(new pcl::PointCloud<PointXYZRGB>);
		//	pcl::PointCloud<pcl::Normal>::Ptr tempnormals(new pcl::PointCloud<pcl::Normal>);

		//	if (kdtree1->nearestKSearch(cloudin->at(i), k + 1, kdpointID, kdpointDis)) {
		//		//tempcloud�е�һ����Ϊ�����,tempnormals��һ��Ϊ����㷨����
		//		int cc = i;
		//		tempkdcloud->push_back(cloudin->at(i));
		//		tempnormals->push_back(normals->at(i));
		//		/*	cloudin->at(i).r = 0;
		//			cloudin->at(i).g = 0;
		//			cloudin->at(i).b = 255;*/
		//		for (int j = 1; j <= k; j++)
		//		{
		//			//��������
		//			tempkdcloud->push_back(cloudin->at(kdpointID[j]));
		//			tempnormals->push_back(normals->at(kdpointID[j]));
		//		}
		//		PointXYZRGB tempnew_point;
		//		tempnew_point = Bilateral_Filters(tempkdcloud, tempnormals, kdpointDis, sigmac, sigmas);
		//		cloudout->push_back(tempnew_point);
		//	}
		//}
		string outpath = outfilepathCircle + "\\" + "Filter_" + getpostfixname(Allname.at(id));
		pcl::io::savePCDFileBinary(outpath, *cloudout);
	}
	time_t t2 = GetTickCount();
	AllCloud.clear();
	Allname.clear();
	cout << "Use time: " << ((t2 - t1)*1.0 / 1000) << " s" << endl;
	cout << "Finished!..." << endl;
	getchar();
}