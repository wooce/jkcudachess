////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 源文件︰Search.cpp                                                                                     //
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


#include <time.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "ucci.h"
#include "Search.h"

static const unsigned int nAdaptive_R = 3;
static const unsigned int nVerified_R = 3;
static const unsigned int BusyCounterMask = 4095;
static const unsigned int BitAttackPieces = 0xF87EF87E;    //11111000011111101111100001111110 兵兵兵兵兵士士象象馬馬炮炮車車將卒卒卒卒卒士士相相馬馬炮炮車車帥


CSearch::CSearch(void)
{	
	// 初始化空著
	StepRecords[0] = 0;	
	
	bPruning = 1 ;							// 允許使用NullMove
	nSelectivity = 0;						// 選擇性系數，通常有0,1,2,3四個級別
	Debug = 0;								// bDegug = 0, 輸出簡明扼要的搜索訊息
	SelectMask = 0;
	nStyle = 1;
	Ponder = 0;
	bStopThinking = 0;
	QhMode = 0;
	bBatch = 0;
	StyleShift = 5;
	bUseOpeningBook = 0;
	NaturalBouts = 120;
	nBanMoveNum = 0;

	// 初始化Hash表，分發21+1=22級Hash表，64MB
	m_Hash.NewHashTable(22, 12);
	m_Hash.ClearHashTable();

	// 初始化歷史表
	UpdateHistoryRecord( 0 );
}

CSearch::~CSearch(void)
{
	m_Hash.DeleteHashTable();
}

// 判斷移動是否為禁著
int CSearch::IsBanMove(CChessMove move)
{
	int m;
	for(m=0; m<nBanMoveNum; m++)
	{
		if( move == BanMoveList[m] )
			return true;
	}
	return 0;
}

//深度迭代搜索
int CSearch::MainSearch(int nDepth, long nProperTimer, long nLimitTimer)
{
	// 初始化一些有用的變量
	int w, MoveStr, score=0;
	nPvLineNum = PvLine[0] = 0;
	
	// 這些變量用于測試搜索樹性能的各種參數
	nNullMoveNodes = nNullMoveCuts = 0;
	nHashMoves = nHashCuts = 0;	
	nAlphaNodes = nPvNodes = nBetaNodes = 0;
	nTreeNodes = nLeafNodes = nQuiescNodes = 0;
	m_Hash.nCollision = m_Hash.nHashAlpha = m_Hash.nHashExact = m_Hash.nHashBeta = 0;
	nTreeHashHit = nLeafHashHit = 0;
	nCheckEvasions = 0;
	nZugzwang = 0;	
	nCapCuts = nCapMoves = 0;
	for(w=0; w<MaxKiller; w++)
		nKillerCuts[w] = nKillerNodes[w] = 0;
	nCheckCounts = nNonCheckCounts = 0;
	// 這些變量用于測試搜索樹性能的各種參數


	//一、分發搜索時間
	StartTimer = clock();
	nMinTimer = StartTimer + unsigned int(nProperTimer*0.618f);
	nMaxTimer = unsigned int(nProperTimer*1.618f);
	if(nMaxTimer > nLimitTimer)
		nMaxTimer = nLimitTimer;
	nMaxTimer += StartTimer;
	bStopThinking = 0;


	//二、輸出當前局面
	fen.BoardToFen(Board, Player, nNonCapNum, nCurrentStep, StepRecords);
	fprintf(OutFile, "info BoardToFen: %s\n", fen.FenStr);
	fflush(OutFile);


	//三、在開局庫中進行搜索
	CChessMove BookMove;
	if(bUseOpeningBook && m_Hash.ProbeOpeningBook(BookMove, Player))
	{
		nPvLineNum = 1;
		PvLine[0] = BookMove;
		score = BookMove >> 16;
		MoveStr = Coord( BookMove );

		fprintf(OutFile, "info depth 0 score %d pv %.4s\n", score, &MoveStr);
		fflush(OutFile);
		fprintf(OutFile, "bestmove %.4s\n", &MoveStr);
		fflush(OutFile);

		return score;
	}


	//四、若對方被將軍，返回吃將帥的走法、終止搜索。
	// 主搜索例程中有將軍檢測，不會出現這樣的局面，主要針對特殊局面。
	if( w = Checked(1-Player) )
	{
		score = WINSCORE;
		MoveStr = Coord( (w << 8) | Piece[(2-Player)<<4] );
		fprintf(OutFile, "info depth 0 score %d pv %.4s\n", score, &MoveStr);
		fflush(OutFile);
		fprintf(OutFile, "nobestmove\n");
		fflush(OutFile);

		return score;
	}


	//五、迭代加深搜索
	nStartStep = nCurrentStep;		// 開始搜索時的半回合數。

	for(w=1; w<=nDepth; w++)
	{	
		MaxDepth = w;
		nFirstLayerMoves = 0;

		// alpha-beta搜索
		score = RootSearch(MaxDepth, -WINSCORE, WINSCORE);

		// 無法解將或者只有一個合理的應著，立即停止搜索
		if( nFirstLayerMoves <= 1 )
			break;

		// 若強行終止思考，停止搜索
		if(bStopThinking)
			break;

		// 無后台思考時，若時間已經達到規定時間的一半，再搜索一層的時間可能不夠，停止搜索。
		if(!Ponder && clock()>nMinTimer)
			break;

		// 在規定的深度內，遇到殺棋，停止思考。
		if(score<-MATEVALUE || score>MATEVALUE)
			break;
	}
		

	//六、返回搜索結果
	// 若有合法的移動，輸出 bestmove %.4s 和 ponder %.4s  以及詳細的搜索訊息。
	PopupInfo(MaxDepth, score, 1);
	if( nPvLineNum )
	{
		MoveStr = Coord(PvLine[0]);
		fprintf(OutFile, "bestmove %.4s", &MoveStr);
		if(Ponder && nPvLineNum>1)
		{
			MoveStr = Coord(PvLine[1]);
			fprintf(OutFile, " ponder %.4s", &MoveStr);
		}
		fprintf(OutFile, "\n");
		fflush(OutFile);
	}
	// 出現循環，不存在合法的移動，返回score。意味著結束遊戲。
	else
	{
		fprintf(OutFile, "depth %d score %d\n", MaxDepth, score);
		fflush(OutFile);
		fprintf(OutFile, "nobestmove\n");
		fflush(OutFile);
	}
	//fprintf(OutFile, "\n\n");
	//fflush(OutFile);


	//七、清除Hash表和歷史啟發表
	StartTimer = clock() - StartTimer;
	m_Hash.ClearHashTable( 2 );
	SaveMoves("SearchInfo.txt");	
	UpdateHistoryRecord( 4 );
	nBanMoveNum = 0;

	return(score);
}

