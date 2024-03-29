////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 源文件︰Evaluation.cpp                                                                                 //
// *******************************************************************************************************//
// 中國象棋通用引擎----兵河五四，支持《中國象棋通用引擎協議》(Universal Chinese Chess Protocol，簡稱ucci) //
// 作者︰ 范 德 軍                                                                                        //
// 單位︰ 中國原子能科學研究院                                                                            //
// 郵箱︰ fan_de_jun@sina.com.cn                                                                          //
//  QQ ︰ 83021504                                                                                        //
// *******************************************************************************************************//
// 功能︰                                                                                                 //
// 1. 先手分=4                                                                                            //
// 2. 棋子價值                                                                                            //
// 3. 位置分                                                                                              //
// 4.                                                                                                     //
// 5.                                                                                                     //
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Evaluation.h"
#include "PreMove.h"
#include "MoveGen.h"

//***********************************************************************************************************************************
// 全局變量
//***********************************************************************************************************************************

int Evalue[2];							// 雙方估值︰0=黑方, 1=紅方	
int PositionValue[14][256];				// 局面估值
// 棋子的基本價值
// 將、車、炮、馬、象、士、兵 (0-6為黑子，7-13為紅子)
const int BasicValues[14] = {1000, 200, 96, 88, 40, 40, 9, 1000, 200, 96, 88, 40, 40, 9};

// 先手分，大約為半個兵的價值。能夠消除奇偶層估值擺動的問題。
const int nOffensiveValue = 4;

// 棋子位置的靜態估值︰以紅子為基準
static const int PosValues[7][256] = {
	{ // 將
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
        0,  0,  0,  0,  0,  0, -7, -9, -7,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0, -6, -8, -6,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  1,  5,  1,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
	}, 

	{ // 車
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  6,  8,  7, 13, 14, 13,  7,  8,  6,  0,  0,  0,  0,
		0,  0,  0,  6, 12,  9, 16, 33, 16,  9, 12,  6,  0,  0,  0,  0,
        0,  0,  0,  6,  8,  7, 14, 16, 14,  7,  8,  6,  0,  0,  0,  0,
        0,  0,  0,  6, 13, 13, 16, 16, 16, 13, 13,  6,  0,  0,  0,  0,
		0,  0,  0,  8, 11, 11, 14, 15, 14, 11, 11,  8,  0,  0,  0,  0,
		0,  0,  0,  8, 12, 12, 14, 15, 14, 12, 12,  8,  0,  0,  0,  0,
        0,  0,  0,  4,  9,  4, 12, 14, 12,  4,  9,  4,  0,  0,  0,  0,
        0,  0,  0, -2,  8,  4, 12, 12, 12,  4,  8, -2,  0,  0,  0,  0,
		0,  0,  0,  5,  8,  3, 12,-40, 12,  3,  8,  5,  0,  0,  0,  0,
		0,  0,  0, -6,  6,  4, 12,  0, 12,  4,  6, -6,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
	}, 

	{ // 炮
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  4,  4,  0, -5, -6, -5,  0,  4,  4,  0,  0,  0,  0,
		0,  0,  0,  2,  2,  0, -4, -7, -4,  0,  2,  2,  0,  0,  0,  0,
        0,  0,  0,  1,  1,  0, -5, -4, -5,  0,  1,  1,  0,  0,  0,  0,
        0,  0,  0,  0,  3,  3,  2,  4,  2,  3,  3,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0, -1,  0,  3,  0,  4,  0,  3,  0, -1,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  1,  0,  4,  3,  5,  3,  4,  0,  1,  0,  0,  0,  0,
		0,  0,  0,  0,  1,  2,  2,-20,  2,  2,  1,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  1,  3,  3,  3,  1,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
	}, 

	{ // 馬
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  2,  2,  2,  8,  2,  8,  2,  2,  2,  0,  0,  0,  0,
		0,  0,  0,  2,  2, 15,  9,  6,  9, 15,  8,  2,  0,  0,  0,  0,
        0,  0,  0,  4, 10, 11, 15, 11, 15, 11, 10,  4,  0,  0,  0,  0,
        0,  0,  0,  5, 20, 12, 19, 12, 19, 12, 29,  5,  0,  0,  0,  0,
		0,  0,  0,  2, 12, 11, 15, 16, 15, 11, 12,  2,  0,  0,  0,  0,
		0,  0,  0,  2, 10, 13, 14, 15, 14, 13, 10,  2,  0,  0,  0,  0,
        0,  0,  0,  4,  6, 10,  7, 10,  7, 10,  6,  4,  0,  0,  0,  0,
        0,  0,  0,  5,  4,  6,  7,  4,  7,  6,  4,  5,  0,  0,  0,  0,
		0,  0,  0, -3,  2,  4,  5,-30,  5,  4,  2, -3,  0,  0,  0,  0,
		0,  0,  0,  0, -3,  2,  0,  2,  0,  2, -3,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
	},

	{ // 象
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
        0,  0,  0, -2,  0,  0,  0,  3,  0,  0,  0, -2,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
	}, 

	{ // 士
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
		0,  0,  0,  0,  0,  0,  0,  5,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
	},

	{ // 兵
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  2,  4,  2,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0, 20, 30, 50, 65, 70, 65, 50, 30, 20,  0,  0,  0,  0,		
        0,  0,  0, 20, 30, 45, 55, 55, 55, 45, 30, 20,  0,  0,  0,  0,
        0,  0,  0, 20, 27, 30, 40, 42, 40, 30, 27, 20,  0,  0,  0,  0,
		0,  0,  0, 10, 18, 22, 35, 40, 35, 22, 18, 10,  0,  0,  0,  0,
		0,  0,  0,  3,  0,  4,  0,  7,  0,  4,  0,  3,  0,  0,  0,  0,
        0,  0,  0, -2,  0, -2,  0,  6,  0, -2,  0, -2,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
	}
};


