////////////////////////////////////////////////////////////////////////////////////////////////////////////
// �����JMoveGen.cpp                                                                                    //
// *******************************************************************************************************//
// ����H�ѳq�Τ���----�L�e���|�A����m����H�ѳq�Τ�����ĳ�n(Universal Chinese Chess Protocol�A²��ucci) //
// �@�̡J �S �w �x                                                                                        //
// ���J �����l���Ǭ�s�|                                                                            //
// �l�c�J fan_de_jun@sina.com.cn                                                                          //
//  QQ �J 83021504                                                                                        //
// *******************************************************************************************************//
// �\��J                                                                                                 //
// 1. ��¦����CMoveGen, CSearch�l���~�Ӥ��C�ѽL�B�Ѥl�B���B��C�B�۪k���ƾڦb�o�������Q�w�q�C           //
// 2. �q�β��ʲ��;�                                                                                      //
// 3. �Y�l���ʲ��;�                                                                                      //
// 4. �N�x�k�ײ��ʲ��;�                                                                                  //
// 5. ���Ⲿ�ʦX�k������                                                                                  //
// 6. �N�x�˴�Checked(Player), Checking(1-Player)                                                         //
////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "MoveGen.h"
#include "PreMove.h"

extern void call_vecAdd();

// �ѽL�ƲթM�Ѥl�Ʋ�
int Board[256];								// �ѽL�ƲաA��ܴѤl�Ǹ��J0��15�A�L�l; 16��31,�¤l; 32��47, ���l�F
int Piece[48];								// �Ѥl�ƲաA��ܴѽL��m�J0, ���b�ѽL�W; 0x33��0xCC, �����ѽL��m�F	

// ���P��C�ѽL
unsigned int xBitBoard[16];					// 16�Ӧ��A���ͨ�������V���ʡA�e12�즳��
unsigned int yBitBoard[16];					// 16�Ӧ�C�A���ͨ������a�V���ʡA�e13�즳��

// ������V�P�a�V���ʪ�16��ѽL�A�u�Τ_���Ⲿ�ʦX�k������B�N�x�˴��M�N�x�k��   							          
unsigned short xBitRookMove[12][512];		//  12288 Bytes, �������ѽL
unsigned short yBitRookMove[13][1024];		//  26624 Bytes  ������C�ѽL
unsigned short xBitCannonMove[12][512];		//  12288 Bytes  �������ѽL
unsigned short yBitCannonMove[13][1024];	//  26624 Bytes  ������C�ѽL
								  // Total: //  77824 Bytes =  76K

unsigned short HistoryRecord[65535];		// ���v�ҵo�A�ƲդU�Ь�: move = (nSrc<<8)|nDst;

