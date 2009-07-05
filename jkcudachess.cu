////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 源文件︰Binghewusi.cpp                                                                                 //
// *******************************************************************************************************//
// 中國象棋通用引擎----兵河五四，支持《中國象棋通用引擎協議》(Universal Chinese Chess Protocol，簡稱ucci) //
// 作者︰ 范 德 軍                                                                                        //
// 單位︰ 中國原子能科學研究院                                                                            //
// 郵箱︰ fan_de_jun@sina.com.cn                                                                          //
//  QQ ︰ 83021504                                                                                        //
// *******************************************************************************************************//
// 功能︰                                                                                                 //
// 1. 控制台應用程式的入口點                                                                              //
// 2. 透過ucci協議與界面程式之間進行通訊                                                                  //
////////////////////////////////////////////////////////////////////////////////////////////////////////////

// includes, system
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
// includes, project
#include <cutil.h>
#include <cutil_inline.h>
// includes, kernels
#include <jkcudachess_kernel.cu>
// includes, Binghewusi
#include <string.h>
#include "ucci.h"
#include "FenBoard.h"
#include "Search.h"

void call_vecAdd();

int main(int argc, char* argv[])
{
	//call_vecAdd();
	int n;
	const char *BoolValue[2] = { "false", "true" };
	const char *ChessStyle[3] = { "solid", "normal", "risky" };
	char *BackSlashPtr;
	char BookFile[1024];
	UcciCommEnum IdleComm;
	UcciCommStruct Command;
	int ThisMove;
	
	printf("*******************************兵河五四 V0.60*********************************\n");
	printf("** 作者︰范德軍                                                             **\n");
	printf("** 支持《中國象棋通用引擎協議》(Universal Chinese Chess Protocol，簡稱UCCI) **\n");
	printf("** 引擎可以用“象堡”ElephantBoard界面程式(作者︰黃晨)加載。                **\n");
	printf("** 歡迎使用“兵河五四 V0.60”中國象棋引擎                                   **\n");	
	printf("******************************************************************************\n");
	printf("請鍵入ucci指令......\n");

	// 引擎接收"ucci"指令
	if(BootLine() == UCCI_COMM_UCCI)
	{
		// 尋找引擎所在的目錄argv[0]，並且把"BOOK.DAT"默認為缺省的開局庫開局庫
		BackSlashPtr = strrchr(argv[0], '\\');
		if (BackSlashPtr == 0) 
			strcpy(BookFile, "BOOK.DAT");
		else
		{
			strncpy(BookFile, argv[0], BackSlashPtr + 1 - argv[0]);
			strcpy(BookFile + (BackSlashPtr + 1 - argv[0]), "BOOK.DAT");
		}

		// 調用CSearch類，構造函數初始化一些相關參數
		//a.初始化著法預產生數組
		//b.初始化Hash表，分發21+1=22級Hash表，64M
		//c.清空歷史啟發表
		CSearch ThisSearch;

		// 顯示引擎的名稱、版本、作者和使用者
		printf("\n");
		printf("id name 中國象棋之“兵河五四V0.60jk”\n");
		fflush(stdout);
		printf("id copyright 版權所有(C) 2005-2008\n");
		fflush(stdout);
		printf("id author 范德軍(中國原子能科學研究院)\n");
		fflush(stdout);
		printf("id user CUDA\n\n");
		fflush(stdout);

		// 顯示引擎ucci指令的回饋訊息，表示引擎所支持的選項
		// option batch %d
		printf("option batch type check default %s\n", BoolValue[ThisSearch.bBatch]);
		fflush(stdout);

		// option debug 讓引擎輸出詳細的搜索訊息，並非真正的調試模式。
		printf("option debug type check default %s\n", BoolValue[ThisSearch.Debug]);
		fflush(stdout);

		// 指定開局庫文件的名稱，可指定多個開局庫文件，用分號“;”隔開，如不讓引擎使用開局庫，可以把值設成空
		ThisSearch.bUseOpeningBook = ThisSearch.m_Hash.LoadBook(BookFile);
		if(ThisSearch.bUseOpeningBook)
			printf("option bookfiles type string default %s\n", BookFile);
		else
			printf("option bookfiles type string default %s\n", 0);
		fflush(stdout);

		// 殘局庫名稱
		printf("option egtbpaths type string default null\n");
		fflush(stdout);

		// 顯示Hash表的大小
		printf("option hashsize type spin default %d MB\n", ThisSearch.m_Hash.nHashSize*2*sizeof(CHashRecord)/1024/1024);
		fflush(stdout);

		// 引擎的線程數
		printf("option threads type spin default %d\n", 0);
		fflush(stdout);

		// 引擎達到自然限著的半回合數
		printf("option drawmoves type spin default %d\n", ThisSearch.NaturalBouts);
		fflush(stdout);

		// 棋規
		printf("option repetition type spin default %d 1999年版《中國象棋競賽規則》\n", UCCI_REPET_CHINESERULE);
		fflush(stdout);

		// 空著裁減是否打開
		printf("option pruning type check %d\n", ThisSearch);
		fflush(stdout);

		// 估值函數的使用情況
		printf("option knowledge type check %d\n", ThisSearch);
		fflush(stdout);

		// 指定選擇性系數，通常有0,1,2,3四個級別。給估值函數加減一定範圍內的隨機數，讓引擎每次走出不相同的棋。
		printf("option selectivity type spin min 0 max 3 default %d\n", ThisSearch.nSelectivity);
		fflush(stdout);

		// 指定下棋的風格，通常有solid(保守)、normal(均衡)和risky(冒進)三種
		printf("option style type combo var solid var normal var risky default %s\n", ChessStyle[ThisSearch.nStyle]);
		fflush(stdout);		

		// copyprotection 顯示版權檢查訊息(正在檢查，版權訊息正確或版權訊息錯誤)。 
		printf("copyprotection ok\n\n");
		fflush(stdout);

		// ucciok 這是ucci指令的最後一條回饋訊息，表示引擎已經進入用UCCI協議通訊的狀態。
		printf("ucciok\n\n");
		fflush(stdout);


		// 設定標準輸出和初始局面
		ThisSearch.OutFile = stdout;	// 標準輸出
		ThisSearch.fen.FenToBoard(Board, Piece, ThisSearch.Player, ThisSearch.nNonCapNum, ThisSearch.nCurrentStep, "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR r - - 0 1");
		ThisSearch.InitBitBoard(ThisSearch.Player, ThisSearch.nCurrentStep);
		printf("position fen rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR r - - 0 1\n\n");
		fflush(stdout);
		

		// 開始解釋執行UCCI命令
		do 
		{
			IdleComm = IdleLine(Command, ThisSearch.Debug);
			switch (IdleComm) 
			{
				// isready 檢測引擎是否處于就緒狀態，其回饋訊息總是readyok，該指令僅僅用來檢測引擎的“指令接收緩沖區”是否能正常容納指令。
				// readyok 表明引擎處于就緒狀態(即可接收指令的狀態)，不管引擎處于空閒狀態還是思考狀態。
				case UCCI_COMM_ISREADY:
					printf("readyok\n");
					fflush(stdout);
					break;

				// stop 中斷引擎的思考，強製出著。后台思考沒有命中時，就用該指令來中止思考，然後重新輸入局面。
				case UCCI_COMM_STOP:
					ThisSearch.bStopThinking = 1;
					//printf("nobestmove\n");
					printf("score 0\n");
					fflush(stdout);
					break;

				// position fen 設置“內置棋盤”的局面，用fen來指定FEN格式串，moves后面跟的是隨后走過的著法
				case UCCI_COMM_POSITION:
					// 將界面傳來的Fen串轉化為棋局訊息
					ThisSearch.fen.FenToBoard(Board, Piece, ThisSearch.Player, ThisSearch.nNonCapNum, ThisSearch.nCurrentStep, Command.Position.szFenStr);
					ThisSearch.InitBitBoard(ThisSearch.Player, ThisSearch.nCurrentStep);

					// 將局面走到當前，主要是為了更新著法記錄，用于循環檢測。
					for(n=0; n<Command.Position.nMoveNum; n++)
					{
						ThisMove = Move(Command.Position.lpdwCoordList[n]);
						if( !ThisMove )
							break;

						ThisSearch.MovePiece( ThisMove );
						ThisSearch.StepRecords[ThisSearch.nCurrentStep-1] |= ThisSearch.Checking(ThisSearch.Player) << 24;
					}

					ThisSearch.nBanMoveNum = 0;
					break;

				// banmoves 為當前局面設置禁手，以解決引擎無法處理的長打問題。當出現長打局面時，棋手可以操控界面向引擎發出禁手指令。
				case UCCI_COMM_BANMOVES:
					ThisSearch.nBanMoveNum = Command.BanMoves.nMoveNum;
					for(n=0; n<Command.BanMoves.nMoveNum; n++)
						ThisSearch.BanMoveList[n] = Move(Command.BanMoves.lpdwCoordList[n]);
					break;

				// setoption 設置引擎各種參數
				case UCCI_COMM_SETOPTION:
					switch(Command.Option.uoType) 
					{
						// setoption batch %d
						case UCCI_OPTION_BATCH:
							ThisSearch.bBatch = (Command.Option.Value.bCheck == TRUE);
							printf("option batch type check default %s\n", BoolValue[ThisSearch.bBatch]);
							fflush(stdout);
							break;

						// setoption debug %d 讓引擎輸出詳細的搜索訊息，並非真正的調試模式。
						case UCCI_OPTION_DEBUG:
							ThisSearch.Debug = (Command.Option.Value.bCheck == TRUE);
							printf("option debug type check default %s\n", BoolValue[ThisSearch.Debug]);
							fflush(stdout);
							break;

						// setoption bookfiles %s  指定開局庫文件的名稱，可指定多個開局庫文件，用分號“;”隔開，如不讓引擎使用開局庫，可以把值設成空
						case UCCI_OPTION_BOOKFILES:
							strcpy(BookFile, Command.Option.Value.szString);
							printf("option bookfiles type string default %s\n", BookFile);
							fflush(stdout);
							break;

						// setoption egtbpaths %s  指定殘局庫文件的名稱，可指定多個殘局庫路徑，用分號“;”隔開，如不讓引擎使用殘局庫，可以把值設成空
						//case e_OptionEgtbPaths:
							// 引擎目前不支持開局庫
							//printf("option egtbpaths type string default null\n");
							//fflush(stdout);
							//break;

						// setoption hashsize %d  以MB為單位規定Hash表的大小，-1表示讓引擎自動分發Hash表。1～1024MB
						// 象堡界面有個Bug，每次設置引擎時，這個命令應在開局庫的前面
						case UCCI_OPTION_HASHSIZE:
							// -1MB(自動), 0MB(自動), 1MB(16), 2MB(17), 4MB(18), 8MB(19), 16MB(20), 32MB(21), 64MB(22), 128MB(23), 256MB(24), 512MB(25), 1024MB(26)
							if( Command.Option.Value.nSpin <= 0)
								n = 22;		// 缺省情況下，引擎自動分發(1<<22)*16=64MB，紅與黑兩各表，雙方各一半。
							else
							{
								n = 15;											// 0.5 MB = 512 KB 以此為基數
								while( Command.Option.Value.nSpin > 0 )
								{
									Command.Option.Value.nSpin >>= 1;			// 每次除以2，直到為0
									n ++;
								}
							}								

							// 應加入內存檢測機製，引擎自動分發時，Hash表大小為可用內存的1/2。
							ThisSearch.m_Hash.DeleteHashTable();					// 必須使用delete先清除舊的Hash表
							ThisSearch.m_Hash.NewHashTable(n > 26 ? 26 : n, 12);	// 為引擎分發新的Hash表
							printf("option hashsize type spin default %d MB\n", ThisSearch.m_Hash.nHashSize*2*sizeof(CHashRecord)/1024/1024);	// 顯示實際分發的Hash表大小，單位︰MB
							fflush(stdout);

							ThisSearch.m_Hash.ClearHashTable();
							ThisSearch.bUseOpeningBook = ThisSearch.m_Hash.LoadBook(BookFile);
							break;

						// setoption threads %d	      引擎的線程數，為多處理器並行運算服務
						case UCCI_OPTION_THREADS:
							// ThisSearch.nThreads = Command.Option.Value.Spin;		// 0(auto),1,2,4,8,16,32
							printf("option drawmoves type spin default %d\n", 0);
							fflush(stdout);
							break;

						// setoption drawmoves %d	  達到自然限著的回合數:50,60,70,80,90,100，象堡已經自動轉化為半回合數
						case UCCI_OPTION_DRAWMOVES:
							ThisSearch.NaturalBouts = Command.Option.Value.nSpin;
							printf("option drawmoves type spin default %d\n", ThisSearch.NaturalBouts);
							fflush(stdout);
							break;

						// setoption repetition %d	  處理循環的棋規，目前只支持“中國象棋棋規1999”
						case UCCI_OPTION_REPETITION:
							// ThisSearch.nRepetitionStyle = Command.Option.Value.Repetition;
							// e_RepetitionAlwaysDraw  不變作和
							// e_RepetitionCheckBan    禁止長將
							// e_RepetitionAsianRule   亞洲規則
							// e_RepetitionChineseRule 中國規則（缺省）
							printf("option repetition type spin default %d", UCCI_REPET_CHINESERULE);
							printf("  “兵河五四”引擎目前支持1999年版《中國象棋競賽規則》\n");
							fflush(stdout);
							break;

						// setoption pruning %d，“空著向前裁剪”是否打開
						case UCCI_OPTION_PRUNING:
							ThisSearch.bPruning = Command.Option.Value.ugGrade;
							printf("option pruning type check %d\n", ThisSearch);
							fflush(stdout);
							break;

						// setoption knowledge %d，估值函數的使用
						case UCCI_OPTION_KNOWLEDGE:
							ThisSearch.bKnowledge = Command.Option.Value.ugGrade;
							printf("option knowledge type check %d\n", ThisSearch);
							fflush(stdout);
							break;

						// setoption selectivity %d  指定選擇性系數，通常有0,1,2,3四個級別
						case UCCI_OPTION_SELECTIVITY:
							switch (Command.Option.Value.ugGrade)
							{
								case UCCI_GRADE_NONE:
									ThisSearch.SelectMask = 0;
									break;
								case UCCI_GRADE_SMALL:
									ThisSearch.SelectMask = 1;
									break;
								case UCCI_GRADE_MEDIUM:
									ThisSearch.SelectMask = 3;
									break;
								case UCCI_GRADE_LARGE:
									ThisSearch.SelectMask = 7;
									break;
								default:
									ThisSearch.SelectMask = 0;
									break;
							}
							printf("option selectivity type spin min 0 max 3 default %d\n", ThisSearch.SelectMask);
							fflush(stdout);
							break;

						// setoption style %d  指定下棋的風格，通常有solid(保守)、normal(均衡)和risky(冒進)三種
						case UCCI_OPTION_STYLE:
							ThisSearch.nStyle = Command.Option.Value.usStyle;
							printf("option style type combo var solid var normal var risky default %s\n", ChessStyle[Command.Option.Value.usStyle]);
							fflush(stdout);
							break;						

						// setoption loadbook  UCCI界面ElephantBoard在每次新建棋局時都會發送這條指令
						case UCCI_OPTION_LOADBOOK:
							ThisSearch.m_Hash.ClearHashTable();
							ThisSearch.bUseOpeningBook = ThisSearch.m_Hash.LoadBook(BookFile);
							
							if(ThisSearch.bUseOpeningBook)
								printf("option loadbook succeed. %s\n", BookFile);		// 成功
							else
								printf("option loadbook failed! %s\n", "Not found file BOOK.DAT");				// 沒有開局庫
							fflush(stdout);
							printf("\n\n");
							fflush(stdout);
							break;

						default:
							break;
					}
					break;

				// Prepare timer strategy according to "go depth %d" or "go ponder depth %d" command
				case UCCI_COMM_GO:
				case UCCI_COMM_GOPONDER:
					switch (Command.Search.utMode)
					{
						// 固定深度
						case UCCI_TIME_DEPTH:
							ThisSearch.Ponder = 2;
							ThisSearch.MainSearch(Command.Search.DepthTime.nDepth);
							break;

						// 時段製︰ 分發時間 = 剩餘時間 / 要走的步數
						case UCCI_TIME_MOVE:							
							ThisSearch.Ponder = (IdleComm == UCCI_COMM_GOPONDER ? 1 : 0);
							printf("%d\n", Command.Search.TimeMode.nMovesToGo);
							ThisSearch.MainSearch(127, Command.Search.DepthTime.nTime * 1000 / Command.Search.TimeMode.nMovesToGo, Command.Search.DepthTime.nTime * 1000);
							break;

						// 加時製︰ 分發時間 = 每步增加的時間 + 剩餘時間 / 20 (即假設棋局會在20步內結束)
						case UCCI_TIME_INC:
							ThisSearch.Ponder = (IdleComm == UCCI_COMM_GOPONDER ? 1 : 0);
							ThisSearch.MainSearch(127, (Command.Search.DepthTime.nTime + Command.Search.TimeMode.nIncrement * 20) * 1000 / 20, Command.Search.DepthTime.nTime * 1000);
							break;

						default:
							break;
					}
					break;
			}
		} while (IdleComm != UCCI_COMM_QUIT);

		printf("bye\n");
		fflush(stdout);
	}

	return 0;
}

