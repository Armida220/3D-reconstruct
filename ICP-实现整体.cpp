#include "PCLlibrary.h"
#include <iostream>
#include <string>
#include <pcl/console/time.h>   // TicToc
#include <time.h>

using namespace std;

typedef pcl::PointXYZRGB PointT;
typedef pcl::PointCloud<PointT> PointCloud;
typedef pcl::PointNormal PointNormalT;
typedef pcl::PointCloud<PointNormalT> PointCloudWithNormals;
typedef pcl::PointCloud<pcl::PointXYZRGB>::Ptr PointXYZRGBPtr;
//����PCLFile�ṹ
struct PCLFile
{
	PointXYZRGBPtr cloud;
	std::string f_name;
	PCLFile() :cloud(new pcl::PointCloud<pcl::PointXYZRGB>) {};
};

//�Զ����࣬���Ա�ʾ������������Ϣ
class MyPointRepresentation :public pcl::PointRepresentation <PointNormalT>
{
	using pcl::PointRepresentation<PointNormalT>::nr_dimensions_;
public:
	MyPointRepresentation()
	{
		nr_dimensions_ = 4;
	}
	virtual void copyToFloatArray(const PointNormalT &p, float * out) const
	{
		out[0] = p.x;
		out[1] = p.y;
		out[2] = p.z;
		out[3] = p.curvature;
	}
};

//��׼�ľ��岽��
void AlignCloud(const PointXYZRGBPtr cloud_first, const PointXYZRGBPtr &cloud_second,
	PointXYZRGBPtr &cloud_afterTrans, PointXYZRGBPtr &cloud_after_add, Eigen::Matrix4f &final_transform, bool downsample = false)
{
	PointXYZRGBPtr First_could(new pcl::PointCloud<pcl::PointXYZRGB>);
	PointXYZRGBPtr Second_could(new pcl::PointCloud<pcl::PointXYZRGB>);

	//�²���
	pcl::VoxelGrid<PointT> grid;//�������ظ񣬽����Ʒ�����ֲ���ά�����У��������еĵ������ݽ����²���
	if (downsample==true)
	{
		grid.setLeafSize(0.2, 0.2, 0.2);
		grid.setInputCloud(cloud_first);
		grid.filter(*First_could);
		grid.setInputCloud(cloud_second);
		grid.filter(*Second_could);
	}
	else
	{
		First_could = cloud_first;
		Second_could = cloud_second;
	}

	//�������淨�ߺ�����
	PointCloudWithNormals::Ptr PointCloudNormals_1(new PointCloudWithNormals);
	PointCloudWithNormals::Ptr PointCloudNormals_2(new PointCloudWithNormals);
	pcl::NormalEstimation<PointT, PointNormalT> Cal_normal;
	pcl::search::KdTree<pcl::PointXYZRGB>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZRGB>());
	Cal_normal.setSearchMethod(tree);
	Cal_normal.setKSearch(30);
	Cal_normal.setInputCloud(First_could);
	Cal_normal.compute(*PointCloudNormals_1);
	//���Ƶ������굽������PointCloudNormals_1��
	pcl::copyPointCloud(*First_could, *PointCloudNormals_1);
	Cal_normal.setInputCloud(Second_could);
	Cal_normal.compute(*PointCloudNormals_2);
	pcl::copyPointCloud(*Second_could, *PointCloudNormals_2);

	//�����Զ�����ʵ��
	MyPointRepresentation Point_present;
	float weight_value[4] = { 1.0, 1.0, 1.0, 1.0 };
	Point_present.setRescaleValues(weight_value);//��������

												 //����������ICP����
	pcl::IterativeClosestPointNonLinear<PointNormalT, PointNormalT> icp;
	icp.setTransformationEpsilon(1e-7);//����������
	icp.setMaxCorrespondenceDistance(0.1);//���ö�Ӧ��֮��������룬0.1��Ϊ0.1m����׼�����У����ڸ���ֵ�����
	icp.setPointRepresentation(boost::make_shared<const MyPointRepresentation>(Point_present));
	icp.setInputCloud(PointCloudNormals_1);
	icp.setInputTarget(PointCloudNormals_2);
	icp.setMaximumIterations(10);//�ڲ��Ż��ĵ�������

	Eigen::Matrix4f MatrixTran = Eigen::Matrix4f::Identity(), prev, sourceToTarget;
	PointCloudWithNormals::Ptr icp_result = PointCloudNormals_1;
	int iter_num = 30;
	for (int i = 0; i < iter_num; ++i)
	{
		PCL_INFO("Iteration No. %d. \n", i);
		PointCloudNormals_1 = icp_result;
		icp.setInputCloud(PointCloudNormals_1);
		icp.align(*icp_result);
		MatrixTran = icp.getFinalTransformation()*MatrixTran;//ÿ�ε����ı任������ˣ��ۻ�
		//����˴�ת����ǰ���ۻ�ת�������С����ֵ����ͨ����С����Ӧ����������
		if (fabs((icp.getLastIncrementalTransformation() - prev).sum()) < icp.getTransformationEpsilon())
			icp.setMaxCorrespondenceDistance(icp.getMaxCorrespondenceDistance() - 0.01);
		prev = icp.getLastIncrementalTransformation();
	}
	//�����Դ���Ƶ�Ŀ�����
	sourceToTarget = MatrixTran;
	pcl::transformPointCloud(*cloud_afterTrans, *cloud_first, sourceToTarget);
	*cloud_after_add = *cloud_afterTrans + *cloud_second;
	final_transform = sourceToTarget;
}

