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
	if (downsample == true)
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
	icp.setInputCloud(PointCloudNormals_2);
	icp.setInputTarget(PointCloudNormals_1);
	icp.setMaximumIterations(10);//�ڲ��Ż��ĵ�������

	Eigen::Matrix4f MatrixTran = Eigen::Matrix4f::Identity(), prev, sourceToTarget;
	PointCloudWithNormals::Ptr icp_result = PointCloudNormals_2;
	int iter_num = 30;
	for (int i = 0; i < iter_num; ++i)
	{
		PCL_INFO("Iteration No. %d. \n", i);
		PointCloudNormals_1 = icp_result;
		icp.setInputCloud(PointCloudNormals_2);
		icp.align(*icp_result);
		MatrixTran = icp.getFinalTransformation()*MatrixTran;//ÿ�ε����ı任������ˣ��ۻ�
		//����˴�ת����ǰ���ۻ�ת�������С����ֵ����ͨ����С����Ӧ����������
		if (fabs((icp.getLastIncrementalTransformation() - prev).sum()) < icp.getTransformationEpsilon())
			icp.setMaxCorrespondenceDistance(icp.getMaxCorrespondenceDistance() - 0.01);
		prev = icp.getLastIncrementalTransformation();
	}
	//�����Դ���Ƶ�Ŀ�����
	sourceToTarget = MatrixTran;
	//pcl::transformPointCloud(*cloud_afterTrans, *cloud_first, sourceToTarget);
	//*cloud_after_add = *cloud_afterTrans + *cloud_second;
	final_transform = sourceToTarget;
}

//��һ���ַ���������Ӣ�������֣��������ܻ������ģ���ȡָ��λ��������
int getNumberInString(string istring, bool &hasnumbr)
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
int GetAllFiles_CertainFormat(string path, vector<string>& files, string format)
{
	intptr_t hFile = 0;//_findnext��������Ϊintprt_t,����long���ͣ���intptr_tת����long���Ͷ�ʧ����
	struct  _finddata_t fileinfo;
	vector<int> SerialNumber;//���ڴ洢���
	string serialnumber;//��������
	bool hasnumber = true;//�����ж��ļ������Ƿ�������
	int nonum = 0;//������û������ʱ����
	string p;
	if ((hFile = _findfirst(p.assign(path).append("/*" + format).c_str(), &fileinfo)) != -1)
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
				int num = getNumberInString(serialnumber, hasnumber);
				//cout << "number:" << num << endl;
				if (hasnumber == true)
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

int main()//��ѭ��1�Ⱥ�2��׼����׼����ں�3��׼��һֱ�����
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
		int error = pcl::io::loadPCDFile(FilesDirName[i], *cloud_IN);
		if (error == -1)
		{
			PCL_WARN("Haven't load the Cloud First(The source one)!");
			return -1;
		}
		PointAll.push_back(cloud_IN);
	}
	PCL_INFO("Loaded");


	Eigen::Matrix4f Trans_matrix_global = Eigen::Matrix4f::Identity();

	//��׼
	vector<PointXYZRGBPtr> PointAfterRegAll;
	PointXYZRGBPtr Cloud_Temp_Sort1(new pcl::PointCloud<pcl::PointXYZRGB>);
	Cloud_Temp_Sort1 = PointAll[0];
	DWORD Start = ::GetTickCount();
	for (int i = 0; i < 5; i++)//for (int i = 0; i < PointAll.size(); i++)
	{
		DWORD PerStart = ::GetTickCount();
		cout << "Registration of " << i + 1 << " and " << i + 2 << endl;
		PointXYZRGBPtr Cloud_after_Reg(new pcl::PointCloud<pcl::PointXYZRGB>);
		//PointXYZRGBPtr Cloud_after_temp_source(new pcl::PointCloud<pcl::PointXYZRGB>);
		PointXYZRGBPtr Cloud_after_temp_target(new pcl::PointCloud<pcl::PointXYZRGB>);
		PointXYZRGBPtr Cloud_comblined(new pcl::PointCloud<pcl::PointXYZRGB>);
		Eigen::Matrix4f Trans_matrix;
		//pcl::copyPointCloud(*Cloud_after_temp_source, *PointAll[i + 1]);
		//pcl::copyPointCloud(*Cloud_after_temp_target, *PointAll[i]);
		AlignCloud(PointAll[i], PointAll[i + 1], Cloud_after_Reg, Cloud_comblined, Trans_matrix, false);
		//pcl::transformPointCloud(*PointAll[i + 1], *PointAll[i], Trans_matrix);
		//*Cloud_Temp_Sort1 = *Cloud_comblined;
		Trans_matrix_global = Trans_matrix_global*Trans_matrix;
		pcl::transformPointCloud(*PointAll[i + 1], *Cloud_after_temp_target, Trans_matrix_global);
		*Cloud_Temp_Sort1 += *Cloud_after_temp_target;
		PointAfterRegAll.push_back(Cloud_after_temp_target);
		DWORD PerEnd = ::GetTickCount();
		cout << "The time taken for PerRegistration is: " << PerEnd - PerStart << endl;
	}
	DWORD End = ::GetTickCount();
	cout << "The time taken for test is: " << End - Start << endl;
	string saveName = "C:/Users/Zhihong MA/Desktop/The_whole2.pcd";
	pcl::io::savePCDFile("C:/Users/Zhihong MA/Desktop/1.pcd", *Cloud_Temp_Sort1);
	return 0;
}
