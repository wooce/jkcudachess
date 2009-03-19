////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 頭文件︰MoveGen.h                                                                                      //
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
#include "FenBoard.h"

#pragma once


// 棋盤數組和棋子數組
extern int Board[256];								// 棋盤數組，表示棋子序號︰0～15，無子; 16～31,黑子; 32～47, 紅子；
extern int Piece[48];								// 棋子數組，表示棋盤位置︰0, 不在棋盤上; 0x33～0xCC, 對應棋盤位置；

// 位行與位列棋盤數組
extern unsigned int xBitBoard[16];					// 16個位行，產生車炮的橫向移動，前12位有效
extern unsigned int yBitBoard[16];					// 16個位列，產生車炮的縱向移動，前13位有效

// 位行與位列棋盤的模
extern const int xBitMask[256];
extern const int yBitMask[256];

// 車炮橫向與縱向移動的16位棋盤，只用于殺手移動合法性檢驗、將軍檢測和將軍逃避   							          
extern unsigned short xBitRookMove[12][512];		//  12288 Bytes, 車的位行棋盤
extern unsigned short yBitRookMove[13][1024];		//  26624 Bytes  車的位列棋盤
extern unsigned short xBitCannonMove[12][512];		//  12288 Bytes  炮的位行棋盤
extern unsigned short yBitCannonMove[13][1024];	    //  26624 Bytes  炮的位列棋盤
									      // Total: //  77824 Bytes =  76K
extern unsigned short HistoryRecord[65535];		// 歷史啟發，數組下標為: move = (nSrc<<8)|nDst;

extern const char nHorseLegTab[512];
extern const char nDirection[512];

class CMoveGen
{
public:
	CMoveGen(void);
	~CMoveGen(void);

	//unsigned short HistoryRecord[65535];		// 歷史啟發，數組下標為: move = (nSrc<<8)|nDst;

// 調試訊息
public:
	unsigned int nCheckCounts;
	unsigned int nNonCheckCounts;
	unsigned int nCheckEvasions;

// 方法
public:	
	// 更新歷史記錄，清零或者衰減
	void UpdateHistoryRecord(unsigned int nMode=0);

	// 通用移動產生器
	int MoveGenerator(const int player, CChessMove* pGenMove);

	// 吃子移動產生器
	int CapMoveGen(const int player, CChessMove* pGenMove);

	// 將軍逃避移動產生器
	int CheckEvasionGen(const int Player, int checkers, CChessMove* pGenMove);
	
	// 殺手移動合法性檢驗
	int IsLegalKillerMove(int Player, const CChessMove KillerMove);

	// 將軍檢測，立即返回是否，用于我方是否被將軍
	int Checked(int player);

	// 將軍檢測，返回將軍類型，用于對方是否被將軍
	int Checking(int Player);

	// 保護判斷
	int Protected(int Player, int from, int nDst);

private:
	// 檢驗棋子piece是否能夠從nSrc移動到nDst，若成功加入到走法隊列ChessMove中
	int AddLegalMove(const int piece, const int nSrc, const int nDst, CChessMove *ChessMove);
};