const int xBitMask[256] = 
{
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   1,   2,   4,   8,  16,  32,  64, 128, 256,   0,   0,   0,   0,
	0,   0,   0,   1,   2,   4,   8,  16,  32,  64, 128, 256,   0,   0,   0,   0,
	0,   0,   0,   1,   2,   4,   8,  16,  32,  64, 128, 256,   0,   0,   0,   0,
	0,   0,   0,   1,   2,   4,   8,  16,  32,  64, 128, 256,   0,   0,   0,   0,
	0,   0,   0,   1,   2,   4,   8,  16,  32,  64, 128, 256,   0,   0,   0,   0,
	0,   0,   0,   1,   2,   4,   8,  16,  32,  64, 128, 256,   0,   0,   0,   0,
	0,   0,   0,   1,   2,   4,   8,  16,  32,  64, 128, 256,   0,   0,   0,   0,
	0,   0,   0,   1,   2,   4,   8,  16,  32,  64, 128, 256,   0,   0,   0,   0,
	0,   0,   0,   1,   2,   4,   8,  16,  32,  64, 128, 256,   0,   0,   0,   0,
	0,   0,   0,   1,   2,   4,   8,  16,  32,  64, 128, 256,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

const int yBitMask[256] = 
{
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,
	0,   0,   0,   2,   2,   2,   2,   2,   2,   2,   2,   2,   0,   0,   0,   0,
	0,   0,   0,   4,   4,   4,   4,   4,   4,   4,   4,   4,   0,   0,   0,   0,
	0,   0,   0,   8,   8,   8,   8,   8,   8,   8,   8,   8,   0,   0,   0,   0,
	0,   0,   0,  16,  16,  16,  16,  16,  16,  16,  16,  16,   0,   0,   0,   0,
	0,   0,   0,  32,  32,  32,  32,  32,  32,  32,  32,  32,   0,   0,   0,   0,
	0,   0,   0,  64,  64,  64,  64,  64,  64,  64,  64,  64,   0,   0,   0,   0,
	0,   0,   0, 128, 128, 128, 128, 128, 128, 128, 128, 128,   0,   0,   0,   0,
	0,   0,   0, 256, 256, 256, 256, 256, 256, 256, 256, 256,   0,   0,   0,   0,
	0,   0,   0, 512, 512, 512, 512, 512, 512, 512, 512, 512,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

// �N�������H�h�L
static const int MvvValues[48] = 
{
      0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  10000, 2000, 2000, 1096, 1096, 1088, 1088, 1040, 1040, 1041, 1041, 1017, 1018, 1020, 1018, 1017,
  10000, 2000, 2000, 1096, 1096, 1088, 1088, 1040, 1040, 1041, 1041, 1017, 1018, 1020, 1018, 1017 
};

// ���L�W�q��J����ӥ\��
// 1. nHorseLegTab[nDst-nSrc+256] != 0				// �����������顨�l�A�O���X�k����
// 2. nLeg = nSrc + nHorseLegTab[nDst-nSrc+256]		// ���L����l
//    Board[nLeg] == 0								// ���L��m�S���Ѥl�A���i�H�qnSrc���ʨ�nDst
const char nHorseLegTab[512] = {
                               0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,-16,  0,-16,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0, -1,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0, -1,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0, 16,  0, 16,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0
};


// ���ʤ�V���ޡJ�A�X��Z�����ʪ��Ѥl
// 0 --- ���ಾ�ʨ쨺��
// 1 --- �W�U���k�A�A�X�N�өM�L�򪺲���
// 2 --- �h�����F
// 3 --- �������F
// 4 --- �H�����F
const char nDirection[512] = {
                                   0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  4,  3,  0,  3,  4,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  3,  2,  1,  2,  3,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  1,  0,  1,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  3,  2,  1,  2,  3,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  4,  3,  0,  3,  4,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0
};


// �۪k�w���ͼƲ�
	// �Y�l���� + ���q����
static unsigned char KingMoves[256][8];				//   2048 Bytes, �N�����ʼƲ�
static unsigned char xRookMoves[12][512][12];		//  73728 Bytes, ������V����
static unsigned char yRookMoves[13][1024][12];		// 159744 Bytes, �����a�V����
static unsigned char xCannonMoves[12][512][12];		//  73728 Bytes, ������V����
static unsigned char yCannonMoves[13][1024][12];	// 159744 Bytes, �����a�V����
static unsigned char KnightMoves[256][12];			//   3072 Bytes, �������ʼƲ�
static unsigned char BishopMoves[256][8];			//   2048 Bytes, �H�����ʼƲ�
static unsigned char GuardMoves[256][8];			//   2048 Bytes, �h�����ʼƲ�
static unsigned char PawnMoves[2][256][4];		    //   2048 Bytes, �L�����ʼƲաJ0-�¨�A 1-���L
										     // Total: 478208 Bytes = 467KB
	// �Y�l����
static unsigned char xRookCapMoves[12][512][4];	//  24576 Bytes, ������V����
static unsigned char yRookCapMoves[13][1024][4];	//  53248 Bytes, �����a�V����
static unsigned char xCannonCapMoves[12][512][4];	//  24576 Bytes, ������V����
static unsigned char yCannonCapMoves[13][1024][4];	//  53248 Bytes, �����a�V����
										     // Total: 155648 Bytes = 152KB

CMoveGen::CMoveGen(void)
{
	// �b��ӵ{���ͦs�L�{���ACPreMove�u�ݽեΤ@��
	CPreMove PreMove;
	
	PreMove.InitKingMoves(KingMoves);
	PreMove.InitRookMoves(xRookMoves, yRookMoves, xRookCapMoves, yRookCapMoves);
	PreMove.InitCannonMoves(xCannonMoves, yCannonMoves, xCannonCapMoves, yCannonCapMoves);
	PreMove.InitKnightMoves(KnightMoves);
	PreMove.InitBishopMoves(BishopMoves);
	PreMove.InitGuardMoves(GuardMoves);
	PreMove.InitPawnMoves(PawnMoves);

	// ��l�ƥΤ_������V�P�a�V���ʪ�16��ѽL
	PreMove.InitBitRookMove(xBitRookMove, yBitRookMove);
	PreMove.InitBitCannonMove(xBitCannonMove, yBitCannonMove);
}

CMoveGen::~CMoveGen(void)
{
}


// ��s���v�O��
// nMode==0  �M�����v�O��
// nMode!=0  �I����v�O���AHistoryRecord[m] >>= nMode;
void CMoveGen::UpdateHistoryRecord(unsigned int nMode)
{
	unsigned int m;
	unsigned int max_move = 0xCCCC;			// ���ʪ��̤j��0xFFFF�A0xCCCC�Z�������ʤ��|�Ψ�

	if( nMode )								// �I����v����
	{
		for(m=0; m<max_move; m++)
			HistoryRecord[m] >>= nMode;
	}
	else									// �M�s�A�Τ_�s������
	{
		for(m=0; m<max_move; m++)
			HistoryRecord[m] = 0;
	}
}

// �ھ�Board[256], ���ͩҦ��X�k�����ʡJ Player==-1(�¤�), Player==+1(����)
// ���ʦ��Ǳq�ĥ� MVV/LVA(�Y�l����) �M ���v�ҵo���X�A���W�����v�ҵo�֤F10%
int CMoveGen::MoveGenerator(const int Player, CChessMove* pGenMove)
{
	call_vecAdd();
	const unsigned int  k = (1+Player) << 4;	    //k=16,�´�; k=32,���ѡC
	unsigned int  move, nSrc, nDst, x, y, nChess;
	CChessMove* ChessMove = pGenMove;		//���ʪ��p�ƾ�
	unsigned char *pMove;
							
	// ���ͱN�Ӫ�����********************************************************************************************
	nChess = k;
	nSrc = Piece[nChess];								// �N�Ӧs�b�JnSrc!=0
	{
		pMove = KingMoves[nSrc];
		while( *pMove )
		{
			nDst = *(pMove++);
			if( !Board[nDst] )
			{
				move = (nSrc<<8) | nDst;
				*(ChessMove++) = (HistoryRecord[move]<<16) | move;
			}
		}
	}
	nChess ++;


	// ���ͨ�������************************************************************************************************
	for( ; nChess<=k+2; nChess++)
	{
		if( nSrc = Piece[nChess] )						// �Ѥl�s�b�JnSrc!=0
		{			
			x = nSrc & 0xF;								// �Z4�즳��
			y = nSrc >> 4;								// �e4�즳��

			//������V���ʡJ
			pMove = xRookMoves[x][xBitBoard[y]];
			while( *pMove )
			{
				nDst = (nSrc & 0xF0) | (*(pMove++));	// 0x y|x  �e4��=y*16�A �Z4��=x
				if( !Board[nDst] )
				{
					move = (nSrc<<8) | nDst;
					*(ChessMove++) = (HistoryRecord[move]<<16) | move;
				}
			}

			//�����a�V����
			pMove = yRookMoves[y][yBitBoard[x]];
			while( *pMove )
			{
				nDst = (*(pMove++)) | x;				// 0x y|x  �e4��=y*16�A �Z4��=x
				if( !Board[nDst] )
				{
					move = (nSrc<<8) | nDst;
					*(ChessMove++) = (HistoryRecord[move]<<16) | move;
				}
			}
		}
	}


	// ���ͬ�������************************************************************************************************
	for( ; nChess<=k+4; nChess++)
	{
		if( nSrc = Piece[nChess] )						// �Ѥl�s�b�JnSrc!=0
		{			
			x = nSrc & 0xF;								// �Z4�즳��
			y = nSrc >> 4;								// �e4�즳��

			//������V����
			pMove = xCannonMoves[x][xBitBoard[y]];
			while( *pMove )
			{
				nDst = (nSrc & 0xF0) | (*(pMove++));	// 0x y|x  �e4��=y*16�A �Z4��=x
				if( !Board[nDst] )
				{
					move = (nSrc<<8) | nDst;
					*(ChessMove++) = (HistoryRecord[move]<<16) | move;
				}
			}

			//�����a�V����
			pMove = yCannonMoves[y][yBitBoard[x]];
			while( *pMove )
			{
				nDst = (*(pMove++)) | x;		// 0x y|x  �e4��=y*16�A �Z4��=x	
				if( !Board[nDst] )
				{
					move = (nSrc<<8) | nDst;
					*(ChessMove++) = (HistoryRecord[move]<<16) | move;
				}
			}
		}
	}


	// ���Ͱ�������******************************************************************************************
	for( ; nChess<=k+6; nChess++)
	{
		if( nSrc = Piece[nChess] )						// �Ѥl�s�b�JnSrc!=0
		{
			pMove = KnightMoves[nSrc];
			while( *pMove )
			{
				nDst = *(pMove++);
				if( !Board[nSrc+nHorseLegTab[nDst-nSrc+256]] )
				{					
					if( !Board[nDst] )
					{
						move = (nSrc<<8) | nDst;
						*(ChessMove++) = (HistoryRecord[move]<<16) | move;
					}
				}
			}
		}
	}


	// ���ͶH������******************************************************************************************
	for( ; nChess<=k+8; nChess++)
	{
		if( nSrc = Piece[nChess] )						// �Ѥl�s�b�JnSrc!=0
		{
			pMove = BishopMoves[nSrc];
			while( *pMove )
			{
				nDst = *(pMove++);
				if( !Board[(nSrc+nDst)>>1] )				//�H���L�l
				{
					if( !Board[nDst] )
					{
						move = (nSrc<<8) | nDst;
						*(ChessMove++) = (HistoryRecord[move]<<16) | move;
					}
				}
			}
		}
	}


	// ���ͤh������******************************************************************************************
	for( ; nChess<=k+10; nChess++)
	{
		if( nSrc = Piece[nChess] )						// �Ѥl�s�b�JnSrc!=0
		{
			pMove = GuardMoves[nSrc];
			while( *pMove )
			{
				nDst = *(pMove++);
				if( !Board[nDst] )
				{
					move = (nSrc<<8) | nDst;
					*(ChessMove++) = (HistoryRecord[move]<<16) | move;
				}
			}
		}
	}


	// ���ͧL�򪺲���******************************************************************************************
	for( ; nChess<=k+15; nChess++)
	{
		if( nSrc = Piece[nChess] )						// �Ѥl�s�b�JnSrc!=0
		{
			pMove = PawnMoves[Player][nSrc];
			while( *pMove )
			{
				nDst = *(pMove++);
				if( !Board[nDst] )
				{
					move = (nSrc<<8) | nDst;
					*(ChessMove++) = (HistoryRecord[move]<<16) | move;
				}
			}
		}
	}

	return int(ChessMove-pGenMove);
}


// ���ͩҦ��X�k���Y�l����
// �g���R�A�Y�l���ʥe�j���ɶ���23%�C
// ���O�̺C���A�]�������K��F�L��䦸�A�D�n�O�L��ƥس̦h�C
int CMoveGen::CapMoveGen(const int Player, CChessMove* pGenMove)
{	
	const unsigned int k = (1+Player) << 4;				// k=16,�´�; k=32,���ѡC
	unsigned int  nSrc, nDst, x, y, nChess, nCaptured;	
	CChessMove  *ChessMove = pGenMove;					// �O�s�̪쪺���ʫ��w
	unsigned char *pMove;

	nChess = k+15;

	// ���ͧL�򪺲���******************************************************************************************
	for( ; nChess>=k+11; nChess--)
	{
		if( nSrc = Piece[nChess] )						// �Ѥl�s�b�JnSrc!=0
		{
			pMove = PawnMoves[Player][nSrc];
			while( *pMove )
			{
				nCaptured = Board[ nDst = *(pMove++) ];
				if( (nChess ^ nCaptured) >= 48 )		// ����Ѥl
					*(ChessMove++) = ((MvvValues[nCaptured] - 20)<<16) | (nSrc<<8) | nDst;
			}
		}
	}


	// ���ͤh������******************************************************************************************
	for( ; nChess>=k+9; nChess--)
	{
		if( nSrc = Piece[nChess] )						// �Ѥl�s�b�JnSrc!=0
		{
			pMove = GuardMoves[nSrc];
			while( *pMove )
			{
				nCaptured = Board[ nDst = *(pMove++) ];
				if( (nChess ^ nCaptured) >= 48 )		// ����Ѥl
					*(ChessMove++) = ((MvvValues[nCaptured] - 41)<<16) | (nSrc<<8) | nDst;
			}
		}
	}


	// ���ͶH������******************************************************************************************
	for( ; nChess>=k+7; nChess--)
	{
		if( nSrc = Piece[nChess] )						// �Ѥl�s�b�JnSrc!=0
		{
			pMove = BishopMoves[nSrc];
			while( *pMove )
			{
				nCaptured = Board[ nDst = *(pMove++) ];
				if( (nChess ^ nCaptured) >= 48 )		// ����Ѥl
				{
					if( !Board[(nSrc+nDst)>>1] )					//�H���L�l
						*(ChessMove++) = ((MvvValues[nCaptured] - 40)<<16) | (nSrc<<8) | nDst;
				}
			}
		}
	}


	// ���Ͱ�������******************************************************************************************
	for( ; nChess>=k+5; nChess--)
	{
		if( nSrc = Piece[nChess] )						// �Ѥl�s�b�JnSrc!=0
		{
			pMove = KnightMoves[nSrc];
			while( *pMove )
			{
				nCaptured = Board[ nDst = *(pMove++) ];
				if( (nChess ^ nCaptured) >= 48 )		// ����Ѥl
				{
					if( !Board[nSrc+nHorseLegTab[nDst-nSrc+256]] )
						*(ChessMove++) = ((MvvValues[nCaptured] - 88)<<16) | (nSrc<<8) | nDst;
				}
			}
		}
	}


	// ���ͬ�������************************************************************************************************
	for( ; nChess>=k+3; nChess--)
	{
		if( nSrc = Piece[nChess] )						// �Ѥl�s�b�JnSrc!=0
		{			
			x = nSrc & 0xF;								// �Z4�즳��
			y = nSrc >> 4;								// �e4�즳��

			//������V����
			pMove = xCannonCapMoves[x][xBitBoard[y]];
			while( *pMove )
			{
				nCaptured = Board[ nDst = (nSrc & 0xF0) | (*(pMove++)) ];	// 0x y|x  �e4��=y*16�A �Z4��=x
				if( (nChess ^ nCaptured) >= 48 )		// ����Ѥl
					*(ChessMove++) = ((MvvValues[nCaptured] - 96)<<16) | (nSrc<<8) | nDst;
			}

			//�����a�V����
			pMove = yCannonCapMoves[y][yBitBoard[x]];
			while( *pMove )
			{		
				nCaptured = Board[ nDst = (*(pMove++)) | x ];		// 0x y|x  �e4��=y*16�A �Z4��=x
				if( (nChess ^ nCaptured) >= 48 )		// ����Ѥl
					*(ChessMove++) = ((MvvValues[nCaptured] - 96)<<16) | (nSrc<<8) | nDst;
			}
		}
	}


	// ���ͨ�������************************************************************************************************
	for( ; nChess>=k+1; nChess--)
	{
		if( nSrc = Piece[nChess] )						// �Ѥl�s�b�JnSrc!=0
		{			
			x = nSrc & 0xF;								// �Z4�즳��
			y = nSrc >> 4;								// �e4�즳��

			//������V����
			pMove = xRookCapMoves[x][xBitBoard[y]];
			while( *pMove )
			{
				nCaptured = Board[ nDst = (nSrc & 0xF0) | (*(pMove++)) ];	// 0x y|x  �e4��=y*16�A �Z4��=x
				if( (nChess ^ nCaptured) >= 48 )		// ����Ѥl
					*(ChessMove++) = ((MvvValues[nCaptured] - 200)<<16) | (nSrc<<8) | nDst;
			}

			//�����a�V����
			pMove = yRookCapMoves[y][yBitBoard[x]];
			while( *pMove )
			{
				nCaptured = Board[ nDst = (*(pMove++)) | x ];		// 0x y|x  �e4��=y*16�A �Z4��=x
				if( (nChess ^ nCaptured) >= 48 )		// ����Ѥl
					*(ChessMove++) = ((MvvValues[nCaptured] - 200)<<16) | (nSrc<<8) | nDst;
			}
		}
	}
                                                                                                                                                                                                                                                                                                                                                     

	// ���ͱN�Ӫ�����********************************************************************************************
	nSrc = Piece[nChess];								// �Ѥl�s�b�JnSrc!=0
	{
		pMove = KingMoves[nSrc];
		while( *pMove )
		{
			nCaptured = Board[ nDst = *(pMove++) ];
			if( (nChess ^ nCaptured) >= 48 )		// ����Ѥl
				*(ChessMove++) = ((MvvValues[nCaptured] - 1000)<<16) | (nSrc<<8) | nDst;
		}
	}	

	return int(ChessMove-pGenMove);
}


// �P�_����ҵo�۪k���X�k��
int CMoveGen::IsLegalKillerMove(int Player, const CChessMove KillerMove)
{	
	int nSrc = (KillerMove & 0xFF00) >> 8;
	int nMovedChs = Board[nSrc];
	if( (nMovedChs >> 4) != Player+1 )			// �Y���⤣�O���誺�Ѥl�A�����D�k����
		return 0;

	int nDst = KillerMove & 0xFF;
	if( (Board[nDst] >> 4) == Player+1 )		// �Y���⬰�Y�l���ʡA�Y�P��Ѥl�����D�k
		return 0;

	int x, y;
	switch( nPieceID[nMovedChs] )
	{
		case 1:		// ��
			x = nSrc & 0xF;
			y = nSrc >> 4;
			if( x == (nDst & 0xF) )
				return yBitRookMove[y][yBitBoard[x]] & yBitMask[nDst];		// x�۵����a�V����
			else if( y == (nDst >> 4) )
				return xBitRookMove[x][xBitBoard[y]] & xBitMask[nDst];		// y�۵����a�V����
			break;

		case 2:		// ��
			x = nSrc & 0xF;
			y = nSrc >> 4;
			if( x == (nDst & 0xF) )
				return yBitCannonMove[y][yBitBoard[x]] & yBitMask[nDst];	// x�۵����a�V����
			else if( y == (nDst >> 4) )
				return xBitCannonMove[x][xBitBoard[y]] & xBitMask[nDst];	// y�۵����a�V����
			break;

		case 3:		// ��
			if( !Board[ nSrc + nHorseLegTab[ nDst-nSrc+256 ] ] )		// ���L�L�l
				return 3;
			break;

		case 4:		// �H
			if( !Board[(nSrc+nDst)>>1] )									// �H���L�l
				return 4;
			break;

		default:
			return 1;														// ����ҵo, �N�h�L�@���M�X�k
			break;
	}

	return 0;
}

// �ϥΦ��P��C�޳N��{���N�x�˴�
// ��Ƥ@���J��N�x�A�ߧY��^�D��0�����ƭ�
// ����ƥΤ_��e���ʤ�O�_�Q�N�x
// �`�N�J���������P��C�ާ@nDst->nSrc�P����۪k�X�z������nSrc->nDst���n�ۤ�
int CMoveGen::Checked(int Player)
{
	nCheckCounts ++;
	
	int nKingSq = Piece[(1+Player)<<4];		// �ڤ�N�Ӫ���m
	int x = nKingSq & 0xF;
	int y = nKingSq >> 4;
	int king = (2-Player) << 4 ;				// ���N�Ӫ��Ǹ�

	int xBitMove = xBitRookMove[x][xBitBoard[y]];
	int yBitMove = yBitRookMove[y][yBitBoard[x]];
	
	// �����ӭ��J�H�����A�ϥΨ�����C�ѽL
	int nSrc = Piece[king];
	if( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] )
		return nSrc;

	// ���N�x
	nSrc = Piece[king+1];
	if( ( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] ) ||
		( y==(nSrc >>  4) && xBitMove & xBitMask[nSrc] ) )
		return nSrc;

	nSrc = Piece[king+2];
	if( ( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] ) ||
		( y==(nSrc >>  4) && xBitMove & xBitMask[nSrc] ) )
		return nSrc;	


	xBitMove = xBitCannonMove[x][xBitBoard[y]];
	yBitMove = yBitCannonMove[y][yBitBoard[x]];

	// ���N�x
	nSrc = Piece[king+3];
	if( ( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] ) ||
		( y==(nSrc >>  4) && xBitMove & xBitMask[nSrc]) )
		return nSrc;

	nSrc = Piece[king+4];
	if( ( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] ) ||
		( y==(nSrc >>  4) && xBitMove & xBitMask[nSrc]) )		
		return nSrc;


	// �Q��誺���N�x
	nSrc = Piece[king+5];
	x = nHorseLegTab[nKingSq - nSrc + 256];
	if( x && !Board[nSrc + x] )							// �Y������B���L�L�l�A���N�x
		return nSrc;

	nSrc = Piece[king+6];
	x = nHorseLegTab[nKingSq - nSrc + 256];
	if( x && !Board[nSrc + x] )							// �Y������B���L�L�l�A���N�x
		return nSrc;


	// �Q���L�e���L��N�x
	if( nPieceType[ Board[Player ? nKingSq-16 : nKingSq+16] ] == 13-7*Player )		// �`�N�J�N�Ӧb���u�ɡA���঳�v�誺�L��s�b
		return Player ? nKingSq-16 : nKingSq+16;

	if( nPieceID[ Board[nKingSq-1] ]==6 )
		return nKingSq-1;
		
	if( nPieceID[ Board[nKingSq+1] ]==6 )
		return nKingSq+1;

	nNonCheckCounts ++;
	return 0;
}


