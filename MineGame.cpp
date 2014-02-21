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
//�����׵���Ŀ
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
	/*����ܱ�ʾ
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
	//ÿ�����ֵ�0,3,6λ��
	for(int i = 1;i <= 5;i+=2)//  1/6    3/6   5/6��λ��
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
	for(int i = 1;i <= 3;i+=2)//  1/4    3/4��λ��
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

//��ȡ��Ϸ�Ĺ�ģ
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

//����������Ϸ��������
void CMineGame::UpDateTable( IplImage* frame )
{
	UpdateNumber(frame);
	UpDateSafe(frame);
}

//�������ָ���
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

//�������ֵ���ɫ��ƥ����ӵ�����
//��������и�ȷ��������7�Ǻ�ɫ�����׺���Ҳ�Ǻ�ɫ��
//������������죬ֻҪ��֤�������֮ǰû�е㵽��(����Ǹ���)��OK��
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

//���°�ȫ��λ��
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

//ƥ���Ƿ��ǰ�ȫ�ĸ���
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

//�������ص����귵�����������
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

//�ж��Ƿ���Ц�����ǿ���
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

//��ͨ����
list<CvPoint>* CMineGame::ExectNextAction()
{
	ChangeFlag = 0;
	list<CvPoint>* list_exe;
	//list_exe[0]�ǰ�ȫ��λ�ã�list_exe[1]��Σ�յ�λ��
	list_exe = new list<CvPoint>[2];
	for(int i = 0;i < Heigth;i++)
	{
		for(int j = 0;j < Width;j++)
		{
			if (Table[i][j] > 0)
			{
				//��i,j����Χ��ȷ������Ŀ
				list<CvPoint> list_unsure = GetSafeNumber(i,j,UNKNOWN);
				//��i,j����Χ�Ѿ�ȷ�����׵���Ŀ
				list<CvPoint> list_mine = GetSafeNumber(i,j,IS_MINE);
				//��ȷ����Ŀ==��������-ȷ�����׵���Ŀ����δ֪λ��һ������
				if (list_unsure.size() == Table[i][j] - list_mine.size())
				{
					for (list<CvPoint>::iterator iter = list_unsure.begin();iter != list_unsure.end();iter++)
					{
						Table[(*iter).x][(*iter).y] = IS_MINE;
						list_exe[1].push_front(*iter);
						ChangeFlag = 1;
					}
				}
				//ȷ�����׵���Ŀ==���е��׵���Ŀ����ʣ��δ֪����һ����ȫ
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

//IsSafe = 1 ���ذ�ȫ����Ŀ
//IsSafe = -1 ���ز�ȷ������Ŀ
//IsSafe = -2 ����Σ�յ���Ŀ
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

//�߼�����
//S1:(i,j)ʣ������� - (i,j)��(i',j')����δ֪�������׵���Ŀ��Ŀ = (i,j)�ķǹ�����ʾ������Ŀ
//->���зǹ���λ�������Ϊ��
//S2:(i,j)ʣ������� = ���������еĵ����� 
//->�ǹ��������Ϊ��ȫ
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

//����-1  �������������޷�ȷ�������޷�ʹ�ø߼�����
int CMineGame::PirorDealConjPoint( int i1,int j1,int i2,int j2, list<CvPoint> &NotCommonArea)
{
	list<CvPoint> CommonArea;//����δ֪�������Ŀ��λ��
	for(int i = i2-1;i <= i2+1;i++)//�õ�CommonArea
	{
		for(int j = j2-1;j <= j2+1 ;j++)
		{
			if (i <0 || j < 0 || i >= Heigth || j >= Width 
				|| (i == i2 && j == j2) || (i == i1 && j == j1))
				continue;//������Χ���߸�λ����Ч
			if (Table[i][j] == -1)
			{
				//�ڷǹ���������δ֪���ף����Իᵼ�¹������������޷�ȷ��
				if(((i-i1)*(i-i1)+(j-j1)*(j-j1))>2)
					return -1;
				else
					CommonArea.push_front(cvPoint(i,j));
			}

		}
	}
	//NotCommonArea��(i1,j1)����Χ����(i2,j2)���õ�δ֪�����λ��
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
	//(i1,j1)ʣ������
	int MainMineLeft = Table[i1][j1] - GetSafeNumber(i1,j1,IS_MINE).size();
	//(i2,j2)ʣ���������ȼ��ڹ�������ʣ������
	int AssistMineLeft = Table[i2][j2] - GetSafeNumber(i2,j2,IS_MINE).size();
	//�߼�����1����ʾNotCommonAreaȫ����
	if(MainMineLeft - AssistMineLeft == NotCommonArea.size())
	{
		if(NotCommonArea.size() != 0)
			return 1;
	}
	//�߼�����2����ʾNotCommonAreaȫ����ȫ
	if(MainMineLeft == AssistMineLeft)
	{
		if(NotCommonArea.size() != 0)
			return 2;
	}
	//�޷�ʹ�ø߼�����
	return -1;
}

//��������뺯��   3��7������ܡ���λ��
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

//�ж�ģ����ָ����λ���Ƿ�ƥ��
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
