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

int main()
{
	string filepath = guigetfilename();
	Mat frog_img,guidefiltered_img,guide_img;
	frog_img = imread(filepath);//读取图片信息
	Mat img_dark;//暗通道
	img_dark = 
	/*寻找暗通道*/
	guide_img = frog_img;
	GuidedFilter(frog_img, guide_img, guidefiltered_img, 3, 10);
	imwrite("guide_frog.jpg", guidefiltered_img);

	//有雾图像的描述模型 I(x) = J(x)t(x) + A(1-t(x)) t(x)代表透射率，A：全局自然光
	/*其中𝑡(𝒙) = 𝑒^−𝛽𝑑𝒙 代表光线通过媒介透射照到相机的过程中
		没有被散射的比例，𝛽 : 大气的散射系数 𝑑 : 景深*/
	/*定义暗通道：𝑱𝑑𝑎𝑟𝑘 𝒙 = min𝒚∈Ω(𝒙)( min𝑐∈{𝑟,𝑔,𝑏}𝑱𝑐(𝒚))*/
	/*首先选取暗通道中最亮的0.1%像素，在这些像素中再选择灰度值最大的像素点作为大气光*/
	//cout << guigetfilename() << endl;

	return 0;
   // image = imread();//读取原图像
    
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