// �ϥΦ��P��C�޳N��{���N�x�˴�
// �p��Ҧ���������N�Ӫ��Ѥlcheckers�A�N�x�k�ר�Ư�������ϥ�checkers�ܶq�A�קK���ƭp��
// ��^�ȡJcheckers!=0�A��ܱN�x�Fcheckers==0�A��ܨS���N�x
// checkers���Z�K���ܱN�x�������A���O��ܡJ�L0x80 �L0x40 ��0x20 ��0x10 ��0x08 ��0x04 ��0x02 ��0x01
// �ϥΦ���ƫe�A�ݥ���Checked()�P�_�ڤ�O�_�Q�N�x�A�M��Q�Φ���ƭp����O�_�Q�N�x
// �`�N�J�]���e���w�g�ϥΤFChecked()�A�ҥH����Ƥ����n�������ӭ��A���i���˴�
// �`�N�J���������P��C�ާ@nDst->nSrc�P����۪k�X�z������nSrc->nDst���n�ۤϡA�o�˳]�p�i�H��֭p��
int CMoveGen::Checking(int Player)
{
	nCheckCounts ++;
	
	int nKingSq = Piece[(1+Player)<<4];		// �p��N�Ӫ���m
	int x = nKingSq & 0xF;
	int y = nKingSq >> 4;
	int king = (2-Player) << 4 ;			// �N��
	int checkers = 0;
	int nSrc;

	int xBitMove = xBitRookMove[x][xBitBoard[y]];
	int yBitMove = yBitRookMove[y][yBitBoard[x]];

	// �����ӭ��J�L���˴��A���|�o��
	//nSrc = Piece[king];
	//if( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] )
	//	checkers |= 0xFF;

	// ���N�x�J0x01��ܨ�king+1, 0x02��ܨ�king+2
	nSrc = Piece[king+1];
	if( ( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] ) ||
		( y==(nSrc >>  4) && xBitMove & xBitMask[nSrc] ) )
		checkers |= 0x01;

	nSrc = Piece[king+2];
	if( ( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] ) ||
		( y==(nSrc >>  4) && xBitMove & xBitMask[nSrc] ) )
		checkers |= 0x02;


	xBitMove = xBitCannonMove[x][xBitBoard[y]];
	yBitMove = yBitCannonMove[y][yBitBoard[x]];
	
	// ���N�x�J0x04��ܬ�king+3, 0x08��ܬ�king+4
	nSrc = Piece[king+3];
	if( ( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] ) ||
		( y==(nSrc >>  4) && xBitMove & xBitMask[nSrc]) )
		checkers |= 0x04;

	nSrc = Piece[king+4];
	if( ( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] ) ||
		( y==(nSrc >>  4) && xBitMove & xBitMask[nSrc]) )		
		checkers |= 0x08;


	// ���N�x�J0x10��ܰ�king+5�A0x20��ܰ�king+6
	nSrc = Piece[king+5];
	x = nHorseLegTab[nKingSq - nSrc + 256];
	if( x && !Board[nSrc + x] )							// �Y������B���L�L�l�A���N�x
		checkers |= 0x10;

	nSrc = Piece[king+6];
	x = nHorseLegTab[nKingSq - nSrc + 256];
	if( x && !Board[nSrc + x] )							// �Y������B���L�L�l�A���N�x
		checkers |= 0x20;


	// �a�V�L��J0x40����a�V���L/��N�x
	if( nPieceType[ Board[Player ? nKingSq-16 : nKingSq+16] ] == 13-7*Player )		// �N�Ӧb���u�ɡA���঳�v�誺���L�s�b
		checkers |= 0x40;

	// ��V�L��J0x80��ܾ�V���L/��N�x
	if( nPieceID[ Board[nKingSq-1] ]==6 || nPieceID[ Board[nKingSq+1] ]==6 )		// �d�߱N�ӥ��k�O�_���L/��s�b
		checkers |= 0x80;


	if(!checkers)
		nNonCheckCounts ++;
	return checkers;
}