//��һ���ַ���������Ӣ�������֣��������ܻ������ģ���ȡָ��λ��������
int getNumberInString(string istring,bool &hasnumbr)
{
	int number = 0;
	string filterstring;
	for (int i = istring.size(); i > istring.size() - 10 && istring[i] != '_'; i--)
	{
		filterstring += istring[i];
	}
	for (int i = filterstring.size(); i >0; i--)
	{
		if (filterstring[i] >= '0'&&filterstring[i] <= '9')
		{
			number = number * 10 + filterstring[i] - '0';
		}
	}
	if (number == 0)
		hasnumbr = false;
	return number;
}

//�����ļ�������ָ�����ִ�С��С��������
template <class T>
int reRangeFileName(vector<T>& files, vector<int> &SerialNumber)
{
	if (files.size() != SerialNumber.size())
	{
		cout << "The number of Files and Serial Number is wrong" << endl;
		return -1;
	}
	for (int i = 0; i<files.size(); i++)
	{
		for (int j = i; j < files.size(); j++)
		{
			if (SerialNumber[i] >= SerialNumber[j])
			{
				//swap(SerialNumber[i], SerialNumber[j]);
			
				int tmp;
				tmp = SerialNumber[j];
				SerialNumber[j] = SerialNumber[i];
				SerialNumber[i] = tmp;
				//swap(files[i], files[j]);
				T tmpname;
				tmpname = files[j];
				files[j] = files[i];
				files[i] = tmpname;
			}
		}

	}
	return 0;
}

//��ȡָ���ļ������ض���ʽ���ļ�
int GetAllFiles_CertainFormat(string path,vector<string>& files, string format)
{
	intptr_t hFile = 0;//_findnext��������Ϊintprt_t,����long���ͣ���intptr_tת����long���Ͷ�ʧ����
	struct  _finddata_t fileinfo;
	vector<int> SerialNumber;//���ڴ洢���
	string serialnumber;//��������
	bool hasnumber = true;//�����ж��ļ������Ƿ�������
	int nonum = 0;//������û������ʱ����
	string p; 
	if ((hFile = _findfirst(p.assign(path).append("/*"+format).c_str(), &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib & _A_SUBDIR))//�ж��Ƿ�Ϊ�ļ���
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, ".") != 0)
				{
					GetAllFiles_CertainFormat((p.assign(path).append("/")).append(fileinfo.name), files, format);

				}
			}
			else
			{
				serialnumber.append(fileinfo.name);
				int num = getNumberInString(serialnumber,hasnumber);
				//cout << "number:" << num << endl;
				if(hasnumber==true)
					SerialNumber.push_back(num);
				else
				{
					SerialNumber.push_back(nonum);
				}
				p.assign(path).append("/").append(fileinfo.name);
				files.push_back(p);
			}
			nonum++;
			serialnumber.clear();

		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
	reRangeFileName(files, SerialNumber);
	cout << "-------------------------------------------------" << endl;
	return 0;
}