// 第一層的搜索算法
int CSearch::RootSearch(int depth, int alpha, int beta)
{
	nTreeNodes ++;	// 統計樹枝節點	

	int score;
	int w, way, nCaptured;	
		
	const int  ply = nCurrentStep - nStartStep;			                    // 獲取當前的回合數
        const unsigned int nNonCaptured = nNonCapNum;
	const int nChecked = (StepRecords[nCurrentStep-1] & 0xFF000000) >> 24;	// 從走法隊列(全局變量)中獲取(我方)將軍標誌
	
	CChessMove ThisMove, BestMove = 0;									// 初始化為空著。若形成困閉，可以返回此空著。
	CChessMove HashMove = 0;												// HashMove
	CKillerMove SubKillerTab;
	SubKillerTab.MoveNum = 0;

	int HashFlag = HashAlpha;												// Hash標誌
	int nBestValue = ply-WINSCORE;											// 搜索窗口返回的極小極大值
	nFirstLayerMoves = 0;
	
	//一、和棋裁剪︰敵我雙方均不存在任何能夠過河攻擊的棋子
	if(!(BitPieces & BitAttackPieces))
		return m_Evalue.Evaluation(Player);

	
	//二、Hash探察，depth=127，故而永遠不會命中(除了將軍局面)，只是為了得到HashMove。	
	int nHashValue = m_Hash.ProbeHash(HashMove, alpha, beta, 127, ply, Player);

	
	//三、HashMove啟發算法
	if(nHashValue == INT_MAX && !IsBanMove(HashMove) )
	{
		nHashMoves ++;
		ThisMove = HashMove;
			
		assert(Board[ThisMove & 0xFF]!=16 && Board[ThisMove & 0xFF]!=32);			// 不應該出現，否則有錯誤，透過將軍檢測，提前一步過濾調這類走法
		MovePiece( ThisMove );
			if(Checked(1-Player))													// 我方被將軍，下一步對方走，無法挽救，返回失敗的分數.免除一層搜索
				score = ply+1-WINSCORE;												// 對方勝利，我方走了一步臭棋。如雙王照面等，多是自己送上門的將軍(應該統計一下，這樣的移動歷史得分應該很低，如何減少無效的運算)
			else
			{
				StepRecords[nCurrentStep-1] |= Checking(Player) << 24;				// 對方被將軍，傳送到下一層
				score = -AlphaBetaSearch(depth-1, 0, SubKillerTab, -beta, -alpha);	// HashMove不允許使用“空著裁減”或“歷史裁減”
			}
		UndoMove();					// 恢復移動，恢復移動方，恢復一切
		nNonCapNum = nNonCaptured;	// 恢復原來的無殺子棋步數目
		
		// Fail-Soft Alpha-Beta MainSearch（Hash和MTDf必須的方法）
		// 採用全窗口搜索時，第一層永遠不會產生beta截斷
		if( score >= beta )				// Beta剪枝
		{
			nHashCuts ++;

			if( HistoryRecord[ThisMove] > 64000 )		// 流出餘地︰65535-64000=1535
				UpdateHistoryRecord(1);
			HistoryRecord[ThisMove] += depth*depth;

			m_Hash.RecordHash(ThisMove, score, HashBeta, depth, ply, Player);
			return score;
		}

		if(score > nBestValue)
		{
			nBestValue = score;
			BestMove = ThisMove;

			if( score > alpha )				// 縮小窗口。
			{
				alpha = score;
				HashFlag = HashPv;

				// 記錄第一層的走法
				m_Hash.RecordHash(BestMove, nBestValue, HashFlag, depth, ply, Player);
				PopupInfo(depth, score);
			}
		}		
		
		// 記錄第一層的所有著法
		if( score < -MATEVALUE )
			BanMoveList[ nBanMoveNum ++ ] = ThisMove;							// 禁止著法
		else
			FirstLayerMoves[ nFirstLayerMoves ++ ] = ThisMove | (score<<16);	// 合理著法
	}

	
	//四、產生所有合法的移動
	//1.將軍局面  ︰產生將軍逃避著法；
	//2.非將軍局面︰吃子著法和非吃子著法，吃子著法附加歷史得分，全部按歷史啟發處理。
	CChessMove ChessMove[111];
	if( nChecked )
		way = CheckEvasionGen(Player, nChecked, ChessMove);					// 產生逃避將軍的著法
	else
	{
		way  = CapMoveGen(Player, ChessMove);								// 產生所有的吃子移動
		for(w=0; w<way; w++)
			ChessMove[w] += HistoryRecord[ChessMove[w] & 0xFFFF] << 16;		// 吃子著法 + 歷史啟發
		way += MoveGenerator(Player, ChessMove+way);						// 產生所有非吃子移動
	}

	
	//五、Alpha-Beta搜索算法
	// 1. 冒泡排序挑選最大值(Bubble Sort for Max Value)
	// 2. 過濾HashMove
	// 3. 主分支搜索(PVS)(Principal Variation Search)
	// 4. Fail-Soft Alpha-Beta Search
	int nChecking;
	for(w=0; w<way; w++)
	{
		BubbleSortMax(ChessMove, w, way);
		ThisMove = ChessMove[w] & 0xFFFF;

		// 過濾HashMove和禁止著法
		if( ThisMove==HashMove || IsBanMove(ThisMove) )
			continue;
		
		assert(Board[ThisMove & 0xFF]!=16 && Board[ThisMove & 0xFF]!=32);		// 不應該出現，否則有錯誤，透過將軍檢測，提前一步過濾調這類走法
		nCaptured = MovePiece( ThisMove );										// 注意︰移動后Player已經表示對方，下面的判斷不要出錯。儘管這樣很別扭，但在其它地方很方便，根本不用管了
			if(Checked(1-Player))												// 我方被將軍，下一步對方走，無法挽救，返回失敗的分數.免除一層搜索
				score = ply+1-WINSCORE;											// 對方勝利，我方走了一步臭棋。如雙王照面等，多是自己送上門的將軍(應該統計一下，這樣的移動歷史得分應該很低，如何減少無效的運算)
			else
			{
				nChecking = Checking(Player) << 24;								// 對方被將軍，傳送到下一層
				StepRecords[nCurrentStep-1] |= nChecking;

				// 歷史剪枝(History Alpha-Beta Pruning)
				if(    nFirstLayerMoves >= 12      		//a. HistoryMoves==6	// 6為了保險
					&& !nChecked						//b. 我方被將軍時，不允許使用History Pruning
				 	&& !nChecking						//c. 對方將軍時，不允許使用History Pruning，稍慢，但更聰明，往往能提前幾步發現殺棋。
					&& !nCaptured						//d. 非吃子移動
				  )
				{
					score = -AlphaBetaSearch(depth-2, bPruning, SubKillerTab, -alpha-1, -alpha);	// 用depth-2進行搜索，允許使用帶風險的剪枝
					if( score > alpha )	
						score = -AlphaBetaSearch(depth-1, 0, SubKillerTab, -beta, -alpha);			// 用depth-1驗証alpha，不允許“空著裁減”或“歷史裁減”
				}
				// 以下禁止使用帶風險的剪枝
				else if( nFirstLayerMoves )
				{
					score = -AlphaBetaSearch(depth-1, bPruning, SubKillerTab, -alpha-1, -alpha);
					if( score>alpha ) //&& score<beta)	
						score = -AlphaBetaSearch(depth-1, 0, SubKillerTab, -beta, -alpha);
				}
				else
					score = -AlphaBetaSearch(depth-1, 0, SubKillerTab, -beta, -alpha);	
			}			
		UndoMove();					// 恢復移動，恢復移動方，恢復一切
		nNonCapNum = nNonCaptured;	// 恢復原來的無殺子棋步數目
		
		
		//********************************************************************************************************
		//Fail-Soft Alpha-Beta MainSearch（Hash和MTDf必須的方法）
		
		if(score > nBestValue)
		{
			nBestValue = score;
			BestMove = ThisMove;

			if( score >= beta )				// Beta剪枝
			{
				nBetaNodes++;
				HashFlag = HashBeta;				
				break;
			}
			if( score > alpha )				// 縮小窗口。
			{
				nPvNodes ++;
				alpha = score;
				HashFlag = HashPv;

				m_Hash.RecordHash(BestMove, nBestValue, HashFlag, depth, ply, Player);
				PopupInfo(depth, score);
			}
			else
				nAlphaNodes ++;
		}
		

		// 記錄第一層的所有著法
		if( score < -MATEVALUE )
			BanMoveList[ nBanMoveNum ++ ] = ThisMove;							// 禁止著法
		else
			FirstLayerMoves[ nFirstLayerMoves ++ ] = ThisMove | (score<<16);	// 合理著法

		// 若沒有啟動后台思考功能，引擎達到了最小搜索時間，多搜索一層已經不可能，引擎發出終止搜索的信號
		if(nFirstLayerMoves && !Ponder && clock() > nMinTimer)
		{
			bStopThinking = 1;
			break;
		}
	}


	//十二、歷史啟發、殺手啟發、記錄Hash表

	if(HashFlag != HashAlpha)
	{
		if( HistoryRecord[BestMove] > 64000 )		// 流出餘地︰65535-64000=1535
			UpdateHistoryRecord(1);
		HistoryRecord[BestMove] += depth*depth;
	}
	
	m_Hash.RecordHash( BestMove, nBestValue, HashFlag, depth, ply, Player );
	return nBestValue;
}