// �O�@�P�_���
// �Ѥl�qfrom->nDst, Player�@��O�_�Φ��O�@
// �������J�O�@��Y�l���k�Ƨǰ_�쪺�@�Ϋܤp�C
int CMoveGen::Protected(int Player, int from, int nDst)
{
	const int king = (2-Player) << 4 ;			// �N��
	
	//****************************************************************************************************
	// �N
	int nSrc = Piece[king];
	if( nDirection[nDst-nSrc+256]==1 && nCityIndex[nDst] )
		return 1000;

	// �H
	nSrc = Piece[king+7];
	if( nSrc && nSrc!=nDst && nDirection[nDst-nSrc+256]==4 && !Board[(nSrc+nDst)>>1] && (nSrc^nDst)<128 )
		return 40;

	nSrc = Piece[king+8];
	if( nSrc && nSrc!=nDst && nDirection[nDst-nSrc+256]==4 && !Board[(nSrc+nDst)>>1] && (nSrc^nDst)<128 )
		return 40;

	// �h
	nSrc = Piece[king+9];
	if( nSrc && nSrc!=nDst && nDirection[nDst-nSrc+256]==2 && nCityIndex[nDst] )
		return 41;

	nSrc = Piece[king+10];
	if( nSrc && nSrc!=nDst && nDirection[nDst-nSrc+256]==2 && nCityIndex[nDst] )
		return 41;

	// �L��
	nSrc = Player ? nDst-16 : nDst+16;
	if( nPieceType[ Board[nSrc] ] == 13-7*Player )		// �`�N�J�N�Ӧb���u�ɡA���঳�v�誺�L��s�b
		return 20;

	if( (Player && nDst<128) || (!Player && nDst>=128) )
	{
		if( nPieceID[ Board[nDst-1] ]==6 )
			return 17;
			
		if( nPieceID[ Board[nDst+1] ]==6 )
			return 17;
	}

	//*****************************************************************************************************
	// ���B���B���O�@�ɡA�����n�M���_�I��mfrom�B���Ѥl�A�~����p��ǽT
	int x = nDst & 0xF;
	int y = nDst >> 4;
	int xBitIndex = xBitBoard[y] ^ xBitMask[from];		// �M��from�����
	int yBitIndex = yBitBoard[x] ^ yBitMask[from];		// �M��from����C

	const int m_Piece = Board[from];					// �O�sfrom���Ѥl
	Piece[m_Piece] = 0;									// �{�ɲM���A�H���_

	// ��
	nSrc = Piece[king+1];
	if( nSrc && nSrc!=nDst )
	{
		if(	( x==(nSrc & 0xF) && yBitRookMove[y][yBitIndex] & yBitMask[nSrc] ) ||
			( y==(nSrc >>  4) && xBitRookMove[x][xBitIndex] & xBitMask[nSrc] ) )
		{
			Piece[m_Piece] = from;
			return 200;
		}
	}

	nSrc = Piece[king+2];
	if( nSrc && nSrc!=nDst )
	{
		if( ( x==(nSrc & 0xF) && yBitRookMove[y][yBitIndex] & yBitMask[nSrc] ) ||
			( y==(nSrc >> 4) && xBitRookMove[x][xBitIndex] & xBitMask[nSrc] ) )
		{
			Piece[m_Piece] = from;
			return 200;
		}
	}

	// ��
	nSrc = Piece[king+3];
	if( nSrc && nSrc!=nDst )
	{
		if( ( x==(nSrc & 0xF) && yBitCannonMove[y][yBitIndex] & yBitMask[nSrc] ) ||
			( y==(nSrc >>  4) && xBitCannonMove[x][xBitIndex] & xBitMask[nSrc]) )
		{
			Piece[m_Piece] = from;
			return 96;
		}
	}

	nSrc = Piece[king+4];
	if( nSrc && nSrc!=nDst )
	{
		if( ( x==(nSrc & 0xF) && yBitCannonMove[y][yBitIndex] & yBitMask[nSrc] ) ||
			( y==(nSrc >>  4) && xBitCannonMove[x][xBitIndex] & xBitMask[nSrc]) )		
		{
			Piece[m_Piece] = from;
			return 96;
		}
	}


	// ��
	nSrc = Piece[king+5];
	if( nSrc!=nDst )
	{
		x = nHorseLegTab[nDst - nSrc + 256];
		if( x && (!Board[nSrc + x] || x==from) )			// �Y������B���L�L�l�A���N�x
		{
			Piece[m_Piece] = from;
			return 88;
		}
	}

	nSrc = Piece[king+6];
	if( nSrc!=nDst )
	{
		x = nHorseLegTab[nDst - nSrc + 256];
		if( x && (!Board[nSrc + x] || x==from) )			// �Y������B���L�L�l�A���N�x
		{
			Piece[m_Piece] = from;
			return 88;
		}
	}

	// ��_from�B���Ѥl
	Piece[m_Piece] = from;

	return 0;
}