//呼叫隨機亂數陣列加法測試
void call_vecAdd()
{
	printf("test start");

	unsigned int num_threads = 4088;
	int  MAX_BLOCKTHREAD=512;
    unsigned int mem_size = sizeof( float) * num_threads;

	// allocate host memory
    float* h_idata1 = (float*) malloc( mem_size);
    // initalize the memory
    for( unsigned int i = 0; i < num_threads; ++i) 
    {
        h_idata1[i] = (float) (rand()%100);
    }
	float* h_idata2 = (float*) malloc( mem_size);
    // initalize the memory
    for( unsigned int i = 0; i < num_threads; ++i) 
    {
        h_idata2[i] = (float) (rand()%100);
    }

    // allocate device memory
    float* d_idata1;
    cutilSafeCall( cudaMalloc( (void**) &d_idata1, mem_size));
    // copy host memory to device
    cutilSafeCall( cudaMemcpy( d_idata1, h_idata1, mem_size, cudaMemcpyHostToDevice) );
    float* d_idata2;
    cutilSafeCall( cudaMalloc( (void**) &d_idata2, mem_size));
    // copy host memory to device
    cutilSafeCall( cudaMemcpy( d_idata2, h_idata2, mem_size, cudaMemcpyHostToDevice) );

    // allocate device memory for result
    float* d_odata;
    cutilSafeCall( cudaMalloc( (void**) &d_odata, mem_size));

    // execute the kernel
	dim3  grid( (( num_threads -1 ) / MAX_BLOCKTHREAD + 1) , 1);
    dim3  threads( MAX_BLOCKTHREAD , 1);
    vecAdd<<< grid, threads, mem_size >>>( d_idata1, d_idata2, d_odata);

    // check if kernel execution generated and error
    cutilCheckMsg("Kernel execution failed");

    // allocate mem for the result on host side
    float* h_odata = (float*) malloc( mem_size);
    // copy result from device to host
    cutilSafeCall( cudaMemcpy( h_odata, d_odata, sizeof( float) * num_threads, cudaMemcpyDeviceToHost) );

	//印出結果
	for( unsigned int i = 0; i < num_threads; ++i) 
    {
        printf("%f + %f = %f \n",h_idata1[i],h_idata2[i],h_odata[i]);
    }

	system("pause");
}