int CSearch::AlphaBetaSearch(int depth, int bDoCut, CKillerMove &KillerTab, int alpha, int beta)
{
	const int  ply = nCurrentStep - nStartStep;			// 獲取當前的回合數
	int score, nBestValue = ply-WINSCORE;				// 搜索窗口返回的極小極大值
        int NextKiller, bHistoryPruning;
        unsigned int ThereIsKillers;

	
	//一、循環判斷
	//1.移動方︰這裡判斷上一步以前是否構成循環，最後的移動是對方。
	//2.回合數︰既然判斷上一步，自然ply = ply-1。不注意這個細節返回值將不會精確。
	//3.技  巧︰nNonCapNum=0,1,2,3時，不可能構成循環，如此大大減少了循環判斷的次數。
	//4.與Hash表衝突︰
	if( nNonCapNum>=4 )	
	{
		score = RepetitionDetect();
		if(score)
			return LoopValue(Player, ply, score);
	}

	//二、殺棋裁剪︰Mate-Distance Pruning
	if(nBestValue >= beta)
		return nBestValue;


	//三、和棋裁剪︰敵我雙方均不存在任何能夠過河攻擊的棋子
	if(!(BitPieces & BitAttackPieces))
		return m_Evalue.Evaluation(Player);


	//四、將軍擴展 & 兌子擴展
	const unsigned int nChecked = (StepRecords[nCurrentStep-1] & 0xFF000000) >> 24;	 // 從走法隊列(全局變量)中獲取(我方)將軍標誌
	int Extended = 0;
	if( nChecked && ply<MaxDepth+16)								 // 將軍擴展，多搜索一層，直到沒有將軍為止
	{
		depth++;
		Extended = 1;
	}
	// 兌子擴展，連續兩步殺子，僅限于車炮馬兌殺。
	// 對子擴展可能會引起一些局面無法退出。
	// 經8小時的檢查發現，原因不在這裡，是由於棋盤字元串FenBoard的數組長度不夠，而陷入死循環。
	else if(bDoCut && nNonCapNum==0 && ply<=MaxDepth+4)
	{
		score = nPieceID[ (StepRecords[nCurrentStep-1] & 0xFF0000)>>16 ];
		if(score>=1 && score<=3)		// 子粒價值為︰車炮馬
		{
			score = nPieceID[ (StepRecords[nCurrentStep-2] & 0xFF0000)>>16 ];
			if(score>=1 && score<=3)	// 子粒價值為︰車炮馬
			{
				depth++;
				Extended = 1;
			}
		}
	}

	
	//五、Hash探察，如果Hash表命中，直接返回估值。能夠極大提升剪枝效率，(不衝突時)副作用極小。
	CChessMove HashMove = 0;
	int nHashValue = m_Hash.ProbeHash(HashMove, alpha, beta, depth, ply, Player);
	if( nHashValue!=INT_MIN && nHashValue!=INT_MAX )	//Hash命中
	{
		if(depth>0)
		{
			nTreeHashHit ++;
			nTreeNodes ++;
		}
		else
		{
			nLeafHashHit ++;
			nLeafNodes ++;
		}
		return nHashValue;
	}


	//六、葉節點放入Hash表，速度快16～20%。若不這樣作，把這段去掉，並把開始的5行語句的註釋去掉。
	//    經試驗，對于簡單的估值函數，這樣作足夠了，若再使用Futility剪枝，會起反作用。由於複雜的判斷，反而使速度稍微降低一些。Razor起到的作用很有限。
	//    曾經嘗試，葉節點不進行散列，Rador和Futility卻是起到了相當大的作用。這兩種剪枝都會不同程度降低解出機率。
	//    今后若使用複雜的估值函數，可以考慮使用Rador和Futility剪枝。
	if( depth <= 0 )
	{
		score = QuiescenceSearch(0, alpha, beta);

		// 記錄Hash值，注意葉節點並沒有真正的移動，插入空著。
		// 為了保守，避免搜索的不穩定性，應分別存儲alpha,beta,score.
		// 全部存儲score, 帶來更高的Hash表命中率和剪枝機率，速度快3.5%。
		// 由於寂靜搜索很不精確，為了安全，還應進行保守的存入。
		if(score >= beta)
			m_Hash.RecordHash(0,  beta, HashBeta,  0, ply, Player);		// score=beta
		else if(score <= alpha)
			m_Hash.RecordHash(0, alpha, HashAlpha, 0, ply, Player);		// score=alpha		
		else
			m_Hash.RecordHash(0, score, HashPv, 0, ply, Player);		// score=score

		return score;
	}

	nTreeNodes ++;	// 統計樹枝節點

	//七、引擎中斷搜索︰返回-32768，上次的移動保證不寫入Hash表，也不會成為Pv節點。
	// 停止思考，強製出著。
	if( bStopThinking )
		return -SHRT_MAX;
	// 4095個節點查看一次，后台思考是否命中
	if(!(nTreeNodes & BusyCounterMask)) 
	{
		if( Interrupt() ) 
			return -SHRT_MAX;
	}


	// 初始化一些有用的變量
	//const unsigned int bInEndGame = ( Count32( BitPieces & (Player ? 0x7E0000:0x7E)) < 3 ) ? 1 : 0;		// 移動方車馬炮數量少于3個，表示移動方進入殘局
	const unsigned int nNonCaptured = nNonCapNum;
	CKillerMove SubKillerTab;
	SubKillerTab.MoveNum = 0;
			
	
	//七、空著裁減︰Null-Move Pruning
	// 1. Adaptive  Null-Move Pruning   開局中局		適應性空著裁剪
	// 2. Verified  Null-Move Pruning	殘    局		帶校驗空著裁剪
	if( 
		   bDoCut        									// 上層是非PV節點，允許本層使用NullMove；若上層是PV節點，本層將禁止使用NullMove
		//&& depth >= 2	    								// depth==1,無法校驗，不使用NullMove,進行正常的搜索
		&& !nChecked      									// 移動方沒有被將軍
		&& BitPieces & (Player ? 0xF87E0000:0xF87E)			// 移動方至少存在一顆夠過河攻擊的棋子，減小Zugzwang發生幾率。
		&& beta>-MATEVALUE && beta<MATEVALUE 				// 非將軍的局面（Fruit的作法）  // 2b1k4/5c3/9/9/9/9/3h2P2/9/8R/5K3 r - - 0 1 去掉限制后，此局面20層25秒得到正解。個別其它局面，往往需要多搜一層。估值不是很精確，即不是最短路徑的殺棋，從實戰角度講，還是去掉的好。
	  )
	{
		// 比較好的局面才使用，速度稍稍快些，應該更安全，空著剪枝率上升20%。
		// 按照NullMove理論來說，我很強壯，已經到了不用走棋的地步。不強壯的局面，往往得不到剪枝。
		// 加上此限定條件后，開局與中局幾乎無需校驗，而不會產生Zugzwang
		// 空著裁剪時，移動方失去先手權，故而用-nOffensiveValue，而不是+nOffensiveValue
		if( Evalue[Player]-Evalue[1-Player]-nOffensiveValue >= beta )
		{
			nNullMoveNodes ++;
			
			// MakeNullMove()
			StepRecords[nCurrentStep] = 0;				// 當前的移動為空著，即我方不走棋
			nCurrentStep ++;
			nNonCapNum = 0;								// 對方連續走棋，不可能構成循環，如此可以把循環判斷次數減少一半
			Player = 1 - Player;						// 改變對手，讓對方連續走兩步棋，中間隔一空著

			// bPruning==0，下一層將不允許使用NullMove，除非再次遇到非PV結點時，才能打開這個開關。
			// 連續兩次使用NullMove是非常危險的，深藍甚至限制一條搜索路線只使用一次NullMove。
			score = -AlphaBetaSearch(depth-nAdaptive_R-1, 0, SubKillerTab, -beta, 1-beta);
			
			// UndoNullMove()
			Player = 1 - Player;						// 還原對手，無需徹著
			nCurrentStep --;
			nNonCapNum = nNonCaptured;					// 恢復原來的數據

			
			if(score >= beta)							// 仿效深藍的做法，加上一個保險閾值︰score >= beta+delta
			{
				// 開局與中局，不進行校驗，因為有Evaluation(Player)>=beta作為保障，阻斷了絕大多數Zugzwang
				if( Count32( BitPieces & (Player ? 0x7E0000:0x7E)) >= 3 )			  // 非殘局︰車馬炮的數量大于等于3個
				{
					nNullMoveCuts++;

					// 未經證實的將軍，返回非殺棋的分數，可以進行剪枝，防止寫入Hash表時被改寫。
					if( score > MATEVALUE )				
						score -= WINSCORE - MATEVALUE;
				
					return score;
				}
				else									// 殘局必須進行校驗
				{
					if(nStyle==2)	// 冒進
						score = AlphaBetaSearch(depth-nVerified_R, 0, KillerTab, beta-1, beta);							// depth-5    進行驗証
					else            // 保守 || 普通
						score = AlphaBetaSearch(depth<=nVerified_R ? 1:depth-nVerified_R, 0, KillerTab, beta-1, beta);	// depth-5～1 進行驗証
										
					if(score >= beta)
					{
						nNullMoveCuts ++;
						return score;
					}
					
					// 若校驗不成功，發現了一個Zugswang，還原深度值，繼續以後的搜索。
					// 這裡可以統計Zugswang出現的幾率，開局與中局較少，攔截后更是鳳毛麟角；殘局的確不容忽視。
					nZugzwang ++;
				}				
			}
			
			//if( !Extended && score==ply+3-WINSCORE && ply<MaxDepth+6 )	// 將軍恐嚇，延伸一層
			//{
			//	depth ++;
			//	Extended = 1;
			//}
		}
	}



	// 初始化一些有用的變量
	int d, w, way, nCaptured;
	int HashFlag = HashAlpha;					// Hash標誌
	int nSearched = 0;
	CChessMove ThisMove;
	CChessMove BestMove = 0;				    // 初始化為空著。若形成困閉，可以返回此空著。
	
	

	//八、內部深度迭代︰ Internal Iterative Deepening if no HashMove found
	// 如果Hash表未命中，使用depth-2進行內部深度迭代，目的是為了獲取HashMove
	// 感覺速度快10～15%，有的局面並不顯得快，但也不會慢。
	// 殘局能夠增加Hash表的命中率，加快解體速度。即提前發現將軍的局面。
	if(    nHashValue == INT_MIN		// Hash表未命中
		&& !bDoCut						// 是個PV節點，非PV節點傳過來的bDoCut==1
		&& depth >= 3 )					// depth=2和depth=1時，沒有必要，直接進行正常搜索就可以了。
	{
		score = AlphaBetaSearch(depth-2, 0, KillerTab, alpha, beta);	// 不使用帶風險的裁剪

		CHashRecord *pHashIndex = m_Hash.pHashList[Player] + (m_Hash.ZobristKey & m_Hash.nHashMask);		//找到當前棋盤Zobrist對應的Hash表的位址
		if((pHashIndex->flag & HashExist) && pHashIndex->zobristlock==m_Hash.ZobristLock)
		{
			if( pHashIndex->move )
			{
				HashMove = pHashIndex->move;
				nHashValue = INT_MAX;				
			}			
		}
	}


	//九、HashMove啟發算法  利用hash表中同一棋局但是未命中的訊息
	if( nHashValue == INT_MAX )
	{
		nHashMoves ++;
		ThisMove = HashMove;
			
		assert(Board[ThisMove & 0xFF]!=16 && Board[ThisMove & 0xFF]!=32);		// 不應該出現，否則有錯誤，透過將軍檢測，提前一步過濾調這類走法
		MovePiece( ThisMove );
			if(Checked(1-Player))												// 我方被將軍，下一步對方走，無法挽救，返回失敗的分數.免除一層搜索
				score = ply+1-WINSCORE;											// 對方勝利，我方走了一步臭棋。如雙王照面等，多是自己送上門的將軍(應該統計一下，這樣的移動歷史得分應該很低，如何減少無效的運算)
			else
			{
				StepRecords[nCurrentStep-1] |= Checking(Player) << 24;					// 對方被將軍，傳送到下一層
				score = -AlphaBetaSearch(depth-1, 0, SubKillerTab, -beta, -alpha);		// HashMove一般是主分支，下一層不允許帶風險的剪枝
			}
		UndoMove();	
		nNonCapNum = nNonCaptured;	// 恢復原來的無殺子棋步數目
		
		//Fail-Soft Alpha-Beta MainSearch（Hash和MTDf必須的方法）
		if(score > nBestValue)
		{
			nBestValue = score;
			BestMove = ThisMove;

			if( score >= beta )				// Beta剪枝
			{
				nHashCuts ++;
				HashFlag = HashBeta;
				goto CUT;
			}

			if( score > alpha )				// 縮小窗口。
			{
				alpha = score;
				HashFlag = HashPv;
			}
		}

		nSearched ++;
	}


	//十、殺手啟發︰Killer Moves
	//1.西洋棋的殺手數目為2；中國象棋殺手的數目2-4個比較合適，太多速度變慢。
	//2.Killer Moves採用先進先出FIFO策略。使用歷史啟發效果並不明顯，殺手倒序有時也很快。
	//3.如果被將軍，不使用殺手啟發，因為有將軍逃避函數，殺手不一定能夠解將。
	//4.殺手移動中若包含HashMove，進行過濾
        ThereIsKillers = 0;        // 殺手位圖
	if( !nChecked )
	{
		for( w=0; w<KillerTab.MoveNum; w++ )
		{
			// 過濾HashMove和不合法的殺手移動
			ThisMove = KillerTab.MoveList[w];
			if( ThisMove==HashMove || !IsLegalKillerMove(Player, ThisMove) )
				continue;

			// 統計殺手的數目
			nKillerNodes[w]++;
			ThereIsKillers ^= 1<<w;		// 更新殺手位圖
				
			assert(Board[ThisMove & 0xFF]!=16 && Board[ThisMove & 0xFF]!=32);		// 不應該出現，否則有錯誤，透過將軍檢測，提前一步過濾調這類走法
			MovePiece( ThisMove );		
				if(Checked(1-Player))					// 我方被將軍，下一步對方走，無法挽救，返回失敗的分數.免除一層搜索
					score = ply+1-WINSCORE;				// 對方勝利，我方走了一步臭棋。如雙王照面等，多是自己送上門的將軍。
				else
				{
					StepRecords[nCurrentStep-1] |= Checking(Player) << 24;		// 對方被將軍，傳送到下一層
					
					score = -AlphaBetaSearch(depth-1, bPruning, SubKillerTab, -beta, 1-beta);
					if(score>alpha && score<beta)
						score = -AlphaBetaSearch(depth-1, 0, SubKillerTab, -beta, -alpha);
				}
			UndoMove();					// 恢復移動，恢復移動方，恢復一切
			nNonCapNum = nNonCaptured;	// 恢復原來的無殺子棋步數目
			

			//Fail-Soft Alpha-Beta MainSearch（Hash和MTDf必須的方法）
			

			if(score > nBestValue)
			{
				nBestValue = score;
				BestMove = ThisMove;

				if( score >= beta )				// Beta剪枝
				{
					nKillerCuts[w] ++;
					HashFlag = HashBeta;
					goto CUT;
				}

				if( score > alpha )				// 縮小窗口。
				{
					alpha = score;
					HashFlag = HashPv;
				}
			}
			
			nSearched ++;	
		}
	}


	//十一、吃子移動︰CaptureMove
	CChessMove ChessMove[111];
        NextKiller = ThereIsKillers & 1 ? 0 : 1;    //奇數從0開始找，偶數至少從1開始找。
	if( !nChecked )
	{
		way = CapMoveGen(Player, ChessMove);					// 產生所有的移動
		for(w=0; w<way; w++)
		{
			BubbleSortMax(ChessMove, w, way);
			ThisMove = ChessMove[w] & 0xFFFF;

			// 過濾HashMove和殺手移動
			if( ThisMove==HashMove )
				continue;

			// 如果有殺手存在，過濾殺手移動。如果所有的殺手都已經過濾，ThereIsKillers = 0;
			if( ThereIsKillers )
			{
				for(d=NextKiller; d<KillerTab.MoveNum; d++)
				{
					if( ThisMove==KillerTab.MoveList[d] )
					{
						if(d==NextKiller)
							NextKiller = d+1;		// 下一個殺手的位置
						ThereIsKillers ^= 1<<d;		// 從ThereIsKillers標誌中刪除這個殺手
						d = -1;						// d < 0, 說明當前的移動是個殺手
						break;
					}
				}
				if( d<0 )							// 當前的移動曾經是個殺手，跳過它，無需再次進行搜索
					continue;
			}

			// 統計吃子移動的數目
			nCapMoves ++;
			
			assert(Board[ThisMove & 0xFF]!=16 && Board[ThisMove & 0xFF]!=32);		// 不應該出現，否則有錯誤，透過將軍檢測，提前一步過濾調這類走法
			nCaptured = MovePiece( ThisMove );										// 注意︰移動后Player已經表示對方，下面的判斷不要出錯。儘管這樣很別扭，但在其它地方很方便，根本不用管了
				if(Checked(1-Player))												// 我方被將軍，下一步對方走，無法挽救，返回失敗的分數.免除一層搜索
					score = ply+1-WINSCORE;											// 對方勝利，我方走了一步臭棋。如雙王照面等，多是自己送上門的將軍(應該統計一下，這樣的移動歷史得分應該很低，如何減少無效的運算)
				else
				{
					StepRecords[nCurrentStep-1] |= Checking(Player) << 24;			// 對方被將軍，傳送到下一層

					if( nSearched )
					{
						score = -AlphaBetaSearch(depth-1, bPruning, SubKillerTab, -alpha-1, -alpha);
						if(score>alpha && score<beta)
							score = -AlphaBetaSearch(depth-1, 0, SubKillerTab, -beta, -alpha);
					}
					else
						score = -AlphaBetaSearch(depth-1, 0, SubKillerTab, -beta, -alpha);	

				}
			UndoMove();					// 恢復移動，恢復移動方，恢復一切
			nNonCapNum = nNonCaptured;	// 恢復原來的無殺子棋步數目
			
			
			//********************************************************************************************************
			//Fail-Soft Alpha-Beta MainSearch（Hash和MTDf必須的方法）
			if(score > nBestValue)
			{
				nBestValue = score;
				BestMove = ThisMove;
				
				if( score >= beta )				// Beta剪枝。吃子移動的剪枝率不很高，故而寫在裡面
				{
					nCapCuts ++;
					HashFlag = HashBeta;
					goto CUT;
				}

				if( score > alpha )				// 縮小窗口。
				{
					alpha = score;
					HashFlag = HashPv;
				}
			}
			
			nSearched ++;
		}
	}
	

	//十二、靜態移動︰QuiesceMove

	if( nChecked )
	{
		way = CheckEvasionGen(Player, nChecked, ChessMove);		// 產生解將的著法。由於使用了將軍擴展，將軍逃避將很大程度上提升搜索效率。
		
		// 不能解將，無需搜索，直接返回估值。
		if( !way )
		{
			nAlphaNodes ++;
			score = ply+1-WINSCORE;								// 下一步老將被對方吃掉。
			m_Hash.RecordHash( 0, score, HashFlag, depth, ply, Player );
			return score;
		}
		for(w=0; w<way; w++)
		{
			ThisMove     = ChessMove[w] & 0xFFFF;
			ChessMove[w] = (HistoryRecord[ThisMove]<<16) | ThisMove;
		}
	}
	else
		way = MoveGenerator(Player, ChessMove);					// 產生所有的移動
	

	
	//十一、非吃子移動的歷史啟發搜索算法
	// 1. 冒泡排序挑選最大值(Bubble Sort for Max Value)
	// 2. 過濾HashMove與KillerMove
	// 3. 主分支搜索(PVS)(Principal Variation Search)
	// 4. 歷史剪枝(History Pruning)
	// 5. Fail-Soft Alpha-Beta Search
	
	int nChecking;
        bHistoryPruning = (alpha+1==beta && nStyle )|| (bDoCut && nStyle==0);

	for(w=0; w<way; w++)
	{
		if( w < 6 )
			BubbleSortMax(ChessMove, w, way);
		ThisMove = ChessMove[w] & 0xFFFF;

		if( ThisMove==HashMove )
			continue;
		// 如果有殺手存在，過濾殺手移動。如果所有的殺手都已經過濾，ThereIsKillers = 0;
		if( ThereIsKillers )
		{
			for(d=NextKiller; d<KillerTab.MoveNum; d++)
			{
				if( ThisMove==KillerTab.MoveList[d] )
				{
					if(d==NextKiller)
						NextKiller = d+1;		// 下一個殺手的位置
					ThereIsKillers ^= 1<<d;		// 從ThereIsKillers標誌中刪除這個殺手
					d = -1;						// d < 0, 說明當前的移動是個殺手
					break;
				}
			}
			if( d<0 )							// 當前的移動曾經是個殺手，跳過它，無需再次進行搜索
				continue;
		}
		
		MovePiece( ThisMove );										// 注意︰移動后Player已經表示對方，下面的判斷不要出錯。儘管這樣很別扭，但在其它地方很方便，根本不用管了
			if( Checked(1-Player) )												// 我方被將軍，下一步對方走，無法挽救，返回失敗的分數.免除一層搜索
				score = ply+1-WINSCORE;											// 對方勝利，我方走了一步臭棋。如雙王照面等，多是自己送上門的將軍(應該統計一下，這樣的移動歷史得分應該很低，如何減少無效的運算)
			else
			{
				nChecking = Checking(Player) << 24;								// 對方被將軍，傳送到下一層
				StepRecords[nCurrentStep-1] |= nChecking;

				// 歷史剪枝(History Alpha-Beta Pruning)
				if(	    bHistoryPruning		//a. 根據界面傳送的下棋風格以及當前局面的類型決定是否使用History Pruning
					 && nSearched >= 3		//b. HistoryMoves==3  西洋棋與中國象棋通用的常數
					 && depth>=3		    //c. HistoryDepth==3  depth-2>=1，保證與NullMove不衝突。假若上層使用NullMove，本層校驗時，需試驗全部著法，防止Zugswang。
					 && !nChecked			//d. 我方被將軍時，不允許使用History Pruning
				 	 && !nChecking			//e. 對方被將軍時，不允許使用History Pruning
				  )
				{
					score = -AlphaBetaSearch(depth-2, bPruning, SubKillerTab, -alpha-1, -alpha);	// 用depth-2進行搜索
					if( score > alpha )																// 若alpha+1==beta，score>=beta
						score = -AlphaBetaSearch(depth-1, 0, SubKillerTab, -beta, -alpha);			// 用depth-1驗証alpha
				}
				// 非標準的PVS搜索(Principal Variation Search)，標準的PVS搜索需要找到score>alpha，即bFoundPV=true
				// 以下是不能進行歷史裁減的分支，
				else if( nSearched )
				{
					score = -AlphaBetaSearch(depth-1, bPruning, SubKillerTab, -alpha-1, -alpha);
					if(score>alpha && score<beta)
						score = -AlphaBetaSearch(depth-1, 0, SubKillerTab, -beta, -alpha);
				}
				else //if( !nSearched )
					score = -AlphaBetaSearch(depth-1, 0, SubKillerTab, -beta, -alpha);
			}
		UndoMove();					// 恢復移動，恢復移動方，恢復一切
		nNonCapNum = nNonCaptured;	// 恢復原來的無殺子棋步數目
		
		//********************************************************************************************************
		//Fail-Soft Alpha-Beta MainSearch（Hash和MTDf必須的方法）
		if(score > nBestValue)
		{
			nBestValue = score;
			BestMove = ThisMove;

			if( score >= beta )				// Beta剪枝，經驗証99%以上的beta節點產生于w<6
			{
				if(w<4)
				nBetaNodes++;
				HashFlag = HashBeta;				
				break;
			}
			if( score > alpha )				// 縮小窗口。
			{
				nPvNodes ++;
				alpha = score;
				HashFlag = HashPv;
			}
			else
				nAlphaNodes ++;
		}
		else
			nAlphaNodes ++;
		nSearched ++;
	}


	//十二、歷史啟發、殺手啟發、記錄Hash表
CUT:
	if(HashFlag != HashAlpha)
	{
		if( HistoryRecord[BestMove] > 64000 )		// 流出餘地︰65535-64000=1535
			UpdateHistoryRecord(1);
		HistoryRecord[BestMove] += depth*depth;

		// 記錄殺手移動
		if(KillerTab.MoveNum < MaxKiller) 
		{
			// 去掉重複的殺手，速度大致快10%
			for(d=0; d<KillerTab.MoveNum; d++)
			{
				if(BestMove==KillerTab.MoveList[d])
				{
					d = MaxKiller;
					break;
				}
			}
			if(d<MaxKiller)
			{
				KillerTab.MoveList[KillerTab.MoveNum] = BestMove;
				KillerTab.MoveNum ++;
			}
		}
	}
	
	// 置換表中存儲的FailLow修正算法︰縱馬奔流(涂志堅)---電腦象棋的設計與實現
	if( nBestValue<alpha && nHashValue==INT_MAX )	// FailLow情況出現(nBestValue<alpha); 置換表中有存儲HashMove(nHashValue==INT_MAX)
		BestMove = HashMove;						// Best為原來置換表中的HashMove，而不改變原來的結果
	m_Hash.RecordHash( BestMove, nBestValue, HashFlag, depth, ply, Player );

	return nBestValue;
}


