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

//������׼��ǰ�����������������
int PerRegistration(vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr> PointAll, vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr>  &Cloud_Temp_Sort1)
{
	int threadNumber = 4;
	vector<int> PointNumber;
	vector<vector<int>>threadnum(threadNumber);
	vector<vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr>>OpenMpPointSet(threadNumber);
	vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr> PointTemp;
#pragma omp parallel for num_threads(threadNumber)
	for (int i = 0; i < PointAll.size(); i = i + 2)
	{
		int k = omp_get_thread_num();                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       
		threadnum[k].push_back(i);
		OpenMpPointSet[k].push_back(PointAll[i]);;
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
	cout << "Cloud_Temp_Sort1.size(): " << Cloud_Temp_Sort1.size() << endl;
	if (Cloud_Temp_Sort1.size() == 1)
	{
		return 0;
	}
	return 0;
}

//�ж��Ƿ������һ����׼������
int SortAndContinue(vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr> Cloud_Temp_Sort1,vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr> &Cloud_Temp_Sort2
, vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr> &PointLeftAll, pcl::PointCloud<pcl::PointXYZRGB>::Ptr Cloud_Temp_Last)
{
	vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr>Cloud_Temp;
	for (auto c : Cloud_Temp_Sort1)
		Cloud_Temp.push_back(c);
	//���ж��Ǹ���Ϊ������С��4�����
	if (Cloud_Temp_Sort1.size() % 2 == 1 && Cloud_Temp_Sort1.size() < 4)
	{
		if (!PointLeftAll.empty())
		{
			for (int i = PointLeftAll.size() - 1; i >= 0; i--)
				//Cloud_Temp_Sort1.push_back(PointLeftAll[i]);
				Cloud_Temp.push_back(PointLeftAll[i]);
		}
		//FinalRegistration(Cloud_Temp_Sort1, Cloud_Temp_Last);
		return 0;
	}
	//Ȼ���жϸ�����Ϊ�������Ǵ���5 �����
	else if (Cloud_Temp_Sort1.size() % 2 == 1 && Cloud_Temp_Sort1.size() >= 4)
	{
		PointLeftAll.push_back(Cloud_Temp_Sort1[Cloud_Temp_Sort1.size()-1]);
		Cloud_Temp.pop_back();
	}

	//Ȼ���жϸ�����Ϊ�������Ǵ���5 �����
	
	PerRegistration(Cloud_Temp, Cloud_Temp_Sort2);
	return 0;
}


int main_0()//��80�飬��1��2��׼��3��4��׼��4��6��׼������Ȼ����12��34��׼��56��78��׼����һֱ�����λ�ã����ڶ��ѭ��
{
	string pathdir = "F:/1-DesktopFile_Pointget/1-DATA/2017.11.11/50_5_2After";
	vector<string> FilesDirName;
	string format = ".pcd";
	vector<int> SerialNumber;//���ڴ洢���
	GetAllFiles_CertainFormat(pathdir, FilesDirName, format);
	for (auto c : FilesDirName)
		cout << c << endl;
	cout << "------------" << endl;
	vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr> PointAll;
	for (int i = 0; i < FilesDirName.size(); i++)
	{
		pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_IN(new pcl::PointCloud<pcl::PointXYZRGB>);
		int error = pcl::io::loadPCDFile(FilesDirName[i], *cloud_IN);
		if (error == -1)
		{
			PCL_WARN("Haven't load the Cloud First(The source one)!");
			return -1;
		}
		PointAll.push_back(cloud_IN);
	}
	PCL_INFO("Loaded");

	//���ڲ���Ԥ,�ȼ�������
	while (PointAll.size() > 10)
	{
		PointAll.pop_back();
	}


	vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr> Cloud_Temp_Sort1;
	vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr> Cloud_Temp_Sort2;
	vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr> Cloud_Temp_Sort3;
	vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr> Cloud_Temp_Sort4;
	vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr> Cloud_Temp_Sort5;
	vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr> Cloud_Temp_Sort6;
	vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr> Cloud_Temp_Sort7;
	vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr> Cloud_Temp_Sort8;
	vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr> Cloud_Temp_Sort9;
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr Cloud_Temp_Last(new pcl::PointCloud<pcl::PointXYZRGB>);
	vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr >PointLeftAll;

	DWORD Start = ::GetTickCount();
	//��1�η�����׼
	if (PointAll.size() % 2 == 1)
	{
		PointLeftAll.push_back(PointAll[PointAll.size()]);
		PointAll.pop_back();
	}
	//PointLeftAll.push_back(PointAll[PointAll.size()]);
	//*Cloud_Temp_Last = *PointAll[PointAll.size()];
	PerRegistration(PointAll, Cloud_Temp_Sort1);
	cout << "Cloud_Temp_Sort1111.size(): " << Cloud_Temp_Sort1.size() << endl;
	//��2�η�����׼
	SortAndContinue(Cloud_Temp_Sort1, Cloud_Temp_Sort2, PointLeftAll, Cloud_Temp_Last);
	//��3�η�����׼
	SortAndContinue(Cloud_Temp_Sort2, Cloud_Temp_Sort3, PointLeftAll, Cloud_Temp_Last);
	//��4�η�����׼
	SortAndContinue(Cloud_Temp_Sort3, Cloud_Temp_Sort4, PointLeftAll, Cloud_Temp_Last);
	//��5�η�����׼
	//SortAndContinue(Cloud_Temp_Sort4, Cloud_Temp_Sort5, PointLeftAll, Cloud_Temp_Last);
	////��6�η�����׼
	//SortAndContinue(Cloud_Temp_Sort5, Cloud_Temp_Sort6, PointLeftAll, Cloud_Temp_Last);
	////��7�η�����׼
	//SortAndContinue(Cloud_Temp_Sort6, Cloud_Temp_Sort7, PointLeftAll, Cloud_Temp_Last);
	////��8�η�����׼
	//SortAndContinue(Cloud_Temp_Sort7, Cloud_Temp_Sort8, PointLeftAll, Cloud_Temp_Last);
	////��9�η�����׼
	//SortAndContinue(Cloud_Temp_Sort8, Cloud_Temp_Sort9, PointLeftAll, Cloud_Temp_Last);

	DWORD End = ::GetTickCount();
	cout << "The time taken for test is: " << End - Start << endl;
	string saveName = "C:/Users/Zhihong MA/Desktop/The_whole3.ply";
	pcl::io::savePLYFile(saveName, *Cloud_Temp_Last);
	return 0;
}

int FinalRegistration(vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr>  Cloud_Temp_Sort1, PointXYZRGBPtr Cloud_Temp_Last)
{
	vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr> Cloud_temp;
	for (int i=0;i<Cloud_Temp_Sort1.size();i++)
		Cloud_temp.push_back(Cloud_Temp_Sort1[i]);
	Cloud_Temp_Last = Cloud_temp[1];
	if (Cloud_Temp_Sort1.size() != 1)
	{
		for (int i = 0; i < Cloud_Temp_Sort1.size(); i++)//for (int i = 0; i < PointAll.size(); i++)
		{
			DWORD PerStart = ::GetTickCount();
			cout << "Registration of: " << i + 1 << "-and-" << i + 2 << endl;
			PointXYZRGBPtr Cloud_after_Reg(new pcl::PointCloud<pcl::PointXYZRGB>);
			PointXYZRGBPtr Cloud_comblined(new pcl::PointCloud<pcl::PointXYZRGB>);
			*Cloud_Temp_Last = *Cloud_comblined;
			DWORD PerEnd = ::GetTickCount();
			cout << "The time taken for PerRegistration is: " << PerEnd - PerStart << endl;
		}
	}
	else
		Cloud_Temp_Last = Cloud_Temp_Sort1[0];
	return 0;
}

int main()
{
	string pathdir = "F:/1-DesktopFile_Pointget/1-DATA/2017.11.11/50_5_2After";
	vector<string> FilesDirName;
	string format = ".pcd";
	vector<int> SerialNumber;//���ڴ洢���
	GetAllFiles_CertainFormat(pathdir, FilesDirName, format);
	for (auto c : FilesDirName)
		cout << c << endl;
	cout << "------------" << endl;
	vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr> PointAll;
	for (int i = 0; i < FilesDirName.size(); i++)
	{
		pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_IN(new pcl::PointCloud<pcl::PointXYZRGB>);
		int error = pcl::io::loadPCDFile(FilesDirName[i], *cloud_IN);
		if (error == -1)
		{
			PCL_WARN("Haven't load the Cloud First(The source one)!");
			return -1;
		}
		PointAll.push_back(cloud_IN);
	}
	PCL_INFO("Loaded");

	//���ڲ���Ԥ,�ȼ�������
	do {
		PointAll.pop_back();
	} while (PointAll.size() > 9);
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr  Cloud_Temp_Last;
	FinalRegistration(PointAll, Cloud_Temp_Last);
	getchar();

}