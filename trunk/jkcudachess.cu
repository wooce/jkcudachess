////////////////////////////////////////////////////////////////////////////////////////////////////////////
// �����JBinghewusi.cpp                                                                                 //
// *******************************************************************************************************//
// ����H�ѳq�Τ���----�L�e���|�A����m����H�ѳq�Τ�����ĳ�n(Universal Chinese Chess Protocol�A²��ucci) //
// �@�̡J �S �w �x                                                                                        //
// ���J �����l���Ǭ�s�|                                                                            //
// �l�c�J fan_de_jun@sina.com.cn                                                                          //
//  QQ �J 83021504                                                                                        //
// *******************************************************************************************************//
// �\��J                                                                                                 //
// 1. ����x���ε{�����J�f�I                                                                              //
// 2. �z�Lucci��ĳ�P�ɭ��{�������i��q�T                                                                  //
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

    printf("*******************************�L�e���| V0.60*********************************\n");
    printf("** �@�̡J�S�w�x                                                             **\n");
    printf("** ����m����H�ѳq�Τ�����ĳ�n(Universal Chinese Chess Protocol�A²��UCCI) **\n");
    printf("** �����i�H�Ρ��H����ElephantBoard�ɭ��{��(�@�̡J����)�[���C                **\n");
    printf("** �w��ϥΡ��L�e���| V0.60������H�Ѥ���                                   **\n");	
    printf("******************************************************************************\n");
    printf("����Jucci���O......\n");

    // ��������"ucci"���O
    if(BootLine() == UCCI_COMM_UCCI)
    {
        // �M������Ҧb���ؿ�argv[0]�A�åB��"BOOK.DAT"�q�{���ʬ٪��}���w�}���w
        BackSlashPtr = strrchr(argv[0], '\\');
        if (BackSlashPtr == 0) 
            strcpy(BookFile, "BOOK.DAT");
        else
        {
            strncpy(BookFile, argv[0], BackSlashPtr + 1 - argv[0]);
            strcpy(BookFile + (BackSlashPtr + 1 - argv[0]), "BOOK.DAT");
        }

        // �ե�CSearch���A�c�y��ƪ�l�Ƥ@�Ǭ����Ѽ�
        //a.��l�Ƶ۪k�w���ͼƲ�
        //b.��l��Hash��A���o21+1=22��Hash��A64M
        //c.�M�ž��v�ҵo��
        CSearch ThisSearch;

        // ��ܤ������W�١B�����B�@�̩M�ϥΪ�
        printf("\n");
        printf("id name ����H�Ѥ����L�e���|V0.60jk��\n");
        fflush(stdout);
        printf("id copyright ���v�Ҧ�(C) 2005-2008\n");
        fflush(stdout);
        printf("id author �S�w�x(�����l���Ǭ�s�|)\n");
        fflush(stdout);
        printf("id user CUDA\n\n");
        fflush(stdout);

        // ��ܤ���ucci���O���^�X�T���A��ܤ����Ҥ�����ﶵ
        // option batch %d
        printf("option batch type check default %s\n", BoolValue[ThisSearch.bBatch]);
        fflush(stdout);

        // option debug ��������X�ԲӪ��j���T���A�ëD�u�����ոռҦ��C
        printf("option debug type check default %s\n", BoolValue[ThisSearch.Debug]);
        fflush(stdout);

        // ���w�}���w��󪺦W�١A�i���w�h�Ӷ}���w���A�Τ�����;���j�}�A�p���������ϥζ}���w�A�i�H��ȳ]����
        ThisSearch.bUseOpeningBook = ThisSearch.m_Hash.LoadBook(BookFile);
        if(ThisSearch.bUseOpeningBook)
            printf("option bookfiles type string default %s\n", BookFile);
        else
            printf("option bookfiles type string default %s\n", 0);
        fflush(stdout);

        // �ݧ��w�W��
        printf("option egtbpaths type string default null\n");
        fflush(stdout);

        // ���Hash���j�p
        printf("option hashsize type spin default %d MB\n", ThisSearch.m_Hash.nHashSize*2*sizeof(CHashRecord)/1024/1024);
        fflush(stdout);

        // �������u�{��
        printf("option threads type spin default %d\n", 0);
        fflush(stdout);

        // �����F��۵M���۪��b�^�X��
        printf("option drawmoves type spin default %d\n", ThisSearch.NaturalBouts);
        fflush(stdout);

        // �ѳW
        printf("option repetition type spin default %d 1999�~���m����H���v�ɳW�h�n\n", UCCI_REPET_CHINESERULE);
        fflush(stdout);

        // �ŵ۵���O�_���}
        printf("option pruning type check %d\n", ThisSearch);
        fflush(stdout);

        // ���Ȩ�ƪ��ϥα��p
        printf("option knowledge type check %d\n", ThisSearch);
        fflush(stdout);

        // ���w��ܩʨt�ơA�q�`��0,1,2,3�|�ӯŧO�C�����Ȩ�ƥ[��@�w�d�򤺪��H���ơA�������C�����X���ۦP���ѡC
        printf("option selectivity type spin min 0 max 3 default %d\n", ThisSearch.nSelectivity);
        fflush(stdout);

        // ���w�U�Ѫ�����A�q�`��solid(�O�u)�Bnormal(����)�Mrisky(�_�i)�T��
        printf("option style type combo var solid var normal var risky default %s\n", ChessStyle[ThisSearch.nStyle]);
        fflush(stdout);		

        // copyprotection ��ܪ��v�ˬd�T��(���b�ˬd�A���v�T�����T�Ϊ��v�T�����~)�C 
        printf("copyprotection ok\n\n");
        fflush(stdout);

        // ucciok �o�Oucci���O���̫�@���^�X�T���A��ܤ����w�g�i�J��UCCI��ĳ�q�T�����A�C
        printf("ucciok\n\n");
        fflush(stdout);


        // �]�w�зǿ�X�M��l����
        ThisSearch.OutFile = stdout;	// �зǿ�X
        ThisSearch.fen.FenToBoard(Board, Piece, ThisSearch.Player, ThisSearch.nNonCapNum, ThisSearch.nCurrentStep, "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR r - - 0 1");
        ThisSearch.InitBitBoard(ThisSearch.Player, ThisSearch.nCurrentStep);
        printf("position fen rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR r - - 0 1\n\n");
        fflush(stdout);


        // �}�l��������UCCI�R�O
        do 
        {
            IdleComm = IdleLine(Command, ThisSearch.Debug);
            switch (IdleComm) 
            {
                // isready �˴������O�_�B�_�N�����A�A��^�X�T���`�Oreadyok�A�ӫ��O�ȶȥΨ��˴������������O�����w�R�ϡ��O�_�ॿ�`�e�ǫ��O�C
                // readyok ��������B�_�N�����A(�Y�i�������O�����A)�A���ޤ����B�_�Ŷ����A�٬O��Ҫ��A�C
            case UCCI_COMM_ISREADY:
                printf("readyok\n");
                fflush(stdout);
                break;

                // stop ���_��������ҡA�j�s�X�ۡC�Z�x��ҨS���R���ɡA�N�θӫ��O�Ӥ����ҡA�M�᭫�s��J�����C
            case UCCI_COMM_STOP:
                ThisSearch.bStopThinking = 1;
                //printf("nobestmove\n");
                printf("score 0\n");
                fflush(stdout);
                break;

                // position fen �]�m�����m�ѽL���������A��fen�ӫ��wFEN�榡��Amoves�Z���򪺬O�H�Z���L���۪k
            case UCCI_COMM_POSITION:
                // �N�ɭ��ǨӪ�Fen����Ƭ��ѧ��T��
                ThisSearch.fen.FenToBoard(Board, Piece, ThisSearch.Player, ThisSearch.nNonCapNum, ThisSearch.nCurrentStep, Command.Position.szFenStr);
                ThisSearch.InitBitBoard(ThisSearch.Player, ThisSearch.nCurrentStep);

                // �N���������e�A�D�n�O���F��s�۪k�O���A�Τ_�`���˴��C
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

                // banmoves ����e�����]�m�T��A�H�ѨM�����L�k�B�z���������D�C��X�{���������ɡA�Ѥ�i�H�ޱ��ɭ��V�����o�X�T����O�C
            case UCCI_COMM_BANMOVES:
                ThisSearch.nBanMoveNum = Command.BanMoves.nMoveNum;
                for(n=0; n<Command.BanMoves.nMoveNum; n++)
                    ThisSearch.BanMoveList[n] = Move(Command.BanMoves.lpdwCoordList[n]);
                break;

                // setoption �]�m�����U�ذѼ�
            case UCCI_COMM_SETOPTION:
                switch(Command.Option.uoType) 
                {
                    // setoption batch %d
                case UCCI_OPTION_BATCH:
                    ThisSearch.bBatch = (Command.Option.Value.bCheck == TRUE);
                    printf("option batch type check default %s\n", BoolValue[ThisSearch.bBatch]);
                    fflush(stdout);
                    break;

                    // setoption debug %d ��������X�ԲӪ��j���T���A�ëD�u�����ոռҦ��C
                case UCCI_OPTION_DEBUG:
                    ThisSearch.Debug = (Command.Option.Value.bCheck == TRUE);
                    printf("option debug type check default %s\n", BoolValue[ThisSearch.Debug]);
                    fflush(stdout);
                    break;

                    // setoption bookfiles %s  ���w�}���w��󪺦W�١A�i���w�h�Ӷ}���w���A�Τ�����;���j�}�A�p���������ϥζ}���w�A�i�H��ȳ]����
                case UCCI_OPTION_BOOKFILES:
                    strcpy(BookFile, Command.Option.Value.szString);
                    printf("option bookfiles type string default %s\n", BookFile);
                    fflush(stdout);
                    break;

                    // setoption egtbpaths %s  ���w�ݧ��w��󪺦W�١A�i���w�h�Ӵݧ��w���|�A�Τ�����;���j�}�A�p���������ϥδݧ��w�A�i�H��ȳ]����
                    //case e_OptionEgtbPaths:
                    // �����ثe������}���w
                    //printf("option egtbpaths type string default null\n");
                    //fflush(stdout);
                    //break;

                    // setoption hashsize %d  �HMB�����W�wHash���j�p�A-1����������۰ʤ��oHash��C1��1024MB
                    // �H���ɭ�����Bug�A�C���]�m�����ɡA�o�өR�O���b�}���w���e��
                case UCCI_OPTION_HASHSIZE:
                    // -1MB(�۰�), 0MB(�۰�), 1MB(16), 2MB(17), 4MB(18), 8MB(19), 16MB(20), 32MB(21), 64MB(22), 128MB(23), 256MB(24), 512MB(25), 1024MB(26)
                    if( Command.Option.Value.nSpin <= 0)
                        n = 22;		// �ʬٱ��p�U�A�����۰ʤ��o(1<<22)*16=64MB�A���P�¨�U��A����U�@�b�C
                    else
                    {
                        n = 15;											// 0.5 MB = 512 KB �H�������
                        while( Command.Option.Value.nSpin > 0 )
                        {
                            Command.Option.Value.nSpin >>= 1;			// �C�����H2�A���쬰0
                            n ++;
                        }
                    }								

                    // ���[�J���s�˴����s�A�����۰ʤ��o�ɡAHash��j�p���i�Τ��s��1/2�C
                    ThisSearch.m_Hash.DeleteHashTable();					// �����ϥ�delete���M���ª�Hash��
                    ThisSearch.m_Hash.NewHashTable(n > 26 ? 26 : n, 12);	// ���������o�s��Hash��
                    printf("option hashsize type spin default %d MB\n", ThisSearch.m_Hash.nHashSize*2*sizeof(CHashRecord)/1024/1024);	// ��ܹ�ڤ��o��Hash��j�p�A���JMB
                    fflush(stdout);

                    ThisSearch.m_Hash.ClearHashTable();
                    ThisSearch.bUseOpeningBook = ThisSearch.m_Hash.LoadBook(BookFile);
                    break;

                    // setoption threads %d	      �������u�{�ơA���h�B�z���æ�B��A��
                case UCCI_OPTION_THREADS:
                    // ThisSearch.nThreads = Command.Option.Value.Spin;		// 0(auto),1,2,4,8,16,32
                    printf("option drawmoves type spin default %d\n", 0);
                    fflush(stdout);
                    break;

                    // setoption drawmoves %d	  �F��۵M���۪��^�X��:50,60,70,80,90,100�A�H���w�g�۰���Ƭ��b�^�X��
                case UCCI_OPTION_DRAWMOVES:
                    ThisSearch.NaturalBouts = Command.Option.Value.nSpin;
                    printf("option drawmoves type spin default %d\n", ThisSearch.NaturalBouts);
                    fflush(stdout);
                    break;

                    // setoption repetition %d	  �B�z�`�����ѳW�A�ثe�u���������H�ѴѳW1999��
                case UCCI_OPTION_REPETITION:
                    // ThisSearch.nRepetitionStyle = Command.Option.Value.Repetition;
                    // e_RepetitionAlwaysDraw  ���ܧ@�M
                    // e_RepetitionCheckBan    �T����N
                    // e_RepetitionAsianRule   �Ȭw�W�h
                    // e_RepetitionChineseRule ����W�h�]�ʬ١^
                    printf("option repetition type spin default %d", UCCI_REPET_CHINESERULE);
                    printf("  ���L�e���|�������ثe���1999�~���m����H���v�ɳW�h�n\n");
                    fflush(stdout);
                    break;

                    // setoption pruning %d�A���ŵۦV�e���š��O�_���}
                case UCCI_OPTION_PRUNING:
                    ThisSearch.bPruning = Command.Option.Value.ugGrade;
                    printf("option pruning type check %d\n", ThisSearch);
                    fflush(stdout);
                    break;

                    // setoption knowledge %d�A���Ȩ�ƪ��ϥ�
                case UCCI_OPTION_KNOWLEDGE:
                    ThisSearch.bKnowledge = Command.Option.Value.ugGrade;
                    printf("option knowledge type check %d\n", ThisSearch);
                    fflush(stdout);
                    break;

                    // setoption selectivity %d  ���w��ܩʨt�ơA�q�`��0,1,2,3�|�ӯŧO
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

                    // setoption style %d  ���w�U�Ѫ�����A�q�`��solid(�O�u)�Bnormal(����)�Mrisky(�_�i)�T��
                case UCCI_OPTION_STYLE:
                    ThisSearch.nStyle = Command.Option.Value.usStyle;
                    printf("option style type combo var solid var normal var risky default %s\n", ChessStyle[Command.Option.Value.usStyle]);
                    fflush(stdout);
                    break;						

                    // setoption loadbook  UCCI�ɭ�ElephantBoard�b�C���s�شѧ��ɳ��|�o�e�o�����O
                case UCCI_OPTION_LOADBOOK:
                    ThisSearch.m_Hash.ClearHashTable();
                    ThisSearch.bUseOpeningBook = ThisSearch.m_Hash.LoadBook(BookFile);

                    if(ThisSearch.bUseOpeningBook)
                        printf("option loadbook succeed. %s\n", BookFile);		// ���\
                    else
                        printf("option loadbook failed! %s\n", "Not found file BOOK.DAT");				// �S���}���w
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
                    // �T�w�`��
                case UCCI_TIME_DEPTH:
                    ThisSearch.Ponder = 2;
                    ThisSearch.MainSearch(Command.Search.DepthTime.nDepth);
                    break;

                    // �ɬq�s�J ���o�ɶ� = �Ѿl�ɶ� / �n�����B��
                case UCCI_TIME_MOVE:							
                    ThisSearch.Ponder = (IdleComm == UCCI_COMM_GOPONDER ? 1 : 0);
                    printf("%d\n", Command.Search.TimeMode.nMovesToGo);
                    ThisSearch.MainSearch(127, Command.Search.DepthTime.nTime * 1000 / Command.Search.TimeMode.nMovesToGo, Command.Search.DepthTime.nTime * 1000);
                    break;

                    // �[�ɻs�J ���o�ɶ� = �C�B�W�[���ɶ� + �Ѿl�ɶ� / 20 (�Y���]�ѧ��|�b20�B������)
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

//�I�s�H���üư}�C�[�k����
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

    //�L�X���G
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
//�w�i�}�B��
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
//�t�mGPU�s�񵲪G���O����
unsigned int* cuda_move;

__constant__ unsigned int cuda_xBitBoard[16];//16*4=64
__constant__ unsigned int cuda_yBitBoard[16];//16*4=64
__constant__ int cuda_Board[256];//256*4=1024
__constant__ int cuda_Piece[48];//48*4=192
__constant__ char cuda_nHorseLegTab[512] = {// ���L�W�q��
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
//�ƻsPreMoveGen�i�}�����k��GPU���O�����
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
//cuda MoveGen�D��
#define WRITE_2_MOVE if(pMove&&nSrc&&nDst&&!cuda_Board[nDst]){move=(nSrc<<8)|nDst;}else{move=0;}
__global__ void cudaMoveGen_warp(const unsigned int k,unsigned int* cuda_move)
{    
    //int tid=blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int  move, nSrc, nDst, x, y, nChess;
    unsigned char pMove;
    //�N��**************************************************************************
    if(blockIdx.x==0)
    {
        if(threadIdx.x<8)
        {
            nChess=k;
            nSrc = cuda_Piece[nChess];// �N�Ӧs�b�JnSrc!=0
            //pMove = cuda_KingMoves[nSrc][tid];
            pMove = tex1Dfetch(texture_KingMoves,nSrc*8+threadIdx.x);
            nDst = pMove;
            WRITE_2_MOVE;
           cuda_move[threadIdx.x]=move;
        }
    }
    //��****************************************************************************
    else if( blockIdx.x==1 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+1;
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            x = nSrc & 0xF;// �Z4�즳��
            y = nSrc >> 4;// �e4�즳��
            //������V���ʡJ
            pMove = tex1Dfetch(texture_xRookMoves,x*512*12+cuda_xBitBoard[y]*12+threadIdx.x);
            nDst = (nSrc & 0xF0) | pMove;	// 0x y|x  �e4��=y*16�A �Z4��=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+8]=move;
        }
    }
    else if( blockIdx.x==2 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+1;
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            x = nSrc & 0xF;// �Z4�즳��
            y = nSrc >> 4;// �e4�즳��
            //������V���ʡJ
            pMove = tex1Dfetch(texture_yRookMoves,y*1024*12+cuda_yBitBoard[x]*12+threadIdx.x);
            nDst = pMove | x;				// 0x y|x  �e4��=y*16�A �Z4��=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+20]=move;
        }
    }
    else if( blockIdx.x==3 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+2;
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            x = nSrc & 0xF;// �Z4�즳��
            y = nSrc >> 4;// �e4�즳��
            //������V���ʡJ
            pMove = tex1Dfetch(texture_xRookMoves,x*512*12+cuda_xBitBoard[y]*12+threadIdx.x);
            nDst = (nSrc & 0xF0) | pMove;	// 0x y|x  �e4��=y*16�A �Z4��=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+32]=move;
        }
    }
    else if( blockIdx.x==4 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+2;
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            x = nSrc & 0xF;// �Z4�즳��
            y = nSrc >> 4;// �e4�즳��
            //������V���ʡJ
            pMove = tex1Dfetch(texture_yRookMoves,y*1024*12+cuda_yBitBoard[x]*12+threadIdx.x);
            nDst = pMove | x;				// 0x y|x  �e4��=y*16�A �Z4��=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+44]=move;
        }
    }    
    //��****************************************************************************
    else if( blockIdx.x==5 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+3;
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            x = nSrc & 0xF;// �Z4�즳��
            y = nSrc >> 4;// �e4�즳��
            //������V���ʡJ
            pMove = tex1Dfetch(texture_xCannonMoves,x*512*12+cuda_xBitBoard[y]*12+threadIdx.x);
            nDst = (nSrc & 0xF0) | pMove;	// 0x y|x  �e4��=y*16�A �Z4��=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+56]=move;
        }
    }
    else if( blockIdx.x==6 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+3;
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            x = nSrc & 0xF;// �Z4�즳��
            y = nSrc >> 4;// �e4�즳��
            //�����a�V����
            pMove = tex1Dfetch(texture_yCannonMoves,y*1024*12+cuda_yBitBoard[x]*12+threadIdx.x);
            nDst = pMove | x;				// 0x y|x  �e4��=y*16�A �Z4��=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+68]=move;
        }
    }
    else if( blockIdx.x==7 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+4;
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            x = nSrc & 0xF;// �Z4�즳��
            y = nSrc >> 4;// �e4�즳��
            //������V���ʡJ
            pMove = tex1Dfetch(texture_xCannonMoves,x*512*12+cuda_xBitBoard[y]*12+threadIdx.x);
            nDst = (nSrc & 0xF0) | pMove;	// 0x y|x  �e4��=y*16�A �Z4��=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+80]=move;
        }
    }
    else if( blockIdx.x==8 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+4;
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            x = nSrc & 0xF;// �Z4�즳��
            y = nSrc >> 4;// �e4�즳��
            //�����a�V����
            pMove = tex1Dfetch(texture_yCannonMoves,y*1024*12+cuda_yBitBoard[x]*12+threadIdx.x);
            nDst = pMove | x;				// 0x y|x  �e4��=y*16�A �Z4��=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+92]=move;
        }
    }
    //��****************************************************************************
    else if( blockIdx.x==9 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+5;
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            pMove = tex1Dfetch(texture_KnightMoves,nSrc*12+threadIdx.x);
            nDst = pMove;
            if( !cuda_Board[nSrc+cuda_nHorseLegTab[nDst-nSrc+256]] )//�䰨�}
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
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            pMove = tex1Dfetch(texture_KnightMoves,nSrc*12+threadIdx.x);
            nDst = pMove;
            if( !cuda_Board[nSrc+cuda_nHorseLegTab[nDst-nSrc+256]] )//�䰨�}
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
    //�H****************************************************************************
    else if( blockIdx.x==11 )
    {
        if(threadIdx.x<8)
        {
            nChess=k+7;
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            pMove = tex1Dfetch(texture_BishopMoves,nSrc*8+threadIdx.x);
            nDst = pMove;
            if( !cuda_Board[(nSrc+nDst)>>1] )//�H���L�l
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
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            pMove = tex1Dfetch(texture_BishopMoves,nSrc*8+threadIdx.x);
            nDst = pMove;
            if( !cuda_Board[(nSrc+nDst)>>1] )//�H���L�l
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
    //�h****************************************************************************
    else if( blockIdx.x==13 )
    {
        if(threadIdx.x<8)
        {
            nChess=k+9;
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
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
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            pMove = tex1Dfetch(texture_GuardMoves,nSrc*8+threadIdx.x);
            nDst = pMove;
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+152]=move;
        }
    }
    //�L****************************************************************************
    else if( blockIdx.x==15 )
    {
        if(threadIdx.x<4)
        {
            nChess=k+11;
            int Player;
            if(k<32){Player=0;}
            else{Player=1;}
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
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
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
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
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
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
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
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
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
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
    //�N��**************************************************************************
    if(blockIdx.x==0)
    {
        if(threadIdx.x<8)
        {
            nChess=k;
            nSrc = cuda_Piece[nChess];// �N�Ӧs�b�JnSrc!=0
            //pMove = cuda_KingMoves[nSrc][tid];
            pMove = tex1Dfetch(texture_KingMoves,nSrc*8+threadIdx.x);
            nDst = pMove;
            WRITE_2_MOVE;
           cuda_move[threadIdx.x]=move;
        }
    }
    //��****************************************************************************
    else if( blockIdx.x==1 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+1;
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            x = nSrc & 0xF;// �Z4�즳��
            y = nSrc >> 4;// �e4�즳��
            //������V���ʡJ
            pMove = tex1Dfetch(texture_xRookMoves,x*512*12+cuda_xBitBoard[y]*12+threadIdx.x);
            nDst = (nSrc & 0xF0) | pMove;	// 0x y|x  �e4��=y*16�A �Z4��=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+8]=move;
        }
    }
    else if( blockIdx.x==2 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+1;
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            x = nSrc & 0xF;// �Z4�즳��
            y = nSrc >> 4;// �e4�즳��
            //������V���ʡJ
            pMove = tex1Dfetch(texture_yRookMoves,y*1024*12+cuda_yBitBoard[x]*12+threadIdx.x);
            nDst = pMove | x;				// 0x y|x  �e4��=y*16�A �Z4��=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+20]=move;
        }
    }
    else if( blockIdx.x==3 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+2;
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            x = nSrc & 0xF;// �Z4�즳��
            y = nSrc >> 4;// �e4�즳��
            //������V���ʡJ
            pMove = tex1Dfetch(texture_xRookMoves,x*512*12+cuda_xBitBoard[y]*12+threadIdx.x);
            nDst = (nSrc & 0xF0) | pMove;	// 0x y|x  �e4��=y*16�A �Z4��=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+32]=move;
        }
    }
    else if( blockIdx.x==4 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+2;
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            x = nSrc & 0xF;// �Z4�즳��
            y = nSrc >> 4;// �e4�즳��
            //������V���ʡJ
            pMove = tex1Dfetch(texture_yRookMoves,y*1024*12+cuda_yBitBoard[x]*12+threadIdx.x);
            nDst = pMove | x;				// 0x y|x  �e4��=y*16�A �Z4��=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+44]=move;
        }
    }    
    //��****************************************************************************
    else if( blockIdx.x==5 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+3;
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            x = nSrc & 0xF;// �Z4�즳��
            y = nSrc >> 4;// �e4�즳��
            //������V���ʡJ
            pMove = tex1Dfetch(texture_xCannonMoves,x*512*12+cuda_xBitBoard[y]*12+threadIdx.x);
            nDst = (nSrc & 0xF0) | pMove;	// 0x y|x  �e4��=y*16�A �Z4��=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+56]=move;
        }
    }
    else if( blockIdx.x==6 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+3;
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            x = nSrc & 0xF;// �Z4�즳��
            y = nSrc >> 4;// �e4�즳��
            //�����a�V����
            pMove = tex1Dfetch(texture_yCannonMoves,y*1024*12+cuda_yBitBoard[x]*12+threadIdx.x);
            nDst = pMove | x;				// 0x y|x  �e4��=y*16�A �Z4��=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+68]=move;
        }
    }
    else if( blockIdx.x==7 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+4;
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            x = nSrc & 0xF;// �Z4�즳��
            y = nSrc >> 4;// �e4�즳��
            //������V���ʡJ
            pMove = tex1Dfetch(texture_xCannonMoves,x*512*12+cuda_xBitBoard[y]*12+threadIdx.x);
            nDst = (nSrc & 0xF0) | pMove;	// 0x y|x  �e4��=y*16�A �Z4��=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+80]=move;
        }
    }
    else if( blockIdx.x==8 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+4;
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            x = nSrc & 0xF;// �Z4�즳��
            y = nSrc >> 4;// �e4�즳��
            //�����a�V����
            pMove = tex1Dfetch(texture_yCannonMoves,y*1024*12+cuda_yBitBoard[x]*12+threadIdx.x);
            nDst = pMove | x;				// 0x y|x  �e4��=y*16�A �Z4��=x
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+92]=move;
        }
    }
    //��****************************************************************************
    else if( blockIdx.x==9 )
    {
        if(threadIdx.x<12)
        {
            nChess=k+5;
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            pMove = tex1Dfetch(texture_KnightMoves,nSrc*12+threadIdx.x);
            nDst = pMove;
            if( !cuda_Board[nSrc+cuda_nHorseLegTab[nDst-nSrc+256]] )//�䰨�}
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
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            pMove = tex1Dfetch(texture_KnightMoves,nSrc*12+threadIdx.x);
            nDst = pMove;
            if( !cuda_Board[nSrc+cuda_nHorseLegTab[nDst-nSrc+256]] )//�䰨�}
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
    //�H****************************************************************************
    else if( blockIdx.x==11 )
    {
        if(threadIdx.x<8)
        {
            nChess=k+7;
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            pMove = tex1Dfetch(texture_BishopMoves,nSrc*8+threadIdx.x);
            nDst = pMove;
            if( !cuda_Board[(nSrc+nDst)>>1] )//�H���L�l
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
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            pMove = tex1Dfetch(texture_BishopMoves,nSrc*8+threadIdx.x);
            nDst = pMove;
            if( !cuda_Board[(nSrc+nDst)>>1] )//�H���L�l
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
    //�h****************************************************************************
    else if( blockIdx.x==13 )
    {
        if(threadIdx.x<8)
        {
            nChess=k+9;
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
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
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
            pMove = tex1Dfetch(texture_GuardMoves,nSrc*8+threadIdx.x);
            nDst = pMove;
            WRITE_2_MOVE;
           cuda_move[threadIdx.x+152]=move;
        }
    }
    //�L****************************************************************************
    else if( blockIdx.x==15 )
    {
        if(threadIdx.x<4)
        {
            nChess=k+11;
            int Player;
            if(k<32){Player=0;}
            else{Player=1;}
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
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
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
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
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
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
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
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
            nSrc = cuda_Piece[nChess];// �Ѥl�s�b�JnSrc!=0
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
//�I�scuda_MoveGen_null
void call_cudaMoveGen_null(const unsigned int nChess,int Board[256],int Piece[48],unsigned int xBitBoard[16],unsigned int yBitBoard[16],unsigned int * &ChessMove,unsigned short HistoryRecord[65535])
{
    cuda_null<<<20,numOfThreads>>>();
    cudaThreadSynchronize();
}
cudaEvent_t start_timer, stop_timer; 
float time_timer;
//�I�scall_cudatimer
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
//�I�scuda_MoveGen
void call_cudaMoveGen(const unsigned int nChess,int Board[256],int Piece[48],unsigned int xBitBoard[16],unsigned int yBitBoard[16],unsigned int * &ChessMove,unsigned short HistoryRecord[65535])
{
    //copy Board[256]�MPiece[48] �ѽL��e���A
    cutilSafeCall(cudaMemcpyToSymbol(cuda_Board,Board,1024));
    cutilSafeCall(cudaMemcpyToSymbol(cuda_Piece,Piece,192));
    //copy  xBitBoard yBitBoard
    cutilSafeCall(cudaMemcpyToSymbol(cuda_xBitBoard,xBitBoard,64));
    cutilSafeCall(cudaMemcpyToSymbol(cuda_yBitBoard,yBitBoard,64));

    //�ˬd�֤߹B��ɶ�
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



    //�⵲�Gcopy�X��
    unsigned int host_move[numOfThreads];
    cutilSafeCall(cudaMemcpy( host_move, cuda_move, 4*numOfThreads, cudaMemcpyDeviceToHost)); 
    //�L�X������
    //for(int i=0;i<numOfThreads;i++)
    //{
    //    printf("thread %d -> move[%d] = %u\n",i,i,host_move[i]);
    //}

    ////������ܥd�O����
    //cudaFree(cuda_move);

    for(int i=0;i<numOfThreads;i++)
    {
        if(host_move[i])
        {
            *(ChessMove++) = (HistoryRecord[host_move[i]]<<16) | host_move[i];
        }
    }
}
