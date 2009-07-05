////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 源文件︰MoveGen.cpp                                                                                    //
// *******************************************************************************************************//
// 中國象棋通用引擎----兵河五四，支持《中國象棋通用引擎協議》(Universal Chinese Chess Protocol，簡稱ucci) //
// 作者︰ 范 德 軍                                                                                        //
// 單位︰ 中國原子能科學研究院                                                                            //
// 郵箱︰ fan_de_jun@sina.com.cn                                                                          //
//  QQ ︰ 83021504                                                                                        //
// *******************************************************************************************************//
// 功能︰                                                                                                 //
// 1. 基礎類型CMoveGen, CSearch子類繼承之。棋盤、棋子、位行、位列、著法等數據在這個類中被定義。           //
// 2. 通用移動產生器                                                                                      //
// 3. 吃子移動產生器                                                                                      //
// 4. 將軍逃避移動產生器                                                                                  //
// 5. 殺手移動合法性檢驗                                                                                  //
// 6. 將軍檢測Checked(Player), Checking(1-Player)                                                         //
////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "MoveGen.h"
#include "PreMove.h"

extern void call_vecAdd();

// 棋盤數組和棋子數組
int Board[256];								// 棋盤數組，表示棋子序號︰0～15，無子; 16～31,黑子; 32～47, 紅子；
int Piece[48];								// 棋子數組，表示棋盤位置︰0, 不在棋盤上; 0x33～0xCC, 對應棋盤位置；	

// 位行與位列棋盤
unsigned int xBitBoard[16];					// 16個位行，產生車炮的橫向移動，前12位有效
unsigned int yBitBoard[16];					// 16個位列，產生車炮的縱向移動，前13位有效

// 車炮橫向與縱向移動的16位棋盤，只用于殺手移動合法性檢驗、將軍檢測和將軍逃避   							          
unsigned short xBitRookMove[12][512];		//  12288 Bytes, 車的位行棋盤
unsigned short yBitRookMove[13][1024];		//  26624 Bytes  車的位列棋盤
unsigned short xBitCannonMove[12][512];		//  12288 Bytes  炮的位行棋盤
unsigned short yBitCannonMove[13][1024];	//  26624 Bytes  炮的位列棋盤
								  // Total: //  77824 Bytes =  76K

unsigned short HistoryRecord[65535];		// 歷史啟發，數組下標為: move = (nSrc<<8)|nDst;

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

// 將車炮馬象士兵
static const int MvvValues[48] = 
{
      0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  10000, 2000, 2000, 1096, 1096, 1088, 1088, 1040, 1040, 1041, 1041, 1017, 1018, 1020, 1018, 1017,
  10000, 2000, 2000, 1096, 1096, 1088, 1088, 1040, 1040, 1041, 1041, 1017, 1018, 1020, 1018, 1017 
};

// 馬腿增量表︰有兩個功能
// 1. nHorseLegTab[nDst-nSrc+256] != 0				// 說明馬走“日”子，是偽合法移動
// 2. nLeg = nSrc + nHorseLegTab[nDst-nSrc+256]		// 馬腿的格子
//    Board[nLeg] == 0								// 馬腿位置沒有棋子，馬可以從nSrc移動到nDst
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


// 移動方向索引︰適合近距離移動的棋子
// 0 --- 不能移動到那裡
// 1 --- 上下左右，適合將帥和兵卒的移動
// 2 --- 士能夠到達
// 3 --- 馬能夠到達
// 4 --- 象能夠到達
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


// 著法預產生數組
	// 吃子移動 + 普通移動
static unsigned char KingMoves[256][8];				//   2048 Bytes, 將的移動數組
static unsigned char xRookMoves[12][512][12];		//  73728 Bytes, 車的橫向移動
static unsigned char yRookMoves[13][1024][12];		// 159744 Bytes, 車的縱向移動
static unsigned char xCannonMoves[12][512][12];		//  73728 Bytes, 炮的橫向移動
static unsigned char yCannonMoves[13][1024][12];	// 159744 Bytes, 炮的縱向移動
static unsigned char KnightMoves[256][12];			//   3072 Bytes, 馬的移動數組
static unsigned char BishopMoves[256][8];			//   2048 Bytes, 象的移動數組
static unsigned char GuardMoves[256][8];			//   2048 Bytes, 士的移動數組
static unsigned char PawnMoves[2][256][4];		    //   2048 Bytes, 兵的移動數組︰0-黑卒， 1-紅兵
										     // Total: 478208 Bytes = 467KB
	// 吃子移動
static unsigned char xRookCapMoves[12][512][4];	//  24576 Bytes, 車的橫向移動
static unsigned char yRookCapMoves[13][1024][4];	//  53248 Bytes, 車的縱向移動
static unsigned char xCannonCapMoves[12][512][4];	//  24576 Bytes, 炮的橫向移動
static unsigned char yCannonCapMoves[13][1024][4];	//  53248 Bytes, 炮的縱向移動
										     // Total: 155648 Bytes = 152KB

CMoveGen::CMoveGen(void)
{
	// 在整個程式生存過程中，CPreMove只需調用一次
	CPreMove PreMove;
	
	PreMove.InitKingMoves(KingMoves);
	PreMove.InitRookMoves(xRookMoves, yRookMoves, xRookCapMoves, yRookCapMoves);
	PreMove.InitCannonMoves(xCannonMoves, yCannonMoves, xCannonCapMoves, yCannonCapMoves);
	PreMove.InitKnightMoves(KnightMoves);
	PreMove.InitBishopMoves(BishopMoves);
	PreMove.InitGuardMoves(GuardMoves);
	PreMove.InitPawnMoves(PawnMoves);

	// 初始化用于車炮橫向與縱向移動的16位棋盤
	PreMove.InitBitRookMove(xBitRookMove, yBitRookMove);
	PreMove.InitBitCannonMove(xBitCannonMove, yBitCannonMove);
}

CMoveGen::~CMoveGen(void)
{
}


// 更新歷史記錄
// nMode==0  清除歷史記錄
// nMode!=0  衰減歷史記錄，HistoryRecord[m] >>= nMode;
void CMoveGen::UpdateHistoryRecord(unsigned int nMode)
{
	unsigned int m;
	unsigned int max_move = 0xCCCC;			// 移動的最大值0xFFFF，0xCCCC后面的移動不會用到

	if( nMode )								// 衰減歷史分數
	{
		for(m=0; m<max_move; m++)
			HistoryRecord[m] >>= nMode;
	}
	else									// 清零，用于新的局面
	{
		for(m=0; m<max_move; m++)
			HistoryRecord[m] = 0;
	}
}

