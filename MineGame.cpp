#include "MineGame.h"

CMineGame::CMineGame(void)
{
	SafePic = cvLoadImage("./image_lib/safe.bmp");
	SmilePic = cvLoadImage("./image_lib/smile.bmp");
	SadPic = cvLoadImage("./image_lib/sad.bmp");
	WinFacePic = cvLoadImage("./image_lib/win_face.bmp");
}

CMineGame::~CMineGame(void)
{
}
//解析雷的数目
void CMineGame::GetMineNum( IplImage* frame)
{
	if(!frame)
	{
		MineNum = -1;
		return;
	}
	CvRect smg_rect;
	smg_rect.x = 17;
	smg_rect.y = 16;
	
	smg_rect.height = 23;
	smg_rect.width = 39;
	/*数码管标示
	    0
      1   2
	    3
	  4   5
	    6
	*/
	int SMG[3][7];
	for(int i = 0;i < 3;i++)
	{
		for(int j = 0;j < 7;j++)
		{
			SMG[i][j] = -1;
		}
	}
	//每个数字的0,3,6位置
	for(int i = 1;i <= 5;i+=2)//  1/6    3/6   5/6的位置
	{
		int y_counter = 0;
		for(int j = 0;j < smg_rect.height;j++)
		{
			if(CV_IMAGE_ELEM(frame,uchar,smg_rect.y+j,3*(smg_rect.x+smg_rect.width*i/6)+2) == 255)
			{
				SMG[(i-1)/2][y_counter] = 1;
				y_counter += 3;
				j += 4;
			}
			else if(CV_IMAGE_ELEM(frame,uchar,smg_rect.y+j,3*(smg_rect.x+smg_rect.width*i/6)+2) == 128)
			{
				SMG[(i-1)/2][y_counter] = 0;
				y_counter += 3;
				j += 4;
			}
		}
	}
	for(int i = 1;i <= 3;i+=2)//  1/4    3/4的位置
	{
		int x_counter = 0;
		for(int j = 0;j < smg_rect.width;j++)
		{
			if(CV_IMAGE_ELEM(frame,uchar,smg_rect.y+smg_rect.height*i/4,3*(smg_rect.x+j)+2) == 255)
			{
				SMG[int(x_counter/2)][x_counter%2+1+3*(i==3)] = 1;
				x_counter ++;
				j += 4;
			}
			else if(CV_IMAGE_ELEM(frame,uchar,smg_rect.y+smg_rect.height*i/4,3*(smg_rect.x+j)+2) == 128)
			{
				SMG[int(x_counter/2)][x_counter%2+1+3*(i==3)] = 0;
				x_counter ++;
				j += 4;
			}
		}
	}
	this->MineNum = Decode(SMG);
}

//获取游戏的规模
void CMineGame::GetGameSize( IplImage* frame ,IplImage* pic_Unknown)
{
	if(!frame || !pic_Unknown)
		return;
	CvPoint pos[99][99];
	CvPoint pos_pointer = cvPoint(0,0);
	for(int i = 0;i < frame->height-pic_Unknown->height;i++)
	{
		for(int j = 0;j < frame->width-pic_Unknown->width;j++)
		{
			if(TemplateMatch(frame,pic_Unknown,i,j))
			{
				pos[pos_pointer.y][pos_pointer.x] = cvPoint(j,i);
				pos_pointer.x++;
				j+=(pic_Unknown->width-1);
				if (j+pic_Unknown->width>= frame->width)
				{
					pos_pointer.y++;
					if(i+2*pic_Unknown->height < frame->height)
						pos_pointer.x = 0;
					
				}
			}
		}
	}
	this->Heigth = pos_pointer.y;
	this->Width = pos_pointer.x;
	Table = new int*[Heigth];
	TablePoint = new CvPoint*[Heigth];
	for(int i = 0;i < Heigth;i++)
	{
		Table[i] = new int[Width];
		TablePoint[i] = new CvPoint[Width];
		for(int j = 0;j < Width;j++)
		{
			Table[i][j] = UNKNOWN;
			TablePoint[i][j] = pos[i][j];
		}
	}
	UnknownGridHeigthSpan = TablePoint[1][0].y - TablePoint[0][0].y;
	UnknownGridWidthSpan = TablePoint[0][1].x - TablePoint[0][0].x;
}

