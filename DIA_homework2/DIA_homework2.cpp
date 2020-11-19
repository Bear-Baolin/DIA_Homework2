// DIA_homework2.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <opencv2/opencv.hpp>
#include <Windows.h>
#include <commdlg.h>
#include <ShlObj.h>
#include <fstream>

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
int main()
{
	Mat hazeimage,deimage;//
	//有雾图像的描述模型 I(x) = J(x)t(x) + A(1-t(x)) t(x)代表透射率，A：全局自然光
	/*其中𝑡(𝒙) = 𝑒^−𝛽𝑑𝒙 代表光线通过媒介透射照到相机的过程中
		没有被散射的比例，𝛽 : 大气的散射系数 𝑑 : 景深*/
	/*定义暗通道：𝑱𝑑𝑎𝑟𝑘 𝒙 = min𝒚∈Ω(𝒙)( min𝑐∈{𝑟,𝑔,𝑏}𝑱𝑐(𝒚))*/
	/*首先选取暗通道中最亮的0.1%像素，在这些像素中再选择灰度值最大的像素点作为大气光*/
	cout << guigetfilename() << endl;

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
