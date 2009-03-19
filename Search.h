////////////////////////////////////////////////////////////////////////////////////////////////////////////
// �Y���JSearch.h                                                                                       //
// *******************************************************************************************************//
// ����H�ѳq�Τ���----�L�e���|�A����m����H�ѳq�Τ�����ĳ�n(Universal Chinese Chess Protocol�A²��ucci) //
// �@�̡J �S �w �x                                                                                        //
// ���J �����l���Ǭ�s�|                                                                            //
// �l�c�J fan_de_jun@sina.com.cn                                                                          //
//  QQ �J 83021504                                                                                        //
// *******************************************************************************************************//
// �\��J                                                                                                 //
// 1. �@��CMoveGen���l���A�~�Ӥ������ƾڡA�p�ѽL�B�Ѥl�B�۪k���C                                          //
// 2. �����ɭ����ƾڡA��l�Ƭ��j���ݭn���T���C                                                            //
// 3. �ե�CHashTable���A����M�M�P�۪k�C                                                                  //
// 4. �ĥΫ_�w�k��۪k�Ƨ�                                                                                //
// 5. �D���j����� MainSearch()                                                                           //
// 6. �ڸ`�I�j������ RootSarch()                                                                          //
// 7. �իپ�j����k AlphaBetaSearch()                                                                    //
// 8. �I�R�j����k QuiescenceSearch()                                                                     //
// 9. ��_1999�~���m����H���v�ɳW�h�n��{�`���˴�                                                        //
//10. �`������^�Ȱ��D�B�PHash��Ĭ���D�C���L�e���|���i��`�J��s�A�W��@�m�C                            //
//11. �Q��Hash������D����                                                                                //
//12. �j���ɶ�������                                                                                      //
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


// CSearch��
class CSearch : public CMoveGen
{
public:
	CSearch(void);
	~CSearch(void);

// �ݩ�
	FILE *OutFile;
	CFenBoard fen;

	int Player;								// ��e���ʤ�	
	int MaxDepth;							// �̤j�j���`��
	unsigned long BitPieces;				// 32���Ѥl����ϡJ0-15���´ѡA16-31������  0 11 11 11 11 11 11111 0 11 11 11 11 11 11111
	
	CHashTable m_Hash;			// ���ƪ�
	CEvaluation m_Evalue;

	// �Pucci��ĳ�q�T���ܶq
	int bPruning;							// �O�_���\�ϥ�NullMove
	int bKnowledge;							// ���Ȩ�ƪ��ϥ�
	int nSelectivity;						// ��ܩʨt�ơA�q�`��0,1,2,3�|�ӯŧO�A�ʬ٬�0
	int Debug;								// bDegug = ~0, ��X�ԲӪ��j���T��
	int SelectMask;							// ��ܩʨt��
	int nStyle;								// �O�u(0)�B���q(1)�B�_�i(2)
	int Ponder;								// �Z�x���
	int bStopThinking;						// ������
	int QhMode;								// �����O�_�ϥβL����ĳ�ק_
	int bBatch;								// �M�Z�x��Ҫ��ɶ���������
	int StyleShift;
	long nMinTimer, nMaxTimer;				// ������Үɶ�
	int bUseOpeningBook;					// �O�_�������ϥζ}���w
	int nMultiPv;							// �D�n�ܨҪ��ƥ�
	int nBanMoveNum;						// �T��ƥ�
	CChessMove BanMoveList[111];			// �T�ⶤ�C
	int NaturalBouts;						// �۵M����
	
	
	unsigned int StartTimer, FinishTimer;	// �j���ɶ�
	unsigned int nNonCapNum;				// ���k���C���Y�l���ƥءA>=120(60�^�X)���M�ѡA>=5�i��X�{�`��
	unsigned int nStartStep;
	unsigned int nCurrentStep;				// ��e���ʪ��O���Ǹ�
	
	unsigned int nPvLineNum;
	CChessMove PvLine[64];					// �D������u
	CChessMove StepRecords[256];			// ���k�O��
	unsigned long nZobristBoard[256];
	

	int nFirstLayerMoves;
	CChessMove FirstLayerMoves[111];		// fen C8/3P1P3/3kP1N2/5P3/4N1P2/7R1/1R7/4B3B/3KA4/2C6 r - - 0 1	// �N�x����    way = 111;
											// fen C8/3P1P3/4k1N2/3P1P3/4N1P2/7R1/1R7/4B3B/3KA4/2C6 r - - 0 1	// �D�N�x����  way = 110;


// ����
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

	//char FenBoard[2048];	// �ǻ��ѧ����r����C�r���ꥲ���������A�_�h��^�X�ƤӤj�ɡAmoves�W�X�A�P�ѽL�ۤ��괫�ɷ|�o�ͦ��`���C


// �ާ@
public:
	void InitBitBoard(const int Player, const int nCurrentStep);		// ��l�ƴѽL�ݭn���Ҧ��ƾڡA��^�ڤ�Q�N�x���лx�C

	int MovePiece(const CChessMove move);	 //&---�i�H�j�T�״��ɷj���t�סA�P�ƻs���N�X�t�׬۵��Ainline���_�@�ΡC
	void UndoMove(void);

	void BubbleSortMax(CChessMove *ChessMove, int w, int way);	//�_�w�ƧǡJ�u�ݹM���@���A�M��̤j�ȡA��^�̫�@���洫�O������m

	int MainSearch(int nDepth, long nProperTimer=0, long nLimitTimer=0);
	int RootSearch(int depth, int alpha, int beta);
	int AlphaBetaSearch(int depth, int bDoCut, CKillerMove &KillerTab, int alpha, int beta);
	int QuiescenceSearch(int depth, int alpha, int beta);	
	
	int RepetitionDetect(int nRepeatNum=3);					// �`���˴�
	int LoopValue(int Player, int ply, int nLoopStyle);		// �`������

	//int Evaluation(int player);

	int IsBanMove(CChessMove move);
	void GetPvLine(void);
	void PopupInfo(int depth, int score, int Debug=0);
	int Interrupt(void);


	char *GetStepName(const CChessMove ChessMove, int *Board) const;
	void SaveMoves(char *FileName);
};