//更新整个游戏的新数据
void CMineGame::UpDateTable( IplImage* frame )
{
	UpdateNumber(frame);
	UpDateSafe(frame);
}

//更新数字格子
void CMineGame::UpdateNumber( IplImage* frame )
{
	for(int i = 0;i < Heigth;i++)
	{
		for(int j = 0;j < Width;j++)
		{
			if (Table[i][j] == UNKNOWN)
			{
				int num = NumberMatch(frame,i,j);
				if(num > 0)
					Table[i][j] = num;
			}
		}
	}
}

//根据数字的颜色来匹配格子的数字
//这个方法有个确定，就是7是黑色，电雷后雷也是黑色的
//但是这个方法快，只要保证检测数字之前没有点到雷(检测那个脸)就OK了
int CMineGame::NumberMatch(IplImage* frame, int ii,int jj)
{
	for(int num = 0;num < 8;num++)
	{
		for(int i = -2;i <= 2;i++)
		{
			for(int j = -2;j <= 2;j++)
			{
				int y = TablePoint[ii][jj].y+UnknownGridHeigthSpan/2+i;
				int x = TablePoint[ii][jj].x+UnknownGridWidthSpan/2+j;
				if(CV_IMAGE_ELEM(frame,uchar,y,3*x) == NumberTemplate[num].CenterColor.val[0] &&
					CV_IMAGE_ELEM(frame,uchar,y,3*x+1) == NumberTemplate[num].CenterColor.val[1] &&
					CV_IMAGE_ELEM(frame,uchar,y,3*x+2) == NumberTemplate[num].CenterColor.val[2])
					return num+1;
			}
		}
	}
	return -1;
}

//更新安全的位置
void CMineGame::UpDateSafe( IplImage* frame )
{
	for(int i = TablePoint[0][0].y;i < frame->height;i++)
	{
		for(int j = TablePoint[0][0].x;j < frame->width;j++)
		{
			if(SafeMatch(frame,i,j))
			{
				CvPoint point = GetSafePoint(j,i);
				Table[point.y][point.x] = 0;
				j += SafePic->width - 2;
			}
		}
	}
}

//匹配是否是安全的格子
int CMineGame::SafeMatch( IplImage* frame,int ii,int jj )
{
	for(int i = 0;i < SafePic->height-1;i++)
	{
		for(int j = 0;j < SafePic->width-1;j++)
		{
			if(CV_IMAGE_ELEM(frame,uchar,ii+i,3*(jj+j)) != CV_IMAGE_ELEM(SafePic,uchar,i,3*j) ||
			   CV_IMAGE_ELEM(frame,uchar,ii+i,3*(jj+j)+1) != CV_IMAGE_ELEM(SafePic,uchar,i,3*j+1) ||
			   CV_IMAGE_ELEM(frame,uchar,ii+i,3*(jj+j)+2) != CV_IMAGE_ELEM(SafePic,uchar,i,3*j+2))
			   return 0;
		}
	}
	return 1;
}

//根据像素的坐标返回数组的坐标
CvPoint CMineGame::GetSafePoint( int x,int y )
{
	x += UnknownGridWidthSpan/2;
	y += UnknownGridHeigthSpan/2;
	CvPoint result = cvPoint(9999,9999);
	int mmmin = 9999;
	for(int i = 0;i < Width;i++)
	{
		if(abs(TablePoint[0][i].x - x)<mmmin)
		{
			mmmin = abs(TablePoint[0][i].x - x);
			result.x = i;
		}
		else
			break;
	}
	mmmin = 9999;
	for(int i = 0;i < Heigth;i++)
	{
		if(abs(TablePoint[i][0].y - y)<mmmin)
		{
			mmmin = abs(TablePoint[i][0].y - y);
			result.y = i;
		}
		else
			break;
	}
	return result;
}

//判断是否是笑脸或是哭脸
int CMineGame::GetSmileFace( IplImage* frame )
{
	for(int i = 0;i < frame->height-SmilePic->height;i++)
	{
		for(int j = 0;j < frame->width-SmilePic->width;j++)
		{
			if(TemplateMatch(frame,SmilePic,i,j))
			{
				FacePos = cvPoint(j,i);
				return 1;
			}
			if (TemplateMatch(frame,SadPic,i,j))
				return GAME_OVER;
			if (TemplateMatch(frame,WinFacePic,i,j))
				return WIN_GAME;
		}
	}
	return 0;
}