//���һ����׼
int FinalRegistration(vector<PointXYZRGBPtr> Cloud_Temp_Sort1, PointXYZRGBPtr &Cloud_Temp_Last)
{
	PointXYZRGBPtr Cloud_Temp;
	vector<PointXYZRGBPtr> Cloud_temp_vector;
	Cloud_Temp = Cloud_Temp_Sort1[0];
	for (int i = 0; i<Cloud_Temp_Sort1.size(); i++)
		Cloud_temp_vector.push_back(Cloud_Temp_Sort1[i]);
	
	if (Cloud_Temp_Sort1.size() != 1)
	{
		for (int i = 0; i < Cloud_temp_vector.size()-1; i++)//for (int i = 0; i < PointAll.size(); i++)
		{
			cout << i << endl;
			cout << Cloud_temp_vector[i + 1]->size() << endl;
			DWORD PerStart = ::GetTickCount();
			ostringstream Registration_of;
			Registration_of << "Registration of " << i + 1 << " and " << i + 2 << endl;
			cout << Registration_of.str();
			PointXYZRGBPtr Cloud_after_Reg(new pcl::PointCloud<pcl::PointXYZRGB>);
			PointXYZRGBPtr Cloud_comblined(new pcl::PointCloud<pcl::PointXYZRGB>);
			Eigen::Matrix4f Trans_matrix_global = Eigen::Matrix4f::Identity(), Trans_matrix;
			AlignCloud(Cloud_Temp, Cloud_temp_vector[i + 1], Cloud_after_Reg, Cloud_comblined, Trans_matrix, true);
			cout << Cloud_temp_vector[i + 1]->size() << endl;
			*Cloud_Temp = *Cloud_comblined;
			DWORD PerEnd = ::GetTickCount();
			cout << "The time taken for PerRegistration is: " << PerEnd - PerStart << endl;
			ofstream fout("C:/Users/Zhihong MA/Desktop/data/1.txt");
			fout << Trans_matrix << endl;
		}

		*Cloud_Temp_Last = *Cloud_Temp;
	}
	else
		*Cloud_Temp_Last = *Cloud_Temp;
	return 0;
}

