////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 頭文件︰HashTable.cpp                                                                                  //
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


#include <stdlib.h>			// 隨機數發生器rand()函數的頭文件
#include <stdio.h>			// 文件操作
#include <time.h>			// 初始化隨機數
#include "HashTable.h"
#include "FenBoard.h"



CHashTable::CHashTable(void)
{
	//srand( (unsigned)time( 0 ) );		// 不使用時，每次都產生相同的局面值，以方便調試
}

CHashTable::~CHashTable(void)
{
}

void CHashTable::ZobristGen(void)
{
	int i, j;

	//ZobristKeyPlayer = Rand32U();

	//((unsigned long *) &ZobristLockPlayer)[0] = Rand32U();
	//((unsigned long *) &ZobristLockPlayer)[1] = Rand32U();

	for (i = 0; i < 14; i ++) 
	{
		for (j = 0; j < 256; j ++) 
		{
			ZobristKeyTable[i][j] = Rand32U();

			((unsigned long *) &ZobristLockTable[i][j])[0] = Rand32U();
			((unsigned long *) &ZobristLockTable[i][j])[1] = Rand32U();
		}
	}
}

//為Hash表分發內存︰以後加入內存檢測，優化大小，不成功則返回0
unsigned long CHashTable::NewHashTable(unsigned long nHashPower, unsigned long nBookPower)
{
	ZobristGen();

	nHashSize = 1<<(nHashPower-1); 
	nHashMask = nHashSize-1;						// 用 & 代替 % 運算

	pHashList[0]  = new CHashRecord[nHashSize];		// 黑方的Hash表
	pHashList[1]  = new CHashRecord[nHashSize];		// 紅方的Hash表
	
	nMaxBookPos = 1<<nBookPower;
	pBookList = new CBookRecord[nMaxBookPos];

	return nHashSize;
}

// 釋放內存
void CHashTable::DeleteHashTable()
{
	// 釋放Hash表的內存
	delete [] pHashList[0];
	delete [] pHashList[1];

	// 釋放開局庫的內存
	delete [] pBookList;
}

// 清空Hash表，並且返回Hash表的覆蓋率
// style==0，完全清除Hash記錄，引擎啟動或者新建棋局時，使用這個參數。
// style!=0，不完全清除，引擎使用(Hash.depth - 2)的策略，基于下面的原理︰
// 1.按回合製，2ply個半回合后，輪到本方走棋。當前搜索樹MaxDepth-2處，存在一顆子樹，就是將要進行完全搜索的樹枝，
// 其餘的樹枝都是無用的。相對于新的搜索而言，那顆子樹少搜索了兩層。把當前的Hash節點深度值減2，正好對應下一回合
// MaxDepth-2的子樹。深度值減2以後，清Hash表之前depth=1和depth-2的的節點將不會命中，但依然能夠為新的搜索樹提供
// HashMove，也將節省一定的時間。根據深度優先覆蓋策略，進行下一回合新的搜索，當迭代深度值depth<MaxDepth-2以前，
// 搜索樹的主分支會直接命中，也不會覆蓋任何Hash節點；之后，由於深度值的增加，將會逐漸覆蓋相同局面的Hash節點。
// 2.若界面程式打開后台思考功能，新的搜索，即使后台思考沒有命中，Hash表中累積的豐富訊息同樣能夠利用。在規定的
// 時間內，引擎可以在此基礎上搜索更深。
// 3.對于殺棋，Hash探察時不採取深度優先策略，清Hash表時，採取完全清除的方法，避免返回值不準確的問題。若使殺棋
// 的局面也能夠利用，value<-MATEVALUE時，value+=2；value>MATEVALUE時，value-=2。若引擎獨自紅黑對戰，則應該±1。
// 為了適應不同的搜索模式，增加安全性，採用保險的措施---完全清除。
// 4.清空Hash表，是個及其耗費時間的工作。所以選擇引擎輸出BestMove后進行，即“后台服務”。
// 5.測試表明，這種清除方案能夠節省10～15%的搜索時間。
float CHashTable::ClearHashTable(int style)
{
	CHashRecord *pHash0 = pHashList[0];
	CHashRecord *pHash1 = pHashList[1];

	CHashRecord *pHashLimit = pHashList[0] + nHashSize;

	nHashCovers = 0;
	if( style )
	{
		while( pHash0 < pHashLimit )
		{
			// 計算Hash表的覆蓋率
			//if(pHash0->flag)
			//	nHashCovers++;
			//if(pHash1->flag)
			//	nHashCovers++;

			// 遇到殺棋，HashFlag置0，表示完全清空
			if( (pHash0->value) < -MATEVALUE || (pHash0->value) > MATEVALUE )
				pHash0->flag = 0;
			if( (pHash1->value) < -MATEVALUE || (pHash1->value) > MATEVALUE )
				pHash1->flag = 0;

			// 深度值減 2，下一回合還可以利用
			(pHash0 ++)->depth -= 2;
			(pHash1 ++)->depth -= 2;
		}
	}
	else
	{
		while( pHash0 < pHashLimit )
		{
			(pHash0 ++)->flag = 0;
			(pHash1 ++)->flag = 0;
		}
	}

	return nHashCovers/(nHashSize*2.0f);
}