int CMoveGen::AddLegalMove(const int nChess, const int nSrc, const int nDst, CChessMove *ChessMove)
{
	int x, y, nCaptured;
	
	switch( nPieceID[nChess] )
	{
		case 0:		// �N��W�U���k�B�b�E�c��
			if( nDirection[nDst-nSrc+256]==1 && nCityIndex[nDst] )
			{
				nCaptured = Board[nDst];
				if( !nCaptured )
					*(ChessMove++) = (nSrc<<8) | nDst;
				else if( (nChess^nCaptured) >= 48 )
					*(ChessMove++) = (MvvValues[nCaptured]<<16) | (nSrc<<8) | nDst;
				return 1;
			}
			break;

		case 1:		// ��
			x = nSrc & 0xF;
			y = nSrc >> 4;
			if( (x==(nDst & 0xF) && yBitRookMove[y][yBitBoard[x]] & yBitMask[nDst]) ||
				(y==(nDst >>  4) && xBitRookMove[x][xBitBoard[y]] & xBitMask[nDst]) )
			{
				nCaptured = Board[nDst];
				if( !nCaptured )
					*(ChessMove++) = (nSrc<<8) | nDst;
				else if( (nChess^nCaptured) >= 48 )
					*(ChessMove++) = ((MvvValues[nCaptured]+1)<<16) | (nSrc<<8) | nDst;
				return 1;
			}
			break;

		case 2:		// ��
			x = nSrc & 0xF;
			y = nSrc >> 4;
			if( (x==(nDst & 0xF) && yBitCannonMove[y][yBitBoard[x]] & yBitMask[nDst]) ||
				(y==(nDst >>  4) && xBitCannonMove[x][xBitBoard[y]] & xBitMask[nDst]) )
			{
				nCaptured = Board[nDst];
				if( !nCaptured )
					*(ChessMove++) = (nSrc<<8) | nDst;
				else if( (nChess^nCaptured) >= 48 )
					*(ChessMove++) = ((MvvValues[nCaptured]+3)<<16) | (nSrc<<8) | nDst;
				return 1;
			}
			break;

		case 3:		// ������B���L�L�l
			x = nHorseLegTab[nDst - nSrc + 256];
			if( x && !Board[nSrc + x] )
			{
				nCaptured = Board[nDst];
				if( !nCaptured )
					*(ChessMove++) = (nSrc<<8) | nDst;
				else if( (nChess^nCaptured) >= 48 )
					*(ChessMove++) = ((MvvValues[nCaptured]+3)<<16) | (nSrc<<8) | nDst;
				return 1;
			}
			break;

		case 4:		// �H���СB�H���L�l�B�H���L�e
			if( nDirection[nDst-nSrc+256]==4 && !Board[(nSrc+nDst)>>1] && (nSrc^nDst)<128 )
			{
				nCaptured = Board[nDst];
				if( !nCaptured )
					*(ChessMove++) = (nSrc<<8) | nDst;
				else if( (nChess^nCaptured) >= 48 )
					*(ChessMove++) = ((MvvValues[nCaptured]+5)<<16) | (nSrc<<8) | nDst;
				return 1;
			}
			break;

		case 5:		// �h���׽u�B�b�E�c��
			if( nDirection[nDst-nSrc+256]==2 && nCityIndex[nDst] )
			{
				nCaptured = Board[nDst];
				if( !nCaptured )
					*(ChessMove++) = (nSrc<<8) | nDst;
				else if( (nChess^nCaptured) >= 48 )
					*(ChessMove++) = ((MvvValues[nCaptured]+7)<<16) | (nSrc<<8) | nDst;
				return 1;
			}
			break;

		case 6:		// �L���W�U���k�B���k�X�z
			x = nDst-nSrc;						// ��e���ʤ�APlayer
			if(  nDirection[x+256]==1 && (
				(nChess<32  && (x==16 || (nSrc>=128 && (x==1 || x==-1)))) ||
				(nChess>=32 && (x==-16 || (nSrc<128 && (x==1 || x==-1)))) ) )
			{
				nCaptured = Board[nDst];
				if( !nCaptured )
					*(ChessMove++) = (nSrc<<8) | nDst;
				else if( (nChess^nCaptured) >= 48 )
					*(ChessMove++) = ((MvvValues[nCaptured]+9)<<16) | (nSrc<<8) | nDst;
				return 1;
			}
			break;

		default:
			break;
	}

	return 0;
}


