#pragma once

#include "cv.h"
#include "highgui.h"
#include "NumberTemplate.h"
#include "list"
using namespace std;
#define UNKNOWN -1
#define IS_MINE -2
#define WIN_GAME 1024
#define GAME_OVER 1023
class CMineGame
{
public:
	CMineGame(void);
	~CMineGame(void);


	void GetMineNum( IplImage* frame);
	void GetGameSize(IplImage* frame,IplImage* pic_Unknown);
	void UpDateTable(IplImage* frame);
	void UpdateNumber(IplImage* frame);
	int NumberMatch(IplImage* frame, int ii,int jj);
	void UpDateSafe(IplImage* frame);
	int SafeMatch(IplImage* frame,int ii,int jj);
	CvPoint GetSafePoint(int x,int y);
	int GetSmileFace(IplImage* frame);
	list<CvPoint>* ExectNextAction();
	list<CvPoint>* PriorExectNextAction();
	list<CvPoint> GetSafeNumber(int i,int j,int IsSafe);
	list<CvPoint>* PirorDealPoint(int i,int j);
	int PirorDealConjPoint( int i1,int j1,int i2,int j2, list<CvPoint> &NotCommonArea);
	CNumberTemplate* NumberTemplate;
	int **Table;
	CvPoint **TablePoint;//每个格子左上角的坐标
	int MineNum;
	int Heigth;
	int Width;
	int UnknownGridHeigthSpan;
	int UnknownGridWidthSpan;
	int ChangeFlag;
	IplImage* SafePic;
	IplImage* SmilePic;
	IplImage* SadPic;
	IplImage* WinFacePic;
	CvPoint FacePos;
};

int Decode(int SMG[3][7]);
int TemplateMatch(IplImage* frame,IplImage* pic_Unknown,int y,int x);