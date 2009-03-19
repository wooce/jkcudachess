////////////////////////////////////////////////////////////////////////////////////////////////////////////
// �Y���JFenBoard.h                                                                                     //
// *******************************************************************************************************//
// ����H�ѳq�Τ���----�L�e���|�A����m����H�ѳq�Τ�����ĳ�n(Universal Chinese Chess Protocol�A²��ucci) //
// �@�̡J �S �w �x                                                                                        //
// ���J �����l���Ǭ�s�|                                                                            //
// �l�c�J fan_de_jun@sina.com.cn                                                                          //
//  QQ �J 83021504                                                                                        //
// *******************************************************************************************************//
// �\��J                                                                                                 //
// 1. �Nfen����Ƭ��ѽL�T��                                                                               //
// 2. �N�ѽL�T����Ƭ�fen��                                                                               //
////////////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#define  CChessMove  unsigned int


const int nPieceType[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// �L�l
	                       0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,  6,  6,  6,		// �¤l�J�Ө������H�h��
					       7,  8,  8,  9,  9, 10, 10, 11, 11, 12, 12, 13, 13, 13, 13, 13  };	// ���l�J�N�������ۥK�L

const int nPieceID[] = {  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// �L�l
	                       0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,  6,  6,  6,		// �¤l�J�Ө������H�h��
					       0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,  6,  6,  6  };	// ���l�J�N�������ۥK�L

class CFenBoard
{
public:
	CFenBoard(void);
	~CFenBoard(void);

public:	
	// �ǻ��ѧ����r����C�r���ꥲ���������A�_�h��^�X�ƤӤj�ɡAmoves�W�X�A�P�ѽL�ۤ��괫�ɷ|�o�ͦ��`���C
	char FenStr[2048];

public:
	// �Nfen����Ƭ��ѧ��T���A��^���\�Υ��Ѫ��лx
	int FenToBoard(int *Board, int *Piece, int &Player, unsigned int &nNonCapNum, unsigned int &nCurrentStep, const char *FenStr);

	// �N��e�ѧ���Ƭ�fen��A��^�ꪺ���w
	char *BoardToFen(const int *Board, int Player, const unsigned int nNonCapNum=0, const unsigned int nCurrentStep=1, CChessMove *StepRecords=0);

private:
	// �Nfen�r����Ƭ��Ѥl�Ǹ�
	int FenToPiece(char fen);	
};


inline unsigned int Coord(const CChessMove move)
{
    unsigned char RetVal[4];
	unsigned int  src = (move & 0xFF00) >> 8;
	unsigned int  dst = move & 0xFF;

	RetVal[0] = unsigned char(src & 0xF) -  3 + 'a';
	RetVal[1] = 12 - unsigned char(src >> 4 ) + '0';
	RetVal[2] = unsigned char(dst & 0xF) -  3 + 'a';
	RetVal[3] = 12 - unsigned char(dst >> 4 ) + '0';

	return *(unsigned int *) RetVal;
}

inline CChessMove Move(const unsigned int MoveStr) 
{
	unsigned char *ArgPtr = (unsigned char *) &MoveStr;
	unsigned int src = ((12-ArgPtr[1]+'0')<<4) + ArgPtr[0]-'a'+3;	// y0x0
	unsigned int dst = ((12-ArgPtr[3]+'0')<<4) + ArgPtr[2]-'a'+3;	// y1x1
	return ( src << 8 ) | dst;										// y0x0y1x1
}