//********************�N�x�k�ײ��;��JCheckEvasionGen()***********************************************
// ����ƨëD�_�짹���ѱN���ت��A�������N�x�˴��P�_�����Ƥj�j��֡C
// �Y�����ѱN�A�N�X�D�`�����A�C���ʤ@���Ѥl�A���n�Ҽ{�O�_����誺�N�������L����o�ۥѦӺc���s���N�x�C
// �J�M�O�������ѱN�A�Z���@�w�n�[�N�x�˴��C�o�˷|�񧹥��ѱN���B��q�֤@�ǡA���i�ണ�e�o��ŪK�C
// Player   ��ܳQ�N�x���@��A�Y��e���ʤ�
// checkers �N�x�˴����Checking(Player)����^�ȡA��ܱN�x�������A�i�H�ߧY���D�Q���X���Ѥl�N�x
//          �Z8�즳�ġA���O��ܡJ��L�B�a�L�B��2�B��1�B��2�B��1�B��2�B��1
//          ���ҥH���]�t�N�Ӫ��N�x�T���A�O�]��Checked(Player)��Ƥw�g�@�F�B�z�C
int CMoveGen::CheckEvasionGen(const int Player, int checkers, CChessMove* pGenMove)
{
	nCheckEvasions ++;							// �έp�ѱN��ƪ���B����

	const int MyKing   = (1+Player) << 4;		// MyKing=16,�´�; MyKing=32,���ѡC
	const int OpKing = (2-Player) << 4;			// �������Ѥl�Ǹ�
	const int nKingSq = Piece[MyKing];			// �p��N�Ӫ���m

	int nDir0=0, nDir1=0;						// �N�x��V�J1�о�V�F16���a�V
	int nCheckSq0, nCheckSq1;					// �N�x�Ѥl����m
	int nMin0, nMax0, nMin1, nMax1;				// �Τ_�����P�N�Ӥ������d��
	int nPaojiazi0, nPaojiazi1;					// ���[�l����m	
	int nPaojiaziID0, nPaojiaziID1;				// ���[�l���Ѥl����

	int nChess, nCaptured, nSrc, nDst, x, y;
	unsigned char *pMove;
	CChessMove *ChessMove = pGenMove;			// ��l�Ʋ��ʪ����w

	// �ѧt���N�x����k�̽����A�w��t���N�x�����@�S��B�z�A�H��֭��ƭp��
	if( checkers & 0x0C )
	{
		// �Ĥ@�Ӭ��N�x����m
		nCheckSq0 = Piece[ OpKing + (checkers&0x04 ? 3:4) ];

		// �N�x��V�J1�о�V�F16���a�V
		nDir0 = (nKingSq&0xF)==(nCheckSq0&0xF) ? 16:1;

		// ���N�������d��[nMin0, nMax0)�A���]�t�N�Ӫ���m
		nMin0 = nKingSq>nCheckSq0 ? nCheckSq0 : nKingSq+nDir0;
		nMax0 = nKingSq<nCheckSq0 ? nCheckSq0 : nKingSq-nDir0;		

		// �M�䬶�[�l����m�JnPaojiazi0
		// �p�⬶�[�l�������JnPaojiaziID0
		for(nDst=nMin0; nDst<=nMax0; nDst+=nDir0)
		{
			if( Board[nDst] && nDst!=nCheckSq0 )
			{
				nPaojiazi0 = nDst;
				nPaojiaziID0 = Board[nDst];
				break;
			}
		}
		
		// �Y���[�l�O��誺���A���ͬɤ_��������������
		if( nPaojiaziID0==OpKing+3 || nPaojiaziID0==OpKing+4 )
		{
			nMin0 = nPaojiazi0>nCheckSq0 ? nCheckSq0 : nPaojiazi0+nDir0;
			nMax0 = nPaojiazi0<nCheckSq0 ? nCheckSq0 : nPaojiazi0-nDir0;
		}
	}

	// �ھڡ��N�x�������i��ѱN
	// �ĥνa�|�k�A�v�Ӥ��R�C�د���ѱN������
	switch( checkers )
	{
		// �樮�N�x�J���B��
		case 0x01:
		case 0x02:
			nCheckSq0 = Piece[ OpKing + (checkers&0x01 ? 1:2) ];			
			nDir0 = (nKingSq&0xF)==(nCheckSq0&0xF) ? 16:1;
			nMin0 = nKingSq>nCheckSq0 ? nCheckSq0 : nKingSq+nDir0;
			nMax0 = nKingSq<nCheckSq0 ? nCheckSq0 : nKingSq-nDir0;

			x = nDir0==1 ? MyKing+10 : MyKing+15;
			for(nDst=nMin0; nDst<=nMax0; nDst+=nDir0)
			{	
				// �������H�h�L�A�����Ϊ̾ר�
				for(nChess=MyKing+1; nChess<=x; nChess++)
				{
					if( (nSrc=Piece[nChess]) != 0 )
						ChessMove += AddLegalMove(nChess, nSrc, nDst, ChessMove);
				}
			}

			break;


		// �欶�N�x�J���B��
		case 0x04:  // ��1
		case 0x08:  // ��2
			// �����B���׬��N�x�F�Ȥ����ͬ��[�l������
			// ���[�l�����]�i�H�����A�����ಣ�ͱN�x��V���D�Y�l����
			x = nDir0==1 ? MyKing+10 : MyKing+15;
			for(nDst=nMin0; nDst<=nMax0; nDst+=nDir0)
			{	
				if( nDst==nPaojiazi0 )							// �����[�l�L�ΡT
					continue;

				// �������H�h�L�A�����Ϊ̾׬�
				for(nChess=MyKing+1; nChess<=x; nChess++)
				{
					nSrc = Piece[nChess];
					if(nSrc && nSrc!=nPaojiazi0)				// ���[�l�O�ڤ�Ѥl�A�Ȥ����ͳo�ز���
						ChessMove += AddLegalMove(nChess, nSrc, nDst, ChessMove);
				}
			}

			// ���ͺM���[�l�����ʡA�Φ����Y���A�Ϭ��ॢ�N�x��O�C
			// �Y���[�l�O���L�A�i�H���N�x�����F���[�l�O���A�i�H�V�L��誺���Ϊ̤v�誺�N���l�k���C
			// ��_�����L�T�ӷƶ��Ѥl�A���ӸT��N�x��V�W���D�Y�l���ʡA���ʦZ�٬O���[�l�F
			// �խY��謶���I�Z�A�s�b�t�@�Ӭ��A�ߦ��ά����Y�A�i�H�ѱN�A�_�h�Ѥl�M���Z�|�Φ���������
			if( (nPaojiaziID0-16)>>4==Player )					// ���[�l�O�v�誺�Ѥl
			{
				nSrc = nPaojiazi0;
				nChess = Board[nSrc];
				x = nSrc & 0xF;								// �Z4�즳��
				y = nSrc >> 4;								// �e4�즳��

				switch( nPieceID[Board[nSrc]] )
				{
					case 1:			
						// ������V���ʡJ�a�V�N�x�ɡA���i�H��V�M��
						//               ��V�N�x�ɡA���i�H����
						pMove = xRookMoves[x][xBitBoard[y]];
						while( *pMove )
						{
							nDst = (nSrc & 0xF0) | (*(pMove++));	// 0x y|x  �e4��=y*16�A �Z4��=x
							nCaptured = Board[nDst];

							if( !nCaptured && nDir0==16 )
								*(ChessMove++) = (nSrc<<8) | nDst;
							else if( (nChess^nCaptured)>=48 )
								*(ChessMove++) = (MvvValues[nCaptured]<<16) | (nSrc<<8) | nDst;
						}
						// �����a�V���ʡJ��V�N�x�ɡA���i�H�a�V�M��
						pMove = yRookMoves[y][yBitBoard[x]];
						while( *pMove )
						{
							nDst = (*(pMove++)) | x;				// 0x y|x  �e4��=y*16�A �Z4��=x
							nCaptured = Board[nDst];

							if( !nCaptured && nDir0==1 )
								*(ChessMove++) = (nSrc<<8) | nDst;
							else if( (nChess^nCaptured)>=48 )
								*(ChessMove++) = (MvvValues[nCaptured]<<16) | (nSrc<<8) | nDst;
						}
						break;

					case 2:
						// ������V���ʡJ�a�V�N�x�ɡA���i�H��V�M���F�L���a��N�x�A�����i�H�Y�l�k��
						pMove = xCannonMoves[x][xBitBoard[y]];
						while( *pMove )
						{
							nDst = (nSrc & 0xF0) | (*(pMove++));	// 0x y|x  �e4��=y*16�A �Z4��=x
							nCaptured = Board[nDst];

							if( !nCaptured && nDir0==16 )
								*(ChessMove++) = (nSrc<<8) | nDst;
							else if( (nChess^nCaptured) >= 48 )
								*(ChessMove++) = (MvvValues[nCaptured]<<16) | (nSrc<<8) | nDst;
						}

						// �����a�V���ʡJ��V�N�x�ɡA���i�H�a�V�M���F�L���a��N�x�A�����i�H�Y�l�k��
						pMove = yCannonMoves[y][yBitBoard[x]];
						while( *pMove )
						{
							nDst = (*(pMove++)) | x;		// 0x y|x  �e4��=y*16�A �Z4��=x
							nCaptured = Board[nDst];

							if( !nCaptured && nDir0==1 )
								*(ChessMove++) = (nSrc<<8) | nDst;
							else if( (nChess^nCaptured) >= 48 )
								*(ChessMove++) = (MvvValues[nCaptured]<<16) | (nSrc<<8) | nDst;
						}
						break;

					case 3:		
						// ���J�k���N�x��V
						pMove = KnightMoves[nSrc];
						while( *pMove )
						{
							nDst = *(pMove++);			
							nCaptured = Board[nDst];

							if( !Board[nSrc+nHorseLegTab[nDst-nSrc+256]] )
							{
								if( !nCaptured )
									*(ChessMove++) = (nSrc<<8) | nDst;
								else if( (nChess^nCaptured) >= 48 )
									*(ChessMove++) = (MvvValues[nCaptured]<<16) | (nSrc<<8) | nDst;
							}
						}
						break;

					case 4:
						// �H�J�k���N�x��V
						pMove = BishopMoves[nSrc];
						while( *pMove )
						{
							nDst = *(pMove++);				
							nCaptured = Board[nDst];

							if( !Board[(nSrc+nDst)>>1] )					//�H���L�l
							{
								if( !nCaptured )
									*(ChessMove++) = (nSrc<<8) | nDst;
								else if( (nChess^nCaptured) >= 48 )
									*(ChessMove++) = (MvvValues[nCaptured]<<16) | (nSrc<<8) | nDst;
							}
						}
						break;

					case 5:
						// �h�J�k���N�x��V
						pMove = GuardMoves[nSrc];
						while( *pMove )
						{
							nDst = *(pMove++);
							nCaptured = Board[nDst];

							if( !nCaptured )
								*(ChessMove++) = (nSrc<<8) | nDst;
							else if( (nChess^nCaptured) >= 48 )
								*(ChessMove++) = (MvvValues[nCaptured]<<16) | (nSrc<<8) | nDst;
						}
						break;

					case 6:
						// �L��J�a�V�N�x�A��V�k���N�x��V�F��V�N�x�A�L�򤣯��F�ѱN��m
						pMove = PawnMoves[Player][nSrc];
						while( *pMove )
						{
							nDst = *(pMove++);
							nCaptured = Board[nDst];
							
							if( !nCaptured && nDir0==16 && x != (nDst&0xF) )			// ��V�k���A�T���a�V����
								*(ChessMove++) = (nSrc<<8) | nDst;
							else if( (nChess^nCaptured)>=48 )							// �a�V���N�x����
								*(ChessMove++) = (MvvValues[nCaptured]<<16) | (nSrc<<8) | nDst;
						}
						break;

					default:
						break;
				}
			}
			// ���[�l�O��誺���A�B�Φ����Z�����աA����β��N�k�ѱN�A�즹����A�{����^
			else if( nPaojiaziID0==OpKing+5 || nPaojiaziID0==OpKing+6 )
			{
				if( nKingSq-nPaojiazi0==2  || nKingSq-nPaojiazi0==-2 || 
					nKingSq-nPaojiazi0==32 || nKingSq-nPaojiazi0==-32 )
				return int(ChessMove-pGenMove);
			}

			break;


		// �����N�x�J
		case 0x05:  // ��1��1
		case 0x06:  // ��1��2
		case 0x09:  // ��2��1
		case 0x0A:  // ��2��2
			nCheckSq1 = Piece[ OpKing + (checkers&0x01 ? 1:2) ];
			nDir1 = (nKingSq&0xF)==(nCheckSq1&0xF) ? 16:1;
			nMin1 = nKingSq>nCheckSq1 ? nCheckSq1 : nKingSq+nDir1;
			nMax1 = nKingSq<nCheckSq1 ? nCheckSq1 : nKingSq-nDir1;

			// �������O�q��Ӥ�V�N�x�A�åB���[�l�O���Ѥl�A�L��
			if( nDir0!=nDir1 && (nPaojiaziID0-16)>>4==1-Player )
				return 0;
			// ���[�l�O��誺���A�B�Φ����Z�����աA�L��
			else if( nPaojiaziID0==OpKing+5 || nPaojiaziID0==OpKing+6 )
			{
				if( nKingSq-nPaojiazi0==2  || nKingSq-nPaojiazi0==-2 || 
					nKingSq-nPaojiazi0==32 || nKingSq-nPaojiazi0==-32 )
				return 0;
			}
			// �Y���[�l�O��誺���A���ͬɤ_���N�������D�Y�l���ʲ��ʡA���]�t���P�N
			else if( nPaojiaziID0==OpKing+1 || nPaojiaziID0==OpKing+2 )
			{
				nMin0 = nKingSq>nPaojiazi0 ? nPaojiazi0+nDir0 : nKingSq+nDir0;
				nMax0 = nKingSq<nPaojiazi0 ? nPaojiazi0-nDir0 : nKingSq-nDir0;

				x = nDir0==1 ? MyKing+10 : MyKing+15;
				for(nDst=nMin0; nDst<=nMax0; nDst+=nDir0)
				{	
					// �������H�h�L�A�����Ϊ̾ר�
					for(nChess=MyKing+1; nChess<=x; nChess++)
					{
						if( (nSrc=Piece[nChess]) != 0 )
							ChessMove += AddLegalMove(nChess, nSrc, nDst, ChessMove);
					}
				}
			}
			// ���[�l�O�v�誺�Ѥl(�����H�h)�A���L�L�k�ѱN
			else if( nPaojiaziID0>=MyKing+3 && nPaojiaziID0<=MyKing+10 )
			{
				// ���ͬ��[�l�����Ϊ̪��ר�������
				nChess = Board[nPaojiazi0];
				for(nDst=nMin1; nDst<=nMax1; nDst+=nDir1)
					ChessMove += AddLegalMove(nChess, nPaojiazi0, nDst, ChessMove);
			}

			break;


		// �����N�x�A�������N�x
		// "4ka2C/9/4b4/9/9/4C4/9/9/9/4K4 b - - 0 1"
		// "4kR2C/9/4b4/9/9/4C4/9/9/9/4K4 b - - 0 1"
		case 0x0C:	// ��1��2		
		case 0x0D:  // ��1��2��1
		case 0x0E:  // ��1��2��2
			// �Y���[�l�O��誺���A���ͬɤ_���N����������
			if( nPaojiaziID0==OpKing+1 || nPaojiaziID0==OpKing+2 )
			{
				nMin0 = nKingSq>nPaojiazi0 ? nPaojiazi0+nDir0 : nKingSq+nDir0;
				nMax0 = nKingSq<nPaojiazi0 ? nPaojiazi0-nDir0 : nKingSq-nDir0;
			}
			

			nCheckSq1 = Piece[ OpKing + 4 ];
			nDir1 = (nKingSq&0xF)==(nCheckSq1&0xF) ? 16:1;
			nMin1 = nKingSq>nCheckSq1 ? nCheckSq1 : nKingSq+nDir1;
			nMax1 = nKingSq<nCheckSq1 ? nCheckSq1 : nKingSq-nDir1;

			// �M�䬶�[�l����m
			for(nDst=nMin1+nDir1; nDst<nMax1; nDst+=nDir1)
			{
				if( Board[nDst] )
				{
					nPaojiazi1 = nDst;
					nPaojiaziID1 = Board[nDst];
					break;
				}
			}

			// �Y���[�l�O��誺���A���ͬɤ_���N����������
			if( nPaojiaziID1==OpKing+1 || nPaojiaziID1==OpKing+2 )
			{
				nMin1 = nKingSq>nPaojiazi1 ? nPaojiazi1+nDir0 : nKingSq+nDir0;
				nMax1 = nKingSq<nPaojiazi1 ? nPaojiazi1-nDir0 : nKingSq-nDir0;
			}
			
			// ���[�l�O�v�誺�Ѥl�J���B�H�B�h
			if( nPaojiaziID0>=MyKing+5 && nPaojiaziID0<=MyKing+10 )
			{
				for(nDst=nMin1; nDst<=nMax1; nDst+=nDir1)
				{
					if( nDst==nPaojiazi1 )
						continue;
					ChessMove += AddLegalMove(nPaojiaziID0, nPaojiazi0, nDst, ChessMove);
				}
			}
			// ���[�l�O�v�誺�Ѥl�J���B�H�B�h
			if( nPaojiaziID1>=MyKing+5 && nPaojiaziID1<=MyKing+10 )	
			{
				for(nDst=nMin0; nDst<=nMax0; nDst+=nDir0)
				{
					if( nDst==nPaojiazi0 )
						continue;
					ChessMove += AddLegalMove(nPaojiaziID1, nPaojiazi1, nDst, ChessMove);
				}
			}

			// ���[�l�O��誺���A�B�Φ����Z�����աA���N�k����ѱN�A�즹��^
			if( nPaojiaziID0==OpKing+5 || nPaojiaziID0==OpKing+6 )
			{
				if( nKingSq-nPaojiazi0==2  || nKingSq-nPaojiazi0==-2 || 
					nKingSq-nPaojiazi0==32 || nKingSq-nPaojiazi0==-32 )
				return int(ChessMove-pGenMove);
			}

			// ���[�l�O��誺���A�B�Φ����Z�����աA���N�k����ѱN�A�즹��^
			if( nPaojiaziID1==OpKing+5 || nPaojiaziID1==OpKing+6 )
			{
				if( nKingSq-nPaojiazi1==2  || nKingSq-nPaojiazi1==-2 || 
					nKingSq-nPaojiazi1==32 || nKingSq-nPaojiazi1==-32 )
				return int(ChessMove-pGenMove);
			}

			break;

		// �����N�x�J
		case 0x14:	// ��1��1
		case 0x18:  // ��2��1
		case 0x24:	// ��1��2
		case 0x28:  // ��2��2
			// ���[�l�O�v�誺�Ѥl�J���B���B���B�H�B�h�A���]�t�N�M�L
			if( nPaojiaziID0>=MyKing+1 && nPaojiaziID0<=MyKing+10 )
			{
				nCheckSq1 = Piece[ OpKing + (checkers&0x10 ? 5:6) ];
				ChessMove += AddLegalMove(nPaojiaziID0, nPaojiazi0, nCheckSq1, ChessMove);
				ChessMove += AddLegalMove(nPaojiaziID0, nPaojiazi0, nCheckSq1+nHorseLegTab[nKingSq-nCheckSq1+256], ChessMove);
			}
			break;


		// �������N�x�J�G�������L��m�ۦP�ɡA���ͬ��[�l(�������h)�̰��L������
		// "3k1a2C/1r3N3/4N4/5n3/9/9/9/9/9/3K5 b - - 0 1"
		case 0x34:  // ��1��1��2
		case 0x38:  // ��2��1��2
			// ���[�l�O�v�誺�Ѥl�J���B���B���B�H�B�h�A���]�t�N�M�L
			if( nPaojiaziID0>=MyKing+1 && nPaojiaziID0<=MyKing+10 )
			{
				nCheckSq0 = Piece[ OpKing + 5 ];
				nCheckSq1 = Piece[ OpKing + 6 ];
				nDst = nCheckSq0+nHorseLegTab[nKingSq-nCheckSq0+256];
				if( nDst==nCheckSq1+nHorseLegTab[nKingSq-nCheckSq1+256] )
					ChessMove += AddLegalMove(nPaojiaziID0, nPaojiazi0, nDst, ChessMove);
			}
			break;


		// ���L�N�x�J���[�l�Y�L�F�Y���[�l�O�L�A�u�ಾ�N�ѱN
		case 0x44:  // ��1�a�L
		case 0x48:  // ��2�a�L
		case 0x84:  // ��1��L
		case 0x88:  // ��2��L
			// ���[�l�O�v�誺�Ѥl
			if( (nPaojiaziID0-16)>>4==Player )					
			{
				// ���[�l���a�V���L
				if( checkers<=0x48 )
					ChessMove += AddLegalMove(nPaojiaziID0, nPaojiazi0, nKingSq+(Player?-16:16), ChessMove);
				// ���[�l����V���L
				else
				{
					nCheckSq0 = nKingSq-1;
					nCheckSq1 = nKingSq+1;
					// ���k���O�L�A����γo�ؤ�k�ѱN
					if( Board[nCheckSq0]!=Board[nCheckSq1] )
					{
						// ���[�l�Y���L
						if( nPieceID[Board[nCheckSq0]]==6 )
							ChessMove += AddLegalMove(nPaojiaziID0, nPaojiazi0, nCheckSq0, ChessMove);
						// ���[�l�Y�k�L
						if( nPieceID[Board[nCheckSq1]]==6 )
							ChessMove += AddLegalMove(nPaojiaziID0, nPaojiazi0, nCheckSq1, ChessMove);
					}
				}
			}
			// ���[�l�O��誺���A�B�Φ����Z�����աA����ѱN
			else if( nPaojiaziID0==OpKing+5 || nPaojiaziID0==OpKing+6 )
			{
				if( nKingSq-nPaojiazi0==2  || nKingSq-nPaojiazi0==-2 || 
					nKingSq-nPaojiazi0==32 || nKingSq-nPaojiazi0==-32 )
				return 0;
			}
			break;


		// �氨�N�x�J�Y���B�̰��L
		case 0x10:
		case 0x20:
			nCheckSq0 = Piece[ OpKing + (checkers&0x10 ? 5:6) ];
			for(nChess=MyKing+1; nChess<=MyKing+10; nChess++)
			{
				if( (nSrc=Piece[nChess]) != 0 )
					ChessMove += AddLegalMove(nChess, nSrc, nCheckSq0, ChessMove);
			}

			nDst = nCheckSq0+nHorseLegTab[nKingSq-nCheckSq0+256];
			for(nChess=MyKing+1; nChess<=MyKing+10; nChess++)
			{
				if( (nSrc=Piece[nChess]) != 0 )
					ChessMove += AddLegalMove(nChess, nSrc, nDst, ChessMove);
			}

			break;

		// �����N�x�J�G�������L��m�ۦP�ɡA���Ͳ̰��L������
		case 0x30:
			nCheckSq0 = Piece[ OpKing + 5 ];
			nCheckSq1 = Piece[ OpKing + 6 ];
			nDst = nCheckSq0+nHorseLegTab[nKingSq-nCheckSq0+256];
			if( nDst == nCheckSq1+nHorseLegTab[nKingSq-nCheckSq1+256] )
			{
				// �������H�h
				for(nChess=MyKing+1; nChess<=MyKing+10; nChess++)
				{
					if( (nSrc=Piece[nChess]) != 0 )
						ChessMove += AddLegalMove(nChess, nSrc, nDst, ChessMove);
				}
			}
			break;

		// �a�V��L�J�Y�L
		case 0x40:
			nDst = nKingSq+(Player?-16:16);
			// �������H�h
			for(nChess=MyKing+1; nChess<=MyKing+10; nChess++)
			{
				if( (nSrc=Piece[nChess]) != 0 )
					ChessMove += AddLegalMove(nChess, nSrc, nDst, ChessMove);
			}
			break;

		// ��V�L��J�Y�L
		case 0x80:
			nCheckSq0 = nKingSq-1;
			nCheckSq1 = nKingSq+1;

			// ���k���O�L�A����γo�ؤ�k�ѱN
			if( Board[nCheckSq0]!=Board[nCheckSq1] )
			{
				// �Y���L
				if( nPieceID[Board[nCheckSq0]]==6 )
				{
					// �������H�h
					for(nChess=MyKing+1; nChess<=MyKing+10; nChess++)
					{
						if( (nSrc=Piece[nChess]) != 0 )
							ChessMove += AddLegalMove(nChess, nSrc, nCheckSq0, ChessMove);
					}
				}

				// �Y�k�L
				if( nPieceID[Board[nCheckSq1]]==6 )
				{
					// �������H�h
					for(nChess=MyKing+1; nChess<=MyKing+10; nChess++)
					{
						if( (nSrc=Piece[nChess]) != 0 )
							ChessMove += AddLegalMove(nChess, nSrc, nCheckSq1, ChessMove);
					}
				}
			}
			break;

		default:
			break;
	}

	// ���N�k�i��ѱN
	pMove = KingMoves[nKingSq];
	while( *pMove )
	{
		nDst = *(pMove++);
		nCaptured = Board[nDst];			
		
		if( !nCaptured && (													// ���ͫD�Y�l����
			(!nDir0 && !nDir1) ||											// �S�������N�x
			(nDir0!=1 && nDir1!=1 && (nKingSq&0xF0)==(nDst&0xF0)) ||		// �N��V���ʡA�������i��V�N�x
			(nDir0!=16 && nDir1!=16 && (nKingSq&0xF)==(nDst&0xF)) ) )		// �N�a�V���ʡA�������i�a�V�N�x
			*(ChessMove++) = (nKingSq<<8) | nDst;
		else if( (MyKing^nCaptured) >= 48 )					// ���ͦY�l����
			*(ChessMove++) = (MvvValues[nCaptured]<<16) | (nKingSq<<8) | nDst;
	}
	
	return int(ChessMove-pGenMove);
}


