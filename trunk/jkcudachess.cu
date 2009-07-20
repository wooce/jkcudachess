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
    //printf("movegen_calls=%d\n",movegen_calls);
    /*
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
    */
}

#define numOfThreads 180

texture<unsigned char, 1, cudaReadModeElementType> texture_KingMoves;
texture<unsigned char, 1, cudaReadModeElementType> texture_xRookMoves;
texture<unsigned char, 1, cudaReadModeElementType> texture_yRookMoves;
texture<unsigned char, 1, cudaReadModeElementType> texture_xCannonMoves;
texture<unsigned char, 1, cudaReadModeElementType> texture_yCannonMoves;
texture<unsigned char, 1, cudaReadModeElementType> texture_KnightMoves;
texture<unsigned char, 1, cudaReadModeElementType> texture_BishopMoves;
texture<unsigned char, 1, cudaReadModeElementType> texture_GuardMoves;
texture<unsigned char, 1, cudaReadModeElementType> texture_PawnMoves;
texture<unsigned char, 1, cudaReadModeElementType> texture_xRookCapMoves;
texture<unsigned char, 1, cudaReadModeElementType> texture_yRookCapMoves;
texture<unsigned char, 1, cudaReadModeElementType> texture_xCannonCapMoves;
texture<unsigned char, 1, cudaReadModeElementType> texture_yCannonCapMoves;
//預展開步數
unsigned char * cuda_KingMoves;
unsigned char * cuda_xRookMoves;
unsigned char * cuda_yRookMoves;
unsigned char * cuda_xCannonMoves;
unsigned char * cuda_yCannonMoves;
unsigned char * cuda_KnightMoves;
unsigned char * cuda_BishopMoves;
unsigned char * cuda_GuardMoves;
unsigned char * cuda_PawnMoves;
unsigned char * cuda_xRookCapMoves;
unsigned char * cuda_yRookCapMoves;
unsigned char * cuda_xCannonCapMoves;
unsigned char * cuda_yCannonCapMoves;
//配置GPU存放結果的記憶體
unsigned int* cuda_move;