//������׼��ǰ�����������������
int PerRegistration(vector<PointXYZRGBPtr> PointAll, vector<PointXYZRGBPtr>  &Cloud_Temp_Sort1)
{
	int threadNumber = 4;
	vector<int> PointNumber;
	vector<vector<int>>threadnum(threadNumber);
	vector<vector<PointXYZRGBPtr>>OpenMpPointSet(threadNumber);
	vector<PointXYZRGBPtr> PointTemp;
#pragma omp parallel for num_threads(threadNumber)
	for (int i = 0; i < PointAll.size(); i = i + 2)
	{
		DWORD PerStart = ::GetTickCount();
		ostringstream Registration_of;
		Registration_of << "Registration of " << i + 1 << " and " << i + 2 << endl;//����д����֤��ӡ��ͬһ��
		cout << Registration_of.str();
		PointXYZRGBPtr Cloud_after_Reg(new pcl::PointCloud<pcl::PointXYZRGB>);
		PointXYZRGBPtr Cloud_comblined(new pcl::PointCloud<pcl::PointXYZRGB>);
		Eigen::Matrix4f Trans_matrix_global = Eigen::Matrix4f::Identity(), Trans_matrix;
		AlignCloud(PointAll[i], PointAll[i + 1], Cloud_after_Reg, Cloud_comblined, Trans_matrix, false);//��׼��������������ͳһ��Cloud_comblined��
		int k = omp_get_thread_num();
		threadnum[k].push_back(i*11+1);
		OpenMpPointSet[k].push_back(Cloud_comblined);
		//Cloud_Temp_Sort1[i] = Cloud_comblined;
		//PointNumber.push_back(i);
		ofstream fout("C:/Users/Zhihong MA/Desktop/data/3/1.txt");
		fout << Trans_matrix << endl;

		DWORD PerEnd = ::GetTickCount();
		ostringstream TakenTime;
		TakenTime << "The time taken for PerRegistration is: " << PerEnd - PerStart << endl;
		cout << TakenTime.str();
	}
	for (int i = 0; i < 4; i++)
	{
		if (threadnum[i].size() != OpenMpPointSet[i].size())
		{
			cout << "The number is wrong!" << endl;
			return -1;
		}
		for (int j = 0; j < threadnum[i].size(); j++)
		{
			PointNumber.push_back(threadnum[i][j]);
			PointTemp.push_back(OpenMpPointSet[i][j]);
		}
	}
	for (int i = 0; i < PointTemp.size(); i++)
	{
		Cloud_Temp_Sort1.push_back(PointTemp[i]);
	}
	if (Cloud_Temp_Sort1.size() == 1)
	{
		return 0;
	}
	//���������յ��Ʊ�Ŵ�С������
	reRangeFileName(Cloud_Temp_Sort1, PointNumber);
	//if (PointNumber.size() != Cloud_Temp_Sort1.size())
	//{
	//	cout << "The number of PointNumber and Cloud_Temp_Sort1 is wrong" << endl;
	//	return -1;
	//}
	//for (int i = 0; i < PointNumber.size(); i++)
	//{
	//	for (int j = i; j < PointNumber.size(); j++)
	//	{
	//		if (PointNumber[i] >= PointNumber[j])
	//		{
	//			swap(PointNumber[i], PointNumber[j]);
	//			swap(Cloud_Temp_Sort1[i], Cloud_Temp_Sort1[j]);
	//		}
	//	}
	//}
	return 0;
}

//�ж��Ƿ������һ����׼������
int SortAndContinue(vector<PointXYZRGBPtr> Cloud_Temp_Sort1, vector<PointXYZRGBPtr> &Cloud_Temp_Sort2,
	vector<PointXYZRGBPtr>&PointLeftAll, PointXYZRGBPtr &Cloud_Temp_Last)
{
	PointXYZRGBPtr Cloud_Final_temp(new pcl::PointCloud<pcl::PointXYZRGB>);
	//���ж��Ǹ���Ϊ������С��4�����6
	if (Cloud_Temp_Sort1.size() % 2 == 1 && Cloud_Temp_Sort1.size() < 4)
	{
		if (!PointLeftAll.empty())
		{
			for (int i= PointLeftAll.size()-1;i>=0 ;i--)
				Cloud_Temp_Sort1.push_back(PointLeftAll[i]);
		}
		cout << "Cloud_Temp_Sort1.size(): " << Cloud_Temp_Sort1.size() << endl;
		FinalRegistration(Cloud_Temp_Sort1, Cloud_Final_temp);
		*Cloud_Temp_Last = *Cloud_Final_temp;
	}
	//Ȼ���жϸ�����Ϊ�������Ǵ���5 �����
	else if (Cloud_Temp_Sort1.size() % 2 == 1 && Cloud_Temp_Sort1.size() >=4 )
	{
		PointLeftAll.push_back(Cloud_Temp_Sort1[Cloud_Temp_Sort1.size()-1]);
		Cloud_Temp_Sort1.pop_back();
		PerRegistration(Cloud_Temp_Sort1, Cloud_Temp_Sort2);
	}
	else
		PerRegistration(Cloud_Temp_Sort1, Cloud_Temp_Sort2);
	return 0;
}