//普通策略
list<CvPoint>* CMineGame::ExectNextAction()
{
	ChangeFlag = 0;
	list<CvPoint>* list_exe;
	//list_exe[0]是安全的位置，list_exe[1]是危险的位置
	list_exe = new list<CvPoint>[2];
	for(int i = 0;i < Heigth;i++)
	{
		for(int j = 0;j < Width;j++)
		{
			if (Table[i][j] > 0)
			{
				//（i,j）周围不确定的数目
				list<CvPoint> list_unsure = GetSafeNumber(i,j,UNKNOWN);
				//（i,j）周围已经确定的雷的数目
				list<CvPoint> list_mine = GetSafeNumber(i,j,IS_MINE);
				//不确定数目==所有雷数-确定的雷的数目，则未知位置一定是累
				if (list_unsure.size() == Table[i][j] - list_mine.size())
				{
					for (list<CvPoint>::iterator iter = list_unsure.begin();iter != list_unsure.end();iter++)
					{
						Table[(*iter).x][(*iter).y] = IS_MINE;
						list_exe[1].push_front(*iter);
						ChangeFlag = 1;
					}
				}
				//确定的雷的数目==所有的雷的数目，则剩余未知区域一定安全
				if (list_mine.size() == Table[i][j] && list_unsure.size() > 0)
				{
					for (list<CvPoint>::iterator iter = list_unsure.begin();iter != list_unsure.end();iter++)
					{
						list_exe[0].push_front(*iter);
						ChangeFlag = 1;
					}
				}
			}
		}
	}
	return list_exe;
}

//IsSafe = 1 返回安全的数目
//IsSafe = -1 返回不确定的数目
//IsSafe = -2 返回危险的数目
list<CvPoint> CMineGame::GetSafeNumber( int ii,int jj,int IsSafe )
{
	list<CvPoint> list;
	for(int i = ii-1;i <= ii+1;i++)
	{
		for(int j = jj-1;j <= jj+1;j++)
		{
			if(i == ii && j == jj)
				continue;
			
			if(i >= 0 && j >= 0 && i < Heigth && j < Width)
			{
				if (IsSafe == 1 && Table[i][j] >= 0)				
					list.push_front(cvPoint(i,j));
				else if(Table[i][j] == IsSafe)
					list.push_front(cvPoint(i,j));
			}
		}
	}
	return list;
}

//高级策略
//S1:(i,j)剩余地雷数 - (i,j)与(i',j')公共未知区域中雷的数目数目 = (i,j)的非公共表示区域数目
//->所有非公共位置区域均为雷
//S2:(i,j)剩余地雷数 = 公共区域中的地雷数 
//->非公共区域均为安全
list<CvPoint>* CMineGame::PriorExectNextAction()
{
	list<CvPoint>* list_exe = NULL;
	for(int i = 0;i < Heigth;i++)
	{
		for(int j = 0;j < Width;j++)
		{
			if (Table[i][j] > 0)
			{
				list_exe = PirorDealPoint(i,j);
				if (list_exe != NULL && (list_exe[0].size() != 0 || list_exe[1].size() != 0))
					return list_exe;
			}
		}
	}
	return NULL;
}

list<CvPoint>* CMineGame::PirorDealPoint( int i,int j )
{
	list<CvPoint>* list_exe = new list<CvPoint>[2];
	list<CvPoint> NotCommonArea;
	for(int ii = i-1;ii <= i+1;ii++)
	{
		for(int jj = j-1;jj <= j+1;jj++)
		{
			if (ii == i && jj == j)
				continue;
			if(ii >= 0 && jj >= 0 && ii < Heigth && jj < Width && Table[ii][jj] > 0)
			{
				int result = PirorDealConjPoint(i,j,ii,jj,NotCommonArea);
				if (result < 0)
				{
					NotCommonArea.clear();
					continue;
				}
				else if(result == 1)
				{
					for (list<CvPoint>::iterator iter = NotCommonArea.begin();iter != NotCommonArea.end();iter++)
					{
						list_exe[1].push_front(*iter);
						Table[(*iter).x][(*iter).y] = IS_MINE;
					}
					return list_exe;
				}
				else if(result == 2)
				{
					for (list<CvPoint>::iterator iter = NotCommonArea.begin();iter != NotCommonArea.end();iter++)
					{
						list_exe[0].push_front(*iter);
					}
					return list_exe;
				}
			}
		}
	}
	return NULL;
}