// 馬對王的威脅
static const char nHorseCheck[512] = {
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
   0,  0,  0,  0,  0,  2,  0,  2,  0,  2,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  2,  0,  2,  0,  2,  0,  2,  0,  0,  0,  0,  0,
   0,  0,  0,  2,  0,  3,  1,  2,  1,  3,  0,  2,  0,  0,  0,  0,
   0,  0,  0,  0,  2,  1,  2,  0,  2,  1,  2,  0,  0,  0,  0,  0,
   0,  0,  0,  2,  0,  2,  0,  0,  0,  2,  0,  2,  0,  0,  0,  0,
   0,  0,  0,  0,  2,  1,  2,  0,  2,  1,  2,  0,  0,  0,  0,  0,
   0,  0,  0,  2,  0,  3,  1,  2,  1,  3,  0,  2,  0,  0,  0,  0,
   0,  0,  0,  0,  2,  0,  2,  0,  2,  0,  2,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  2,  0,  2,  0,  2,  0,  0,  0,  0,  0,  0,
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

// 兵對王的威脅
static const char nPawnCheck[512] = {
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
   0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  4,  3,  4,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  4,  3,  2,  3,  4,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  4,  3,  2,  1,  2,  3,  4,  0,  0,  0,  0,  0,
   0,  0,  0,  4,  3,  2,  1,  0,  1,  2,  3,  4,  0,  0,  0,  0,
   0,  0,  0,  0,  4,  3,  2,  1,  2,  3,  4,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  4,  3,  2,  3,  4,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  4,  3,  4,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0,  0,
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


//***********************************************************************************************************************************
// 靜態局部變量
//***********************************************************************************************************************************
//static const int nDis[32] = { 0, 0, 0, 0, 0, 0, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0, 0, 0, 0, 0 };
static unsigned short xBitSupperCannon[12][512];
static unsigned short yBitSupperCannon[13][1024];


//***********************************************************************************************************************************
// 估值函數
//***********************************************************************************************************************************

CEvaluation::CEvaluation(void)
{
	// 初始化超級遠程炮
	CPreMove PreMove;
	PreMove.InitBitSupperCannon(xBitSupperCannon, yBitSupperCannon);

	//計算所有棋子位置的靜態估值
	int m, n, p;
	for(m=0; m<14; m++)
	{
		for(n=0; n<256; n++)
		{
			if(m<7)
			{
				p = ((15 - (n>>4)) << 4 ) + (n & 0xF);
				PositionValue[m][n] = PosValues[m][p];	// 黑子
			}
			else
				PositionValue[m][n] = PosValues[m-7][n];	// 紅子
		}
	}

	// 初始化黑棋與紅棋的估值為"0".
	Evalue[0] = Evalue[1] = 0;
}

CEvaluation::~CEvaluation(void)
{
}


int CEvaluation::Evaluation(int Player, int alpha, int beta)
{
	return Evalue[Player] - Evalue[1-Player] + nOffensiveValue;
}