// 寂靜搜索(能夠明顯提升棋力，無法避免有露算的可能)。主要技術︰
// 1. 循環檢測----否則會出現長將；
// 2. 空著探測----不被將軍時，比較不走棋好還是殺子更好；
// 3. 吃子移動產生器----CapMoveGen()；
// 4. 解將移動產生器----CheckEvasionGen()；
// 5. 冒泡排序尋找最大值
// 6. 位棋盤快速將軍檢測；
// 7. Fail Soft Alpha-Beta搜索。
int CSearch::QuiescenceSearch(int depth, int alpha, int beta)
{
	int score;
	const int ply = nCurrentStep - nStartStep;
	const int nChecked = (StepRecords[nCurrentStep-1] & 0xFF000000) >> 24;
	int nBestValue = ply-WINSCORE;		//搜索窗口返回的極小極大值

	if( depth < 0 )
	{
		// 統計寂靜搜索的次數
		nQuiescNodes++;
		
		//一、循環判斷
		//1.移動方︰這裡判斷上一步以前是否構成循環，根據負值極大原理，必須返回負值。	
		//2.回合數︰既然判斷上一步，自然ply = ply-1。不注意這個細節返回值將不會精確。
		//2.技巧 1︰若前面的條件失敗，程式不進行后面的判斷，nNonCapNum=0,1,2,3時，不可能構成循環，如此大大減少了循環判斷的次數。
		//3.技巧 2︰depth==0，已經在上層搜索時進行了判斷，無需重複。
		if( nNonCapNum>=4 )
		{
			score = RepetitionDetect();
			if(score)
				return LoopValue(Player, ply, score);
		}

		// 殺棋裁剪
		if(nBestValue>=beta)
			return nBestValue;

		// 和棋裁剪︰敵我雙方不存在任何能夠過河攻擊的棋子
		if(!(BitPieces & BitAttackPieces))
			return m_Evalue.Evaluation(Player);

		/*
		//三、Hash探察 (寂靜搜索的命中率極低)
		CChessMove HashMove;
		score = m_Hash.ProbeHash(HashMove, alpha, beta, depth, ply, Player);
		if(score!=INT_MIN && score!=INT_MAX)	//Hash命中
		{
			nLeafHashHit ++;
			return score;
		}
		*/
	}
	else
		nLeafNodes ++;


	//二、若非將軍的局面，首先進行Null-Move測試。
	// 目的是為了比較吃子好還是不吃子好，如果不吃子的分數已經能夠剪枝，吃子后也必然剪枝。
	if(!nChecked)
	{
		score = m_Evalue.Evaluation(Player, alpha, beta);
		if(score >= beta)
			return score;
		if(score>nBestValue)
		{
			nBestValue = score;
			if(score > alpha)
				alpha = score;
		}
	}

	int w, way;
	const unsigned int nNonCaptured = nNonCapNum;
	
	//fen.BoardToFen(Board, Player, nNonCapNum, nCurrentStep, StepRecords, fen.FenStr);

	
	//三、產生吃子的移動以及解將的移動
	CChessMove ChessMove[32];					// 使用了解將移動產生器后，數組可以設得小些，32就足夠了。Max=19

	// 試驗過一些棋局，殘局不進行將軍擴展時，電腦經常走出昏著而丟子。
	if( nChecked )
	{
		way = CheckEvasionGen(Player, nChecked, ChessMove);		// 我方被將軍，展開所有的走法，應該先產生解將的步法
		if(!way)
			return ply+1-MATEVALUE;								// 因為寂靜搜索並不精確，若無解，返回非殺棋分數，防止改寫Hash表，但可以進行剪枝。
	}
	else
		way = CapMoveGen(Player, ChessMove);					// 產生所有的吃子著法


	CChessMove ThisMove;

	//四、Soft Alpha-Beta搜索
	for(w=0; w<way; w++)
	{	
		BubbleSortMax(ChessMove, w, way);
		ThisMove = ChessMove[w] & 0xFFFF;

		assert(Board[ThisMove & 0xFF]!=16 && Board[ThisMove & 0xFF]!=32);		// 不應該出現，否則有錯誤，透過將軍檢測，提前一步過濾調這類走法
		MovePiece( ThisMove );

		// 勝負檢驗: verify game		
		if(Checked(1-Player))					// 我方被將軍，下一步對方走，無法挽救，返回失敗的分數.免除一層搜索
			score = ply+1-MATEVALUE;		    // 對方勝利，由於寂靜搜索中的殺棋並非精確，故而使用MATEVALUE，不使用WINSCORE。
		else								    // 這種策略意味著不把此局面當作殺棋，以免返回時對Hash表造成影響，但又不會影響剪枝。
		{
			StepRecords[nCurrentStep-1] |= Checking(Player) << 24;
			score = -QuiescenceSearch(depth-1, -beta, -alpha);
		}

		UndoMove();					// 恢復移動，恢復移動方，恢復一切
		nNonCapNum = nNonCaptured;	// 恢復原來的無殺子棋步數目


		if(score >= beta)
			return score;
		if(score>nBestValue)
		{
			nBestValue = score;			
			if(score > alpha)
				alpha = score;
		}
	}

	return nBestValue;
}