__constant__ unsigned int cuda_xBitBoard[16];//16*4=64
__constant__ unsigned int cuda_yBitBoard[16];//16*4=64
__constant__ int cuda_Board[256];//256*4=1024
__constant__ int cuda_Piece[48];//48*4=192
__constant__ char cuda_nHorseLegTab[512] = {// 馬腿增量表
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
//複製PreMoveGen展開的走法到GPU的記憶體裡
void copyPreMoveToGPU(unsigned char host_KingMoves[256][8],unsigned char host_xRookMoves[12][512][12],unsigned char host_yRookMoves[13][1024][12],unsigned char host_xCannonMoves[12][512][12],unsigned char host_yCannonMoves[13][1024][12],unsigned char host_KnightMoves[256][12],unsigned char host_BishopMoves[256][8],unsigned char host_GuardMoves[256][8],unsigned char host_PawnMoves[2][256][4],unsigned char host_xRookCapMoves[12][512][4],unsigned char host_yRookCapMoves[13][1024][4],unsigned char host_xCannonCapMoves[12][512][4],unsigned char host_yCannonCapMoves[13][1024][4])
{
    int MEMSIZE_KingMoves=2048;
    int MEMSIZE_xRookMoves=73728;
    int MEMSIZE_yRookMoves=159744;
    int MEMSIZE_xCannonMoves=73728;
    int MEMSIZE_yCannonMoves=159744;
    int MEMSIZE_KnightMoves=3072;
    int MEMSIZE_BishopMoves=2048;
    int MEMSIZE_GuardMoves=2048;
    int MEMSIZE_PawnMoves=2048;
    int MEMSIZE_xRookCapMoves=24576;
    int MEMSIZE_yRookCapMoves=53248;
    int MEMSIZE_xCannonCapMoves=24576;
    int MEMSIZE_yCannonCapMoves=53248;

    //cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc<unsigned char>();
    //const cudaExtent KingMoves_volumeSize = make_cudaExtent(256, 8, 0);
    //cudaArray *cuda_KingMovesArray;
    //cutilSafeCall(cudaMalloc3DArray(&cuda_KingMovesArray, &channelDesc, KingMoves_volumeSize) );
    //// copy data to 3D array
    //cudaMemcpy3DParms KingMoves_copyParams = {0};
    //KingMoves_copyParams.srcPtr   = make_cudaPitchedPtr((void*)host_KingMoves, KingMoves_volumeSize.width*sizeof(unsigned char), KingMoves_volumeSize.width, KingMoves_volumeSize.height);
    //KingMoves_copyParams.dstArray = cuda_KingMovesArray;
    //KingMoves_copyParams.extent   = KingMoves_volumeSize;
    //KingMoves_copyParams.kind     = cudaMemcpyHostToDevice;
    //cutilSafeCall(cudaMemcpy3D(&KingMoves_copyParams));
    //cutilSafeCall(cudaBindTextureToArray(texture_KingMoves,cuda_KingMovesArray,channelDesc));

    cutilSafeCall(cudaMalloc( (void**) &cuda_KingMoves, MEMSIZE_KingMoves));
    cutilSafeCall(cudaMemcpy( cuda_KingMoves, host_KingMoves, MEMSIZE_KingMoves, cudaMemcpyHostToDevice));
    cutilSafeCall(cudaBindTexture(0, texture_KingMoves,cuda_KingMoves,MEMSIZE_KingMoves));

    cutilSafeCall(cudaMalloc( (void**) &cuda_xRookMoves, MEMSIZE_xRookMoves));
    cutilSafeCall(cudaMemcpy( cuda_xRookMoves, host_xRookMoves, MEMSIZE_xRookMoves, cudaMemcpyHostToDevice));
    cutilSafeCall(cudaBindTexture(0, texture_xRookMoves,cuda_xRookMoves,MEMSIZE_xRookMoves));

    cutilSafeCall(cudaMalloc( (void**) &cuda_yRookMoves, MEMSIZE_yRookMoves));
    cutilSafeCall(cudaMemcpy( cuda_yRookMoves, host_yRookMoves, MEMSIZE_yRookMoves, cudaMemcpyHostToDevice));
    cutilSafeCall(cudaBindTexture(0, texture_yRookMoves,cuda_yRookMoves,MEMSIZE_yRookMoves));

    cutilSafeCall(cudaMalloc( (void**) &cuda_xCannonMoves, MEMSIZE_xCannonMoves));
    cutilSafeCall(cudaMemcpy( cuda_xCannonMoves, host_xCannonMoves, MEMSIZE_xCannonMoves, cudaMemcpyHostToDevice));
    cutilSafeCall(cudaBindTexture(0, texture_xCannonMoves,cuda_xCannonMoves,MEMSIZE_xCannonMoves));

    cutilSafeCall(cudaMalloc( (void**) &cuda_yCannonMoves, MEMSIZE_yCannonMoves));
    cutilSafeCall(cudaMemcpy( cuda_yCannonMoves, host_yCannonMoves, MEMSIZE_yCannonMoves, cudaMemcpyHostToDevice));
    cutilSafeCall(cudaBindTexture(0, texture_yCannonMoves,cuda_yCannonMoves,MEMSIZE_yCannonMoves));

    cutilSafeCall(cudaMalloc( (void**) &cuda_KnightMoves, MEMSIZE_KnightMoves));
    cutilSafeCall(cudaMemcpy( cuda_KnightMoves, host_KnightMoves, MEMSIZE_KnightMoves, cudaMemcpyHostToDevice));
    cutilSafeCall(cudaBindTexture(0, texture_KnightMoves,cuda_KnightMoves,MEMSIZE_KnightMoves));

    cutilSafeCall(cudaMalloc( (void**) &cuda_BishopMoves, MEMSIZE_BishopMoves));
    cutilSafeCall(cudaMemcpy( cuda_BishopMoves, host_BishopMoves, MEMSIZE_BishopMoves, cudaMemcpyHostToDevice));
    cutilSafeCall(cudaBindTexture(0, texture_BishopMoves,cuda_BishopMoves,MEMSIZE_BishopMoves));

    cutilSafeCall(cudaMalloc( (void**) &cuda_GuardMoves, MEMSIZE_GuardMoves));
    cutilSafeCall(cudaMemcpy( cuda_GuardMoves, host_GuardMoves, MEMSIZE_GuardMoves, cudaMemcpyHostToDevice));
    cutilSafeCall(cudaBindTexture(0, texture_GuardMoves,cuda_GuardMoves,MEMSIZE_GuardMoves));

    cutilSafeCall(cudaMalloc( (void**) &cuda_PawnMoves, MEMSIZE_PawnMoves));
    cutilSafeCall(cudaMemcpy( cuda_PawnMoves, host_PawnMoves, MEMSIZE_PawnMoves, cudaMemcpyHostToDevice));
    cutilSafeCall(cudaBindTexture(0, texture_PawnMoves,cuda_PawnMoves,MEMSIZE_PawnMoves));

    cutilSafeCall(cudaMalloc( (void**) &cuda_xRookCapMoves, MEMSIZE_xRookCapMoves));
    cutilSafeCall(cudaMemcpy( cuda_xRookCapMoves, host_xRookCapMoves, MEMSIZE_xRookCapMoves, cudaMemcpyHostToDevice));
    cutilSafeCall(cudaBindTexture(0, texture_xRookCapMoves,cuda_xRookCapMoves,MEMSIZE_xRookCapMoves));

    cutilSafeCall(cudaMalloc( (void**) &cuda_yRookCapMoves, MEMSIZE_yRookCapMoves));
    cutilSafeCall(cudaMemcpy( cuda_yRookCapMoves, host_yRookCapMoves, MEMSIZE_yRookCapMoves, cudaMemcpyHostToDevice));
    cutilSafeCall(cudaBindTexture(0, texture_yRookCapMoves,cuda_yRookCapMoves,MEMSIZE_yRookCapMoves));

    cutilSafeCall(cudaMalloc( (void**) &cuda_xCannonCapMoves, MEMSIZE_xCannonCapMoves));
    cutilSafeCall(cudaMemcpy( cuda_xCannonCapMoves, host_xCannonCapMoves, MEMSIZE_xCannonCapMoves, cudaMemcpyHostToDevice));
    cutilSafeCall(cudaBindTexture(0, texture_xCannonCapMoves,cuda_xCannonCapMoves,MEMSIZE_xCannonCapMoves));

    cutilSafeCall(cudaMalloc( (void**) &cuda_yCannonCapMoves, MEMSIZE_yCannonCapMoves));
    cutilSafeCall(cudaMemcpy( cuda_yCannonCapMoves, host_yCannonCapMoves, MEMSIZE_yCannonCapMoves, cudaMemcpyHostToDevice));
    cutilSafeCall(cudaBindTexture(0, texture_yCannonCapMoves,cuda_yCannonCapMoves,MEMSIZE_yCannonCapMoves));

    cutilSafeCall(cudaMalloc( (void**) &cuda_move, 4*numOfThreads));
}
//cuda MoveGen主體
#define WRITE_2_MOVE if(pMove&&nSrc&&nDst&&!cuda_Board[nDst]){move=(nSrc<<8)|nDst;}else{move=0;}
__global__ void cudaMoveGen_warp(const unsigned int k,unsigned int* cuda_move)
{    
    //int tid=blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int  move, nSrc, nDst, x, y, nChess;
    unsigned char pMove;
    //將帥**************************************************************************
    if(blockIdx.x==0)
    {
        if(threadIdx.x<8)
        {
            nChess=k;
            nSrc = cuda_Piece[nChess];// 將帥存在︰nSrc!=0
            //pMove = cuda_KingMoves[nSrc][tid];
            pMove = tex1Dfetch(texture_KingMoves,nSrc*8+threadIdx.x);
            nDst = pMove;
            WRITE_2_MOVE;
           cuda_move[threadIdx.x]=move;
        }
    }
    //車****************************************************************************
    else if( blockIdx.x==1 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+1;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            x = nSrc & 0xF;// 后4位有效
            y = nSrc >> 4;// 前4位有效
            //車的橫向移動︰
            pMove = tex1Dfetch(texture_xRookMoves,x*512*12+cuda_xBitBoard[y]*12+threadIdx.x);
            nDst = (nSrc & 0xF0) | pMove;	// 0x y|x  前4位=y*16， 后4位=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+8]=move;
        }
    }
    else if( blockIdx.x==2 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+1;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            x = nSrc & 0xF;// 后4位有效
            y = nSrc >> 4;// 前4位有效
            //車的橫向移動︰
            pMove = tex1Dfetch(texture_yRookMoves,y*1024*12+cuda_yBitBoard[x]*12+threadIdx.x);
            nDst = pMove | x;				// 0x y|x  前4位=y*16， 后4位=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+20]=move;
        }
    }
    else if( blockIdx.x==3 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+2;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            x = nSrc & 0xF;// 后4位有效
            y = nSrc >> 4;// 前4位有效
            //車的橫向移動︰
            pMove = tex1Dfetch(texture_xRookMoves,x*512*12+cuda_xBitBoard[y]*12+threadIdx.x);
            nDst = (nSrc & 0xF0) | pMove;	// 0x y|x  前4位=y*16， 后4位=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+32]=move;
        }
    }
    else if( blockIdx.x==4 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+2;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            x = nSrc & 0xF;// 后4位有效
            y = nSrc >> 4;// 前4位有效
            //車的橫向移動︰
            pMove = tex1Dfetch(texture_yRookMoves,y*1024*12+cuda_yBitBoard[x]*12+threadIdx.x);
            nDst = pMove | x;				// 0x y|x  前4位=y*16， 后4位=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+44]=move;
        }
    }    
    //炮****************************************************************************
    else if( blockIdx.x==5 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+3;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            x = nSrc & 0xF;// 后4位有效
            y = nSrc >> 4;// 前4位有效
            //炮的橫向移動︰
            pMove = tex1Dfetch(texture_xCannonMoves,x*512*12+cuda_xBitBoard[y]*12+threadIdx.x);
            nDst = (nSrc & 0xF0) | pMove;	// 0x y|x  前4位=y*16， 后4位=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+56]=move;
        }
    }
    else if( blockIdx.x==6 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+3;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            x = nSrc & 0xF;// 后4位有效
            y = nSrc >> 4;// 前4位有效
            //炮的縱向移動
            pMove = tex1Dfetch(texture_yCannonMoves,y*1024*12+cuda_yBitBoard[x]*12+threadIdx.x);
            nDst = pMove | x;				// 0x y|x  前4位=y*16， 后4位=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+68]=move;
        }
    }
    else if( blockIdx.x==7 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+4;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            x = nSrc & 0xF;// 后4位有效
            y = nSrc >> 4;// 前4位有效
            //炮的橫向移動︰
            pMove = tex1Dfetch(texture_xCannonMoves,x*512*12+cuda_xBitBoard[y]*12+threadIdx.x);
            nDst = (nSrc & 0xF0) | pMove;	// 0x y|x  前4位=y*16， 后4位=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+80]=move;
        }
    }
    else if( blockIdx.x==8 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+4;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            x = nSrc & 0xF;// 后4位有效
            y = nSrc >> 4;// 前4位有效
            //炮的縱向移動
            pMove = tex1Dfetch(texture_yCannonMoves,y*1024*12+cuda_yBitBoard[x]*12+threadIdx.x);
            nDst = pMove | x;				// 0x y|x  前4位=y*16， 后4位=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+92]=move;
        }
    }
    //馬****************************************************************************
    else if( blockIdx.x==9 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+5;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            pMove = tex1Dfetch(texture_KnightMoves,nSrc*12+threadIdx.x);
            nDst = pMove;
            if( !cuda_Board[nSrc+cuda_nHorseLegTab[nDst-nSrc+256]] )//拐馬腳
            {					
                WRITE_2_MOVE;
            }
            else
            {
                move=0;
            }
           cuda_move[threadIdx.x+104]=move;
        }
    }
    else if( blockIdx.x==10 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+6;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            pMove = tex1Dfetch(texture_KnightMoves,nSrc*12+threadIdx.x);
            nDst = pMove;
            if( !cuda_Board[nSrc+cuda_nHorseLegTab[nDst-nSrc+256]] )//拐馬腳
            {					
                WRITE_2_MOVE;
            }
            else
            {
                move=0;
            }
           cuda_move[threadIdx.x+116]=move;
        }
    }
    //象****************************************************************************
    else if( blockIdx.x==11 )
    {
        if(threadIdx.x<8)
        {
            nChess=k+7;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            pMove = tex1Dfetch(texture_BishopMoves,nSrc*8+threadIdx.x);
            nDst = pMove;
            if( !cuda_Board[(nSrc+nDst)>>1] )//象眼無子
            {
                WRITE_2_MOVE;
            }
            else
            {
                move=0;
            }
           cuda_move[threadIdx.x+128]=move;
        }
    }
    else if( blockIdx.x==12 )
    {
        if(threadIdx.x<8)
        {
            nChess=k+8;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            pMove = tex1Dfetch(texture_BishopMoves,nSrc*8+threadIdx.x);
            nDst = pMove;
            if( !cuda_Board[(nSrc+nDst)>>1] )//象眼無子
            {
                WRITE_2_MOVE;
            }
            else
            {
                move=0;
            }
           cuda_move[threadIdx.x+136]=move;
        }
    }
    //士****************************************************************************
    else if( blockIdx.x==13 )
    {
        if(threadIdx.x<8)
        {
            nChess=k+9;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            pMove = tex1Dfetch(texture_GuardMoves,nSrc*8+threadIdx.x);
            nDst = pMove;
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+144]=move;
        }
    }
    else if( blockIdx.x==14 )
    {
        if(threadIdx.x<8)
        {
            nChess=k+10;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            pMove = tex1Dfetch(texture_GuardMoves,nSrc*8+threadIdx.x);
            nDst = pMove;
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+152]=move;
        }
    }
    //兵****************************************************************************
    else if( blockIdx.x==15 )
    {
        if(threadIdx.x<4)
        {
            nChess=k+11;
            int Player;
            if(k<32){Player=0;}
            else{Player=1;}
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            //pMove = cuda_PawnMoves[Player][nSrc][(tid-160)%4];
            pMove = tex1Dfetch(texture_PawnMoves,Player*256*4+nSrc*4+threadIdx.x);
            nDst = pMove;
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+160]=move;
        }
    }  
    else if( blockIdx.x==16 )
    {
        if(threadIdx.x<4)
        {
            nChess=k+12;
            int Player;
            if(k<32){Player=0;}
            else{Player=1;}
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            //pMove = cuda_PawnMoves[Player][nSrc][(tid-160)%4];
            pMove = tex1Dfetch(texture_PawnMoves,Player*256*4+nSrc*4+threadIdx.x);
            nDst = pMove;
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+164]=move;
        }
    } 
    else if( blockIdx.x==17 )
    {
        if(threadIdx.x<4)
        {
            nChess=k+13;
            int Player;
            if(k<32){Player=0;}
            else{Player=1;}
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            //pMove = cuda_PawnMoves[Player][nSrc][(tid-160)%4];
            pMove = tex1Dfetch(texture_PawnMoves,Player*256*4+nSrc*4+threadIdx.x);
            nDst = pMove;
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+168]=move;
        }
    } 
    else if( blockIdx.x==18 )
    {
        if(threadIdx.x<4)
        {
            nChess=k+14;
            int Player;
            if(k<32){Player=0;}
            else{Player=1;}
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            //pMove = cuda_PawnMoves[Player][nSrc][(tid-160)%4];
            pMove = tex1Dfetch(texture_PawnMoves,Player*256*4+nSrc*4+threadIdx.x);
            nDst = pMove;
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+172]=move;
        }
    } 
    else if( blockIdx.x==19 )
    {
        if(threadIdx.x<4)
        {
            nChess=k+15;
            int Player;
            if(k<32){Player=0;}
            else{Player=1;}
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            pMove = tex1Dfetch(texture_PawnMoves,Player*256*4+nSrc*4+threadIdx.x);
            nDst = pMove;
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+176]=move;
        }
    } 
}

