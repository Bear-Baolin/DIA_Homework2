// DIA_homework2.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/cvconfig.h>
#include <Windows.h>
#include <commdlg.h>
#include <ShlObj.h>
#include <fstream>
#include <opencv_modules.hpp>
#include <opencv2/core.hpp>

using namespace std;
using namespace cv;

//宽字节类型转化为窄字节类型
char* wcharTochar(const wchar_t* _wchar)
{
	char* _char;
	int len = WideCharToMultiByte(CP_ACP, 0, _wchar, (int)wcslen(_wchar), NULL, 0, NULL, NULL);
	_char = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, _wchar, (int)wcslen(_wchar), _char, len, NULL, NULL);
	_char[len] = '\0';
	return _char;
}
// 通过文件选择框获得选择文件的路径以及名字
char* guigetfilename()
{
	TCHAR szBuffer[MAX_PATH] = { 0 };
	OPENFILENAME ofn = { 0 };
	wchar_t* t = ofn.lpstrFile;
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szBuffer;
	ofn.lpstrFile[0] = { '\0' };
	ofn.nMaxFile = sizeof(szBuffer);
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = L"C:\\Program Files";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	if (GetOpenFileName(&ofn))
	{
		Mat HazeImage;//有雾的图像
		Mat Image;//去雾的图像
		t = ofn.lpstrFile;
		//cout << wcharTochar(t) << endl;
	}
	return wcharTochar(t);
}
//导向滤波器 输入：待滤波函数p,引导图像I,滤波后的图像q，窗的半径r，和调整参数eps
//输出类型void
void Darkchannels(Mat& input, Mat& output, int patchsize);//计算输入图像的暗通道并输出
void Transmission(Mat& dark_channels,Mat& Transmission,int ax,double w);  //计算投射率
void GuidedFilter(Mat& p, Mat& I, Mat& q,int r,double eps)
{
	//
	int p_row = p.rows;
	int p_col = p.cols;
    int p_type = p.type();
	Mat p_boxfilter,I_mean,p_mean,I_cor,Ip_cor,temp;//
	boxFilter(p,p_mean,-1,cv::Size(r,r));//对输入图像进行boxfilter滤波，本质上是一个均值滤波
	boxFilter(I, I_mean, -1, cv::Size(r, r));  //对引导图像进行均值滤波
	temp = I.mul(I);//计算导向图的自相关均值
	boxFilter(temp, I_cor, -1, cv::Size(r, r));
	temp = I.mul(p);//计算导向图与待滤波函数的点积
	boxFilter(temp, Ip_cor, -1, cv::Size(r, r));
	Mat I_var, Ip_cov;
	I_var = I_cor - I_mean.mul(I_mean);
	Ip_cov = Ip_cor - I_mean.mul(p_mean);
	Mat a, b,a_mean,b_mean;
	a = Ip_cov / (I_var + eps);
	b = p_mean - a.mul(I_mean);
	boxFilter(a, a_mean, -1, cv::Size(r, r));
	boxFilter(b, b_mean, -1, cv::Size(r, r));
	q = a_mean.mul(I) + b_mean;
}