int main_1()//��ѭ��1�Ⱥ�2��׼����׼����ں�3��׼��һֱ�����
{

	string pathdir = "F:/1-DesktopFile_Pointget/1-DATA/2017.11.11/50_5_2After";
	vector<string> FilesDirName;
	string format = ".pcd";
	GetAllFiles_CertainFormat(pathdir, FilesDirName, format);
	for (auto c : FilesDirName)
		cout << c << endl;
	cout << "------------" << endl;

	vector<PointXYZRGBPtr> PointAll;
	
	for (int i = 0; i < FilesDirName.size(); i++)
	{
		PointXYZRGBPtr cloud_IN(new pcl::PointCloud<pcl::PointXYZRGB>);
		int error=pcl::io::loadPCDFile(FilesDirName[i], *cloud_IN);
		if (error == -1)
		{
			PCL_WARN("Haven't load the Cloud First(The source one)!");
			return -1;
		}
		PointAll.push_back(cloud_IN);
	}

	PCL_INFO("Loaded");
	vector<PointXYZRGBPtr> PointAfterRegAll;
	PointXYZRGBPtr Cloud_Temp_Sort1(new pcl::PointCloud<pcl::PointXYZRGB>);
	Cloud_Temp_Sort1 = PointAll[0];
	DWORD Start = ::GetTickCount();
	for (int i = 0; i < 5; i++)//for (int i = 0; i < PointAll.size(); i++)
	{
		DWORD PerStart = ::GetTickCount();
		cout << "Registration of " << i + 1 << " and " << i + 2<< endl;
		PointXYZRGBPtr Cloud_after_Reg(new pcl::PointCloud<pcl::PointXYZRGB>);
		PointXYZRGBPtr Cloud_comblined(new pcl::PointCloud<pcl::PointXYZRGB>);
		Eigen::Matrix4f Trans_matrix_global = Eigen::Matrix4f::Identity(), Trans_matrix;
		AlignCloud(Cloud_Temp_Sort1, PointAll[i + 1], Cloud_after_Reg, Cloud_comblined, Trans_matrix, false);
		*Cloud_Temp_Sort1 = *Cloud_comblined;
		PointAfterRegAll.push_back(Cloud_after_Reg);
		DWORD PerEnd = ::GetTickCount();
		cout << "The time taken for PerRegistration is: " << PerEnd - PerStart << endl;
	}
	DWORD End = ::GetTickCount();
	cout << "The time taken for test is: " << End - Start << endl;
	string saveName = "C:/Users/Zhihong MA/Desktop/The_whole2.ply";
	pcl::io::savePLYFile(saveName, *Cloud_Temp_Sort1);
	return 0;
}

