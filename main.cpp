#ifdef _DEBUG
#pragma comment ( lib, "cxcore200d.lib" )
#pragma comment ( lib, "cv200d.lib" )
#pragma comment ( lib, "highgui200d.lib" )
#else
#pragma comment ( lib, "cxcore200.lib" )
#pragma comment ( lib, "cv200.lib" )
#pragma comment ( lib, "highgui200.lib" )
#endif

#include <afxwin.h>
#include "cv.h"
#include "highgui.h"
#include <time.h>
#include <iostream>
#include <stdlib.h>
#include <conio.h>
#include <afxwin.h>
#include <winuser.h>
#include "MineGame.h"
#include "NumberTemplate.h"
using namespace std;
#define WINDOW_NAME "window"
#define OBJECT_WINDOW_NAME _T("Minesweeper")
#define LEFT_BUTTON_PRESS 1
#define RIGHT_BUTTON_PRESS -1

#define ABS(x) (x>0?x:(x*-1))
#define M_MAX(x,y) (x>y ? x : y)
#define M_MIN(x,y) (x<y ? x : y)

//获取指定窗口图像
static void GetScreenShot(IplImage* &frame)
{
	CDC *pDC;//屏幕DC
	pDC = CDC::FromHandle(::GetDC(::FindWindow( NULL,OBJECT_WINDOW_NAME)));
	static int Flag = 0;
	static int BitPerPixel;
	static int Width;
	static int Height;
	static CDC memDC;//内存DC
	static CBitmap memBitmap;
	CBitmap *oldmemBitmap;//建立和屏幕兼容的bitmap
	if(!Flag)
	{
		BitPerPixel = pDC->GetDeviceCaps(BITSPIXEL);//获得颜色模式
		Width = pDC->GetDeviceCaps(HORZRES);
		Height = pDC->GetDeviceCaps(VERTRES);
		memDC.CreateCompatibleDC(pDC);
		memBitmap.CreateCompatibleBitmap(pDC, Width, Height);
	}
	oldmemBitmap = memDC.SelectObject(&memBitmap);//将memBitmap选入内存DC
	memDC.BitBlt(0, 0, Width, Height, pDC, 0, 0, SRCCOPY);//复制屏幕图像到内存DC

	BITMAP bmp;
	memBitmap.GetBitmap(&bmp);//获得位图信息

	static BITMAPINFOHEADER bih = {0};//位图信息头
	static byte* p;
	if(!Flag)
	{
		bih.biBitCount = bmp.bmBitsPixel;//每个像素字节大小
		bih.biCompression = BI_RGB;
		bih.biHeight = bmp.bmHeight;//高度
		bih.biPlanes = 1;
		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biSizeImage = bmp.bmWidthBytes * bmp.bmHeight;//图像数据大小
		bih.biWidth = bmp.bmWidth;//宽度
		p = new byte[bmp.bmWidthBytes * bmp.bmHeight];//申请内存保存位图数据

		Flag = 1;
	}

	GetDIBits(memDC.m_hDC, (HBITMAP) memBitmap.m_hObject, 0, Height, p,
		(LPBITMAPINFO) &bih, DIB_RGB_COLORS);//获取位图数据

	if(!frame)
	{
		CRect rect;
		GetClientRect(::FindWindow( NULL,OBJECT_WINDOW_NAME),&rect);
		//GetWindowRect获取的数据包括非客户区
		frame = cvCreateImage(cvSize(rect.Width(),rect.Height()),IPL_DEPTH_8U,3);
	}
	for(int i = 0;i < frame->height;i++)
	{
		for(int j = 0;j < frame->width;j++)
		{
			CV_IMAGE_ELEM(frame,uchar,i,3*j+2) = p[(bmp.bmHeight-i-1)*bmp.bmWidth*4+4*j+2];
			CV_IMAGE_ELEM(frame,uchar,i,3*j+1) = p[(bmp.bmHeight-i-1)*bmp.bmWidth*4+4*j+1];
			CV_IMAGE_ELEM(frame,uchar,i,3*j) = p[(bmp.bmHeight-i-1)*bmp.bmWidth*4+4*j];
		}
	}

}

//初始化数字的图片的模板，用中心颜色作为特征
void InitNumberTemplate(CNumberTemplate *&NumTemplate)
{
	NumTemplate = new CNumberTemplate[8];
	IplImage* img_num = NULL;
	char file_name[1024];
	for(int i = 1;i <= 8;i++)
	{
		sprintf_s(file_name,"./image_lib/%d.bmp",i);
		img_num = cvLoadImage(file_name);
		NumTemplate[i-1].Init(img_num,i);
		cvReleaseImage(&img_num);
	}
}