void CSearch::BubbleSortMax(CChessMove *ChessMove, int w, int way)
{
	int i;
	unsigned int temp;

	for(i=way-1; i>w; i--)
	{
		if(ChessMove[i-1] < ChessMove[i])
		{
			temp = ChessMove[i-1];
			ChessMove[i-1] = ChessMove[i];
			ChessMove[i] = temp;
		}
	}
}


int CSearch::MovePiece(const CChessMove move)
{	
	int nSrc = (move & 0xFF00) >> 8;
	int nDst = move & 0xFF;
	int nMovedChs = Board[nSrc];
	int nCaptured = Board[nDst];
	int nMovedPiece = nPieceType[nMovedChs];

	assert( nMovedChs>=16 && nMovedChs<48 );
	assert( nCaptured>=0 && nCaptured<48 );
	

	//更新棋盤
	Board[nSrc] = 0;
	Board[nDst] = nMovedChs;


	m_Hash.ZobristKey  ^= m_Hash.ZobristKeyTable[nMovedPiece][nSrc] ^ m_Hash.ZobristKeyTable[nMovedPiece][nDst];
	m_Hash.ZobristLock ^= m_Hash.ZobristLockTable[nMovedPiece][nSrc] ^ m_Hash.ZobristLockTable[nMovedPiece][nDst];

	
	//更新棋子坐標
	Piece[nMovedChs] = nDst;
	Evalue[Player] += PositionValue[nMovedPiece][nDst] - PositionValue[nMovedPiece][nSrc];	// 更新估值
	

	if( nCaptured )
	{
		nNonCapNum = 0;
		Piece[nCaptured] = 0;
		BitPieces ^= 1<<(nCaptured-16);

		int nKilledPiece = nPieceType[nCaptured];
		
		m_Hash.ZobristKey  ^= m_Hash.ZobristKeyTable[nKilledPiece][nDst];
		m_Hash.ZobristLock ^= m_Hash.ZobristLockTable[nKilledPiece][nDst];

		Evalue[1-Player] -= PositionValue[nKilledPiece][nDst] + BasicValues[nKilledPiece];
	}
	else
		nNonCapNum ++;				// 雙方無殺子的半回合數


	// 擦除起點nSrc的位行與位列，置“0”
	xBitBoard[ nSrc >> 4 ]  ^= xBitMask[nSrc];
	yBitBoard[ nSrc & 0xF ] ^= yBitMask[nSrc];	

	// 更新終點nDst的位行與位列，置“1”
	xBitBoard[ nDst >> 4 ]  |= xBitMask[nDst];
	yBitBoard[ nDst & 0xF ] |= yBitMask[nDst];

	
	//記錄當前局面的ZobristKey，用于循環探測、將軍檢測
	StepRecords[nCurrentStep] = move  | (nCaptured<<16);
	nZobristBoard[nCurrentStep] = m_Hash.ZobristKey;		// 當前局面的索引
	nCurrentStep++;


	Player = 1 - Player;		// 改變移動方
	//m_Hash.ZobristKey ^= m_Hash.ZobristKeyPlayer;
    //m_Hash.ZobristLock ^= m_Hash.ZobristLockPlayer;
	
	return(nCaptured);
}

