////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 頭文件︰Search.h                                                                                       //
// *******************************************************************************************************//
// 中國象棋通用引擎----兵河五四，支持《中國象棋通用引擎協議》(Universal Chinese Chess Protocol，簡稱ucci) //
// 作者︰ 范 德 軍                                                                                        //
// 單位︰ 中國原子能科學研究院                                                                            //
// 郵箱︰ fan_de_jun@sina.com.cn                                                                          //
//  QQ ︰ 83021504                                                                                        //
// *******************************************************************************************************//
// 功能︰                                                                                                 //
// 1. 作為CMoveGen的子類，繼承父類的數據，如棋盤、棋子、著法等。                                          //
// 2. 接收界面的數據，初始化為搜索需要的訊息。                                                            //
// 3. 調用CHashTable類，執行和撤銷著法。                                                                  //
// 4. 採用冒泡法對著法排序                                                                                //
// 5. 主控搜索函數 MainSearch()                                                                           //
// 6. 根節點搜索控制 RootSarch()                                                                          //
// 7. 博弈樹搜索算法 AlphaBetaSearch()                                                                    //
// 8. 寂靜搜索算法 QuiescenceSearch()                                                                     //
// 9. 基于1999年版《中國象棋競賽規則》實現循環檢測                                                        //
//10. 循環的返回值問題、與Hash表衝突問題。“兵河五四”進行深入研究，獨樹一幟。                            //
//11. 利用Hash表獲取主分支                                                                                //
//12. 搜索時間的控制                                                                                      //
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "MoveGen.h"
#include "HashTable.h"
#include "Evaluation.h"

#pragma once

// 
const int MaxKiller = 4;
struct CKillerMove
{
	int MoveNum;
	CChessMove MoveList[MaxKiller];
};


// CSearch類
class CSearch : public CMoveGen
{
public:
	CSearch(void);
	~CSearch(void);

// 屬性
	FILE *OutFile;
	CFenBoard fen;

	int Player;								// 當前移動方	
	int MaxDepth;							// 最大搜索深度
	unsigned long BitPieces;				// 32顆棋子的位圖︰0-15為黑棋，16-31為紅棋  0 11 11 11 11 11 11111 0 11 11 11 11 11 11111
	
	CHashTable m_Hash;			// 哈希表
	CEvaluation m_Evalue;

	// 與ucci協議通訊的變量
	int bPruning;							// 是否允許使用NullMove
	int bKnowledge;							// 估值函數的使用
	int nSelectivity;						// 選擇性系數，通常有0,1,2,3四個級別，缺省為0
	int Debug;								// bDegug = ~0, 輸出詳細的搜索訊息
	int SelectMask;							// 選擇性系數
	int nStyle;								// 保守(0)、普通(1)、冒進(2)
	int Ponder;								// 后台思考
	int bStopThinking;						// 停止思考
	int QhMode;								// 引擎是否使用淺紅協議＝否
	int bBatch;								// 和后台思考的時間策略有關
	int StyleShift;
	long nMinTimer, nMaxTimer;				// 引擎思考時間
	int bUseOpeningBook;					// 是否讓引擎使用開局庫
	int nMultiPv;							// 主要變例的數目
	int nBanMoveNum;						// 禁手數目
	CChessMove BanMoveList[111];			// 禁手隊列
	int NaturalBouts;						// 自然限著
	
	
	unsigned int StartTimer, FinishTimer;	// 搜索時間
	unsigned int nNonCapNum;				// 走法隊列未吃子的數目，>=120(60回合)為和棋，>=5可能出現循環
	unsigned int nStartStep;
	unsigned int nCurrentStep;				// 當前移動的記錄序號
	
	unsigned int nPvLineNum;
	CChessMove PvLine[64];					// 主分支路線
	CChessMove StepRecords[256];			// 走法記錄
	unsigned long nZobristBoard[256];
	

	int nFirstLayerMoves;
	CChessMove FirstLayerMoves[111];		// fen C8/3P1P3/3kP1N2/5P3/4N1P2/7R1/1R7/4B3B/3KA4/2C6 r - - 0 1	// 將軍局面    way = 111;
											// fen C8/3P1P3/4k1N2/3P1P3/4N1P2/7R1/1R7/4B3B/3KA4/2C6 r - - 0 1	// 非將軍局面  way = 110;


// 雜項
public:
	unsigned int nTreeNodes;	
	unsigned int nLeafNodes;	
	unsigned int nQuiescNodes;

	unsigned int nTreeHashHit;
	unsigned int nLeafHashHit;

	unsigned int nNullMoveNodes;
	unsigned int nNullMoveCuts;

	unsigned int nHashMoves;
	unsigned int nHashCuts;	

	unsigned int nKillerNodes[MaxKiller];
	unsigned int nKillerCuts[MaxKiller];

	unsigned int nCapCuts;
	unsigned int nCapMoves;

	unsigned int nBetaNodes;
	unsigned int nPvNodes;
	unsigned int nAlphaNodes;

	
	unsigned int nZugzwang;

	//char FenBoard[2048];	// 傳遞棋局的字元串。字元串必須足夠長，否則當回合數太大時，moves超出，與棋盤相互串換時會發生死循環。


// 操作
public:
	void InitBitBoard(const int Player, const int nCurrentStep);		// 初始化棋盤需要的所有數據，返回我方被將軍的標誌。

	int MovePiece(const CChessMove move);	 //&---可以大幅度提升搜索速度，與複製源代碼速度相等，inline不起作用。
	void UndoMove(void);

	void BubbleSortMax(CChessMove *ChessMove, int w, int way);	//冒泡排序︰只需遍歷一次，尋找最大值，返回最後一次交換記錄的位置

	int MainSearch(int nDepth, long nProperTimer=0, long nLimitTimer=0);
	int RootSearch(int depth, int alpha, int beta);
	int AlphaBetaSearch(int depth, int bDoCut, CKillerMove &KillerTab, int alpha, int beta);
	int QuiescenceSearch(int depth, int alpha, int beta);	
	
	int RepetitionDetect(int nRepeatNum=3);					// 循環檢測
	int LoopValue(int Player, int ply, int nLoopStyle);		// 循環估值

	//int Evaluation(int player);

	int IsBanMove(CChessMove move);
	void GetPvLine(void);
	void PopupInfo(int depth, int score, int Debug=0);
	int Interrupt(void);


	char *GetStepName(const CChessMove ChessMove, int *Board) const;
	void SaveMoves(char *FileName);
};