void Minfilter(Mat& input, Mat& output, int patchsize)//最小值滤波
{
	int rows = input.rows;
	int cols = input.cols;
	int local_min = 255;
	int paddel = patchsize / 2;
	int temp;
	double minx, maxx;
	Rect patch_rect;
	Rect copy_rect;
	Mat copymat = Mat::ones(rows + patchsize, cols + patchsize, CV_8UC1)*255;
	Mat patchmat;
	patchmat.create(patchsize, patchsize, CV_8UC1);
	copy_rect.x = paddel;
	copy_rect.y = paddel;
	copy_rect.width = cols;
	copy_rect.height = rows;
	patch_rect.width = patchsize;
	patch_rect.height = patchsize;
	input.copyTo(copymat(copy_rect));//

	for (int i = 0; i < input.rows; i++)
	{
		for (int j = 0; j < input.cols; j++)
		{
			patch_rect.x = j;
			patch_rect.y = i;
			copymat(patch_rect).copyTo(patchmat);
			minMaxLoc(patchmat, &minx, &maxx, NULL, NULL);
			output.at<uchar>(i, j) = minx;
		}
	}
}
int main()
{
#pragma region 变量
	Mat frog_img;								//原有雾的图像
	Mat frog_img_channels[3];					//将原有雾的图像进行拆分
	Mat guide_img;								//导向滤波的引导图像
	Mat guidefiltered_img;						//导向滤波之后的图像
	Mat img_dark;								//暗通道先验图像
	Mat min_img_dark;							//对暗通道先验进行最小值滤波后的图像
	Mat DeFrog_img_channels[3];					//各通道的去雾结果
	Mat transmission;							//计算透射率的矩阵
	Mat DeFrog_img;								//去雾后的结果
	Mat frogimg_resize;							//
	Mat frogimg_sort;
	Mat sel_dark;
	int sizeofpatch;							//patch的大小
	int temp;						
	int cols;
	int rows;
	int num;
	int min_channels=255;						//暂存三个通道中的最小值
	double Al_intensity[3] = { 0,0,0 };			//保存三个通道计算的大气光强的值
	double min_dark;
	double max_dark;
	double atmosp=0;
	Point min_point;
	Point max_point;
#pragma endregion

	string filepath = guigetfilename();			//读取文件名称
	string path;								//获取文件路径
	string name;								//获取文件名
	//SortFlags sortflags;
	int lengthstr = filepath.length();			
	int strbegin;									
	strbegin = filepath.find_last_of("\\");
	name = filepath.substr(strbegin, lengthstr);//获取文件名
	path = filepath.substr(0, strbegin);		//获取文件路径
	frog_img = imread(filepath);				//读取图片信息  
	cols = frog_img.cols;
	rows = frog_img.rows;
	img_dark.create(frog_img.rows,frog_img.cols, CV_8UC1);
	min_img_dark.create(frog_img.rows, frog_img.cols, CV_8UC1);
	int channel = frog_img.channels();			//通道数
	Darkchannels(frog_img, img_dark, 11);		//计算暗通道先验
	//min_img_dark = img_dark;
	string darkname = filepath;
	darkname.insert(strbegin + 1, "dark_");
	imwrite(darkname, img_dark);
	transmission.create(rows, cols, CV_8UC1);
//	Minfilter(img_dark, min_img_dark, 3);		//对暗通道进行最小值滤波，得到最小值化之后的暗通道
	min_img_dark = img_dark;
	string min_darkname = filepath;
	min_darkname.insert(strbegin+1, "min_dark_");
	imwrite(min_darkname, min_img_dark);
	transmission.create(rows, cols, CV_8UC1);	//创建投射矩阵
	guidefiltered_img.create(rows, cols, CV_8UC1);
	string transname = filepath;
	transname.insert(strbegin + 1, "trans_");	
	imwrite(transname, transmission);			//保存投射模板
	minMaxLoc(min_img_dark, &min_dark, &max_dark, &min_point, &max_point);
	split(frog_img, frog_img_channels);			//对不同的通道进行操作
	frogimg_resize = frog_img;
	resize(min_img_dark, frogimg_resize, Size(cols * rows, 1));		    //将原图像存储为一维数组的形式
	cv::sort(frogimg_resize, frogimg_sort,16);							//将数组进行降序排列
	double thread = frogimg_sort.at<uchar>(0,int(cols * rows * 0.001)); //第0.1%的像素值
	sel_dark.create(min_img_dark.rows, min_img_dark.cols, min_img_dark.type());	//
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)			//估计自然光
		{
			for (int k = 0; k < 3; k++)
			{
				if (min_img_dark.at<uchar>(i, j) >= thread)
				{
					max_dark = frog_img.at<Vec3b>(i, j)[k];
					if (max_dark > atmosp)atmosp = max_dark;
				}
			}
			if (min_img_dark.at<uchar>(i, j) == frogimg_sort.at<uchar>(0,0))
			{
				max_dark = frog_img_channels[0].at<uchar>(i, j);
				if (max_dark > Al_intensity[0])Al_intensity[0] = max_dark;

				max_dark = frog_img_channels[1].at<uchar>(i, j);
				if (max_dark > Al_intensity[1])Al_intensity[1] = max_dark;

				max_dark = frog_img_channels[2].at<uchar>(i, j);
				if (max_dark > Al_intensity[2])Al_intensity[2] = max_dark;
			}
		}
	}

	Transmission(min_img_dark, sel_dark, atmosp, 1);
	string sel_path = filepath;
	sel_path.insert(strbegin + 1, "sel_");
	imwrite(sel_path, sel_dark);
	sel_dark.copyTo(transmission);
	guide_img = frog_img_channels[1];
	GuidedFilter(transmission, guide_img, guidefiltered_img, 3, 0.95);
	//计算恢复图像
	double tx[3];
	double src;
	double irc;
	for (int k = 0; k < 3; k++)
	{
		DeFrog_img_channels[k].create(rows, cols, CV_8UC1);
	}
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				tx[k] = transmission.at<uchar>(i, j);
				tx[k] = tx[k] / atmosp; 
				if (tx[k] < 0.1)tx[k] = 0.1;				//对投射率进行一定的处理
				src = frog_img_channels[k].at<uchar>(i, j);
				irc = (src - atmosp) / tx[k] + atmosp;
				irc = int(irc);
				if (irc > 255)irc = 255;
				if (irc < 0)irc = 0;
				DeFrog_img_channels[k].at<uchar>(i, j) = irc;
			}
		}
	}
	merge(DeFrog_img_channels,3,DeFrog_img);
	string defrogname = filepath;
	defrogname.insert(strbegin + 1, "defrog1_");
	imwrite(defrogname, DeFrog_img);
	return 0;
   // image = imread();//读取原图像
    
}
void Darkchannels(Mat& input, Mat& output, int patchsize)
{
	int rows = input.rows;								//行数
	int cols = input.cols;								//列数
	int paddel = patchsize / 2;							//需要补全的边
	double patch_min = 255;								//指向最小的值
	double x;											//暂存
	double min_channels[3];										//三个通道的最小值
	double max_channels[3];										//三个通道的最大值
	Mat temp[3];											//temp矩阵
	Mat patch;											//分布的patch
	Mat in_channels[3];									//将temp分为三个通道
	Rect rect_patch;									//ROI区域
	Rect rect_copy(paddel, paddel, cols, rows);
	split(input, in_channels);
	for (int k = 0; k < 3; k++)
	{
		temp[k] = Mat::ones(rows + patchsize, cols + patchsize, CV_8UC1) * 255;
		in_channels[k].copyTo(temp[k](rect_copy));
	}			
	rect_patch.width = patchsize;
	rect_patch.height = patchsize;
	for (int i = 0; i < rows-1; i++)
	{
		for (int j = 0; j < cols-1; j++)
		{
			rect_patch.x = j;
			rect_patch.y = i;
			for (int k = 0; k < 3; k++)					//分别计算每个通道的最小值
			{
				temp[k](rect_patch).copyTo(patch);
				minMaxLoc(patch, &min_channels[k], &max_channels[k], NULL,NULL);
			}
			patch_min = min(min_channels[0],min_channels[1],min_channels[2]);	//返回三个通道中最小值的最小值
			output.at<uchar>(i, j) = patch_min;									//并把其赋值给output中对应的元素
		}
	}
}
void Transmission(Mat& dark_channels, Mat& transmission, int ax, double w)
{
	double n;
	double m;
	for (int i = 0; i < dark_channels.rows; i++)
	{
		for (int j = 0; j < dark_channels.cols; j++)
		{
			m = dark_channels.at<uchar>(i, j);
			n = (ax - w * m);
			;
			transmission.at<uchar>(i, j) = n;
		}
	}
}