__global__ void cudaMoveGen(const unsigned int k,unsigned int* cuda_move)
{    
    //int tid=blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int  move, nSrc, nDst, x, y, nChess;
    unsigned char pMove;
    //將帥**************************************************************************
    if(blockIdx.x==0)
    {
        if(threadIdx.x<8)
        {
            nChess=k;
            nSrc = cuda_Piece[nChess];// 將帥存在︰nSrc!=0
            //pMove = cuda_KingMoves[nSrc][tid];
            pMove = tex1Dfetch(texture_KingMoves,nSrc*8+threadIdx.x);
            nDst = pMove;
            WRITE_2_MOVE;
           cuda_move[threadIdx.x]=move;
        }
    }
    //車****************************************************************************
    else if( blockIdx.x==1 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+1;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            x = nSrc & 0xF;// 后4位有效
            y = nSrc >> 4;// 前4位有效
            //車的橫向移動︰
            pMove = tex1Dfetch(texture_xRookMoves,x*512*12+cuda_xBitBoard[y]*12+threadIdx.x);
            nDst = (nSrc & 0xF0) | pMove;	// 0x y|x  前4位=y*16， 后4位=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+8]=move;
        }
    }
    else if( blockIdx.x==2 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+1;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            x = nSrc & 0xF;// 后4位有效
            y = nSrc >> 4;// 前4位有效
            //車的橫向移動︰
            pMove = tex1Dfetch(texture_yRookMoves,y*1024*12+cuda_yBitBoard[x]*12+threadIdx.x);
            nDst = pMove | x;				// 0x y|x  前4位=y*16， 后4位=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+20]=move;
        }
    }
    else if( blockIdx.x==3 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+2;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            x = nSrc & 0xF;// 后4位有效
            y = nSrc >> 4;// 前4位有效
            //車的橫向移動︰
            pMove = tex1Dfetch(texture_xRookMoves,x*512*12+cuda_xBitBoard[y]*12+threadIdx.x);
            nDst = (nSrc & 0xF0) | pMove;	// 0x y|x  前4位=y*16， 后4位=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+32]=move;
        }
    }
    else if( blockIdx.x==4 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+2;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            x = nSrc & 0xF;// 后4位有效
            y = nSrc >> 4;// 前4位有效
            //車的橫向移動︰
            pMove = tex1Dfetch(texture_yRookMoves,y*1024*12+cuda_yBitBoard[x]*12+threadIdx.x);
            nDst = pMove | x;				// 0x y|x  前4位=y*16， 后4位=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+44]=move;
        }
    }    
    //炮****************************************************************************
    else if( blockIdx.x==5 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+3;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            x = nSrc & 0xF;// 后4位有效
            y = nSrc >> 4;// 前4位有效
            //炮的橫向移動︰
            pMove = tex1Dfetch(texture_xCannonMoves,x*512*12+cuda_xBitBoard[y]*12+threadIdx.x);
            nDst = (nSrc & 0xF0) | pMove;	// 0x y|x  前4位=y*16， 后4位=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+56]=move;
        }
    }
    else if( blockIdx.x==6 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+3;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            x = nSrc & 0xF;// 后4位有效
            y = nSrc >> 4;// 前4位有效
            //炮的縱向移動
            pMove = tex1Dfetch(texture_yCannonMoves,y*1024*12+cuda_yBitBoard[x]*12+threadIdx.x);
            nDst = pMove | x;				// 0x y|x  前4位=y*16， 后4位=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+68]=move;
        }
    }
    else if( blockIdx.x==7 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+4;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            x = nSrc & 0xF;// 后4位有效
            y = nSrc >> 4;// 前4位有效
            //炮的橫向移動︰
            pMove = tex1Dfetch(texture_xCannonMoves,x*512*12+cuda_xBitBoard[y]*12+threadIdx.x);
            nDst = (nSrc & 0xF0) | pMove;	// 0x y|x  前4位=y*16， 后4位=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+80]=move;
        }
    }
    else if( blockIdx.x==8 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+4;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            x = nSrc & 0xF;// 后4位有效
            y = nSrc >> 4;// 前4位有效
            //炮的縱向移動
            pMove = tex1Dfetch(texture_yCannonMoves,y*1024*12+cuda_yBitBoard[x]*12+threadIdx.x);
            nDst = pMove | x;				// 0x y|x  前4位=y*16， 后4位=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+92]=move;
        }
    }
    //馬****************************************************************************
    else if( blockIdx.x==9 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+5;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            pMove = tex1Dfetch(texture_KnightMoves,nSrc*12+threadIdx.x);
            nDst = pMove;
            if( !cuda_Board[nSrc+cuda_nHorseLegTab[nDst-nSrc+256]] )//拐馬腳
            {					
                WRITE_2_MOVE;
            }
            else
            {
                move=0;
            }
           cuda_move[threadIdx.x+104]=move;
        }
    }
    else if( blockIdx.x==10 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+6;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            pMove = tex1Dfetch(texture_KnightMoves,nSrc*12+threadIdx.x);
            nDst = pMove;
            if( !cuda_Board[nSrc+cuda_nHorseLegTab[nDst-nSrc+256]] )//拐馬腳
            {					
                WRITE_2_MOVE;
            }
            else
            {
                move=0;
            }
           cuda_move[threadIdx.x+116]=move;
        }
    }
    //象****************************************************************************
    else if( blockIdx.x==11 )
    {
        if(threadIdx.x<8)
        {
            nChess=k+7;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            pMove = tex1Dfetch(texture_BishopMoves,nSrc*8+threadIdx.x);
            nDst = pMove;
            if( !cuda_Board[(nSrc+nDst)>>1] )//象眼無子
            {
                WRITE_2_MOVE;
            }
            else
            {
                move=0;
            }
           cuda_move[threadIdx.x+128]=move;
        }
    }
    else if( blockIdx.x==12 )
    {
        if(threadIdx.x<8)
        {
            nChess=k+8;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            pMove = tex1Dfetch(texture_BishopMoves,nSrc*8+threadIdx.x);
            nDst = pMove;
            if( !cuda_Board[(nSrc+nDst)>>1] )//象眼無子
            {
                WRITE_2_MOVE;
            }
            else
            {
                move=0;
            }
           cuda_move[threadIdx.x+136]=move;
        }
    }
    //士****************************************************************************
    else if( blockIdx.x==13 )
    {
        if(threadIdx.x<8)
        {
            nChess=k+9;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            pMove = tex1Dfetch(texture_GuardMoves,nSrc*8+threadIdx.x);
            nDst = pMove;
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+144]=move;
        }
    }
    else if( blockIdx.x==14 )
    {
        if(threadIdx.x<8)
        {
            nChess=k+10;
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            pMove = tex1Dfetch(texture_GuardMoves,nSrc*8+threadIdx.x);
            nDst = pMove;
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+152]=move;
        }
    }
    //兵****************************************************************************
    else if( blockIdx.x==15 )
    {
        if(threadIdx.x<4)
        {
            nChess=k+11;
            int Player;
            if(k<32){Player=0;}
            else{Player=1;}
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            //pMove = cuda_PawnMoves[Player][nSrc][(tid-160)%4];
            pMove = tex1Dfetch(texture_PawnMoves,Player*256*4+nSrc*4+threadIdx.x);
            nDst = pMove;
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+160]=move;
        }
    }  
    else if( blockIdx.x==16 )
    {
        if(threadIdx.x<4)
        {
            nChess=k+12;
            int Player;
            if(k<32){Player=0;}
            else{Player=1;}
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            //pMove = cuda_PawnMoves[Player][nSrc][(tid-160)%4];
            pMove = tex1Dfetch(texture_PawnMoves,Player*256*4+nSrc*4+threadIdx.x);
            nDst = pMove;
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+164]=move;
        }
    } 
    else if( blockIdx.x==17 )
    {
        if(threadIdx.x<4)
        {
            nChess=k+13;
            int Player;
            if(k<32){Player=0;}
            else{Player=1;}
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            //pMove = cuda_PawnMoves[Player][nSrc][(tid-160)%4];
            pMove = tex1Dfetch(texture_PawnMoves,Player*256*4+nSrc*4+threadIdx.x);
            nDst = pMove;
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+168]=move;
        }
    } 
    else if( blockIdx.x==18 )
    {
        if(threadIdx.x<4)
        {
            nChess=k+14;
            int Player;
            if(k<32){Player=0;}
            else{Player=1;}
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            //pMove = cuda_PawnMoves[Player][nSrc][(tid-160)%4];
            pMove = tex1Dfetch(texture_PawnMoves,Player*256*4+nSrc*4+threadIdx.x);
            nDst = pMove;
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+172]=move;
        }
    } 
    else if( blockIdx.x==19 )
    {
        if(threadIdx.x<4)
        {
            nChess=k+15;
            int Player;
            if(k<32){Player=0;}
            else{Player=1;}
            nSrc = cuda_Piece[nChess];// 棋子存在︰nSrc!=0
            pMove = tex1Dfetch(texture_PawnMoves,Player*256*4+nSrc*4+threadIdx.x);
            nDst = pMove;
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+176]=move;
        }
    } 
}
__global__ void cuda_null()
{

}
//呼叫cuda_MoveGen_null
void call_cudaMoveGen_null(const unsigned int nChess,int Board[256],int Piece[48],unsigned int xBitBoard[16],unsigned int yBitBoard[16],unsigned int * &ChessMove,unsigned short HistoryRecord[65535])
{
    cuda_null<<<20,numOfThreads>>>();
    cudaThreadSynchronize();
}
cudaEvent_t start_timer, stop_timer; 
float time_timer;
//呼叫call_cudatimer
float call_cudatimer(int i)
{
    if(i)
    {
        cudaEventRecord( stop_timer, 0 ); 
        cudaEventSynchronize( stop_timer ); 
        cudaEventElapsedTime( &time_timer, start_timer, stop_timer );
        cudaEventDestroy( start_timer );
        cudaEventDestroy( stop_timer );
        return time_timer;
    }
    else
    {
        cudaEventCreate(&start_timer);
        cudaEventCreate(&stop_timer); 
        cudaEventRecord( start_timer, 0 ); 
        return 0.0;
    }
}
//呼叫cuda_MoveGen
void call_cudaMoveGen(const unsigned int nChess,int Board[256],int Piece[48],unsigned int xBitBoard[16],unsigned int yBitBoard[16],unsigned int * &ChessMove,unsigned short HistoryRecord[65535])
{
    //copy Board[256]和Piece[48] 棋盤當前狀態
    cutilSafeCall(cudaMemcpyToSymbol(cuda_Board,Board,1024));
    cutilSafeCall(cudaMemcpyToSymbol(cuda_Piece,Piece,192));
    //copy  xBitBoard yBitBoard
    cutilSafeCall(cudaMemcpyToSymbol(cuda_xBitBoard,xBitBoard,64));
    cutilSafeCall(cudaMemcpyToSymbol(cuda_yBitBoard,yBitBoard,64));

    //檢查核心運行時間
<<<<<<< .mine
    int testLoop=100000;
    cudaEvent_t start1, stop1; 
    float time1;
    cudaEventCreate(&start1);
    cudaEventCreate(&stop1); 
    cudaEventRecord( start1, 0 ); 
    for(int i=0;i<testLoop;i++)
    {
    cuda_null<<<1,numOfThreads>>>();
    cudaThreadSynchronize();
    }
    cudaEventRecord( stop1, 0 ); 
    cudaEventSynchronize( stop1 ); 
    cudaEventElapsedTime( &time1, start1, stop1 );
    cudaEventDestroy( start1 );
    cudaEventDestroy( stop1 );

    cudaEvent_t start2, stop2; 
    float time2;
    cudaEventCreate(&start2);
    cudaEventCreate(&stop2); 
    cudaEventRecord( start2, 0 ); 
    for(int i=0;i<testLoop;i++)
    {
    cudaMoveGen<<<1,numOfThreads>>>(nChess,cuda_move);
    }
    cudaThreadSynchronize();
    cudaEventRecord( stop2, 0 ); 
    cudaEventSynchronize( stop2 ); 
    cudaEventElapsedTime( &time2, start2, stop2 );
    cudaEventDestroy( start2 );
    cudaEventDestroy( stop2 );
    printf("time[gpu]: %g ms\n",time2);
=======
    //int testLoop=10000;
    //double t0=(double)clock()/CLOCKS_PER_SEC;
    //for(int i=0;i<testLoop;i++)
    //{
    //cuda_null<<<1,numOfThreads>>>();
    //cudaThreadSynchronize();
    //}
    //t0=((double)clock()/CLOCKS_PER_SEC-t0);
    //printf("time[null]: %g ms\n",t0*1000);
    //double t1=(double)clock()/CLOCKS_PER_SEC;
    //for(int i=0;i<testLoop;i++)
    //{
    cudaMoveGen<<<20,32>>>(nChess,cuda_move);
    //cudaThreadSynchronize();
    //}
    //t1=((double)clock()/CLOCKS_PER_SEC-t1);
    //printf("time[gpu]: %g ms\n",t1*1000);
    //printf("***warp*** pure time: %g ms\n",(t1-t0)*1000);
>>>>>>> .r15



    //把結果copy出來
    unsigned int host_move[numOfThreads];
    cutilSafeCall(cudaMemcpy( host_move, cuda_move, 4*numOfThreads, cudaMemcpyDeviceToHost)); 
    //印出來檢驗
    //for(int i=0;i<numOfThreads;i++)
    //{
    //    printf("thread %d -> move[%d] = %u\n",i,i,host_move[i]);
    //}

    ////釋放顯示卡記憶體
    //cudaFree(cuda_move);

    for(int i=0;i<numOfThreads;i++)
    {
        if(host_move[i])
        {
            *(ChessMove++) = (HistoryRecord[host_move[i]]<<16) | host_move[i];
        }
    }
}