void CSearch::UndoMove(void)
{
	CChessMove move = StepRecords[nCurrentStep-1];
	int nSrc = (move & 0xFF00) >> 8;;
	int nDst = move & 0xFF;
	int nMovedChs = Board[nDst];
	int nMovedPiece = nPieceType[nMovedChs];
	int nCaptured = (move & 0xFF0000) >> 16;


	// 首先恢復移動方
	Player = 1 - Player;		
	//m_Hash.ZobristKey ^= m_Hash.ZobristKeyPlayer;
    //m_Hash.ZobristLock ^= m_Hash.ZobristLockPlayer;

	m_Hash.ZobristKey  ^= m_Hash.ZobristKeyTable[nMovedPiece][nSrc] ^ m_Hash.ZobristKeyTable[nMovedPiece][nDst];
	m_Hash.ZobristLock ^= m_Hash.ZobristLockTable[nMovedPiece][nSrc] ^ m_Hash.ZobristLockTable[nMovedPiece][nDst];

	//更新棋盤與棋子
	Board[nSrc] = nMovedChs;
	Board[nDst] = nCaptured;
	Piece[nMovedChs] = nSrc;
	Evalue[Player] -= PositionValue[nMovedPiece][nDst] - PositionValue[nMovedPiece][nSrc];	// 更新估值


	// 恢復位行與位列的起始位置nSrc，使用“|”操作符置“1”
	xBitBoard[ nSrc >> 4 ]  |= xBitMask[nSrc];
	yBitBoard[ nSrc & 0xF ] |= yBitMask[nSrc];
	
	if( nCaptured )							//吃子移動
	{
		int nKilledPiece = nPieceType[nCaptured];

		Piece[nCaptured] = nDst;			//恢復被殺棋子的位置。
		BitPieces ^= 1<<(nCaptured-16);

		m_Hash.ZobristKey  ^= m_Hash.ZobristKeyTable[nKilledPiece][nDst];
		m_Hash.ZobristLock ^= m_Hash.ZobristLockTable[nKilledPiece][nDst];

		Evalue[1-Player] += PositionValue[nKilledPiece][nDst] + BasicValues[nKilledPiece];
	}
	else									//若是非吃子移動，必須把俘獲位置置"0"
	{
		// 清除位行與位列的起始位置nDst，使用“^”操作符置“0”
		// 反之，若是吃子移動，終點位置本來就是"1"，所以不用恢復。
		xBitBoard[ nDst >> 4 ]  ^= xBitMask[nDst];
		yBitBoard[ nDst & 0xF ] ^= yBitMask[nDst];
	}

	//清除走法隊列
	nCurrentStep --;
	nNonCapNum --;
}

// 根據棋子位置訊息，初始化所有棋盤與棋子的數據
// 也可使用棋盤訊息，需循環256次，速度稍慢。
void CSearch::InitBitBoard(const int Player, const int nCurrentStep)
{
	int m,n,x,y;	
	int chess;
	
	// 初始化，清零
	BitPieces = 0;
	Evalue[0] = Evalue[1] = 0;
	m_Hash.ZobristKey  = 0;
	m_Hash.ZobristLock = 0;
	for(x=0; x<16; x++)
		xBitBoard[x] = 0;
	for(y=0; y<16; y++)
		yBitBoard[y] = 0;
	
	// 根據32顆棋子位置更新
	for(n=16; n<48; n++)
	{
		m = Piece[n];													// 棋子位置
		if( m )															// 在棋盤上
		{
			chess               = nPieceType[n];						// 棋子類型︰0～14
			BitPieces          |= 1<<(n-16);							// 32顆棋子位圖
			xBitBoard[m >> 4 ] |= xBitMask[m];							// 位行
			yBitBoard[m & 0xF] |= yBitMask[m];							// 位列
			m_Hash.ZobristKey  ^= m_Hash.ZobristKeyTable[chess][m];		// Hash鍵
			m_Hash.ZobristLock ^= m_Hash.ZobristLockTable[chess][m];	// Hash鎖
			
			if(n!=16 && n!=32)			
				Evalue[ (n-16)>>4 ] += PositionValue[chess][m] + BasicValues[chess];
		}
	}

	//m_Hash.InitZobristPiecesOnBoard( Piece );

	// 用于循環檢測
	nZobristBoard[nCurrentStep-1] =  m_Hash.ZobristKey;

	// 當前移動方是否被將軍，寫入走法隊列
	StepRecords[nCurrentStep-1] &= 0xFF000000;
	StepRecords[nCurrentStep-1] |= Checking(Player) << 24;
}

