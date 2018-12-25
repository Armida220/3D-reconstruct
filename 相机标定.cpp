#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <opencv.hpp>
#include <fstream>
#include <vector>
#include<opencv\cv.hpp>
#include <string>
#include <iostream>
#include<io.h>
//#include <iostream>  

using namespace std;
using namespace cv;


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

int reRangeFileName(vector<string>& files, vector<int> &SerialNumber)
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
				int tmp;
				tmp = SerialNumber[j];
				SerialNumber[j] = SerialNumber[i];
				SerialNumber[i] = tmp;
				string tmpname;
				tmpname = files[j];
				files[j] = files[i];
				files[i] = tmpname;
			}
		}

	}
	return 0;
}

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
	cout << "-------------------------------------------------" << endl;
	reRangeFileName(files, SerialNumber);//��������
	return 0;
}

int main()
{
	double time0 = static_cast<double>(getTickCount());
	ofstream fout("caliberation_result.txt");  /**    ���涨�������ļ�     **/
											   /************************************************************************
											   ��ȡÿһ��ͼ�񣬴�����ȡ���ǵ㣬Ȼ��Խǵ���������ؾ�ȷ��
											   *************************************************************************/

	cout << "��ʼ��ȡ�ǵ㡭����������" << endl;
	int image_count = 14;                    /****    ͼ������     ****/
	int corner_count;						 /****    ����ǵ�����     ****/
	Size image_size;                         /****    ͼ��ĳߴ�      ****/
	Size board_size = Size(14, 11);            /****    �������ÿ�С��еĽǵ���     ****/
	vector<Point2f> corners;                 /****    ����ÿ��ͼ���ϼ�⵽�Ľǵ�    ****/
	vector<vector<Point2f>>  corners_Seq;    /****    �����⵽�����нǵ�     ****/
	vector<Mat>  image_Seq;
	string DirAddress = "C:/Users/user/Desktop/Kinect2.0/CalibrationPictures";
	string format = ".png";
	stringstream address1;
	address1 << DirAddress << "/";
	vector<string> FilesName;
	GetAllFiles_CertainFormat(DirAddress, FilesName, format);
	vector<Mat>imageALL;
	for (int i = 0; i < FilesName.size(); i++)
	{
		imageALL[i] = imread(FilesName[i], 1);
	}

	int count = 0;
	for (int i = 0; i != image_count; i++)
	{
		/*��ȡ�ļ�*/
		stringstream filename, numoffile;
		filename << "C:/Users/user/Desktop/Kinect2.0/CalibrationPictures/Depth_" << i + 1 << ".png";
		numoffile << "ͼ��" << i + 1 << endl;								//����ͼ�񴰿�������������õ�
		string imageFileName, numoffilesrt;									//�����ļ������ļ��������ַ�����
		filename >> imageFileName;											//���ַ�������Ϣ���ݵ��ַ����С����ļ���
		cout << imageFileName << endl;										//����ļ���
		numoffile >> numoffilesrt;											//���ַ�������Ϣ���ݵ�
		Mat image = imread(imageFileName, 1);								//��ȡ�ļ���rgb��ʽ,0Ϊ�Ҷȸ�ʽ
		if (image.data == NULL)												//���δ��⵽���ļ�������ʾ����
		{
			cout << "�ļ���ȡ����/δ�ҵ����ļ�..." << endl;
			return -1;
		}
		image_size = image.size();											//��ȡͼ��ߴ�
		imshow("", image);
		waitKey(1);
		/* ��ȡ�ǵ� */
		Mat imageGray;														//����mat����Ҷ�ͼ
		cvtColor(image, imageGray, CV_RGB2GRAY);							// RGBͼ��ת��Ϊ�Ҷ�ͼ
		bool patternfound = findChessboardCorners(image, board_size, corners, CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK);
		//�������ͣ��ж��Ƿ������нǵ㣬�����ˣ�findchessboardcorners����1��bool�ж�Ϊtrue������Ϊfalse
		if (!patternfound)													//���patternfoundΪfasle����ô��ʾ���󣬽������򲢷��ظ�ϵͳ1
		{
			cout << "can not find chessboard corners!\n";
			continue;
			exit(1);
		}
		else																//���patternfoundΪtrue��ִ�����³���
		{	/* �����ؾ�ȷ��*/  //���������ؼ��ǵ�λ��
						 // ����ͼ�񣬽ǵ㣬�������ڣ��������������������������������ȣ�
			cornerSubPix(imageGray, corners, Size(19, 13), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
			/* ���Ƽ�⵽�Ľǵ㲢���� */
			Mat imageTemp = image.clone();									//����ͼ�����ڽ��нǵ���ȡ
			for (int j = 0; j < corners.size(); j++)
			{
				//circle(imageTemp, corners[j], 7, Scalar(0, 0, 255),1.8,8, 0);
				drawChessboardCorners(imageTemp, board_size, corners, patternfound);
				//���Ƽ�⵽�����̽ǵ�
				//����ͼ��ÿ��ÿ�нǵ���Ŀ����⵽�Ľǵ����飬ָʾ�����������Ƿ񱻷���
			}

			string imageFileName2;                                          //�����ǽǵ㲢���ߺ��ͼ��
			std::stringstream StrStm;
			StrStm << address1.str() << i + 1;
			StrStm >> imageFileName2;
			imageFileName2 += "-�ǵ�.jpg";
			/*			namedWindow(numoffilesrt, CV_WINDOW_NORMAL);
			imshow(numoffilesrt, imageTemp);
			waitKey(10);		          */                                   //�����Ϊˢ��
			imwrite(imageFileName2, imageTemp);		                         //���ͼ��
			cout << "ͼ��" << i + 1 << "�ǵ���ȡ���" << endl;
			count = count + corners.size();									 //count��¼�ǵ�������Corners.sizeΪÿ��ͼ��⵽�Ľǵ���
			corners_Seq.push_back(corners);									 //�����⵽�Ľǵ㵽����vector<Point2f>��

		}
		image_Seq.push_back(image);											 //����ͼ��mat������
	}
	cout << "�ǵ���ȡ��ɣ�\n";
	//getchar();
	//getchar();
	/************************************************************************
	���������
	*************************************************************************/
	cout << "��ʼ���ꡭ����������" << endl;
	Size square_size = Size(1, 1);											  /**** ʵ�ʲ����õ��Ķ������ÿ�����̸�Ĵ�С ****/
	vector<vector<Point3f>>  object_Points;                                   /****  ���涨����Ͻǵ����ά����   ****/
	Mat image_points = Mat(1, count, CV_32FC2, Scalar::all(0));				  /*****   ������ȡ�����нǵ�   *****/
	vector<int>  point_counts;												  /*****    ÿ��ͼ���нǵ������    ****/
	Mat intrinsic_matrix = Mat(3, 3, CV_32FC1, Scalar::all(0));               /*****    ������ڲ�������    ****/
	Mat distortion_coeffs = Mat(1, 4, CV_32FC1, Scalar::all(0));              /* �������4������ϵ����k1,k2,p1,p2 */
	vector<cv::Mat> rotation_vectors;										  /* ÿ��ͼ�����ת���� */
	vector<cv::Mat> translation_vectors;									  /* ÿ��ͼ���ƽ������ */

																			  /* ��ʼ��������Ͻǵ����ά���� */
	for (int t = 0; t<image_count; t++)
	{
		vector<Point3f> tempPointSet;										   //������ά������
		for (int i = 0; i<board_size.height; i++)                              //��ÿ������г�ʼ������
		{
			for (int j = 0; j<board_size.width; j++)
			{
				/* ���趨��������������ϵ��z=0��ƽ���� */
				Point3f tempPoint;
				tempPoint.x = i*square_size.width;
				tempPoint.y = j*square_size.height;
				tempPoint.z = 0;
				tempPointSet.push_back(tempPoint);
				//cout << "��ά������" << tempPoint << endl;
			}
		}
		object_Points.push_back(tempPointSet);                                   // ���涨����Ͻǵ����ά����
	}

	/* ��ʼ��ÿ��ͼ���еĽǵ��������������Ǽ���ÿ��ͼ���ж����Կ��������Ķ���� */
	for (int i = 0; i< image_count; i++)
	{
		point_counts.push_back(board_size.width*board_size.height);
	}

	/* ��ʼ���� */
	calibrateCamera(object_Points, corners_Seq, image_size, intrinsic_matrix, distortion_coeffs, rotation_vectors, translation_vectors, 0);

	//cout << "ÿ��ͼ��Ķ�����" << endl;

	cout << "������ɣ�\n";
	/************************************************************************
	�Զ�������������
	*************************************************************************/
	cout << "��ʼ���۶�����������������" << endl;
	double total_err = 0.0;                   /* ����ͼ���ƽ�������ܺ� */
	double err = 0.0;                        /* ÿ��ͼ���ƽ����� */
	vector<Point2f>  image_points2;             /****   �������¼���õ���ͶӰ��    ****/

	cout << "ÿ��ͼ��Ķ�����" << endl;
	//cout << "ÿ��ͼ��Ķ�����" << endl << endl;
	ofstream Internalparameterscout("C:/Users/user/Desktop/Kinect2.0/CalibrationPictures/ Depth�ڲ�ϵ��(�����ڲξ���).txt");
	for (int i = 0; i<image_count; i++)
	{
		vector<Point3f> tempPointSet = object_Points[i];
		/****    ͨ���õ������������������Կռ����ά���������ͶӰ���㣬�õ��µ�ͶӰ��     ****/
		projectPoints(tempPointSet, rotation_vectors[i], translation_vectors[i], intrinsic_matrix, distortion_coeffs, image_points2);
		/* �����µ�ͶӰ��;ɵ�ͶӰ��֮������*/
		vector<Point2f> tempImagePoint = corners_Seq[i];
		Mat tempImagePointMat = Mat(1, tempImagePoint.size(), CV_32FC2);
		Mat image_points2Mat = Mat(1, image_points2.size(), CV_32FC2);
		for (size_t i = 0; i != tempImagePoint.size(); i++)
		{
			image_points2Mat.at<Vec2f>(0, i) = Vec2f(image_points2[i].x, image_points2[i].y);
			tempImagePointMat.at<Vec2f>(0, i) = Vec2f(tempImagePoint[i].x, tempImagePoint[i].y);
		}
		err = norm(image_points2Mat, tempImagePointMat, NORM_L2);//��Ӧ��ֵ=����1��ӦԪ��-����2��ӦԪ�أ�L2���ֵƽ�����ۼӺͺ󿪸��ţ�ŷ����÷���
		total_err += err /= point_counts[i];
		cout << "��" << i + 1 << "��ͼ���ƽ����" << err << "����" << endl;
		fout << "��" << i + 1 << "��ͼ���ƽ����" << err << "����" << endl;
		Internalparameterscout << "��" << i + 1 << "��ͼ���ƽ����" << err << "����" << endl;
	}
	cout << "����ƽ����" << total_err / image_count << "����" << endl;
	fout << "����ƽ����" << total_err / image_count << "����" << endl << endl;
	cout << "������ɣ�" << endl;
	//getchar();
	/************************************************************************
	���涨����
	*************************************************************************/
	cout << "��ʼ���涨����������������" << endl;
	Mat rotation_matrix = Mat(3, 3, CV_32FC1, Scalar::all(0)); /* ����ÿ��ͼ�����ת���� */

	cout << "����ڲ�������" << endl;
	cout << intrinsic_matrix << endl;
	cout << "����ϵ����\n";
	cout << distortion_coeffs << endl;
	fout << "����ڲ�������" << endl;
	fout << intrinsic_matrix << endl;
	fout << "����ϵ����\n";
	fout << distortion_coeffs << endl;
	//����ڲ�ϵ��
	//ofstream Internalparameterscout("C:/Users/user/Desktop/����/������س���/grabcut/�ڲ�ϵ��(�����ڲξ���).txt");

	Internalparameterscout << "����ƽ����" << total_err / image_count << "����" << endl << endl;
	Internalparameterscout << "����ڲ�������" << endl;
	Internalparameterscout << intrinsic_matrix << endl;
	Internalparameterscout << "����ϵ����\n";
	Internalparameterscout << distortion_coeffs << endl;





	for (int i = 0; i<image_count; i++)
	{
		fout << "��" << i + 1 << "��ͼ�����ת������" << endl;
		fout << rotation_vectors[i] << endl;

		/* ����ת����ת��Ϊ���Ӧ����ת���� */
		Rodrigues(rotation_vectors[i], rotation_matrix);
		fout << "��" << i + 1 << "��ͼ�����ת����" << endl;
		fout << rotation_matrix << endl;
		fout << "��" << i + 1 << "��ͼ���ƽ��������" << endl;
		fout << translation_vectors[i] << endl;
		//������ļ���
		Internalparameterscout << "��" << i + 1 << "��ͼ�����ת����" << endl;
		Internalparameterscout << rotation_matrix << endl;
		Internalparameterscout << "��" << i + 1 << "��ͼ���ƽ��������" << endl;
		Internalparameterscout << translation_vectors[i] << endl;
	}
	Internalparameterscout.close();
	cout << "��ɱ���" << endl;
	fout << endl;
	getchar();
	/************************************************************************
	��ʾ������
	*************************************************************************/
	Mat mapx = Mat(image_size, CV_32FC1);
	Mat mapy = Mat(image_size, CV_32FC1);
	Mat R = Mat::eye(3, 3, CV_32F);
	cout << "�������ͼ��" << endl;
	for (int i = 0; i != image_count; i++)
	{
		cout << "Frame #" << i + 1 << "..." << endl;
		Mat newCameraMatrix = Mat(3, 3, CV_32FC1, Scalar::all(0));
		initUndistortRectifyMap(intrinsic_matrix, distortion_coeffs, R, intrinsic_matrix, image_size, CV_32FC1, mapx, mapy);
		Mat t = image_Seq[i].clone();
		cv::remap(image_Seq[i], t, mapx, mapy, INTER_LINEAR);
		string imageFileName;
		std::stringstream StrStm;
		StrStm << address1.str() << i + 1;
		StrStm >> imageFileName;
		imageFileName += "_d.jpg";
		imwrite(imageFileName, t);
	}
	cout << "�������" << endl;

	time0 = ((double)getTickCount() - time0) / getTickFrequency();
	cout << "�궨��ʱ:" << time0 << "��" << endl;

	/////************************************************************************
	////����һ��ͼƬz`z
	////*************************************************************************/
	//double time1 = static_cast<double>(getTickCount());
	//	cout << "TestImage ..." << endl;
	//	Mat newCameraMatrix = Mat(3, 3, CV_32FC1, Scalar::all(0));
	//	/*Mat testImage = imread("C:\\Users\\user\\Desktop\\1\\1.jpg", 1);*/
	//	for (int i = 0; i < 8; ++i)
	//	{
	//		stringstream testname,testasvename;
	//		//testname << "C:/Users/user/Desktop/test/1" << i << ".jpg" << endl;
	//		//testasvename << "C:/Users/user/Desktop/test/1" << i << "-����.jpg" << endl;
	//		testname << "C:/Users/user/Desktop/pointcloudshow/pic2/" << i+1 << ".png";
	//		testasvename << "C:/Users/user/Desktop/pointcloudshow/pic2/" << i+1 << "-����.jpg";
	//		cout << testasvename.str() << endl;
	//		Mat testImage = imread(testname.str(), 1);
	//		if (&(testImage.data) == NULL)
	//		{
	//			cout << "�޷��ҵ��ļ�" << endl;
	//			return -1;
	//		}
	//		//image_size.height = MAX(image_size.height,image_size.width);
	//		//image_size.width = MAX(image_size.height,image_size.width);
	//		initUndistortRectifyMap(intrinsic_matrix, distortion_coeffs, R, intrinsic_matrix, image_size, CV_32FC1, mapx, mapy);
	//		Mat t = testImage.clone();
	//		cv::remap(testImage, t, mapx, mapy, INTER_LINEAR);
	//		imwrite(testasvename.str(), t);
	//		
	//		cout << "�������" << endl;
	//	}

	//time1 = ((double)getTickCount() - time1) / getTickFrequency();
	//cout << "У����ʱ:" << time1 << "��" << endl;
	getchar();
	return 0;
}



