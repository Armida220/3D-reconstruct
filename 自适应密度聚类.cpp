#include "PCLlibrary.h"
#include <iostream>
#include <string>
#include <pcl/console/time.h>   // TicToc
#include <time.h>
#include <pcl/search/kdtree.h>
#include <direct.h> 
#include "MyPointType.h"
using namespace std;

#define Pi 3.141592657;
typedef pcl::PointXYZRGB PointT;
typedef pcl::PointCloud<PointT> PointCloud;
typedef pcl::PointNormal PointNormalT;
typedef pcl::PointCloud<PointNormalT> PointCloudWithNormals;
typedef pcl::PointCloud<pcl::PointXYZRGB>::Ptr PointXYZRGBPtr;
typedef pcl::PointCloud<pcl::PointXYZ>::Ptr PointXYZPtr;


//class myPointCloud {
//	PointT mycloud;
//	int PointId;
//	int ClusterId;
//	bool is_edgepoint;
//};


void ConfigFileRead(map<string, string>& m_mapConfigInfo)
{
	ifstream configFile;
	string path = "./����.ini";
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

void IsCenter_Iteration(pcl::PointCloud<myPointCloud>::Ptr &tempcloud, int &id, double &iniRadius,int &MinPts,int &clusterID)
{
	if (tempcloud->at(id).clusterId >= 0 )
		return ;
	vector<int> inRadiusID;
	myPointCloud oripoint = tempcloud->at(id);
	int counts = 0;
	//ͳ����iniRadius�е����е�����
	for (int j = 0; j < tempcloud->size(); j++) {
		double dist;
		if(j==id)
			continue;
		else
		{
			double x = oripoint.x - tempcloud->at(j).x;
			double y = oripoint.y - tempcloud->at(j).y;
			double z = oripoint.z - tempcloud->at(j).z;
			dist= sqrt(x * x + y * y + z * z);
		}
		if (dist <= iniRadius)
		{
			inRadiusID.push_back(j);
			counts++;
		}
			
	}
	//���ܵ����������õ���С��������ôΪ���ĵ㣬����Ϊ�߽��
	if (counts >= MinPts)
	{
		//��Ϊ���ĵ㣬��õ�Ϊ�ѱ��״̬�����Ҿ����IDΪclusterID
		tempcloud->at(id).is_centerpoint = true;
		tempcloud->at(id).clusterId = clusterID;
		for (int idner = 0; idner < inRadiusID.size(); idner++) {
			IsCenter_Iteration(tempcloud, inRadiusID.at(idner), iniRadius, MinPts, clusterID);
		}
		return;//true
	}
	else
	{
		tempcloud->at(id).is_centerpoint = false;
		tempcloud->at(id).clusterId = 0;
		return;// false;
	}
}

void IsCenter(pcl::PointCloud<myPointCloud>::Ptr &tempcloud, int &id, double &iniRadius, int &MinPts, int &clusterID)
{
	if (tempcloud->at(id).clusterId >= 0)
		return;
	vector<int> inRadiusID;
	vector<int> hasputinId;
	vector<myPointCloud> centerPoint;//���ĵ�����
	vector<myPointCloud> DealPoint;//��������������
	DealPoint.push_back(tempcloud->at(id));
	hasputinId.push_back(id);
	for (int i = 0;i< DealPoint.size();i++)
	{
		int sizebegin = DealPoint.size();
		cout << "Point-" << i << "-DealPoint.size now is " << DealPoint.size() << endl;
		myPointCloud oripoint = DealPoint.at(i);
		int counts = 0;
		//ͳ����iniRadius�е����е�����
		for (int j = 0; j < tempcloud->size(); j++) {
			double dist;
			if (j == id)
				continue;
			else
			{
				double x = oripoint.x - tempcloud->at(j).x;
				double y = oripoint.y - tempcloud->at(j).y;
				double z = oripoint.z - tempcloud->at(j).z;
				dist = sqrt(x * x + y * y + z * z);
			}
			if (dist <= iniRadius)
			{
				inRadiusID.push_back(j);
				sort(inRadiusID.begin(), inRadiusID.end());
				inRadiusID.erase(unique(inRadiusID.begin(), inRadiusID.end()), inRadiusID.end());
				counts++;
			}
		}
		//���ܵ����������õ���С��������ôΪ���ĵ㣬����Ϊ�߽��
		if (counts >= MinPts)
		{
			//��Ϊ���ĵ㣬��õ�Ϊ�ѱ��״̬�����Ҿ����IDΪclusterID
			tempcloud->at(id).is_centerpoint = true;
			tempcloud->at(id).clusterId = clusterID;
			//���Ұ������ŵ��������������
			for (int idner = 0; idner < inRadiusID.size(); idner++) {
				if (find(hasputinId.begin(), hasputinId.end(), inRadiusID.at(idner)) == hasputinId.end())
				{
					DealPoint.push_back(tempcloud->at(inRadiusID.at(idner)));
					hasputinId.push_back(inRadiusID.at(idner));
				}		
			}
		}
		else
		{
			tempcloud->at(id).is_centerpoint = false;
			tempcloud->at(id).clusterId = 0;
			continue;
		}
		int sizenext = DealPoint.size();
		if (sizebegin != sizenext || i< DealPoint.size())
			continue;
		else
			break;
	}
	
}


//void clusterDeal(pcl::PointCloud<myPointCloud>::Ptr &cloud, int &id, double &iniRadius, int &MinPts, int &clusterID)
//{
//	//����õ㲻��center�㲢��clusterID����0�����߸�clusterIDС��0��˵���õ㻹δ����������
//	if ((cloud->at(id).clusterId >0 && cloud->at(id).is_centerpoint == false)|| cloud->at(id).clusterId < 0)
//	{
//		bool PointIsCenter = IsCenter(cloud, id, iniRadius, MinPts, clusterID);
//	}
//	else
//	{
//		return;
//	}
//
//}

int main()
{
	cout << "Ready.....";
	getchar();
	cout << "Start working" << endl;
	time_t t1 = GetTickCount();

	//��ȡ�ļ�·��
	map<string, string> param;
	ConfigFileRead(param);
	string filepath = param["filepath"], outfilepath = param["outfilepath"];
	istringstream iniRadiusstr(param["iniRadius"]), MinPtsstr(param["MinPts"]);
	double iniRadius;
	int MinPts;
	iniRadiusstr >> iniRadius;
	MinPtsstr >> MinPts;
	//��ȡ�����ļ�
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloudin(new pcl::PointCloud<pcl::PointXYZRGB>);
	pcl::PointCloud<myPointCloud>::Ptr cloud(new pcl::PointCloud<myPointCloud>);
	int errors =pcl::io::loadPCDFile(filepath, *cloudin);
	if (errors == -1)
	{
		cout << "Can't find file" << endl;
		return -1;
	}
	for (int i = 0; i < cloudin->size(); i++)
	{
		myPointCloud tempcloud;
		tempcloud.x = cloudin->at(i).x;
		tempcloud.y = cloudin->at(i).y;
		tempcloud.z = cloudin->at(i).z;
		tempcloud.r = cloudin->at(i).r;
		tempcloud.g = cloudin->at(i).g;
		tempcloud.b = cloudin->at(i).b;
		cloud->push_back(tempcloud);
	}

	//������Ķ���
	vector<myPointCloud> CenterCloud;
	vector<myPointCloud> edgeCloud;
	vector<myPointCloud> CalCloud;
	int countsID = 1;
	for (int id = 0; id < cloud->size(); id++)//cloud->size()
	{
		//���ѱ�ǣ�������
		if (cloud->at(id).clusterId >= 0)
		{
			cout << "---------------------" << endl;
			if(cloud->at(id).clusterId=0)
				cout << "number " << id << " Point is edge point" << endl;
			else
				cout << "number " << id << " Point is center point" << endl;
			continue;
		}
		else
		{
			cout << "---------------------" << endl;
			cout << "Now is deal with point " << id << endl;
			/*clusterDeal(cloud, id, iniRadius, MinPts, countsID);
			int tempcounts = 0;*/
			IsCenter(cloud, id, iniRadius, MinPts, countsID);
			if (cloud->at(id).is_centerpoint == true)
			{
				//��������ĵ㣬��ô�����id��ΪcountsID��Ȼ��countsID+1��
				//cloud->at(id).is_centerpoint = true;
				//cloud->at(id).clusterId = countsID;
				countsID++;
			}
			else
			{
				cout << "The point " << id << " is edgePoint" << endl;
				cout << endl;
			}
		}
	}
	
	for (int i = 0; i < cloud->size(); i++)
	{
		for (int tempid = 0; tempid <= countsID; tempid++) {
			if (cloud->at(i).clusterId == tempid)
			{
				cloud->at(i).r = 255 - tempid * 10;
				cloud->at(i).g = 50 + tempid * 15;
				cloud->at(i).b = 0 + tempid * 5;
			}
		}
	}
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloudout(new pcl::PointCloud<pcl::PointXYZRGB>);
	for (int i = 0; i < cloud->size(); i++)
	{
		pcl::PointXYZRGB tempcloud;
		tempcloud.x = cloud->at(i).x;
		tempcloud.y = cloud->at(i).y;
		tempcloud.z = cloud->at(i).z;
		tempcloud.r = cloud->at(i).r;
		tempcloud.g = cloud->at(i).g;
		tempcloud.b = cloud->at(i).b;
		cloudout->push_back(tempcloud);
	}

	time_t t2 = GetTickCount();
	cout << "Numbers of cluster is: " << countsID << endl;
	cout << "Use time: " << ((t2 - t1)*1.0 / 1000) << " s" << endl;
	cout << "Finished!..." << endl;
	pcl::io::savePCDFileBinary(outfilepath, *cloudout);



	//pcl::NormalEstimation<pcl::PointXYZRGB, pcl::Normal> ne;
	//pcl::PointCloud<pcl::Normal>::Ptr normals(new pcl::PointCloud<pcl::Normal>);
	//ne.setInputCloud(cloud);
	//ne.setKSearch(knum);
	//ne.compute(*normals);
	//// Create an empty kdtree representation, and pass it to the normal estimation object.
	//// Its content will be filled inside the object, based on the given input dataset (as no other search surface is given).
	//pcl::PointCloud<pcl::PointXYZRGB>::Ptr outcloud(new pcl::PointCloud<pcl::PointXYZRGB>);
	//pcl::PointCloud<pcl::Normal>::Ptr cloud_normals(new pcl::PointCloud<pcl::Normal>);

	//vector<int>kdpointID(knum + 1);
	//vector<float>kdpointDis(knum + 1);
	//// ����kdtree  
	//pcl::search::KdTree<PointXYZRGB>::Ptr kdtree1(new pcl::search::KdTree<PointXYZRGB>);
	////pcl::KdTreeFLANN<PointXYZ>::Ptr tree1(new pcl::KdTreeFLANN<PointXYZ>);
	//kdtree1->setInputCloud(cloud);
	////����ÿ���㷨����
	//n.setInputCloud(cloud);
	////���Ʒ������ʱ����Ҫ���ѵĽ��ڵ��С
	//n.setKSearch(knum);
	////��ʼ���з������
	//n.compute(*normals);

	//int cloudinsize = cloud->size();
	//for (int i = 0; i < cloudinsize; i++) {//cloudin->size()
	//	pcl::PointCloud<PointXYZRGB>::Ptr tempkdcloud(new pcl::PointCloud<PointXYZRGB>);
	//	pcl::PointCloud<pcl::Normal>::Ptr tempnormals(new pcl::PointCloud<pcl::Normal>);
	//	tempkdcloud->push_back(cloud->at(i));
	//	if (kdtree1->nearestKSearch(cloud->at(i), knum + 1, kdpointID, kdpointDis))
	//	{
	//		for (int j = 1; j <= knum; j++)
	//		{
	//			tempkdcloud->push_back(cloud->at(kdpointID[j]));
	//		}
	//		tempnormals->push_back(normals->at(i));
	//		if (NormalIsOk(tempkdcloud, threshold_angle, tempnormals, comFlag))
	//		{
	//			outcloud->push_back(cloud->at(i));
	//			cloud_normals->push_back(tempnormals->at(4));
	//		}
	//	}
	//}

	//boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer(new pcl::visualization::PCLVisualizer("3D Viewer"));
	//viewer->addPointCloud<pcl::PointXYZRGB>(outcloud, "cloud");
	//viewer->addPointCloudNormals<pcl::PointXYZRGB, pcl::Normal>(outcloud, cloud_normals, 2, 0.008, "normals");
	//time_t t2 = GetTickCount();
	//cout << "Use time: " << ((t2 - t1)*1.0 / 1000) << " s" << endl;
	//cout << "Finished!..." << endl;
	//while (!viewer->wasStopped())
	//{
	//	viewer->spinOnce(100);
	//	boost::this_thread::sleep(boost::posix_time::microseconds(100000));
	//}
	getchar();

}