int main()//��80�飬��1��2��׼��3��4��׼��4��6��׼������Ȼ����12��34��׼��56��78��׼����һֱ�����λ�ã����ڶ��ѭ��
{
	//string pathdir = "F:/1-DesktopFile_Pointget/1-DATA/2017.11.11/50_5_2After";
	//string pathdir = "F:/1-DesktopFile_Pointget/1-DATA/xlr data/New folder";
	string pathdir = "C:/Users/Zhihong MA/Desktop/data/3";
	vector<string> FilesDirName;
	string format = ".pcd";
	vector<int> SerialNumber;//���ڴ洢���
	GetAllFiles_CertainFormat(pathdir, FilesDirName, format);
	for (auto c : FilesDirName)
		cout << c << endl;
	cout << "------------" << endl;
	vector<PointXYZRGBPtr> PointAll;
	for (int i = 0; i < FilesDirName.size(); i++)
	{
		PointXYZRGBPtr cloud_IN(new pcl::PointCloud<pcl::PointXYZRGB>);
		int error = pcl::io::loadPCDFile(FilesDirName[i], *cloud_IN);
		if (error == -1)
		{
			PCL_WARN("Haven't load the Cloud First(The source one)!");
			return -1;
		}
		PointAll.push_back(cloud_IN);
	}
	PCL_INFO("Loaded");
	cout << endl;
	////���ڲ���Ԥ,�ȼ�������
	int savenum = 60;
	while (PointAll.size() > savenum)
	{
		PointAll.pop_back();
	}

	vector<PointXYZRGBPtr> Cloud_Temp_Sort1;
	vector<PointXYZRGBPtr> Cloud_Temp_Sort2;
	vector<PointXYZRGBPtr> Cloud_Temp_Sort3;
	vector<PointXYZRGBPtr> Cloud_Temp_Sort4;
	vector<PointXYZRGBPtr> Cloud_Temp_Sort5;
	vector<PointXYZRGBPtr> Cloud_Temp_Sort6;
	vector<PointXYZRGBPtr> Cloud_Temp_Sort7;
	vector<PointXYZRGBPtr> Cloud_Temp_Sort8;
	vector<PointXYZRGBPtr> Cloud_Temp_Sort9;
	PointXYZRGBPtr Cloud_Temp_Last(new pcl::PointCloud<pcl::PointXYZRGB>);
	vector<PointXYZRGBPtr >PointLeftAll;

	DWORD Start = ::GetTickCount();
	//��1�η�����׼
	if (PointAll.size() % 2 == 1)
	{
		PointLeftAll.push_back(PointAll[PointAll.size()-1]);
		PointAll.pop_back();
	}
	PerRegistration(PointAll, Cloud_Temp_Sort1);
	//��2�η�����׼
	cout << "The Second  registration " << endl;
	SortAndContinue(Cloud_Temp_Sort1, Cloud_Temp_Sort2, PointLeftAll, Cloud_Temp_Last);
	//��3�η�����׼
	cout << "The Third  registration " << endl;
	SortAndContinue(Cloud_Temp_Sort2, Cloud_Temp_Sort3, PointLeftAll, Cloud_Temp_Last);
	//��4�η�����׼
	cout << "The Fourth  registration " << endl;
	SortAndContinue(Cloud_Temp_Sort3, Cloud_Temp_Sort4, PointLeftAll, Cloud_Temp_Last);
	//��5�η�����׼
	cout << "The Fifth  registration " << endl;
	SortAndContinue(Cloud_Temp_Sort4, Cloud_Temp_Sort5, PointLeftAll, Cloud_Temp_Last);
	//��6�η�����׼
	cout << "The Sixth  registration " << endl;
	SortAndContinue(Cloud_Temp_Sort5, Cloud_Temp_Sort6, PointLeftAll, Cloud_Temp_Last);
	//��7�η�����׼
	cout << "The Seventh  registration " << endl;
	SortAndContinue(Cloud_Temp_Sort6, Cloud_Temp_Sort7, PointLeftAll, Cloud_Temp_Last);
	//��8�η�����׼
	cout << "The Eighth  registration " << endl;
	SortAndContinue(Cloud_Temp_Sort7, Cloud_Temp_Sort8, PointLeftAll, Cloud_Temp_Last);
	//��9�η�����׼
	cout << "The Nineth  registration " << endl;
	SortAndContinue(Cloud_Temp_Sort8, Cloud_Temp_Sort9, PointLeftAll, Cloud_Temp_Last);

	DWORD End = ::GetTickCount();
	cout << "The time taken for test is: " << End - Start << endl;
	ostringstream saveName;
	saveName << "C:/Users/Zhihong MA/Desktop/data/" << savenum << ".pcd";
	//saveName << "C:/Users/Zhihong MA/Desktop/The_whole" << savenum << ".ply";
	pcl::io::savePCDFile(saveName.str(), *Cloud_Temp_Last);
	return 0;
}