//還應參照棋規作更詳細的判斷，為了通用，最好不使用CString類
char *CSearch::GetStepName(CChessMove ChessMove, int *Board) const
{
	//棋子編號
	static char StepName[12];	// 必須用靜態變量，否則不能返回

	static const char ChessName[14][4] = {"  ","  ","炮","馬","象","士","卒", "  ","  ","炮","  ","相","仕","兵"};

	static const char PostionName[2][16][4] = { {"", "", "", "１","２","３","４","５","６","７","８","９", "", "", "", ""}, 
	                                            {"", "", "", "九","八","七","六","五","四","三","二","一", "", "", "", ""} };

	const int nSrc = (ChessMove & 0xFF00) >> 8;
	const int nDst = ChessMove & 0xFF;

	if( !ChessMove )
		return("HashMove");

	const int nMovedChs = Board[nSrc];
	const int x0 = nSrc & 0xF;
	const int y0 = nSrc >> 4;
	const int x1 = nDst & 0xF;
	const int y1 = nDst >> 4;

	const int Player = (nMovedChs-16) >> 4;	

	strcpy( StepName, ChessName[nPieceType[nMovedChs]] );
	strcat( StepName, PostionName[Player][x0] );
	
	//檢查此列x0是否存在另一顆成對的棋子.
	int y,chess;
	if( nPieceID[nMovedChs]!=0 && nPieceID[nMovedChs]!=4 && nPieceID[nMovedChs]!=5 )	// 將、士、象不用區分
	{
		for(y=3; y<13; y++)
		{
			chess = Board[ (y<<4) | x0 ];

			if( !chess || y==y0)														// 無子或者同一顆棋子，繼續搜索
				continue;

			if( nPieceType[nMovedChs] == nPieceType[chess] )							// 遇到另一個相同的棋子
			{
				if( !Player )			// 黑子
				{
					if(y > y0)
						strcpy( StepName, "前" );
					else
						strcpy( StepName, "后" );
				}
				else					// 紅子
				{
					if(y < y0)
						strcpy( StepName, "前" );
					else
						strcpy( StepName, "后" );
				}

				strcat( StepName, ChessName[nPieceType[nMovedChs]] );
				break;
			}
		}
	}

	int piece = nPieceID[nMovedChs];

	//進, 退, 平
	if(y0==y1)
	{
		strcat( StepName, "平" );
		strcat( StepName, PostionName[Player][x1]);					// 平，任何棋子都以絕對位置表示
	}
	else if((!Player && y1>y0) || (Player && y1<y0))
	{
		strcat( StepName, "進" );

		if(piece==3 || piece==4 || piece==5)						// 馬、象、士用絕對位置表示
			strcat( StepName, PostionName[Player][x1] );			
		else if(Player)												// 將、車、炮、兵用相對位置表示
			strcat( StepName, PostionName[1][y1-y0+12] );			// 紅方
		else
			strcat( StepName, PostionName[0][y1-y0+2] );			// 黑方
	}
	else
	{
		strcat( StepName, "退" );

		if(piece==3 || piece==4 || piece==5)						// 馬、象、士用絕對位置表示
			strcat( StepName, PostionName[Player][x1] );			
		else if(Player)												// 將、車、炮、兵用相對位置表示
			strcat( StepName, PostionName[1][y0-y1+12] );			// 紅方
		else
			strcat( StepName, PostionName[0][y0-y1+2] );			// 黑方		
	}

	return(StepName);
}



// 獲得主分支
// 由於使用了Hash表，有時主分支是錯誤的，有待修正﹗？？？
void CSearch::GetPvLine(void)
{
	CHashRecord *pHashIndex = m_Hash.pHashList[Player] + (m_Hash.ZobristKey & m_Hash.nHashMask);		//找到當前棋盤Zobrist對應的Hash表的位址

	if((pHashIndex->flag & HashExist) && pHashIndex->zobristlock==m_Hash.ZobristLock)
	{
		if( pHashIndex->move )
		{
			PvLine[nPvLineNum] = pHashIndex->move;
			
			MovePiece( PvLine[nPvLineNum] );

			nPvLineNum++;

			if( nNonCapNum<4 || !RepetitionDetect() )
				GetPvLine();

			UndoMove();
		}
	}
}

void CSearch::PopupInfo(int depth, int score, int Debug)
{
	unsigned int n;
	int MoveStr;
	if(depth)
	{
		fprintf(OutFile, "info depth %d score %d pv", depth, score);
		
		n = nNonCapNum;
		nPvLineNum = 0;
		GetPvLine();
		nNonCapNum = n;

		for(n=0; n<nPvLineNum; n++) 
		{
			MoveStr = Coord(PvLine[n]);
			fprintf(OutFile, " %.4s", &MoveStr);
		}

		fprintf(OutFile, "\n");
		fflush(OutFile);
	}

	if(Debug)
	{
		n = nTreeNodes + nLeafNodes + nQuiescNodes;
		fprintf(OutFile, "info Nodes %d = %d(T) + %d(L) + %d(Q)\n", n, nTreeNodes, nLeafNodes, nQuiescNodes);
		fflush(OutFile);

		float SearchTime = (clock() - StartTimer)/(float)CLOCKS_PER_SEC;
		fprintf(OutFile, "info TimeSpan = %.3f s\n", SearchTime);
		fflush(OutFile);

		fprintf(OutFile, "info NPS = %d\n", int(n/SearchTime));
		fflush(OutFile);
	}	
}


void CSearch::SaveMoves(char *szFileName)
{
	unsigned int m, n;
	int k, nSrc, nDst, nCaptured;
	
	// 創建文件，並刪除原來的內容。利用這種格式化輸出比較方便。
	FILE *out = fopen(szFileName, "w+");

	fprintf(out, "***************************搜索訊息***************************\n\n");

	fprintf(out, "搜索深度︰%d\n", MaxDepth);
	n = nTreeNodes + nLeafNodes + nQuiescNodes;
	fprintf(out, "TreeNodes : %u\n", n);
	fprintf(out, "TreeStruct: BranchNodes = %10u\n", nTreeNodes);
	fprintf(out, "            LeafNodes   = %10u\n", nLeafNodes);
	fprintf(out, "            QuiescNodes = %10u\n\n", nQuiescNodes);

	float TimeSpan = StartTimer/1000.0f;
	fprintf(out, "搜索時間    :   %8.3f 秒\n", TimeSpan);
	fprintf(out, "枝葉搜索速度:   %8.0f NPS\n", (nTreeNodes+nLeafNodes)/TimeSpan);
	fprintf(out, "整體搜索速度:   %8.0f NPS\n\n", n/TimeSpan);

	fprintf(out, "Hash表大小: %d Bytes  =  %d M\n", m_Hash.nHashSize*2*sizeof(CHashRecord), m_Hash.nHashSize*2*sizeof(CHashRecord)/1024/1024);
	fprintf(out, "Hash覆蓋率: %d / %d = %.2f%%\n\n", m_Hash.nHashCovers, m_Hash.nHashSize*2, m_Hash.nHashCovers/float(m_Hash.nHashSize*2.0f)*100.0f);

	unsigned int nHashHits = m_Hash.nHashAlpha+m_Hash.nHashExact+m_Hash.nHashBeta;
	fprintf(out, "Hash命中: %d = %d(alpha:%.2f%%) + %d(exact:%.2f%%) +%d(beta:%.2f%%)\n", nHashHits, m_Hash.nHashAlpha, m_Hash.nHashAlpha/(float)nHashHits*100.0f, m_Hash.nHashExact, m_Hash.nHashExact/(float)nHashHits*100.0f, m_Hash.nHashBeta, m_Hash.nHashBeta/(float)nHashHits*100.0f);
	fprintf(out, "命中機率: %.2f%%\n", nHashHits/float(nTreeNodes+nLeafNodes)*100.0f);
	fprintf(out, "樹枝命中: %d / %d = %.2f%%\n", nTreeHashHit, nTreeNodes, nTreeHashHit/(float)nTreeNodes*100.0f);
	fprintf(out, "葉子命中: %d / %d = %.2f%%\n\n", nLeafHashHit, nLeafNodes, nLeafHashHit/(float)nLeafNodes*100.0f);

	fprintf(out, "NullMoveCuts   = %u\n", nNullMoveCuts);
	fprintf(out, "NullMoveNodes  = %u\n", nNullMoveNodes);
	fprintf(out, "NullMove剪枝率 = %.2f%%\n\n", nNullMoveCuts/(float)nNullMoveNodes*100.0f);

	fprintf(out, "Hash衝突   : %d\n", m_Hash.nCollision);
	fprintf(out, "Null&Kill  : %d\n", m_Hash.nCollision-nHashMoves);
	fprintf(out, "HashMoves  : %d\n", nHashMoves);
	fprintf(out, "HashCuts   : %d\n", nHashCuts);
	fprintf(out, "Hash剪枝率 : %.2f%%\n\n", nHashCuts/(float)nHashMoves*100.0f);

	fprintf(out, "殺手移動 : \n");
	k = n = 0;
	for(m=0; m<MaxKiller; m++)
	{
		fprintf(out, "    Killer   %d : %8d /%8d = %.2f%%\n", m+1, nKillerCuts[m], nKillerNodes[m], nKillerCuts[m]/float(nKillerNodes[m]+0.001f)*100.0f);
		n += nKillerCuts[m];
		k += nKillerNodes[m];
	}
	fprintf(out, "    殺手剪枝率 : %8d /%8d = %.2f%%\n\n", n, k, n/float(k+0.001f)*100.0f);


	fprintf(out, "吃子移動剪枝率 = %d / %d = %.2f%%\n\n", nCapCuts, nCapMoves, nCapCuts/(float)nCapMoves*100.0f);


	m = nBetaNodes + nPvNodes + nAlphaNodes;
	fprintf(out, "非吃子移動: %d\n", m);	
	fprintf(out, "    BetaNodes: %10d  %4.2f%%\n", nBetaNodes, nBetaNodes/float(m)*100.0f);
	fprintf(out, "    PvNodes  : %10d  %4.2f%%\n", nPvNodes, nPvNodes/float(m)*100.0f);
	fprintf(out, "    AlphaNode: %10d  %4.2f%%\n\n", nAlphaNodes, nAlphaNodes/float(m)*100.0f);

	m += nNullMoveCuts + nHashMoves + nKillerNodes[0] + nKillerNodes[1] + nCapMoves;
	fprintf(out, "TotalTreeNodes: %d\n\n\n", m);

	n = nCheckCounts-nNonCheckCounts;
	fprintf(out, "將軍次數: %d\n", n);
	fprintf(out, "探測次數: %d\n", nCheckCounts);
	fprintf(out, "成功機率: %.2f%%\n\n", n/(float)nCheckCounts*100.0f);

	fprintf(out, "CheckEvasions = %d\n", nCheckEvasions);
	fprintf(out, "解將 / 將軍   = %d / %d = %.2f%%\n\n", nCheckEvasions, n, nCheckEvasions/float(n)*100.0f);


	// 顯示主分支
	int BoardStep[256];
	for(n=0; n<256; n++)
		BoardStep[n] = Board[n];

	static const char ChessName[14][4] = {"  ","  ","炮","馬","象","士","卒", "  ","  ","炮","  ","相","仕","兵"};

	fprintf(out, "\n主分支︰PVLine***HashDepth**************************************\n");
	for(m=0; m<nPvLineNum; m++)
	{
		nSrc = (PvLine[m] & 0xFF00) >> 8;
		nDst = PvLine[m] & 0xFF;
		nCaptured = BoardStep[nDst];

		// 回合數與棋步名稱
		fprintf(out, "    %2d. %s", m+1, GetStepName( PvLine[m], BoardStep ));

		// 吃子著法
		if( nCaptured )
			fprintf(out, " k-%s", ChessName[nPieceType[nCaptured]]);
		else
			fprintf(out, "     ");

		// 搜索深度
		fprintf(out, "  depth = %2d", PvLine[m]>>16);

		// 將軍標誌
		nCaptured = (PvLine[m] & 0xFF0000) >> 16;
		if(nCaptured)
			fprintf(out, "   Check Extended 1 ply ");
		fprintf(out, "\n");

		BoardStep[nDst] = BoardStep[nSrc];
		BoardStep[nSrc] = 0;
	}

	fprintf(out, "\n\n***********************第%2d 回合********************************\n\n", (nCurrentStep+1)/2);
	fprintf(out, "***********************引擎生成︰%d 個理合著法**********************\n\n", nFirstLayerMoves);
	for(m=0; m<(unsigned int)nFirstLayerMoves; m++)
	{
		nSrc = (FirstLayerMoves[m] & 0xFF00) >> 8;
		nDst = FirstLayerMoves[m] & 0xFF;

		// 尋找主分支
		if(PvLine[0] == FirstLayerMoves[m])
		{
			fprintf(out, "*PVLINE=%d***********Nodes******History**************************\n", m+1);
			fprintf(out, "*%2d.  ", m+1);
		}
		else
			fprintf(out, "%3d.  ", m+1);

		//n = m==0 ? FirstLayerMoves[m].key : FirstLayerMoves[m].key-FirstLayerMoves[m-1].key;	// 統計分支數目
		n = FirstLayerMoves[m] >> 16;																// 統計估值
		fprintf(out, "%s = %6d    hs = %6d\n", GetStepName(FirstLayerMoves[m], Board), n, HistoryRecord[FirstLayerMoves[m]&0xFFFF]);
	}
	
	fprintf(out, "\n\n********************引擎過濾︰%d個禁止著法********************************\n\n", nBanMoveNum);
	for(m=0; m<(unsigned int)nBanMoveNum; m++)
	{
		fprintf(out, "%3d. %s\n", m+1, GetStepName( BanMoveList[m], Board ));
	}

	fprintf(out, "\n\n***********************歷史記錄********************************\n\n", (nCurrentStep+1)/2);
	
	int MoveStr; 
	for(m=0; m<=(int)nCurrentStep; m++)
	{
		MoveStr = Coord(StepRecords[m]);
		fprintf(out, "%3d. %s  %2d  %2d  %12u\n", m, &MoveStr, (StepRecords[m] & 0xFF0000)>>16, (StepRecords[m] & 0xFF000000)>>24, nZobristBoard[m]);
	}


	// 關閉文件
	fclose(out);
}

