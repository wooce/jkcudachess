////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 頭文件︰HashTable.h                                                                                    //
// *******************************************************************************************************//
// 中國象棋通用引擎----兵河五四，支持《中國象棋通用引擎協議》(Universal Chinese Chess Protocol，簡稱ucci) //
// 作者︰ 范 德 軍                                                                                        //
// 單位︰ 中國原子能科學研究院                                                                            //
// 郵箱︰ fan_de_jun@sina.com.cn                                                                          //
//  QQ ︰ 83021504                                                                                        //
// *******************************************************************************************************//
// 功能︰                                                                                                 //
// 1. 為引擎分發Hash表，1～1024MB                                                                         //
// 2. Hash探察 與 Hash存儲                                                                                //
// 3. 初始化開局庫                                                                                        //
// 4. 查找開局庫中的著法                                                                                  //
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "FenBoard.h"

#pragma once


const int WINSCORE = 10000;				// 勝利的分數WINSCORE，失敗為-WINSCORE
const int MATEVALUE = 9000;				// score<-MATEVALUE || score>+MATEVALUE, 說明是個將軍的局面
//const int INT_MIN = -2147483648L;
//const int INT_MAX = 2147483647L;
//const int SHRT_MAX = 32767;

// 常量
const int BookUnique = 1;
const int BookMulti = 2;
const int BookExist = BookUnique | BookMulti;
const int HashAlpha = 4;
const int HashBeta = 8;
const int HashPv = 16;
const int HashExist = HashAlpha | HashBeta | HashPv;
const int MaxBookMove = 15;


// 開局庫著法架構
struct CBookRecord
{
	int MoveNum;
	CChessMove MoveList[MaxBookMove];
};

// 哈西表記錄架構
struct CHashRecord					// 14Byte --> 16Byte
{
	unsigned __int64   zobristlock;	// 8 byte  64位標識
	unsigned char      flag;        // 1 byte  flag==0, Hash值被清除
	char			   depth;		// 1 byte  搜索深度
	short			   value;       // 2 byte  估值
	unsigned short     move;		// 2 byte
};


// Hash表類
class CHashTable
{
public:
	CHashTable(void);
	virtual ~CHashTable(void);

// 
public:
	//unsigned long    ZobristKeyPlayer;
	//unsigned __int64 ZobristLockPlayer;

	unsigned long    ZobristKey;
	unsigned __int64 ZobristLock;

	unsigned long    ZobristKeyTable[14][256];
	unsigned __int64 ZobristLockTable[14][256];

	unsigned long  nHashSize;			// Hash表的實際大小
	unsigned long  nHashMask;			// nHashMask = nHashSize-1;
	CHashRecord    *pHashList[2];		// Hash表，黑紅雙方各用一個，以免發生衝突，這樣解決最徹底
										// 參考王曉春著《PC遊戲編程----人機博弈.pdf》

	unsigned int   nMaxBookPos;
	unsigned int   nBookPosNum;
	CBookRecord    *pBookList;


// 調試訊息
public:
	unsigned long nCollision;	//Hash衝突計數器
	unsigned long nHashAlpha, nHashExact, nHashBeta;
	unsigned long nHashCovers;


public:
	// 為Hash表分發和開局庫分發內存
	unsigned long NewHashTable(unsigned long nHashPower, unsigned long nBookPower);

	// 清除Hash表和開局庫
	void DeleteHashTable();

	// Hash表清零，並且返回Hash表的覆蓋率
	float ClearHashTable(int style=0);

	// 初始化與Hash表相關的棋子數據
	void InitZobristPiecesOnBoard(int *Piece);

	// Hash探察
	int ProbeHash(CChessMove &hashmove, int alpha, int beta, int nDepth, int ply, int player);

	// Hash存儲
	void RecordHash(const CChessMove hashmove, int score, int nFlag, int nDepth, int ply, int player);
	
	// 初始化開局庫
	int LoadBook(const char *BookFile);

	// 在開局庫中尋找著法
	int ProbeOpeningBook(CChessMove &BookMove, int Player);

private:
	// 32位隨機數發生器
	unsigned long Rand32U();

	// 初始化ZobristKeyTable[14][256]和ZobristLockTable[14][256]，賦予32位和64位隨即數
	// 只需一次，在CHashTable()構造函數中自動進行，無需引擎調用
	// 除非重新啟動程式，否則隨即數將不會改變
	void ZobristGen(void);
};