unsigned long CHashTable::Rand32U()
{
	//return rand() ^ ((long)rand() << 15) ^ ((long)rand() << 30);
	//許多資料使用上面的隨機數，0<=rand()<=32767, 只能產生0～max(unsigned long)/2之間的隨機數，顯然很不均勻，會增加一倍的衝突幾率

	return ((unsigned long)(rand()+rand())<<16) ^ (unsigned long)(rand()+rand());	//改進后應該是均勻分佈的
}


//根據棋子的位置訊息，初始化ZobristKey和ZobristLock
//當打開一個新的棋局或者載入棋局時，應當調用此函數
void CHashTable::InitZobristPiecesOnBoard(int *Piece)
{
	int m, n, chess;
	
	ZobristKey  = 0;
	ZobristLock = 0;

	for(n=16; n<48; n++)
	{
		m = Piece[n];
		if( m )
		{
			chess = nPieceType[n];

			ZobristKey  ^= ZobristKeyTable[chess][m];
			ZobristLock ^= ZobristLockTable[chess][m];
		}
	}
}

// 探察Hash表，若不成功返回"INT_MIN"；若發生衝突，則返回HashMove，標誌為"INT_MAX"
// 散列法︰
// 1. 相除散列法－－h(x) = ZobristKey % MaxHashZize =ZobristKey & (MaxHashSize-1)  M = MaxHashSize = 1<<k = 1024*1204, k=20 
//    用2的冪表示，可以把除法變成位(與)運算，速度很快。但是這正是散列表的大忌，2^k-1=11111111111111111111(B),僅僅用到了ZobristKey低20位訊息，高12位訊息浪費
//    由於採用了ZobristKey已經時均勻分佈的隨機數，相信相除散列法會工作得很好。唯一的缺點是存在奇偶現象，即h(x) 與 ZobristKey的奇偶相同。
// 2. 平方取中散列法－－h(x)=[M/W((x*x)%W)] = (x*x)>>(w-k) = (x*x)>>(32-20) = (x*x)>>12  前行零x<sqrt(W/M)和尾隨零x=n*sqrt(W)的關鍵字會衝突
//    把x*x右移動12位，剩下左面20位，能夠產生0～M-1之間的數。
// 3. Fibonacci(斐波納契數列)相乘散列法, h(x) = (x*2654435769)>>(32-k)  2654435769是個質數，且2654435769/2^32 = 0.618 黃金分割點，即使連續的鍵值都能均勻分佈
//    2^16  40503
//    2^32  2654435769					倒數︰ 340573321   a%W
//    2^64  11400714819323198485
// CHashRecord *pHashIndex = pHashList[player] + ((ZobristKey * 2654435769)>>(32 - nHashPower));		// Fibonacci(斐波納契數列)相乘散列法
// 經試驗，相除散列法營運時間最快。Fibonacci主要涉及到複雜的乘法和位移運算，故而速度反而不如簡單的方法。
int CHashTable::ProbeHash(CChessMove &hashmove, int alpha, int beta, int nDepth, int ply, int player)
{
	CHashRecord HashIndex = pHashList[player][ZobristKey & nHashMask];								//找到當前棋盤Zobrist對應的Hash表的位址
	
	if( HashIndex.zobristlock == ZobristLock )				//已經存在Hash值, 是同一棋局
	{	
		if( HashIndex.flag & HashExist )
		{
			// 修正將軍局面的Hash值，即使這樣，電腦仍然不能找到最短路線，Hash表命中后的步法失去準確性。
			// 但是距離將軍的分數是正確的，更為可觀的是，殘局時搜索速度快30～40%.
			// 將軍的著法與深度無關，無論在哪層將軍，只要盤面相同，便可以調用Hash表中的值。
			bool bMated = false;
			if( HashIndex.value > MATEVALUE )				// 獲勝局面
			{
				bMated = true;
				HashIndex.value -= ply;						// 減去當前的回合數，即score = WINSCORE-Hash(ply)-ply, 需要更多的步數才能獲勝，如此能夠得到最短的將軍路線
			}
			else if( HashIndex.value < -MATEVALUE )			// 失敗局面
			{
				bMated = true;
				HashIndex.value += ply;						// 加上當前的回合數，即score = Hash(ply)+ply-WINSCORE, 如此電腦會爭取熬更多的回合，頑強抵抗，等待人類棋手走出露著而逃避將軍
			}

			if( HashIndex.depth >= nDepth || bMated )		// 靠近根節點的值更精確，深度優先；將軍局面，不管深度，只要能正確返回距離勝敗的回合數即可
			{
				//配合深度迭代時會出現問題，必須每次都清除Hash表。
				//因為採用深度優先覆蓋，把靠近根節點的值當成最精確的。
				//但是以前的淺層搜索的估值是不精確的，深度優先時應該拋棄這些內容。
				if(HashIndex.flag & HashBeta)			
				{
					if (HashIndex.value >= beta)
					{
						nHashBeta++;					// 95-98%
						return HashIndex.value;
					}
				}
				else if(HashIndex.flag & HashAlpha)		// 也能減少一部分搜索，從而節省時間
				{
					if (HashIndex.value <= alpha)	
					{
						nHashAlpha++;					// 2-5%
						return HashIndex.value;
					}
				}
				else if(HashIndex.flag & HashPv)		// 
				{
					nHashExact++;						// 0.1-1.0%
					return HashIndex.value;
				}

				// 剩下很少量的分支，是一些剛剛展開的分支(跟蹤發現，其數目與HashAlpha相近)，beta=+32767, 上面的的探察會失敗
			}
		}

		// 對于NullMove移動和葉節點，沒有HashMove。
		if( !HashIndex.move )
			return(INT_MIN);

		// Hash表未命中，但局面相同，產生HashMove。
		// 也可能是上一回合遺留下來的移動，清除Hash表時，只是把flag=0，HashMove仍然保留在記錄中。
		// 試驗發現︰以後多次搜索同一局面，時間會越來越少，最終趨于平衡。
		if( HashIndex.depth > 0 )
		{
			nCollision ++;
			hashmove = HashIndex.move;
			return INT_MAX;		// 表示相同局面的陳舊Hash值
		}
	}

	return(INT_MIN);		// 不是相同的棋局
}

