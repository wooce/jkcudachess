////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 頭文件︰PreMove.h                                                                                      //
// *******************************************************************************************************//
// 中國象棋通用引擎----兵河五四，支持《中國象棋通用引擎協議》(Universal Chinese Chess Protocol，簡稱ucci) //
// 作者︰ 范 德 軍                                                                                        //
// 單位︰ 中國原子能科學研究院                                                                            //
// 郵箱︰ fan_de_jun@sina.com.cn                                                                          //
//  QQ ︰ 83021504                                                                                        //
// *******************************************************************************************************//
// 功能︰                                                                                                 //
// 1. 初始化棋子（將車炮馬象士兵）的移動，產生偽合法的著法。                                              //
// 2. 初始化車炮位行與位列棋盤                                                                            //
//                                                                                                        //
// 注︰CPreMove類不包含實際的著法數組，初始化完成后，即可釋放此類，程式啟動后只需調用一次                 //
////////////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

static const char nBoardIndex[256] = 
{
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,
    0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,
    0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,
    0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,
    0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,
    0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,
    0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,
    0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,
    0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,
    0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

const int nCityIndex[256] = 
{
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   1,   1,   1,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   1,   1,   1,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   1,   1,   1,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   1,   1,   1,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   1,   1,   1,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   1,   1,   1,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};


// 移動預產生類
class CPreMove
{
public:
	CPreMove(void);
	virtual ~CPreMove(void);


// 著法預產生
public:	
	void InitKingMoves(unsigned char KingMoves[][8]);

	void InitRookMoves(unsigned char xRookMoves[][512][12], unsigned char yRookMoves[][1024][12], unsigned char xRookCapMoves[][512][4], unsigned char yRookCapMoves[][1024][4]);
	void InitCannonMoves(unsigned char xCannonMoves[][512][12], unsigned char yCannonMoves[][1024][12], unsigned char xCannonCapMoves[][512][4], unsigned char yCannonCapMoves[][1024][4]);

	void InitKnightMoves(unsigned char KnightMoves[][12]);
	void InitBishopMoves(unsigned char BishopMoves[][8]);

	void InitGuardMoves(unsigned char GuardMoves[][8]);
	void InitPawnMoves(unsigned char PawnMoves[][256][4]);



// 車炮橫向與縱向移動的位棋盤
public:						
	void InitBitRookMove( unsigned short xBitRookMove[][512], unsigned short yBitRookMove[][1024] );
	void InitBitCannonMove( unsigned short xBitCannonMove[][512], unsigned short yBitCannonMove[][1024] );
	void InitBitSupperCannon( unsigned short xBitSupperCannon[][512], unsigned short yBitSupperCannon[][1024] );
};