//鼠标点击API
void GameClick(int j,int i,int IsLeftbutton,CMineGame &Game)
{
	CPoint CurPoint;
	CRect rect;
	GetWindowRect(::FindWindow( NULL,OBJECT_WINDOW_NAME),&rect);
	GetCursorPos(&CurPoint);
	SetCursorPos(rect.TopLeft().x + Game.TablePoint[j][i].x + Game.UnknownGridWidthSpan/2,
		rect.TopLeft().y + Game.TablePoint[j][i].y + Game.UnknownGridHeigthSpan/2+48);
	if(IsLeftbutton == LEFT_BUTTON_PRESS)
	{
		mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);
		mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);
	}
	else if (IsLeftbutton == RIGHT_BUTTON_PRESS)
	{
		mouse_event(MOUSEEVENTF_RIGHTDOWN,0,0,0,0);
		mouse_event(MOUSEEVENTF_RIGHTUP,0,0,0,0);
	}
}

//随机点击一个未知的位置
void Randam_Click(CMineGame Game)
{
	int unknown_number = 0;
	for(int i = 0;i < Game.Heigth;i++)
	{
		for(int j = 0;j < Game.Width;j++)
		{
			if (Game.Table[i][j] == UNKNOWN)
				unknown_number++;
		}
	}
	if(0 == unknown_number)
		return;
	int ran_x,ran_y;
	while (1)
	{
		ran_x = rand() % Game.Width;
		ran_y = rand() % Game.Heigth;
		if (Game.Table[ran_y][ran_x] == -1)
		{
			GameClick(ran_y,ran_x,1,Game);
			return;
		}
	}
}

int main(int argc,char* argv[])
{
	IplImage* frame = NULL;
	IplImage* pic_Unknown = cvLoadImage("./image_lib/unknown.bmp");
	CMineGame Game;
	//初始化各个数字图像模板
	InitNumberTemplate(Game.NumberTemplate);
	//如果游戏窗口不是最前端，就不开始计算
	while(GetForegroundWindow() != (::FindWindow( NULL,OBJECT_WINDOW_NAME)))
		;
	GetScreenShot(frame);
	//如果检测不到笑脸，也不断循环
	while(Game.GetSmileFace(frame) != 1)
		GetScreenShot(frame);
	//解析出雷的数目
	Game.GetMineNum(frame);
	//解析出游戏规模大小
	Game.GetGameSize(frame,pic_Unknown);

	cout<<"Mine Number:"<<Game.MineNum<<endl;
	cout<<"Game Size:"<<Game.Heigth<<"X"<<Game.Width<<endl;

	list<CvPoint>* Operator_List = NULL;
	srand(time(0));
	//判断脸作为终结条件
	int game_state = Game.GetSmileFace(frame);
	while(game_state != GAME_OVER && game_state != WIN_GAME)
	{
		//获取图片
		GetScreenShot(frame);
		//根据图片更新数据
		Game.UpDateTable(frame);
		//获取低级策略结果
		Operator_List = Game.ExectNextAction();
		if (Game.ChangeFlag == 0) //低级策略不管用
		{
			//获取高级策略结果
			Operator_List = Game.PriorExectNextAction();  
			if(Operator_List == NULL)
				Randam_Click(Game);
			else if(Operator_List[0].size() != 0)//返回的是安全的位置
			{
				CvPoint temp = *(Operator_List[0].begin());
				GameClick(temp.x,temp.y,LEFT_BUTTON_PRESS,Game);
			}
			else if(Operator_List[1].size() != 0)//返回的是危险的位置
			{
				CvPoint temp = *(Operator_List[1].begin());
				GameClick(temp.x,temp.y,RIGHT_BUTTON_PRESS,Game);
			}
		}
		else //低级策略
		{
			if(Operator_List[1].size() != 0)
			{
				for (list<CvPoint>::iterator iter = Operator_List[1].begin();iter != Operator_List[1].end();iter++)
				{
					GameClick((*iter).x,(*iter).y,-1,Game);
				}
			}
			if(Operator_List[0].size() != 0)
			{
				for (list<CvPoint>::iterator iter = Operator_List[0].begin();iter != Operator_List[0].end();iter++)
				{
					GameClick((*iter).x,(*iter).y,1,Game);
				}
			}
		}
		//内存释放
		if(Operator_List != NULL && Operator_List[0].size() > 0)
			Operator_List[0].clear();
		if(Operator_List != NULL && Operator_List[1].size() > 0)
			Operator_List[1].clear();
		if (Operator_List != NULL)
			delete []Operator_List;
		game_state = Game.GetSmileFace(frame);
	}
	return 0;
}