void CHashTable::RecordHash(const CChessMove hashmove, int score, int nFlag, int nDepth, int ply, int player)
{
	//找到當前棋盤Zobrist對應的Hash表的位址
	CHashRecord *pHashIndex = pHashList[player] + (ZobristKey & nHashMask);

	// 過濾，深度優先原則
	if((pHashIndex->flag & HashExist) && pHashIndex->depth > nDepth)
		return;

	// 過濾循環局面。循環局面不能寫入Hash表﹗
	//     假如某條路線存在循環，那么搜索樹返回到循環的初始局面時，路線的每個局面都將被存入Hash表，不是贏棋就是輸棋。
	// 再次搜索時，又遇到這條路線上的棋局，但沒有走完整條循環路線，理論上就不構成循環。但程式會從Hash表中發現，走這條
	// 路線會贏棋或者輸棋。如果是輸棋，程式則會被以前的局面嚇倒，絕不敢踏上這條路。實際上，走兩次是可以的，但不能走三次，
	// 因為三次就構成了循環。
	//    殘局時，這種情況屢見不鮮。有些局面，程式走這種路線會贏得勝利；弱勢時可以頑強反抗。
	//    這又涉及到循環的返回值問題。實際上，對于不是和棋的局面，循環的著法從來不是最頑強的應對模式。即使弱勢情況，
	// 不使用循環應對才可以維持得更久。基于這個原理，循環局面可以返回小于-WINSCORE或者大于+WINSCORE的值。返回到搜索樹
	// 的上一層，RecordHash()函數就可以利用這個值來判斷出現了循環的局面。于是不記錄在Hash表中，只是簡單的返回。對后續的
	// 搜索不會造成影響。
	if(score<-WINSCORE || score>+WINSCORE)
		return;

	// 修正將軍局面的Hash值，可以獲得最短的將軍路線。
	if( score > MATEVALUE )
		score += ply;
	else if( score < -MATEVALUE )
		score -= ply;

	// 記錄Hash值
	pHashIndex->zobristlock = ZobristLock;
	pHashIndex->flag    	= (unsigned char)nFlag;
	pHashIndex->depth   	= (char)nDepth;
	pHashIndex->value       = (short)score;
	pHashIndex->move        = (unsigned short)hashmove;		//僅保存后16位的訊息
}

