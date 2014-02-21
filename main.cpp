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

//��ȡָ������ͼ��
static void GetScreenShot(IplImage* &frame)
{
	CDC *pDC;//��ĻDC
	pDC = CDC::FromHandle(::GetDC(::FindWindow( NULL,OBJECT_WINDOW_NAME)));
	static int Flag = 0;
	static int BitPerPixel;
	static int Width;
	static int Height;
	static CDC memDC;//�ڴ�DC
	static CBitmap memBitmap;
	CBitmap *oldmemBitmap;//��������Ļ���ݵ�bitmap
	if(!Flag)
	{
		BitPerPixel = pDC->GetDeviceCaps(BITSPIXEL);//�����ɫģʽ
		Width = pDC->GetDeviceCaps(HORZRES);
		Height = pDC->GetDeviceCaps(VERTRES);
		memDC.CreateCompatibleDC(pDC);
		memBitmap.CreateCompatibleBitmap(pDC, Width, Height);
	}
	oldmemBitmap = memDC.SelectObject(&memBitmap);//��memBitmapѡ���ڴ�DC
	memDC.BitBlt(0, 0, Width, Height, pDC, 0, 0, SRCCOPY);//������Ļͼ���ڴ�DC

	BITMAP bmp;
	memBitmap.GetBitmap(&bmp);//���λͼ��Ϣ

	static BITMAPINFOHEADER bih = {0};//λͼ��Ϣͷ
	static byte* p;
	if(!Flag)
	{
		bih.biBitCount = bmp.bmBitsPixel;//ÿ�������ֽڴ�С
		bih.biCompression = BI_RGB;
		bih.biHeight = bmp.bmHeight;//�߶�
		bih.biPlanes = 1;
		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biSizeImage = bmp.bmWidthBytes * bmp.bmHeight;//ͼ�����ݴ�С
		bih.biWidth = bmp.bmWidth;//���
		p = new byte[bmp.bmWidthBytes * bmp.bmHeight];//�����ڴ汣��λͼ����

		Flag = 1;
	}

	GetDIBits(memDC.m_hDC, (HBITMAP) memBitmap.m_hObject, 0, Height, p,
		(LPBITMAPINFO) &bih, DIB_RGB_COLORS);//��ȡλͼ����

	if(!frame)
	{
		CRect rect;
		GetClientRect(::FindWindow( NULL,OBJECT_WINDOW_NAME),&rect);
		//GetWindowRect��ȡ�����ݰ����ǿͻ���
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

//��ʼ�����ֵ�ͼƬ��ģ�壬��������ɫ��Ϊ����
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

//�����API
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

//������һ��δ֪��λ��
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
	//��ʼ����������ͼ��ģ��
	InitNumberTemplate(Game.NumberTemplate);
	//�����Ϸ���ڲ�����ǰ�ˣ��Ͳ���ʼ����
	while(GetForegroundWindow() != (::FindWindow( NULL,OBJECT_WINDOW_NAME)))
		;
	GetScreenShot(frame);
	//�����ⲻ��Ц����Ҳ����ѭ��
	while(Game.GetSmileFace(frame) != 1)
		GetScreenShot(frame);
	//�������׵���Ŀ
	Game.GetMineNum(frame);
	//��������Ϸ��ģ��С
	Game.GetGameSize(frame,pic_Unknown);

	cout<<"Mine Number:"<<Game.MineNum<<endl;
	cout<<"Game Size:"<<Game.Heigth<<"X"<<Game.Width<<endl;

	list<CvPoint>* Operator_List = NULL;
	srand(time(0));
	//�ж�����Ϊ�ս�����
	int game_state = Game.GetSmileFace(frame);
	while(game_state != GAME_OVER && game_state != WIN_GAME)
	{
		//��ȡͼƬ
		GetScreenShot(frame);
		//����ͼƬ��������
		Game.UpDateTable(frame);
		//��ȡ�ͼ����Խ��
		Operator_List = Game.ExectNextAction();
		if (Game.ChangeFlag == 0) //�ͼ����Բ�����
		{
			//��ȡ�߼����Խ��
			Operator_List = Game.PriorExectNextAction();  
			if(Operator_List == NULL)
				Randam_Click(Game);
			else if(Operator_List[0].size() != 0)//���ص��ǰ�ȫ��λ��
			{
				CvPoint temp = *(Operator_List[0].begin());
				GameClick(temp.x,temp.y,LEFT_BUTTON_PRESS,Game);
			}
			else if(Operator_List[1].size() != 0)//���ص���Σ�յ�λ��
			{
				CvPoint temp = *(Operator_List[1].begin());
				GameClick(temp.x,temp.y,RIGHT_BUTTON_PRESS,Game);
			}
		}
		else //�ͼ�����
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
		//�ڴ��ͷ�
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