//返回-1  公共区域雷数无法确定或者无法使用高级策略
int CMineGame::PirorDealConjPoint( int i1,int j1,int i2,int j2, list<CvPoint> &NotCommonArea)
{
	list<CvPoint> CommonArea;//公共未知区域的数目和位置
	for(int i = i2-1;i <= i2+1;i++)//得到CommonArea
	{
		for(int j = j2-1;j <= j2+1 ;j++)
		{
			if (i <0 || j < 0 || i >= Heigth || j >= Width 
				|| (i == i2 && j == j2) || (i == i1 && j == j1))
				continue;//超出范围或者该位置无效
			if (Table[i][j] == -1)
			{
				//在非公共区域有未知的雷，所以会导致公共区域雷数无法确定
				if(((i-i1)*(i-i1)+(j-j1)*(j-j1))>2)
					return -1;
				else
					CommonArea.push_front(cvPoint(i,j));
			}

		}
	}
	//NotCommonArea是(i1,j1)点周围不与(i2,j2)公用的未知区域的位置
	NotCommonArea.clear();
	for(int i = i1-1;i <= i1+1;i++)
	{
		for(int j = j1-1;j <= j1+1 ;j++)
		{
			if (i <0 || j < 0 || i >= Heigth || j >= Width || (i == i2 && j == j2) || (i == i1 && j == j1))
				continue;
			if (Table[i][j] == -1 && ((i-i2)*(i-i2)+(j-j2)*(j-j2))>2)
				NotCommonArea.push_front(cvPoint(i,j));
		}
	}
	//(i1,j1)剩余雷数
	int MainMineLeft = Table[i1][j1] - GetSafeNumber(i1,j1,IS_MINE).size();
	//(i2,j2)剩余雷数，等价于公共区域剩余雷数
	int AssistMineLeft = Table[i2][j2] - GetSafeNumber(i2,j2,IS_MINE).size();
	//高级策略1，表示NotCommonArea全是雷
	if(MainMineLeft - AssistMineLeft == NotCommonArea.size())
	{
		if(NotCommonArea.size() != 0)
			return 1;
	}
	//高级策略2，表示NotCommonArea全部安全
	if(MainMineLeft == AssistMineLeft)
	{
		if(NotCommonArea.size() != 0)
			return 2;
	}
	//无法使用高级策略
	return -1;
}

//数码管译码函数   3个7段数码管→三位数
int Decode( int SMG[3][7] )
{
	int temp[3] = {0,0,0};
	for(int i = 0;i < 3;i++)
	{
		int tt = 1;
		for (int j = 0;j < 7;j++)
		{
			temp[i] += tt * (SMG[i][j]==1);
			tt *= 2;
		}
		switch(temp[i])
		{
			case 119:
				temp[i] = 0;
				break;
			case 36:
				temp[i] = 1;
				break;
			case 93:
				temp[i] = 2;
				break;
			case 109:
				temp[i] = 3;
				break;
			case 46:
				temp[i] = 4;
				break;
			case 107:
				temp[i] = 5;
				break;
			case 123:
				temp[i] = 6;
				break;
			case 37:
				temp[i] = 7;
				break;
			case 127:
				temp[i] = 8;
				break;
			case 111:
				temp[i] = 9;
				break;
		}
	}
	return temp[0]*100+temp[1]*10+temp[2];
}

//判断模板在指定点位置是否匹配
int TemplateMatch( IplImage* frame,IplImage* Template,int y,int x )
{
	for(int i = 0;i < Template->height;i++)
	{
		for(int j = 0;j < Template->width;j++)
		{
			for(int k = 0;k < 3;k++)
			{
				if(CV_IMAGE_ELEM(frame,uchar,y+i,3*(x+j)+k) != CV_IMAGE_ELEM(Template,uchar,i,3*j+k))
					return 0;
			}
		}
	}
	return 1;
}