// 根據Board[256], 產生所有合法的移動︰ Player==-1(黑方), Player==+1(紅方)
// 移動次序從採用 MVV/LVA(吃子移動) 和 歷史啟發結合，比單獨的歷史啟發快了10%
int CMoveGen::MoveGenerator(const int Player, CChessMove* pGenMove)
{
	call_vecAdd();
	const unsigned int  k = (1+Player) << 4;	    //k=16,黑棋; k=32,紅棋。
	unsigned int  move, nSrc, nDst, x, y, nChess;
	CChessMove* ChessMove = pGenMove;		//移動的計數器
	unsigned char *pMove;
							
	// 產生將帥的移動********************************************************************************************
	nChess = k;
	nSrc = Piece[nChess];								// 將帥存在︰nSrc!=0
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


	// 產生車的移動************************************************************************************************
	for( ; nChess<=k+2; nChess++)
	{
		if( nSrc = Piece[nChess] )						// 棋子存在︰nSrc!=0
		{			
			x = nSrc & 0xF;								// 后4位有效
			y = nSrc >> 4;								// 前4位有效

			//車的橫向移動︰
			pMove = xRookMoves[x][xBitBoard[y]];
			while( *pMove )
			{
				nDst = (nSrc & 0xF0) | (*(pMove++));	// 0x y|x  前4位=y*16， 后4位=x
				if( !Board[nDst] )
				{
					move = (nSrc<<8) | nDst;
					*(ChessMove++) = (HistoryRecord[move]<<16) | move;
				}
			}

			//車的縱向移動
			pMove = yRookMoves[y][yBitBoard[x]];
			while( *pMove )
			{
				nDst = (*(pMove++)) | x;				// 0x y|x  前4位=y*16， 后4位=x
				if( !Board[nDst] )
				{
					move = (nSrc<<8) | nDst;
					*(ChessMove++) = (HistoryRecord[move]<<16) | move;
				}
			}
		}
	}


	// 產生炮的移動************************************************************************************************
	for( ; nChess<=k+4; nChess++)
	{
		if( nSrc = Piece[nChess] )						// 棋子存在︰nSrc!=0
		{			
			x = nSrc & 0xF;								// 后4位有效
			y = nSrc >> 4;								// 前4位有效

			//炮的橫向移動
			pMove = xCannonMoves[x][xBitBoard[y]];
			while( *pMove )
			{
				nDst = (nSrc & 0xF0) | (*(pMove++));	// 0x y|x  前4位=y*16， 后4位=x
				if( !Board[nDst] )
				{
					move = (nSrc<<8) | nDst;
					*(ChessMove++) = (HistoryRecord[move]<<16) | move;
				}
			}

			//炮的縱向移動
			pMove = yCannonMoves[y][yBitBoard[x]];
			while( *pMove )
			{
				nDst = (*(pMove++)) | x;		// 0x y|x  前4位=y*16， 后4位=x	
				if( !Board[nDst] )
				{
					move = (nSrc<<8) | nDst;
					*(ChessMove++) = (HistoryRecord[move]<<16) | move;
				}
			}
		}
	}


	// 產生馬的移動******************************************************************************************
	for( ; nChess<=k+6; nChess++)
	{
		if( nSrc = Piece[nChess] )						// 棋子存在︰nSrc!=0
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


	// 產生象的移動******************************************************************************************
	for( ; nChess<=k+8; nChess++)
	{
		if( nSrc = Piece[nChess] )						// 棋子存在︰nSrc!=0
		{
			pMove = BishopMoves[nSrc];
			while( *pMove )
			{
				nDst = *(pMove++);
				if( !Board[(nSrc+nDst)>>1] )				//象眼無子
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


	// 產生士的移動******************************************************************************************
	for( ; nChess<=k+10; nChess++)
	{
		if( nSrc = Piece[nChess] )						// 棋子存在︰nSrc!=0
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


	// 產生兵卒的移動******************************************************************************************
	for( ; nChess<=k+15; nChess++)
	{
		if( nSrc = Piece[nChess] )						// 棋子存在︰nSrc!=0
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


// 產生所有合法的吃子移動
// 經分析，吃子移動占搜索時間的23%。
// 馬是最慢的，因為馬跳八方；兵卒其次，主要是兵卒數目最多。
int CMoveGen::CapMoveGen(const int Player, CChessMove* pGenMove)
{	
	const unsigned int k = (1+Player) << 4;				// k=16,黑棋; k=32,紅棋。
	unsigned int  nSrc, nDst, x, y, nChess, nCaptured;	
	CChessMove  *ChessMove = pGenMove;					// 保存最初的移動指針
	unsigned char *pMove;

	nChess = k+15;

	// 產生兵卒的移動******************************************************************************************
	for( ; nChess>=k+11; nChess--)
	{
		if( nSrc = Piece[nChess] )						// 棋子存在︰nSrc!=0
		{
			pMove = PawnMoves[Player][nSrc];
			while( *pMove )
			{
				nCaptured = Board[ nDst = *(pMove++) ];
				if( (nChess ^ nCaptured) >= 48 )		// 異色棋子
					*(ChessMove++) = ((MvvValues[nCaptured] - 20)<<16) | (nSrc<<8) | nDst;
			}
		}
	}


	// 產生士的移動******************************************************************************************
	for( ; nChess>=k+9; nChess--)
	{
		if( nSrc = Piece[nChess] )						// 棋子存在︰nSrc!=0
		{
			pMove = GuardMoves[nSrc];
			while( *pMove )
			{
				nCaptured = Board[ nDst = *(pMove++) ];
				if( (nChess ^ nCaptured) >= 48 )		// 異色棋子
					*(ChessMove++) = ((MvvValues[nCaptured] - 41)<<16) | (nSrc<<8) | nDst;
			}
		}
	}


	// 產生象的移動******************************************************************************************
	for( ; nChess>=k+7; nChess--)
	{
		if( nSrc = Piece[nChess] )						// 棋子存在︰nSrc!=0
		{
			pMove = BishopMoves[nSrc];
			while( *pMove )
			{
				nCaptured = Board[ nDst = *(pMove++) ];
				if( (nChess ^ nCaptured) >= 48 )		// 異色棋子
				{
					if( !Board[(nSrc+nDst)>>1] )					//象眼無子
						*(ChessMove++) = ((MvvValues[nCaptured] - 40)<<16) | (nSrc<<8) | nDst;
				}
			}
		}
	}


	// 產生馬的移動******************************************************************************************
	for( ; nChess>=k+5; nChess--)
	{
		if( nSrc = Piece[nChess] )						// 棋子存在︰nSrc!=0
		{
			pMove = KnightMoves[nSrc];
			while( *pMove )
			{
				nCaptured = Board[ nDst = *(pMove++) ];
				if( (nChess ^ nCaptured) >= 48 )		// 異色棋子
				{
					if( !Board[nSrc+nHorseLegTab[nDst-nSrc+256]] )
						*(ChessMove++) = ((MvvValues[nCaptured] - 88)<<16) | (nSrc<<8) | nDst;
				}
			}
		}
	}


	// 產生炮的移動************************************************************************************************
	for( ; nChess>=k+3; nChess--)
	{
		if( nSrc = Piece[nChess] )						// 棋子存在︰nSrc!=0
		{			
			x = nSrc & 0xF;								// 后4位有效
			y = nSrc >> 4;								// 前4位有效

			//炮的橫向移動
			pMove = xCannonCapMoves[x][xBitBoard[y]];
			while( *pMove )
			{
				nCaptured = Board[ nDst = (nSrc & 0xF0) | (*(pMove++)) ];	// 0x y|x  前4位=y*16， 后4位=x
				if( (nChess ^ nCaptured) >= 48 )		// 異色棋子
					*(ChessMove++) = ((MvvValues[nCaptured] - 96)<<16) | (nSrc<<8) | nDst;
			}

			//炮的縱向移動
			pMove = yCannonCapMoves[y][yBitBoard[x]];
			while( *pMove )
			{		
				nCaptured = Board[ nDst = (*(pMove++)) | x ];		// 0x y|x  前4位=y*16， 后4位=x
				if( (nChess ^ nCaptured) >= 48 )		// 異色棋子
					*(ChessMove++) = ((MvvValues[nCaptured] - 96)<<16) | (nSrc<<8) | nDst;
			}
		}
	}


	// 產生車的移動************************************************************************************************
	for( ; nChess>=k+1; nChess--)
	{
		if( nSrc = Piece[nChess] )						// 棋子存在︰nSrc!=0
		{			
			x = nSrc & 0xF;								// 后4位有效
			y = nSrc >> 4;								// 前4位有效

			//車的橫向移動
			pMove = xRookCapMoves[x][xBitBoard[y]];
			while( *pMove )
			{
				nCaptured = Board[ nDst = (nSrc & 0xF0) | (*(pMove++)) ];	// 0x y|x  前4位=y*16， 后4位=x
				if( (nChess ^ nCaptured) >= 48 )		// 異色棋子
					*(ChessMove++) = ((MvvValues[nCaptured] - 200)<<16) | (nSrc<<8) | nDst;
			}

			//車的縱向移動
			pMove = yRookCapMoves[y][yBitBoard[x]];
			while( *pMove )
			{
				nCaptured = Board[ nDst = (*(pMove++)) | x ];		// 0x y|x  前4位=y*16， 后4位=x
				if( (nChess ^ nCaptured) >= 48 )		// 異色棋子
					*(ChessMove++) = ((MvvValues[nCaptured] - 200)<<16) | (nSrc<<8) | nDst;
			}
		}
	}
                                                                                                                                                                                                                                                                                                                                                     

	// 產生將帥的移動********************************************************************************************
	nSrc = Piece[nChess];								// 棋子存在︰nSrc!=0
	{
		pMove = KingMoves[nSrc];
		while( *pMove )
		{
			nCaptured = Board[ nDst = *(pMove++) ];
			if( (nChess ^ nCaptured) >= 48 )		// 異色棋子
				*(ChessMove++) = ((MvvValues[nCaptured] - 1000)<<16) | (nSrc<<8) | nDst;
		}
	}	

	return int(ChessMove-pGenMove);
}


// 判斷殺手啟發著法的合法性
int CMoveGen::IsLegalKillerMove(int Player, const CChessMove KillerMove)
{	
	int nSrc = (KillerMove & 0xFF00) >> 8;
	int nMovedChs = Board[nSrc];
	if( (nMovedChs >> 4) != Player+1 )			// 若殺手不是本方的棋子，視為非法移動
		return 0;

	int nDst = KillerMove & 0xFF;
	if( (Board[nDst] >> 4) == Player+1 )		// 若殺手為吃子移動，吃同色棋子視為非法
		return 0;

	int x, y;
	switch( nPieceID[nMovedChs] )
	{
		case 1:		// 車
			x = nSrc & 0xF;
			y = nSrc >> 4;
			if( x == (nDst & 0xF) )
				return yBitRookMove[y][yBitBoard[x]] & yBitMask[nDst];		// x相等的縱向移動
			else if( y == (nDst >> 4) )
				return xBitRookMove[x][xBitBoard[y]] & xBitMask[nDst];		// y相等的縱向移動
			break;

		case 2:		// 炮
			x = nSrc & 0xF;
			y = nSrc >> 4;
			if( x == (nDst & 0xF) )
				return yBitCannonMove[y][yBitBoard[x]] & yBitMask[nDst];	// x相等的縱向移動
			else if( y == (nDst >> 4) )
				return xBitCannonMove[x][xBitBoard[y]] & xBitMask[nDst];	// y相等的縱向移動
			break;

		case 3:		// 馬
			if( !Board[ nSrc + nHorseLegTab[ nDst-nSrc+256 ] ] )		// 馬腿無子
				return 3;
			break;

		case 4:		// 象
			if( !Board[(nSrc+nDst)>>1] )									// 象眼無子
				return 4;
			break;

		default:
			return 1;														// 殺手啟發, 將士兵　必然合法
			break;
	}

	return 0;
}

// 使用位行與位列技術實現的將軍檢測
// 函數一旦遇到將軍，立即返回非“0”的數值
// 此函數用于當前移動方是否被將軍
// 注意︰車炮的位行與位列操作nDst->nSrc與殺手著法合理性檢驗nSrc->nDst正好相反
int CMoveGen::Checked(int Player)
{
	nCheckCounts ++;
	
	int nKingSq = Piece[(1+Player)<<4];		// 我方將帥的位置
	int x = nKingSq & 0xF;
	int y = nKingSq >> 4;
	int king = (2-Player) << 4 ;				// 對方將帥的序號

	int xBitMove = xBitRookMove[x][xBitBoard[y]];
	int yBitMove = yBitRookMove[y][yBitBoard[x]];
	
	// 雙王照面︰以講當車，使用車的位列棋盤
	int nSrc = Piece[king];
	if( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] )
		return nSrc;

	// 車將軍
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

	// 炮將軍
	nSrc = Piece[king+3];
	if( ( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] ) ||
		( y==(nSrc >>  4) && xBitMove & xBitMask[nSrc]) )
		return nSrc;

	nSrc = Piece[king+4];
	if( ( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] ) ||
		( y==(nSrc >>  4) && xBitMove & xBitMask[nSrc]) )		
		return nSrc;


	// 被對方的馬將軍
	nSrc = Piece[king+5];
	x = nHorseLegTab[nKingSq - nSrc + 256];
	if( x && !Board[nSrc + x] )							// 若馬走日且馬腿無子，馬將軍
		return nSrc;

	nSrc = Piece[king+6];
	x = nHorseLegTab[nKingSq - nSrc + 256];
	if( x && !Board[nSrc + x] )							// 若馬走日且馬腿無子，馬將軍
		return nSrc;


	// 被對方過河的兵卒將軍
	if( nPieceType[ Board[Player ? nKingSq-16 : nKingSq+16] ] == 13-7*Player )		// 注意︰將帥在中線時，不能有己方的兵卒存在
		return Player ? nKingSq-16 : nKingSq+16;

	if( nPieceID[ Board[nKingSq-1] ]==6 )
		return nKingSq-1;
		
	if( nPieceID[ Board[nKingSq+1] ]==6 )
		return nKingSq+1;

	nNonCheckCounts ++;
	return 0;
}


// 使用位行與位列技術實現的將軍檢測
// 計算所有能夠攻擊將帥的棋子checkers，將軍逃避函數能夠直接使用checkers變量，避免重複計算
// 返回值︰checkers!=0，表示將軍；checkers==0，表示沒有將軍
// checkers的后八位表示將軍的類型，分別表示︰兵0x80 兵0x40 馬0x20 馬0x10 炮0x08 炮0x04 車0x02 車0x01
// 使用此函數前，需先用Checked()判斷我方是否被將軍，然後利用此函數計算對方是否被將軍
// 注意︰因為前面已經使用了Checked()，所以此函數不必要對雙王照面再次進行檢測
// 注意︰車炮的位行與位列操作nDst->nSrc與殺手著法合理性檢驗nSrc->nDst正好相反，這樣設計可以減少計算
int CMoveGen::Checking(int Player)
{
	nCheckCounts ++;
	
	int nKingSq = Piece[(1+Player)<<4];		// 計算將帥的位置
	int x = nKingSq & 0xF;
	int y = nKingSq >> 4;
	int king = (2-Player) << 4 ;			// 將帥
	int checkers = 0;
	int nSrc;

	int xBitMove = xBitRookMove[x][xBitBoard[y]];
	int yBitMove = yBitRookMove[y][yBitBoard[x]];

	// 雙王照面︰無需檢測，不會發生
	//nSrc = Piece[king];
	//if( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] )
	//	checkers |= 0xFF;

	// 車將軍︰0x01表示車king+1, 0x02表示車king+2
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
	
	// 炮將軍︰0x04表示炮king+3, 0x08表示炮king+4
	nSrc = Piece[king+3];
	if( ( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] ) ||
		( y==(nSrc >>  4) && xBitMove & xBitMask[nSrc]) )
		checkers |= 0x04;

	nSrc = Piece[king+4];
	if( ( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] ) ||
		( y==(nSrc >>  4) && xBitMove & xBitMask[nSrc]) )		
		checkers |= 0x08;


	// 馬將軍︰0x10表示馬king+5，0x20表示馬king+6
	nSrc = Piece[king+5];
	x = nHorseLegTab[nKingSq - nSrc + 256];
	if( x && !Board[nSrc + x] )							// 若馬走日且馬腿無子，馬將軍
		checkers |= 0x10;

	nSrc = Piece[king+6];
	x = nHorseLegTab[nKingSq - nSrc + 256];
	if( x && !Board[nSrc + x] )							// 若馬走日且馬腿無子，馬將軍
		checkers |= 0x20;


	// 縱向兵卒︰0x40表示縱向的兵/卒將軍
	if( nPieceType[ Board[Player ? nKingSq-16 : nKingSq+16] ] == 13-7*Player )		// 將帥在中線時，不能有己方的中兵存在
		checkers |= 0x40;

	// 橫向兵卒︰0x80表示橫向的兵/卒將軍
	if( nPieceID[ Board[nKingSq-1] ]==6 || nPieceID[ Board[nKingSq+1] ]==6 )		// 查詢將帥左右是否有兵/卒存在
		checkers |= 0x80;


	if(!checkers)
		nNonCheckCounts ++;
	return checkers;
}

// 保護判斷函數
// 棋子從from->nDst, Player一方是否形成保護
// 試驗表明︰保護對吃子走法排序起到的作用很小。
int CMoveGen::Protected(int Player, int from, int nDst)
{
	const int king = (2-Player) << 4 ;			// 將帥
	
	//****************************************************************************************************
	// 將
	int nSrc = Piece[king];
	if( nDirection[nDst-nSrc+256]==1 && nCityIndex[nDst] )
		return 1000;

	// 象
	nSrc = Piece[king+7];
	if( nSrc && nSrc!=nDst && nDirection[nDst-nSrc+256]==4 && !Board[(nSrc+nDst)>>1] && (nSrc^nDst)<128 )
		return 40;

	nSrc = Piece[king+8];
	if( nSrc && nSrc!=nDst && nDirection[nDst-nSrc+256]==4 && !Board[(nSrc+nDst)>>1] && (nSrc^nDst)<128 )
		return 40;

	// 士
	nSrc = Piece[king+9];
	if( nSrc && nSrc!=nDst && nDirection[nDst-nSrc+256]==2 && nCityIndex[nDst] )
		return 41;

	nSrc = Piece[king+10];
	if( nSrc && nSrc!=nDst && nDirection[nDst-nSrc+256]==2 && nCityIndex[nDst] )
		return 41;

	// 兵卒
	nSrc = Player ? nDst-16 : nDst+16;
	if( nPieceType[ Board[nSrc] ] == 13-7*Player )		// 注意︰將帥在中線時，不能有己方的兵卒存在
		return 20;

	if( (Player && nDst<128) || (!Player && nDst>=128) )
	{
		if( nPieceID[ Board[nDst-1] ]==6 )
			return 17;
			
		if( nPieceID[ Board[nDst+1] ]==6 )
			return 17;
	}

	//*****************************************************************************************************
	// 車、炮、馬保護時，有必要清除起點位置from處的棋子，才能夠計算準確
	int x = nDst & 0xF;
	int y = nDst >> 4;
	int xBitIndex = xBitBoard[y] ^ xBitMask[from];		// 清除from之位行
	int yBitIndex = yBitBoard[x] ^ yBitMask[from];		// 清除from之位列

	const int m_Piece = Board[from];					// 保存from之棋子
	Piece[m_Piece] = 0;									// 臨時清除，以後恢復

	// 車
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

	// 炮
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


	// 馬
	nSrc = Piece[king+5];
	if( nSrc!=nDst )
	{
		x = nHorseLegTab[nDst - nSrc + 256];
		if( x && (!Board[nSrc + x] || x==from) )			// 若馬走日且馬腿無子，馬將軍
		{
			Piece[m_Piece] = from;
			return 88;
		}
	}

	nSrc = Piece[king+6];
	if( nSrc!=nDst )
	{
		x = nHorseLegTab[nDst - nSrc + 256];
		if( x && (!Board[nSrc + x] || x==from) )			// 若馬走日且馬腿無子，馬將軍
		{
			Piece[m_Piece] = from;
			return 88;
		}
	}

	// 恢復from處的棋子
	Piece[m_Piece] = from;

	return 0;
}

int CMoveGen::AddLegalMove(const int nChess, const int nSrc, const int nDst, CChessMove *ChessMove)
{
	int x, y, nCaptured;
	
	switch( nPieceID[nChess] )
	{
		case 0:		// 將行上下左右、在九宮中
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

		case 1:		// 車
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

		case 2:		// 炮
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

		case 3:		// 馬走日、馬腿無子
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

		case 4:		// 象走田、象眼無子、象未過河
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

		case 5:		// 士走斜線、在九宮中
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

		case 6:		// 兵卒行上下左右、走法合理
			x = nDst-nSrc;						// 當前移動方，Player
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


//********************將軍逃避產生器︰CheckEvasionGen()***********************************************
// 此函數並非起到完全解將的目的，但能夠把將軍檢測判斷的次數大大減少。
// 若完全解將，代碼非常複雜，每當移動一顆棋子，都要考慮是否讓對方的將車炮馬兵等獲得自由而構成新的將軍。
// 既然是不完全解將，后面一定要加將軍檢測。這樣會比完全解將的運算量少一些，有可能提前得到剪枝。
// Player   表示被將軍的一方，即當前移動方
// checkers 將軍檢測函數Checking(Player)的返回值，表示將軍的類型，可以立即知道被哪幾顆棋子將軍
//          后8位有效，分別表示︰橫兵、縱兵、馬2、馬1、炮2、炮1、車2、車1
//          之所以不包含將帥的將軍訊息，是因為Checked(Player)函數已經作了處理。
int CMoveGen::CheckEvasionGen(const int Player, int checkers, CChessMove* pGenMove)
{
	nCheckEvasions ++;							// 統計解將函數的營運次數

	const int MyKing   = (1+Player) << 4;		// MyKing=16,黑棋; MyKing=32,紅棋。
	const int OpKing = (2-Player) << 4;			// 對方王的棋子序號
	const int nKingSq = Piece[MyKing];			// 計算將帥的位置

	int nDir0=0, nDir1=0;						// 將軍方向︰1－橫向；16－縱向
	int nCheckSq0, nCheckSq1;					// 將軍棋子的位置
	int nMin0, nMax0, nMin1, nMax1;				// 用于車炮與將帥之間的範圍
	int nPaojiazi0, nPaojiazi1;					// 炮架子的位置	
	int nPaojiaziID0, nPaojiaziID1;				// 炮架子的棋子類型

	int nChess, nCaptured, nSrc, nDst, x, y;
	unsigned char *pMove;
	CChessMove *ChessMove = pGenMove;			// 初始化移動的指針

	// 解含炮將軍的方法最複雜，針對含炮將軍局面作特殊處理，以減少重複計算
	if( checkers & 0x0C )
	{
		// 第一個炮將軍的位置
		nCheckSq0 = Piece[ OpKing + (checkers&0x04 ? 3:4) ];

		// 將軍方向︰1－橫向；16－縱向
		nDir0 = (nKingSq&0xF)==(nCheckSq0&0xF) ? 16:1;

		// 炮將之間的範圍[nMin0, nMax0)，不包含將帥的位置
		nMin0 = nKingSq>nCheckSq0 ? nCheckSq0 : nKingSq+nDir0;
		nMax0 = nKingSq<nCheckSq0 ? nCheckSq0 : nKingSq-nDir0;		

		// 尋找炮架子的位置︰nPaojiazi0
		// 計算炮架子的類型︰nPaojiaziID0
		for(nDst=nMin0; nDst<=nMax0; nDst+=nDir0)
		{
			if( Board[nDst] && nDst!=nCheckSq0 )
			{
				nPaojiazi0 = nDst;
				nPaojiaziID0 = Board[nDst];
				break;
			}
		}
		
		// 若炮架子是對方的炮，產生界于雙炮之間的移動
		if( nPaojiaziID0==OpKing+3 || nPaojiaziID0==OpKing+4 )
		{
			nMin0 = nPaojiazi0>nCheckSq0 ? nCheckSq0 : nPaojiazi0+nDir0;
			nMax0 = nPaojiazi0<nCheckSq0 ? nCheckSq0 : nPaojiazi0-nDir0;
		}
	}

	// 根據“將軍類型”進行解將
	// 採用窮舉法，逐個分析每種能夠解將的類型
	switch( checkers )
	{
		// 單車將軍︰殺、擋
		case 0x01:
		case 0x02:
			nCheckSq0 = Piece[ OpKing + (checkers&0x01 ? 1:2) ];			
			nDir0 = (nKingSq&0xF)==(nCheckSq0&0xF) ? 16:1;
			nMin0 = nKingSq>nCheckSq0 ? nCheckSq0 : nKingSq+nDir0;
			nMax0 = nKingSq<nCheckSq0 ? nCheckSq0 : nKingSq-nDir0;

			x = nDir0==1 ? MyKing+10 : MyKing+15;
			for(nDst=nMin0; nDst<=nMax0; nDst+=nDir0)
			{	
				// 車炮馬象士兵，殺車或者擋車
				for(nChess=MyKing+1; nChess<=x; nChess++)
				{
					if( (nSrc=Piece[nChess]) != 0 )
						ChessMove += AddLegalMove(nChess, nSrc, nDst, ChessMove);
				}
			}

			break;


		// 單炮將軍︰殺、擋
		case 0x04:  // 炮1
		case 0x08:  // 炮2
			// 殺炮、阻擋炮將軍；暫不產生炮架子的移動
			// 炮架子本身也可以殺炮，但不能產生將軍方向的非吃子移動
			x = nDir0==1 ? MyKing+10 : MyKing+15;
			for(nDst=nMin0; nDst<=nMax0; nDst+=nDir0)
			{	
				if( nDst==nPaojiazi0 )							// 殺炮架子無用﹗
					continue;

				// 車炮馬象士兵，殺炮或者擋炮
				for(nChess=MyKing+1; nChess<=x; nChess++)
				{
					nSrc = Piece[nChess];
					if(nSrc && nSrc!=nPaojiazi0)				// 炮架子是我方棋子，暫不產生這種移動
						ChessMove += AddLegalMove(nChess, nSrc, nDst, ChessMove);
				}
			}

			// 產生撤炮架子的移動，形成空頭炮，使炮喪失將軍能力。
			// 若炮架子是車兵，可以殺將軍的炮；炮架子是炮，可以越過對方的炮或者己方的將殺子逃離。
			// 對于車砲兵三個滑塊棋子，應該禁止將軍方向上的非吃子移動，移動后還是炮架子；
			// 倘若對方炮的背后，存在另一個炮，唯有用炮打吃，可以解將，否則棋子撤離后會形成雙炮之勢
			if( (nPaojiaziID0-16)>>4==Player )					// 炮架子是己方的棋子
			{
				nSrc = nPaojiazi0;
				nChess = Board[nSrc];
				x = nSrc & 0xF;								// 后4位有效
				y = nSrc >> 4;								// 前4位有效

				switch( nPieceID[Board[nSrc]] )
				{
					case 1:			
						// 車的橫向移動︰縱向將軍時，車可以橫向撤離
						//               橫向將軍時，車可以殺炮
						pMove = xRookMoves[x][xBitBoard[y]];
						while( *pMove )
						{
							nDst = (nSrc & 0xF0) | (*(pMove++));	// 0x y|x  前4位=y*16， 后4位=x
							nCaptured = Board[nDst];

							if( !nCaptured && nDir0==16 )
								*(ChessMove++) = (nSrc<<8) | nDst;
							else if( (nChess^nCaptured)>=48 )
								*(ChessMove++) = (MvvValues[nCaptured]<<16) | (nSrc<<8) | nDst;
						}
						// 車的縱向移動︰橫向將軍時，車可以縱向撤離
						pMove = yRookMoves[y][yBitBoard[x]];
						while( *pMove )
						{
							nDst = (*(pMove++)) | x;				// 0x y|x  前4位=y*16， 后4位=x
							nCaptured = Board[nDst];

							if( !nCaptured && nDir0==1 )
								*(ChessMove++) = (nSrc<<8) | nDst;
							else if( (nChess^nCaptured)>=48 )
								*(ChessMove++) = (MvvValues[nCaptured]<<16) | (nSrc<<8) | nDst;
						}
						break;

					case 2:
						// 炮的橫向移動︰縱向將軍時，炮可以橫向撤離；無論縱橫將軍，炮都可以吃子逃離
						pMove = xCannonMoves[x][xBitBoard[y]];
						while( *pMove )
						{
							nDst = (nSrc & 0xF0) | (*(pMove++));	// 0x y|x  前4位=y*16， 后4位=x
							nCaptured = Board[nDst];

							if( !nCaptured && nDir0==16 )
								*(ChessMove++) = (nSrc<<8) | nDst;
							else if( (nChess^nCaptured) >= 48 )
								*(ChessMove++) = (MvvValues[nCaptured]<<16) | (nSrc<<8) | nDst;
						}

						// 炮的縱向移動︰橫向將軍時，炮可以縱向撤離；無論縱橫將軍，炮都可以吃子逃離
						pMove = yCannonMoves[y][yBitBoard[x]];
						while( *pMove )
						{
							nDst = (*(pMove++)) | x;		// 0x y|x  前4位=y*16， 后4位=x
							nCaptured = Board[nDst];

							if( !nCaptured && nDir0==1 )
								*(ChessMove++) = (nSrc<<8) | nDst;
							else if( (nChess^nCaptured) >= 48 )
								*(ChessMove++) = (MvvValues[nCaptured]<<16) | (nSrc<<8) | nDst;
						}
						break;

					case 3:		
						// 馬︰逃離將軍方向
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
						// 象︰逃離將軍方向
						pMove = BishopMoves[nSrc];
						while( *pMove )
						{
							nDst = *(pMove++);				
							nCaptured = Board[nDst];

							if( !Board[(nSrc+nDst)>>1] )					//象眼無子
							{
								if( !nCaptured )
									*(ChessMove++) = (nSrc<<8) | nDst;
								else if( (nChess^nCaptured) >= 48 )
									*(ChessMove++) = (MvvValues[nCaptured]<<16) | (nSrc<<8) | nDst;
							}
						}
						break;

					case 5:
						// 士︰逃離將軍方向
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
						// 兵卒︰縱向將軍，橫向逃離將軍方向；橫向將軍，兵卒不能到達解將位置
						pMove = PawnMoves[Player][nSrc];
						while( *pMove )
						{
							nDst = *(pMove++);
							nCaptured = Board[nDst];
							
							if( !nCaptured && nDir0==16 && x != (nDst&0xF) )			// 橫向逃離，禁止縱向移動
								*(ChessMove++) = (nSrc<<8) | nDst;
							else if( (nChess^nCaptured)>=48 )							// 縱向殺將軍的炮
								*(ChessMove++) = (MvvValues[nCaptured]<<16) | (nSrc<<8) | nDst;
						}
						break;

					default:
						break;
				}
			}
			// 炮架子是對方的馬，且形成馬后炮之勢，不能用移將法解將，到此為止，程式返回
			else if( nPaojiaziID0==OpKing+5 || nPaojiaziID0==OpKing+6 )
			{
				if( nKingSq-nPaojiazi0==2  || nKingSq-nPaojiazi0==-2 || 
					nKingSq-nPaojiazi0==32 || nKingSq-nPaojiazi0==-32 )
				return int(ChessMove-pGenMove);
			}

			break;


		// 炮車將軍︰
		case 0x05:  // 炮1車1
		case 0x06:  // 炮1車2
		case 0x09:  // 炮2車1
		case 0x0A:  // 炮2車2
			nCheckSq1 = Piece[ OpKing + (checkers&0x01 ? 1:2) ];
			nDir1 = (nKingSq&0xF)==(nCheckSq1&0xF) ? 16:1;
			nMin1 = nKingSq>nCheckSq1 ? nCheckSq1 : nKingSq+nDir1;
			nMax1 = nKingSq<nCheckSq1 ? nCheckSq1 : nKingSq-nDir1;

			// 車炮分別從兩個方向將軍，並且炮架子是對方棋子，無解
			if( nDir0!=nDir1 && (nPaojiaziID0-16)>>4==1-Player )
				return 0;
			// 炮架子是對方的馬，且形成馬后炮之勢，無解
			else if( nPaojiaziID0==OpKing+5 || nPaojiaziID0==OpKing+6 )
			{
				if( nKingSq-nPaojiazi0==2  || nKingSq-nPaojiazi0==-2 || 
					nKingSq-nPaojiazi0==32 || nKingSq-nPaojiazi0==-32 )
				return 0;
			}
			// 若炮架子是對方的車，產生界于車將之間的非吃子移動移動，不包含車與將
			else if( nPaojiaziID0==OpKing+1 || nPaojiaziID0==OpKing+2 )
			{
				nMin0 = nKingSq>nPaojiazi0 ? nPaojiazi0+nDir0 : nKingSq+nDir0;
				nMax0 = nKingSq<nPaojiazi0 ? nPaojiazi0-nDir0 : nKingSq-nDir0;

				x = nDir0==1 ? MyKing+10 : MyKing+15;
				for(nDst=nMin0; nDst<=nMax0; nDst+=nDir0)
				{	
					// 車炮馬象士兵，殺車或者擋車
					for(nChess=MyKing+1; nChess<=x; nChess++)
					{
						if( (nSrc=Piece[nChess]) != 0 )
							ChessMove += AddLegalMove(nChess, nSrc, nDst, ChessMove);
					}
				}
			}
			// 炮架子是己方的棋子(炮馬象士)，車兵無法解將
			else if( nPaojiaziID0>=MyKing+3 && nPaojiaziID0<=MyKing+10 )
			{
				// 產生炮架子殺車或者阻擋車的移動
				nChess = Board[nPaojiazi0];
				for(nDst=nMin1; nDst<=nMax1; nDst+=nDir1)
					ChessMove += AddLegalMove(nChess, nPaojiazi0, nDst, ChessMove);
			}

			break;


		// 炮炮將軍，炮炮車將軍
		// "4ka2C/9/4b4/9/9/4C4/9/9/9/4K4 b - - 0 1"
		// "4kR2C/9/4b4/9/9/4C4/9/9/9/4K4 b - - 0 1"
		case 0x0C:	// 炮1炮2		
		case 0x0D:  // 炮1炮2車1
		case 0x0E:  // 炮1炮2車2
			// 若炮架子是對方的車，產生界于車將之間的移動
			if( nPaojiaziID0==OpKing+1 || nPaojiaziID0==OpKing+2 )
			{
				nMin0 = nKingSq>nPaojiazi0 ? nPaojiazi0+nDir0 : nKingSq+nDir0;
				nMax0 = nKingSq<nPaojiazi0 ? nPaojiazi0-nDir0 : nKingSq-nDir0;
			}
			

			nCheckSq1 = Piece[ OpKing + 4 ];
			nDir1 = (nKingSq&0xF)==(nCheckSq1&0xF) ? 16:1;
			nMin1 = nKingSq>nCheckSq1 ? nCheckSq1 : nKingSq+nDir1;
			nMax1 = nKingSq<nCheckSq1 ? nCheckSq1 : nKingSq-nDir1;

			// 尋找炮架子的位置
			for(nDst=nMin1+nDir1; nDst<nMax1; nDst+=nDir1)
			{
				if( Board[nDst] )
				{
					nPaojiazi1 = nDst;
					nPaojiaziID1 = Board[nDst];
					break;
				}
			}

			// 若炮架子是對方的車，產生界于車將之間的移動
			if( nPaojiaziID1==OpKing+1 || nPaojiaziID1==OpKing+2 )
			{
				nMin1 = nKingSq>nPaojiazi1 ? nPaojiazi1+nDir0 : nKingSq+nDir0;
				nMax1 = nKingSq<nPaojiazi1 ? nPaojiazi1-nDir0 : nKingSq-nDir0;
			}
			
			// 炮架子是己方的棋子︰馬、象、士
			if( nPaojiaziID0>=MyKing+5 && nPaojiaziID0<=MyKing+10 )
			{
				for(nDst=nMin1; nDst<=nMax1; nDst+=nDir1)
				{
					if( nDst==nPaojiazi1 )
						continue;
					ChessMove += AddLegalMove(nPaojiaziID0, nPaojiazi0, nDst, ChessMove);
				}
			}
			// 炮架子是己方的棋子︰馬、象、士
			if( nPaojiaziID1>=MyKing+5 && nPaojiaziID1<=MyKing+10 )	
			{
				for(nDst=nMin0; nDst<=nMax0; nDst+=nDir0)
				{
					if( nDst==nPaojiazi0 )
						continue;
					ChessMove += AddLegalMove(nPaojiaziID1, nPaojiazi1, nDst, ChessMove);
				}
			}

			// 炮架子是對方的馬，且形成馬后炮之勢，移將法不能解將，到此返回
			if( nPaojiaziID0==OpKing+5 || nPaojiaziID0==OpKing+6 )
			{
				if( nKingSq-nPaojiazi0==2  || nKingSq-nPaojiazi0==-2 || 
					nKingSq-nPaojiazi0==32 || nKingSq-nPaojiazi0==-32 )
				return int(ChessMove-pGenMove);
			}

			// 炮架子是對方的馬，且形成馬后炮之勢，移將法不能解將，到此返回
			if( nPaojiaziID1==OpKing+5 || nPaojiaziID1==OpKing+6 )
			{
				if( nKingSq-nPaojiazi1==2  || nKingSq-nPaojiazi1==-2 || 
					nKingSq-nPaojiazi1==32 || nKingSq-nPaojiazi1==-32 )
				return int(ChessMove-pGenMove);
			}

			break;

		// 炮馬將軍︰
		case 0x14:	// 炮1馬1
		case 0x18:  // 炮2馬1
		case 0x24:	// 炮1馬2
		case 0x28:  // 炮2馬2
			// 炮架子是己方的棋子︰車、炮、馬、象、士，不包含將和兵
			if( nPaojiaziID0>=MyKing+1 && nPaojiaziID0<=MyKing+10 )
			{
				nCheckSq1 = Piece[ OpKing + (checkers&0x10 ? 5:6) ];
				ChessMove += AddLegalMove(nPaojiaziID0, nPaojiazi0, nCheckSq1, ChessMove);
				ChessMove += AddLegalMove(nPaojiaziID0, nPaojiazi0, nCheckSq1+nHorseLegTab[nKingSq-nCheckSq1+256], ChessMove);
			}
			break;


		// 炮馬馬將軍︰二馬的馬腿位置相同時，產生炮架子(車炮馬士)絆馬腿的移動
		// "3k1a2C/1r3N3/4N4/5n3/9/9/9/9/9/3K5 b - - 0 1"
		case 0x34:  // 炮1馬1馬2
		case 0x38:  // 炮2馬1馬2
			// 炮架子是己方的棋子︰車、炮、馬、象、士，不包含將和兵
			if( nPaojiaziID0>=MyKing+1 && nPaojiaziID0<=MyKing+10 )
			{
				nCheckSq0 = Piece[ OpKing + 5 ];
				nCheckSq1 = Piece[ OpKing + 6 ];
				nDst = nCheckSq0+nHorseLegTab[nKingSq-nCheckSq0+256];
				if( nDst==nCheckSq1+nHorseLegTab[nKingSq-nCheckSq1+256] )
					ChessMove += AddLegalMove(nPaojiaziID0, nPaojiazi0, nDst, ChessMove);
			}
			break;


		// 砲兵將軍︰炮架子吃兵；若炮架子是兵，只能移將解將
		case 0x44:  // 炮1縱兵
		case 0x48:  // 炮2縱兵
		case 0x84:  // 炮1橫兵
		case 0x88:  // 炮2橫兵
			// 炮架子是己方的棋子
			if( (nPaojiaziID0-16)>>4==Player )					
			{
				// 炮架子殺縱向之兵
				if( checkers<=0x48 )
					ChessMove += AddLegalMove(nPaojiaziID0, nPaojiazi0, nKingSq+(Player?-16:16), ChessMove);
				// 炮架子殺橫向之兵
				else
				{
					nCheckSq0 = nKingSq-1;
					nCheckSq1 = nKingSq+1;
					// 左右都是兵，不能用這種方法解將
					if( Board[nCheckSq0]!=Board[nCheckSq1] )
					{
						// 炮架子吃左兵
						if( nPieceID[Board[nCheckSq0]]==6 )
							ChessMove += AddLegalMove(nPaojiaziID0, nPaojiazi0, nCheckSq0, ChessMove);
						// 炮架子吃右兵
						if( nPieceID[Board[nCheckSq1]]==6 )
							ChessMove += AddLegalMove(nPaojiaziID0, nPaojiazi0, nCheckSq1, ChessMove);
					}
				}
			}
			// 炮架子是對方的馬，且形成馬后炮之勢，不能解將
			else if( nPaojiaziID0==OpKing+5 || nPaojiaziID0==OpKing+6 )
			{
				if( nKingSq-nPaojiazi0==2  || nKingSq-nPaojiazi0==-2 || 
					nKingSq-nPaojiazi0==32 || nKingSq-nPaojiazi0==-32 )
				return 0;
			}
			break;


		// 單馬將軍︰吃馬、絆馬腿
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

		// 雙馬將軍︰二馬的馬腿位置相同時，產生絆馬腿的移動
		case 0x30:
			nCheckSq0 = Piece[ OpKing + 5 ];
			nCheckSq1 = Piece[ OpKing + 6 ];
			nDst = nCheckSq0+nHorseLegTab[nKingSq-nCheckSq0+256];
			if( nDst == nCheckSq1+nHorseLegTab[nKingSq-nCheckSq1+256] )
			{
				// 車炮馬象士
				for(nChess=MyKing+1; nChess<=MyKing+10; nChess++)
				{
					if( (nSrc=Piece[nChess]) != 0 )
						ChessMove += AddLegalMove(nChess, nSrc, nDst, ChessMove);
				}
			}
			break;

		// 縱向單兵︰吃兵
		case 0x40:
			nDst = nKingSq+(Player?-16:16);
			// 車炮馬象士
			for(nChess=MyKing+1; nChess<=MyKing+10; nChess++)
			{
				if( (nSrc=Piece[nChess]) != 0 )
					ChessMove += AddLegalMove(nChess, nSrc, nDst, ChessMove);
			}
			break;

		// 橫向兵卒︰吃兵
		case 0x80:
			nCheckSq0 = nKingSq-1;
			nCheckSq1 = nKingSq+1;

			// 左右都是兵，不能用這種方法解將
			if( Board[nCheckSq0]!=Board[nCheckSq1] )
			{
				// 吃左兵
				if( nPieceID[Board[nCheckSq0]]==6 )
				{
					// 車炮馬象士
					for(nChess=MyKing+1; nChess<=MyKing+10; nChess++)
					{
						if( (nSrc=Piece[nChess]) != 0 )
							ChessMove += AddLegalMove(nChess, nSrc, nCheckSq0, ChessMove);
					}
				}

				// 吃右兵
				if( nPieceID[Board[nCheckSq1]]==6 )
				{
					// 車炮馬象士
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

	// 移將法進行解將
	pMove = KingMoves[nKingSq];
	while( *pMove )
	{
		nDst = *(pMove++);
		nCaptured = Board[nDst];			
		
		if( !nCaptured && (													// 產生非吃子移動
			(!nDir0 && !nDir1) ||											// 沒有車炮將軍
			(nDir0!=1 && nDir1!=1 && (nKingSq&0xF0)==(nDst&0xF0)) ||		// 將橫向移動，車炮不可橫向將軍
			(nDir0!=16 && nDir1!=16 && (nKingSq&0xF)==(nDst&0xF)) ) )		// 將縱向移動，車炮不可縱向將軍
			*(ChessMove++) = (nKingSq<<8) | nDst;
		else if( (MyKing^nCaptured) >= 48 )					// 產生吃子移動
			*(ChessMove++) = (MvvValues[nCaptured]<<16) | (nKingSq<<8) | nDst;
	}
	
	return int(ChessMove-pGenMove);
}