// 開局庫格式︰
// 移動 權重 局面，例b2e2 5895 rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1
int CHashTable::LoadBook(const char *BookFile) 
{
	FILE *FilePtr;
	char *LinePtr;
	char LineStr[256];
	CChessMove  BookMove, temp;
	CHashRecord TempHash;
	CFenBoard fen;

	int Board[256], Piece[48];
	int Player;
	unsigned int nNonCapNum;
	unsigned int nCurrentStep;

	// 下面的兩個臨時變量用來保存舊的棋局，函數結束時用來恢復；
	// 因為整理開局庫時，會改變Hash表中當前的局面，否則引擎不能恢復原來的局面。
	unsigned long    Old_ZobristKey = ZobristKey;
	unsigned __int64 Old_ZobristLock = ZobristLock;

	if(!BookFile)
		return 0;

	// 以只讀文本模式打開開局庫
	if((FilePtr = fopen(BookFile, "rt"))==0)
		return 0;										// 不成功，返回0

	nBookPosNum = 0;
	LineStr[254] = LineStr[255] = '\0';
	
	// 從開局庫中讀取254個字符
	while(fgets(LineStr, 254, FilePtr) != 0)
	{
		// 移動
		LinePtr = LineStr;
		BookMove = Move(*(long *) LinePtr);				// 將移動從字符型轉化為數字型，BookMove低16位有效

		if( BookMove )
		{
			// 權重
			LinePtr += 5;								// 跳過著法的4個字符和1個空格
			temp = 0;
			while(*LinePtr >= '0' && *LinePtr <= '9')
			{
				temp *= 10;
				temp += *LinePtr - '0';
				LinePtr ++;
			}
			
			BookMove |= temp<<16;						// 將此移動的權值(得分)保存在Book的高16位，低16位是移動的著法

			// 局面
			LinePtr ++;														// 跳過空格
			fen.FenToBoard(Board, Piece, Player, nNonCapNum, nCurrentStep, LinePtr);	// 把LinePtr字元串轉化為棋盤數據
			InitZobristPiecesOnBoard( Piece );								// 根據棋盤上的棋子計算ZobristKey和ZobristLock

			if( Board[(BookMove & 0xFF00)>>8] )								// 此位置有棋子存在
			{
				TempHash = pHashList[Player][ZobristKey & nHashMask];
				if(TempHash.flag)											// Hash表中有內容
				{
					if(TempHash.zobristlock == ZobristLock)					// 而且是相同的局面
					{
						if(TempHash.flag & BookUnique)						// 開局庫中是唯一著法
						{
							if(nBookPosNum < nMaxBookPos)					// 沒有超出開局庫的範圍
							{
								TempHash.zobristlock = ZobristLock;
								TempHash.flag = BookMulti;
								TempHash.value = (short)nBookPosNum;

								pBookList[nBookPosNum].MoveNum = 2;
								pBookList[nBookPosNum].MoveList[0] = (TempHash.value<<16) | TempHash.move;
								pBookList[nBookPosNum].MoveList[1] = BookMove;
								
								nBookPosNum ++;
								pHashList[Player][ZobristKey & nHashMask] = TempHash;
							}
						} 
						else															// 開局庫中有兩個以上的變招 
						{
							if(pBookList[TempHash.value].MoveNum < MaxBookMove)
							{
								pBookList[TempHash.value].MoveList[pBookList[TempHash.value].MoveNum] = BookMove;
								pBookList[TempHash.value].MoveNum ++;
							}
						}
					}
				} 
				else					// Hash表中沒有當前的局面，寫入BestMove
				{
					TempHash.zobristlock = ZobristLock;
					TempHash.flag = BookUnique;
					TempHash.move = unsigned short(BookMove & 0xFFFF);
					TempHash.value = unsigned short(BookMove >> 16);
					pHashList[Player][ZobristKey & nHashMask] = TempHash;
				}
			}
		}
	}
	fclose(FilePtr);

	// 恢復棋局，引擎可以接著原來的棋局繼續對弈。
	ZobristKey = Old_ZobristKey;
	ZobristLock = Old_ZobristLock;

	return 1;
}



int CHashTable::ProbeOpeningBook(CChessMove &BookMove, int Player)
{
	CHashRecord TempHash = pHashList[Player][ZobristKey & nHashMask];
	
	if((TempHash.flag & BookExist) && TempHash.zobristlock == ZobristLock)
	{
		if(TempHash.flag & BookUnique)			// 開局庫中存在唯一的著法，命中。
		{
			BookMove = (TempHash.value<<16) | TempHash.move;
			return INT_MAX;
		} 
		else
		{			
			CBookRecord *pBookIndex = pBookList + TempHash.value;

			int m, ThisValue = 0;
			for(m=0; m<pBookIndex->MoveNum; m++)
				ThisValue += (pBookIndex->MoveList[m] & 0xFFFF0000) >> 16;

			if(ThisValue) 
			{
				ThisValue = Rand32U() % ThisValue;
				for(m=0; m<pBookIndex->MoveNum; m++)
				{
					ThisValue -= (pBookIndex->MoveList[m] & 0xFFFF0000) >> 16;

					if( ThisValue < 0 ) 
						break;
				}

				BookMove = pBookIndex->MoveList[m];
				return INT_MAX;
			}
		}
	}

	return 0;
}