/*
int CSearch::Evaluation(int player)
{
	return Evalue[player] - Evalue[1-player] + nOffensiveValue;
}
*/

// 循環檢測︰透過比較歷史記錄中的zobrist鍵值來判斷。
// 改進方案︰使用微型Hash表，LoopHash[zobrist & LoopMask] = zobrist  LoopMask=1023=0B1111111111  可以省去檢測過程中的循環判斷。
// 表現為雙方的兩個或多個棋子，在兩個或者兩個以上的位置循環往複運動。
// 根據中國象棋的棋規，先手將軍出現循環，不變作負。下面列舉測試程式時發現的將軍循環類型︰
	// 5.         10101  01010  00100 10001 11111
	// 6.        001010
	// 7.       1001001
	// 8.      00001010
	// 9.     100000001  101000001  010001000 000001000
	//12. 1000000000001  0000001000000
// 如此看來，各種各樣的循環都可能出現。循環的第一步可以是吃子移動，以後的是非吃子移動。
// 最少5步棋可以構成循環，非吃子移動的最少數目是4。5循環是最常見的類型，6～120的循環類型，象棋棋規並沒有定義。
// 循環檢測，實際上起到剪枝的作用，可以減小搜索數的分支。如果不進行處理，帶延伸的程式有時會無法退出。
int CSearch::RepetitionDetect(int nRepeatNum)
{
	// 120步(60回合)無殺子，視為達到自然限著，判定為和棋
	// NaturalBou也可以是其它值，由界面程式來發送，如50回合。
	// 寫在這裡是必要的，以免引擎在優勢的情況下和棋。
	if(nNonCapNum >= NaturalBouts)
		return(-NaturalBouts);
	
	unsigned int m, n;
	unsigned long *pBoard = &nZobristBoard[nCurrentStep-1];
	
	for(m=4; m<=nNonCapNum; m++)	
	{
		if( *pBoard == *(pBoard-m) )		// Zobrist相同，構成循環
		{
			// 統計循環中出現的將軍次數。
			CChessMove *pMove = &StepRecords[nCurrentStep-1];
			int nOwnChecks = 1;
			int nOppChecks = 1;
			for(n=0; n<=m; n++)
			{
				if((*(pMove-n)) & 0xFF000000)
				{
					if( 1 & n )
					{
						if( nOppChecks )			// 若對方沒有連將，計數器不增加
							nOppChecks ++;			// 統計對方的連將次數
					}
					else
					{
						if( nOwnChecks )			// 若我方沒有連將，計數器不增加
							nOwnChecks ++;			// 統計我方的連將次數
					}
				}
				else								// 一旦有一次沒有將軍，則此方就不構成連將
				{
					if( 1 & n )
						nOppChecks = 0;
					else
						nOwnChecks = 0;
				}

				// 雙方都沒有長將，和棋，不必繼續檢測
				if( !nOwnChecks && !nOppChecks )
					break;
			}

			// 查看循環的種類
			// 我方長將對手, 不一定是移動的棋子將軍, 移動后其他的棋子將軍也屬于此類。
			if( nOwnChecks>=nRepeatNum && !nOppChecks )				// 如 10101
				return 1;

			// 對手長將我方，主控權在我方，因為最後一步我方移動。
			// 最後移動的位置，有時是被迫的，有時可以自己主動構成循環，造成對手犯規。
			else if( nOppChecks>=nRepeatNum && !nOwnChecks )		// 如 01010
				return 2;
			
			// 其他情況，如長捉等，不形成將軍，視為和棋。
			// 中國象棋的棋規相當複雜，例如阻擋對方某顆棋子的下一步將軍，雖然循環體內不構成將軍，但若不阻擋則必死無疑。
			// 根據棋規，此情況視為對方不變為負。實現此算法相當複雜。
			else
				return -int(m);
		}
	}

	return(0);
}


// 循環的返回值
// 輸棋︰ - WINSCORE * 2	不記錄于Hash表
// 贏棋︰ + WINSCORE * 2	不記錄于Hash表
// 和棋︰估值 * 藐視因子    
int CSearch::LoopValue(int Player, int ply, int nLoopStyle)
{
	// 我方循環，是由於長將對方，故而對方勝利。不記錄在Hash表。
	// 循環檢測上一層的走法，本層還沒有走棋，故而使用ply-1
	if( nLoopStyle == 2 )
		return (ply-1)-WINSCORE*2;

	// 對方長將，我方勝利，返回勝利的分數。不記錄在Hash表。
	// 循環檢測上一層的走法，本層還沒有走棋，故而使用ply-1
	else if( nLoopStyle == 1 )
		return WINSCORE*2-(ply-1);

	// 和棋︰“藐視因子”(ContemptFactor=1/8=0.125)。
	// 優勢時，Evaluation(Player)遠遠大于0，返回負數，程式可能放棄當前的循環，尋找次好的著法，避免和棋。
	// 劣勢時，Evaluation(Player)遠遠小于0，返回正數，程式依然追求當前的循環，60回合內無殺子，自然和棋。
	// 均勢時，Evaluation(Player)接近于0，返回值更接近0，程式變招的可能性很小，以免冒進而輸棋。
	else // nLoopStyle < 0 
		return -(m_Evalue.Evaluation(Player) >> 3);
	// 國象的作法是使用一個藐視估值ContemptValue，優勢為負，表示程式想贏棋；弱勢為正，表示程式想和棋。
	// 根據西洋棋的經驗，開局為0.50個兵，中局為0.25個兵，殘局接近于0。
}


// 中斷引擎思考
int CSearch::Interrupt(void)
{
	if(!Ponder && clock() > nMaxTimer)
		bStopThinking = 1;
	else if(!bBatch) 
	{
		switch(BusyLine(Debug)) 
		{
			case UCCI_COMM_ISREADY:
				fprintf(OutFile, "readyok\n");
				fflush(OutFile);
				break;

			case UCCI_COMM_PONDERHIT:
				if(Ponder != 2) 
					Ponder = 0;
				break;

			case UCCI_COMM_STOP:
				bStopThinking = 1;
				break;
		}
	}

	return bStopThinking;
}
