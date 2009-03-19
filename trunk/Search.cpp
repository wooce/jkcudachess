////////////////////////////////////////////////////////////////////////////////////////////////////////////
// �����JSearch.cpp                                                                                     //
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


#include <time.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "ucci.h"
#include "Search.h"

static const unsigned int nAdaptive_R = 3;
static const unsigned int nVerified_R = 3;
static const unsigned int BusyCounterMask = 4095;
static const unsigned int BitAttackPieces = 0xF87EF87E;    //11111000011111101111100001111110 �L�L�L�L�L�h�h�H�H�������������N������h�h�۬۰�������������


CSearch::CSearch(void)
{	
	// ��l�ƪŵ�
	StepRecords[0] = 0;	
	
	bPruning = 1 ;							// ���\�ϥ�NullMove
	nSelectivity = 0;						// ��ܩʨt�ơA�q�`��0,1,2,3�|�ӯŧO
	Debug = 0;								// bDegug = 0, ��X²����n���j���T��
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

	// ��l��Hash��A���o21+1=22��Hash��A64MB
	m_Hash.NewHashTable(22, 12);
	m_Hash.ClearHashTable();

	// ��l�ƾ��v��
	UpdateHistoryRecord( 0 );
}

CSearch::~CSearch(void)
{
	m_Hash.DeleteHashTable();
}

// �P�_���ʬO�_���T��
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

//�`�׭��N�j��
int CSearch::MainSearch(int nDepth, long nProperTimer, long nLimitTimer)
{
	// ��l�Ƥ@�Ǧ��Ϊ��ܶq
	int w, MoveStr, score=0;
	nPvLineNum = PvLine[0] = 0;
	
	// �o���ܶq�Τ_���շj����ʯ઺�U�ذѼ�
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
	// �o���ܶq�Τ_���շj����ʯ઺�U�ذѼ�


	//�@�B���o�j���ɶ�
	StartTimer = clock();
	nMinTimer = StartTimer + unsigned int(nProperTimer*0.618f);
	nMaxTimer = unsigned int(nProperTimer*1.618f);
	if(nMaxTimer > nLimitTimer)
		nMaxTimer = nLimitTimer;
	nMaxTimer += StartTimer;
	bStopThinking = 0;


	//�G�B��X��e����
	fen.BoardToFen(Board, Player, nNonCapNum, nCurrentStep, StepRecords);
	fprintf(OutFile, "info BoardToFen: %s\n", fen.FenStr);
	fflush(OutFile);


	//�T�B�b�}���w���i��j��
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


	//�|�B�Y���Q�N�x�A��^�Y�N�Ӫ����k�B�פ�j���C
	// �D�j���ҵ{�����N�x�˴��A���|�X�{�o�˪������A�D�n�w��S�����C
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


	//���B���N�[�`�j��
	nStartStep = nCurrentStep;		// �}�l�j���ɪ��b�^�X�ơC

	for(w=1; w<=nDepth; w++)
	{	
		MaxDepth = w;
		nFirstLayerMoves = 0;

		// alpha-beta�j��
		score = RootSearch(MaxDepth, -WINSCORE, WINSCORE);

		// �L�k�ѱN�Ϊ̥u���@�ӦX�z�����ۡA�ߧY����j��
		if( nFirstLayerMoves <= 1 )
			break;

		// �Y�j��פ��ҡA����j��
		if(bStopThinking)
			break;

		// �L�Z�x��ҮɡA�Y�ɶ��w�g�F��W�w�ɶ����@�b�A�A�j���@�h���ɶ��i�ण���A����j���C
		if(!Ponder && clock()>nMinTimer)
			break;

		// �b�W�w���`�פ��A�J����ѡA�����ҡC
		if(score<-MATEVALUE || score>MATEVALUE)
			break;
	}
		

	//���B��^�j�����G
	// �Y���X�k�����ʡA��X bestmove %.4s �M ponder %.4s  �H�θԲӪ��j���T���C
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
	// �X�{�`���A���s�b�X�k�����ʡA��^score�C�N���۵����C���C
	else
	{
		fprintf(OutFile, "depth %d score %d\n", MaxDepth, score);
		fflush(OutFile);
		fprintf(OutFile, "nobestmove\n");
		fflush(OutFile);
	}
	//fprintf(OutFile, "\n\n");
	//fflush(OutFile);


	//�C�B�M��Hash��M���v�ҵo��
	StartTimer = clock() - StartTimer;
	m_Hash.ClearHashTable( 2 );
	SaveMoves("SearchInfo.txt");	
	UpdateHistoryRecord( 4 );
	nBanMoveNum = 0;

	return(score);
}

// �Ĥ@�h���j����k
int CSearch::RootSearch(int depth, int alpha, int beta)
{
	nTreeNodes ++;	// �έp��K�`�I	

	int score;
	int w, way, nCaptured;	
		
	const int  ply = nCurrentStep - nStartStep;			                    // �����e���^�X��
        const unsigned int nNonCaptured = nNonCapNum;
	const int nChecked = (StepRecords[nCurrentStep-1] & 0xFF000000) >> 24;	// �q���k���C(�����ܶq)�����(�ڤ�)�N�x�лx
	
	CChessMove ThisMove, BestMove = 0;									// ��l�Ƭ��ŵۡC�Y�Φ��x���A�i�H��^���ŵۡC
	CChessMove HashMove = 0;												// HashMove
	CKillerMove SubKillerTab;
	SubKillerTab.MoveNum = 0;

	int HashFlag = HashAlpha;												// Hash�лx
	int nBestValue = ply-WINSCORE;											// �j�����f��^�����p���j��
	nFirstLayerMoves = 0;
	
	//�@�B�M�ѵ��šJ�ħ����觡���s�b�������L�e�������Ѥl
	if(!(BitPieces & BitAttackPieces))
		return m_Evalue.Evaluation(Player);

	
	//�G�BHash����Adepth=127�A�G�ӥû����|�R��(���F�N�x����)�A�u�O���F�o��HashMove�C	
	int nHashValue = m_Hash.ProbeHash(HashMove, alpha, beta, 127, ply, Player);

	
	//�T�BHashMove�ҵo��k
	if(nHashValue == INT_MAX && !IsBanMove(HashMove) )
	{
		nHashMoves ++;
		ThisMove = HashMove;
			
		assert(Board[ThisMove & 0xFF]!=16 && Board[ThisMove & 0xFF]!=32);			// �����ӥX�{�A�_�h�����~�A�z�L�N�x�˴��A���e�@�B�L�o�ճo�����k
		MovePiece( ThisMove );
			if(Checked(1-Player))													// �ڤ�Q�N�x�A�U�@�B��訫�A�L�k���ϡA��^���Ѫ�����.�K���@�h�j��
				score = ply+1-WINSCORE;												// ���ӧQ�A�ڤ訫�F�@�B��ѡC�p�����ӭ����A�h�O�ۤv�e�W�����N�x(���Ӳέp�@�U�A�o�˪����ʾ��v�o�����ӫܧC�A�p���ֵL�Ī��B��)
			else
			{
				StepRecords[nCurrentStep-1] |= Checking(Player) << 24;				// ���Q�N�x�A�ǰe��U�@�h
				score = -AlphaBetaSearch(depth-1, 0, SubKillerTab, -beta, -alpha);	// HashMove�����\�ϥΡ��ŵ۵���Ρ����v���
			}
		UndoMove();					// ��_���ʡA��_���ʤ�A��_�@��
		nNonCapNum = nNonCaptured;	// ��_��Ӫ��L���l�ѨB�ƥ�
		
		// Fail-Soft Alpha-Beta MainSearch�]Hash�MMTDf��������k�^
		// �ĥΥ����f�j���ɡA�Ĥ@�h�û����|����beta�I�_
		if( score >= beta )				// Beta�ŪK
		{
			nHashCuts ++;

			if( HistoryRecord[ThisMove] > 64000 )		// �y�X�l�a�J65535-64000=1535
				UpdateHistoryRecord(1);
			HistoryRecord[ThisMove] += depth*depth;

			m_Hash.RecordHash(ThisMove, score, HashBeta, depth, ply, Player);
			return score;
		}

		if(score > nBestValue)
		{
			nBestValue = score;
			BestMove = ThisMove;

			if( score > alpha )				// �Y�p���f�C
			{
				alpha = score;
				HashFlag = HashPv;

				// �O���Ĥ@�h�����k
				m_Hash.RecordHash(BestMove, nBestValue, HashFlag, depth, ply, Player);
				PopupInfo(depth, score);
			}
		}		
		
		// �O���Ĥ@�h���Ҧ��۪k
		if( score < -MATEVALUE )
			BanMoveList[ nBanMoveNum ++ ] = ThisMove;							// �T��۪k
		else
			FirstLayerMoves[ nFirstLayerMoves ++ ] = ThisMove | (score<<16);	// �X�z�۪k
	}

	
	//�|�B���ͩҦ��X�k������
	//1.�N�x����  �J���ͱN�x�k�׵۪k�F
	//2.�D�N�x�����J�Y�l�۪k�M�D�Y�l�۪k�A�Y�l�۪k���[���v�o���A���������v�ҵo�B�z�C
	CChessMove ChessMove[111];
	if( nChecked )
		way = CheckEvasionGen(Player, nChecked, ChessMove);					// ���Ͱk�ױN�x���۪k
	else
	{
		way  = CapMoveGen(Player, ChessMove);								// ���ͩҦ����Y�l����
		for(w=0; w<way; w++)
			ChessMove[w] += HistoryRecord[ChessMove[w] & 0xFFFF] << 16;		// �Y�l�۪k + ���v�ҵo
		way += MoveGenerator(Player, ChessMove+way);						// ���ͩҦ��D�Y�l����
	}

	
	//���BAlpha-Beta�j����k
	// 1. �_�w�ƧǬD��̤j��(Bubble Sort for Max Value)
	// 2. �L�oHashMove
	// 3. �D����j��(PVS)(Principal Variation Search)
	// 4. Fail-Soft Alpha-Beta Search
	int nChecking;
	for(w=0; w<way; w++)
	{
		BubbleSortMax(ChessMove, w, way);
		ThisMove = ChessMove[w] & 0xFFFF;

		// �L�oHashMove�M�T��۪k
		if( ThisMove==HashMove || IsBanMove(ThisMove) )
			continue;
		
		assert(Board[ThisMove & 0xFF]!=16 && Board[ThisMove & 0xFF]!=32);		// �����ӥX�{�A�_�h�����~�A�z�L�N�x�˴��A���e�@�B�L�o�ճo�����k
		nCaptured = MovePiece( ThisMove );										// �`�N�J���ʦZPlayer�w�g��ܹ��A�U�����P�_���n�X���C���޳o�˫ܧO��A���b�䥦�a��ܤ�K�A�ڥ����κޤF
			if(Checked(1-Player))												// �ڤ�Q�N�x�A�U�@�B��訫�A�L�k���ϡA��^���Ѫ�����.�K���@�h�j��
				score = ply+1-WINSCORE;											// ���ӧQ�A�ڤ訫�F�@�B��ѡC�p�����ӭ����A�h�O�ۤv�e�W�����N�x(���Ӳέp�@�U�A�o�˪����ʾ��v�o�����ӫܧC�A�p���ֵL�Ī��B��)
			else
			{
				nChecking = Checking(Player) << 24;								// ���Q�N�x�A�ǰe��U�@�h
				StepRecords[nCurrentStep-1] |= nChecking;

				// ���v�ŪK(History Alpha-Beta Pruning)
				if(    nFirstLayerMoves >= 12      		//a. HistoryMoves==6	// 6���F�O�I
					&& !nChecked						//b. �ڤ�Q�N�x�ɡA�����\�ϥ�History Pruning
				 	&& !nChecking						//c. ���N�x�ɡA�����\�ϥ�History Pruning�A�y�C�A�����o���A�����ണ�e�X�B�o�{���ѡC
					&& !nCaptured						//d. �D�Y�l����
				  )
				{
					score = -AlphaBetaSearch(depth-2, bPruning, SubKillerTab, -alpha-1, -alpha);	// ��depth-2�i��j���A���\�ϥαa���I���ŪK
					if( score > alpha )	
						score = -AlphaBetaSearch(depth-1, 0, SubKillerTab, -beta, -alpha);			// ��depth-1���alpha�A�����\���ŵ۵���Ρ����v���
				}
				// �H�U�T��ϥαa���I���ŪK
				else if( nFirstLayerMoves )
				{
					score = -AlphaBetaSearch(depth-1, bPruning, SubKillerTab, -alpha-1, -alpha);
					if( score>alpha ) //&& score<beta)	
						score = -AlphaBetaSearch(depth-1, 0, SubKillerTab, -beta, -alpha);
				}
				else
					score = -AlphaBetaSearch(depth-1, 0, SubKillerTab, -beta, -alpha);	
			}			
		UndoMove();					// ��_���ʡA��_���ʤ�A��_�@��
		nNonCapNum = nNonCaptured;	// ��_��Ӫ��L���l�ѨB�ƥ�
		
		
		//********************************************************************************************************
		//Fail-Soft Alpha-Beta MainSearch�]Hash�MMTDf��������k�^
		
		if(score > nBestValue)
		{
			nBestValue = score;
			BestMove = ThisMove;

			if( score >= beta )				// Beta�ŪK
			{
				nBetaNodes++;
				HashFlag = HashBeta;				
				break;
			}
			if( score > alpha )				// �Y�p���f�C
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
		

		// �O���Ĥ@�h���Ҧ��۪k
		if( score < -MATEVALUE )
			BanMoveList[ nBanMoveNum ++ ] = ThisMove;							// �T��۪k
		else
			FirstLayerMoves[ nFirstLayerMoves ++ ] = ThisMove | (score<<16);	// �X�z�۪k

		// �Y�S���ҰʦZ�x��ҥ\��A�����F��F�̤p�j���ɶ��A�h�j���@�h�w�g���i��A�����o�X�פ�j�����H��
		if(nFirstLayerMoves && !Ponder && clock() > nMinTimer)
		{
			bStopThinking = 1;
			break;
		}
	}


	//�Q�G�B���v�ҵo�B����ҵo�B�O��Hash��

	if(HashFlag != HashAlpha)
	{
		if( HistoryRecord[BestMove] > 64000 )		// �y�X�l�a�J65535-64000=1535
			UpdateHistoryRecord(1);
		HistoryRecord[BestMove] += depth*depth;
	}
	
	m_Hash.RecordHash( BestMove, nBestValue, HashFlag, depth, ply, Player );
	return nBestValue;
}


int CSearch::AlphaBetaSearch(int depth, int bDoCut, CKillerMove &KillerTab, int alpha, int beta)
{
	const int  ply = nCurrentStep - nStartStep;			// �����e���^�X��
	int score, nBestValue = ply-WINSCORE;				// �j�����f��^�����p���j��
        int NextKiller, bHistoryPruning;
        unsigned int ThereIsKillers;

	
	//�@�B�`���P�_
	//1.���ʤ�J�o�̧P�_�W�@�B�H�e�O�_�c���`���A�̫᪺���ʬO���C
	//2.�^�X�ơJ�J�M�P�_�W�@�B�A�۵Mply = ply-1�C���`�N�o�ӲӸ`��^�ȱN���|��T�C
	//3.��  ���JnNonCapNum=0,1,2,3�ɡA���i��c���`���A�p���j�j��֤F�`���P�_�����ơC
	//4.�PHash��Ĭ�J
	if( nNonCapNum>=4 )	
	{
		score = RepetitionDetect();
		if(score)
			return LoopValue(Player, ply, score);
	}

	//�G�B���ѵ��šJMate-Distance Pruning
	if(nBestValue >= beta)
		return nBestValue;


	//�T�B�M�ѵ��šJ�ħ����觡���s�b�������L�e�������Ѥl
	if(!(BitPieces & BitAttackPieces))
		return m_Evalue.Evaluation(Player);


	//�|�B�N�x�X�i & �I�l�X�i
	const unsigned int nChecked = (StepRecords[nCurrentStep-1] & 0xFF000000) >> 24;	 // �q���k���C(�����ܶq)�����(�ڤ�)�N�x�лx
	int Extended = 0;
	if( nChecked && ply<MaxDepth+16)								 // �N�x�X�i�A�h�j���@�h�A����S���N�x����
	{
		depth++;
		Extended = 1;
	}
	// �I�l�X�i�A�s���B���l�A�ȭ��_�������I���C
	// ��l�X�i�i��|�ް_�@�ǧ����L�k�h�X�C
	// �g8�p�ɪ��ˬd�o�{�A��]���b�o�̡A�O�ѩ�ѽL�r����FenBoard���Ʋժ��פ����A�ӳ��J���`���C
	else if(bDoCut && nNonCapNum==0 && ply<=MaxDepth+4)
	{
		score = nPieceID[ (StepRecords[nCurrentStep-1] & 0xFF0000)>>16 ];
		if(score>=1 && score<=3)		// �l�ɻ��Ȭ��J������
		{
			score = nPieceID[ (StepRecords[nCurrentStep-2] & 0xFF0000)>>16 ];
			if(score>=1 && score<=3)	// �l�ɻ��Ȭ��J������
			{
				depth++;
				Extended = 1;
			}
		}
	}

	
	//���BHash����A�p�GHash��R���A������^���ȡC������j���ɰŪK�Ĳv�A(���Ĭ��)�Ƨ@�η��p�C
	CChessMove HashMove = 0;
	int nHashValue = m_Hash.ProbeHash(HashMove, alpha, beta, depth, ply, Player);
	if( nHashValue!=INT_MIN && nHashValue!=INT_MAX )	//Hash�R��
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


	//���B���`�I��JHash��A�t�ק�16��20%�C�Y���o�˧@�A��o�q�h���A�ç�}�l��5��y�y�������h���C
	//    �g����A��_²�檺���Ȩ�ơA�o�˧@�����F�A�Y�A�ϥ�Futility�ŪK�A�|�_�ϧ@�ΡC�ѩ�������P�_�A�ϦӨϳt�׵y�L���C�@�ǡCRazor�_�쪺�@�Ϋܦ����C
	//    ���g���աA���`�I���i�洲�C�ARador�MFutility�o�O�_��F�۷�j���@�ΡC�o��ذŪK���|���P�{�׭��C�ѥX���v�C
	//    ���Z�Y�ϥν��������Ȩ�ơA�i�H�Ҽ{�ϥ�Rador�MFutility�ŪK�C
	if( depth <= 0 )
	{
		score = QuiescenceSearch(0, alpha, beta);

		// �O��Hash�ȡA�`�N���`�I�èS���u�������ʡA���J�ŵۡC
		// ���F�O�u�A�קK�j������í�w�ʡA�����O�s�xalpha,beta,score.
		// �����s�xscore, �a�ӧ󰪪�Hash��R���v�M�ŪK���v�A�t�ק�3.5%�C
		// �ѩ�I�R�j���ܤ���T�A���F�w���A�����i��O�u���s�J�C
		if(score >= beta)
			m_Hash.RecordHash(0,  beta, HashBeta,  0, ply, Player);		// score=beta
		else if(score <= alpha)
			m_Hash.RecordHash(0, alpha, HashAlpha, 0, ply, Player);		// score=alpha		
		else
			m_Hash.RecordHash(0, score, HashPv, 0, ply, Player);		// score=score

		return score;
	}

	nTreeNodes ++;	// �έp��K�`�I

	//�C�B�������_�j���J��^-32768�A�W�������ʫO�Ҥ��g�JHash��A�]���|����Pv�`�I�C
	// �����ҡA�j�s�X�ۡC
	if( bStopThinking )
		return -SHRT_MAX;
	// 4095�Ӹ`�I�d�ݤ@���A�Z�x��ҬO�_�R��
	if(!(nTreeNodes & BusyCounterMask)) 
	{
		if( Interrupt() ) 
			return -SHRT_MAX;
	}


	// ��l�Ƥ@�Ǧ��Ϊ��ܶq
	//const unsigned int bInEndGame = ( Count32( BitPieces & (Player ? 0x7E0000:0x7E)) < 3 ) ? 1 : 0;		// ���ʤ訮�����ƶq�֤_3�ӡA��ܲ��ʤ�i�J�ݧ�
	const unsigned int nNonCaptured = nNonCapNum;
	CKillerMove SubKillerTab;
	SubKillerTab.MoveNum = 0;
			
	
	//�C�B�ŵ۵���JNull-Move Pruning
	// 1. Adaptive  Null-Move Pruning   �}������		�A���ʪŵ۵���
	// 2. Verified  Null-Move Pruning	��    ��		�a����ŵ۵���
	if( 
		   bDoCut        									// �W�h�O�DPV�`�I�A���\���h�ϥ�NullMove�F�Y�W�h�OPV�`�I�A���h�N�T��ϥ�NullMove
		//&& depth >= 2	    								// depth==1,�L�k����A���ϥ�NullMove,�i�楿�`���j��
		&& !nChecked      									// ���ʤ�S���Q�N�x
		&& BitPieces & (Player ? 0xF87E0000:0xF87E)			// ���ʤ�ܤ֦s�b�@�����L�e�������Ѥl�A��pZugzwang�o�ʹX�v�C
		&& beta>-MATEVALUE && beta<MATEVALUE 				// �D�N�x�������]Fruit���@�k�^  // 2b1k4/5c3/9/9/9/9/3h2P2/9/8R/5K3 r - - 0 1 �h������Z�A������20�h25��o�쥿�ѡC�ӧO�䥦�����A�����ݭn�h�j�@�h�C���Ȥ��O�ܺ�T�A�Y���O�̵u���|�����ѡA�q��Ԩ������A�٬O�h�����n�C
	  )
	{
		// ����n�������~�ϥΡA�t�׵y�y�֨ǡA���ӧ�w���A�ŵ۰ŪK�v�W��20%�C
		// ����NullMove�z�רӻ��A�ګܱj���A�w�g��F���Ψ��Ѫ��a�B�C���j���������A�����o����ŪK�C
		// �[�W�����w����Z�A�}���P�����X�G�L�ݮ���A�Ӥ��|����Zugzwang
		// �ŵ۵��ŮɡA���ʤ襢�h�����v�A�G�ӥ�-nOffensiveValue�A�Ӥ��O+nOffensiveValue
		if( Evalue[Player]-Evalue[1-Player]-nOffensiveValue >= beta )
		{
			nNullMoveNodes ++;
			
			// MakeNullMove()
			StepRecords[nCurrentStep] = 0;				// ��e�����ʬ��ŵۡA�Y�ڤ褣����
			nCurrentStep ++;
			nNonCapNum = 0;								// ���s�򨫴ѡA���i��c���`���A�p���i�H��`���P�_���ƴ�֤@�b
			Player = 1 - Player;						// ���ܹ��A�����s�򨫨�B�ѡA�����j�@�ŵ�

			// bPruning==0�A�U�@�h�N�����\�ϥ�NullMove�A���D�A���J��DPV���I�ɡA�~�ॴ�}�o�Ӷ}���C
			// �s��⦸�ϥ�NullMove�O�D�`�M�I���A�`�ŬƦܭ���@���j�����u�u�ϥΤ@��NullMove�C
			score = -AlphaBetaSearch(depth-nAdaptive_R-1, 0, SubKillerTab, -beta, 1-beta);
			
			// UndoNullMove()
			Player = 1 - Player;						// �٭���A�L�ݹ���
			nCurrentStep --;
			nNonCapNum = nNonCaptured;					// ��_��Ӫ��ƾ�

			
			if(score >= beta)							// ��Ĳ`�Ū����k�A�[�W�@�ӫO�I�H�ȡJscore >= beta+delta
			{
				// �}���P�����A���i�����A�]����Evaluation(Player)>=beta�@���O�١A���_�F���j�h��Zugzwang
				if( Count32( BitPieces & (Player ? 0x7E0000:0x7E)) >= 3 )			  // �D�ݧ��J���������ƶq�j�_���_3��
				{
					nNullMoveCuts++;

					// ���g�ҹꪺ�N�x�A��^�D���Ѫ����ơA�i�H�i��ŪK�A����g�JHash��ɳQ��g�C
					if( score > MATEVALUE )				
						score -= WINSCORE - MATEVALUE;
				
					return score;
				}
				else									// �ݧ������i�����
				{
					if(nStyle==2)	// �_�i
						score = AlphaBetaSearch(depth-nVerified_R, 0, KillerTab, beta-1, beta);							// depth-5    �i�����
					else            // �O�u || ���q
						score = AlphaBetaSearch(depth<=nVerified_R ? 1:depth-nVerified_R, 0, KillerTab, beta-1, beta);	// depth-5��1 �i�����
										
					if(score >= beta)
					{
						nNullMoveCuts ++;
						return score;
					}
					
					// �Y���礣���\�A�o�{�F�@��Zugswang�A�٭�`�׭ȡA�~��H�᪺�j���C
					// �o�̥i�H�έpZugswang�X�{���X�v�A�}���P�������֡A�d�I�Z��O����﨤�F�ݧ����T���e�����C
					nZugzwang ++;
				}				
			}
			
			//if( !Extended && score==ply+3-WINSCORE && ply<MaxDepth+6 )	// �N�x���~�A�����@�h
			//{
			//	depth ++;
			//	Extended = 1;
			//}
		}
	}



	// ��l�Ƥ@�Ǧ��Ϊ��ܶq
	int d, w, way, nCaptured;
	int HashFlag = HashAlpha;					// Hash�лx
	int nSearched = 0;
	CChessMove ThisMove;
	CChessMove BestMove = 0;				    // ��l�Ƭ��ŵۡC�Y�Φ��x���A�i�H��^���ŵۡC
	
	

	//�K�B�����`�׭��N�J Internal Iterative Deepening if no HashMove found
	// �p�GHash���R���A�ϥ�depth-2�i�椺���`�׭��N�A�ت��O���F���HashMove
	// �Pı�t�ק�10��15%�A���������ä���o�֡A���]���|�C�C
	// �ݧ�����W�[Hash���R���v�A�[�ָ���t�סC�Y���e�o�{�N�x�������C
	if(    nHashValue == INT_MIN		// Hash���R��
		&& !bDoCut						// �O��PV�`�I�A�DPV�`�I�ǹL�Ӫ�bDoCut==1
		&& depth >= 3 )					// depth=2�Mdepth=1�ɡA�S�����n�A�����i�楿�`�j���N�i�H�F�C
	{
		score = AlphaBetaSearch(depth-2, 0, KillerTab, alpha, beta);	// ���ϥαa���I������

		CHashRecord *pHashIndex = m_Hash.pHashList[Player] + (m_Hash.ZobristKey & m_Hash.nHashMask);		//����e�ѽLZobrist������Hash����}
		if((pHashIndex->flag & HashExist) && pHashIndex->zobristlock==m_Hash.ZobristLock)
		{
			if( pHashIndex->move )
			{
				HashMove = pHashIndex->move;
				nHashValue = INT_MAX;				
			}			
		}
	}


	//�E�BHashMove�ҵo��k  �Q��hash���P�@�ѧ����O���R�����T��
	if( nHashValue == INT_MAX )
	{
		nHashMoves ++;
		ThisMove = HashMove;
			
		assert(Board[ThisMove & 0xFF]!=16 && Board[ThisMove & 0xFF]!=32);		// �����ӥX�{�A�_�h�����~�A�z�L�N�x�˴��A���e�@�B�L�o�ճo�����k
		MovePiece( ThisMove );
			if(Checked(1-Player))												// �ڤ�Q�N�x�A�U�@�B��訫�A�L�k���ϡA��^���Ѫ�����.�K���@�h�j��
				score = ply+1-WINSCORE;											// ���ӧQ�A�ڤ訫�F�@�B��ѡC�p�����ӭ����A�h�O�ۤv�e�W�����N�x(���Ӳέp�@�U�A�o�˪����ʾ��v�o�����ӫܧC�A�p���ֵL�Ī��B��)
			else
			{
				StepRecords[nCurrentStep-1] |= Checking(Player) << 24;					// ���Q�N�x�A�ǰe��U�@�h
				score = -AlphaBetaSearch(depth-1, 0, SubKillerTab, -beta, -alpha);		// HashMove�@��O�D����A�U�@�h�����\�a���I���ŪK
			}
		UndoMove();	
		nNonCapNum = nNonCaptured;	// ��_��Ӫ��L���l�ѨB�ƥ�
		
		//Fail-Soft Alpha-Beta MainSearch�]Hash�MMTDf��������k�^
		if(score > nBestValue)
		{
			nBestValue = score;
			BestMove = ThisMove;

			if( score >= beta )				// Beta�ŪK
			{
				nHashCuts ++;
				HashFlag = HashBeta;
				goto CUT;
			}

			if( score > alpha )				// �Y�p���f�C
			{
				alpha = score;
				HashFlag = HashPv;
			}
		}

		nSearched ++;
	}


	//�Q�B����ҵo�JKiller Moves
	//1.��v�Ѫ�����ƥج�2�F����H�ѱ��⪺�ƥ�2-4�Ӥ���X�A�A�Ӧh�t���ܺC�C
	//2.Killer Moves�ĥΥ��i���XFIFO�����C�ϥξ��v�ҵo�ĪG�ä�����A����˧Ǧ��ɤ]�ܧ֡C
	//3.�p�G�Q�N�x�A���ϥα���ҵo�A�]�����N�x�k�ר�ơA���⤣�@�w����ѱN�C
	//4.���Ⲿ�ʤ��Y�]�tHashMove�A�i��L�o
        ThereIsKillers = 0;        // ������
	if( !nChecked )
	{
		for( w=0; w<KillerTab.MoveNum; w++ )
		{
			// �L�oHashMove�M���X�k�����Ⲿ��
			ThisMove = KillerTab.MoveList[w];
			if( ThisMove==HashMove || !IsLegalKillerMove(Player, ThisMove) )
				continue;

			// �έp���⪺�ƥ�
			nKillerNodes[w]++;
			ThereIsKillers ^= 1<<w;		// ��s������
				
			assert(Board[ThisMove & 0xFF]!=16 && Board[ThisMove & 0xFF]!=32);		// �����ӥX�{�A�_�h�����~�A�z�L�N�x�˴��A���e�@�B�L�o�ճo�����k
			MovePiece( ThisMove );		
				if(Checked(1-Player))					// �ڤ�Q�N�x�A�U�@�B��訫�A�L�k���ϡA��^���Ѫ�����.�K���@�h�j��
					score = ply+1-WINSCORE;				// ���ӧQ�A�ڤ訫�F�@�B��ѡC�p�����ӭ����A�h�O�ۤv�e�W�����N�x�C
				else
				{
					StepRecords[nCurrentStep-1] |= Checking(Player) << 24;		// ���Q�N�x�A�ǰe��U�@�h
					
					score = -AlphaBetaSearch(depth-1, bPruning, SubKillerTab, -beta, 1-beta);
					if(score>alpha && score<beta)
						score = -AlphaBetaSearch(depth-1, 0, SubKillerTab, -beta, -alpha);
				}
			UndoMove();					// ��_���ʡA��_���ʤ�A��_�@��
			nNonCapNum = nNonCaptured;	// ��_��Ӫ��L���l�ѨB�ƥ�
			

			//Fail-Soft Alpha-Beta MainSearch�]Hash�MMTDf��������k�^
			

			if(score > nBestValue)
			{
				nBestValue = score;
				BestMove = ThisMove;

				if( score >= beta )				// Beta�ŪK
				{
					nKillerCuts[w] ++;
					HashFlag = HashBeta;
					goto CUT;
				}

				if( score > alpha )				// �Y�p���f�C
				{
					alpha = score;
					HashFlag = HashPv;
				}
			}
			
			nSearched ++;	
		}
	}


	//�Q�@�B�Y�l���ʡJCaptureMove
	CChessMove ChessMove[111];
        NextKiller = ThereIsKillers & 1 ? 0 : 1;    //�_�Ʊq0�}�l��A���Ʀܤֱq1�}�l��C
	if( !nChecked )
	{
		way = CapMoveGen(Player, ChessMove);					// ���ͩҦ�������
		for(w=0; w<way; w++)
		{
			BubbleSortMax(ChessMove, w, way);
			ThisMove = ChessMove[w] & 0xFFFF;

			// �L�oHashMove�M���Ⲿ��
			if( ThisMove==HashMove )
				continue;

			// �p�G������s�b�A�L�o���Ⲿ�ʡC�p�G�Ҧ������ⳣ�w�g�L�o�AThereIsKillers = 0;
			if( ThereIsKillers )
			{
				for(d=NextKiller; d<KillerTab.MoveNum; d++)
				{
					if( ThisMove==KillerTab.MoveList[d] )
					{
						if(d==NextKiller)
							NextKiller = d+1;		// �U�@�ӱ��⪺��m
						ThereIsKillers ^= 1<<d;		// �qThereIsKillers�лx���R���o�ӱ���
						d = -1;						// d < 0, ������e�����ʬO�ӱ���
						break;
					}
				}
				if( d<0 )							// ��e�����ʴ��g�O�ӱ���A���L���A�L�ݦA���i��j��
					continue;
			}

			// �έp�Y�l���ʪ��ƥ�
			nCapMoves ++;
			
			assert(Board[ThisMove & 0xFF]!=16 && Board[ThisMove & 0xFF]!=32);		// �����ӥX�{�A�_�h�����~�A�z�L�N�x�˴��A���e�@�B�L�o�ճo�����k
			nCaptured = MovePiece( ThisMove );										// �`�N�J���ʦZPlayer�w�g��ܹ��A�U�����P�_���n�X���C���޳o�˫ܧO��A���b�䥦�a��ܤ�K�A�ڥ����κޤF
				if(Checked(1-Player))												// �ڤ�Q�N�x�A�U�@�B��訫�A�L�k���ϡA��^���Ѫ�����.�K���@�h�j��
					score = ply+1-WINSCORE;											// ���ӧQ�A�ڤ訫�F�@�B��ѡC�p�����ӭ����A�h�O�ۤv�e�W�����N�x(���Ӳέp�@�U�A�o�˪����ʾ��v�o�����ӫܧC�A�p���ֵL�Ī��B��)
				else
				{
					StepRecords[nCurrentStep-1] |= Checking(Player) << 24;			// ���Q�N�x�A�ǰe��U�@�h

					if( nSearched )
					{
						score = -AlphaBetaSearch(depth-1, bPruning, SubKillerTab, -alpha-1, -alpha);
						if(score>alpha && score<beta)
							score = -AlphaBetaSearch(depth-1, 0, SubKillerTab, -beta, -alpha);
					}
					else
						score = -AlphaBetaSearch(depth-1, 0, SubKillerTab, -beta, -alpha);	

				}
			UndoMove();					// ��_���ʡA��_���ʤ�A��_�@��
			nNonCapNum = nNonCaptured;	// ��_��Ӫ��L���l�ѨB�ƥ�
			
			
			//********************************************************************************************************
			//Fail-Soft Alpha-Beta MainSearch�]Hash�MMTDf��������k�^
			if(score > nBestValue)
			{
				nBestValue = score;
				BestMove = ThisMove;
				
				if( score >= beta )				// Beta�ŪK�C�Y�l���ʪ��ŪK�v���ܰ��A�G�Ӽg�b�̭�
				{
					nCapCuts ++;
					HashFlag = HashBeta;
					goto CUT;
				}

				if( score > alpha )				// �Y�p���f�C
				{
					alpha = score;
					HashFlag = HashPv;
				}
			}
			
			nSearched ++;
		}
	}
	

	//�Q�G�B�R�A���ʡJQuiesceMove

	if( nChecked )
	{
		way = CheckEvasionGen(Player, nChecked, ChessMove);		// ���͸ѱN���۪k�C�ѩ�ϥΤF�N�x�X�i�A�N�x�k�ױN�ܤj�{�פW���ɷj���Ĳv�C
		
		// ����ѱN�A�L�ݷj���A������^���ȡC
		if( !way )
		{
			nAlphaNodes ++;
			score = ply+1-WINSCORE;								// �U�@�B�ѱN�Q���Y���C
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
		way = MoveGenerator(Player, ChessMove);					// ���ͩҦ�������
	

	
	//�Q�@�B�D�Y�l���ʪ����v�ҵo�j����k
	// 1. �_�w�ƧǬD��̤j��(Bubble Sort for Max Value)
	// 2. �L�oHashMove�PKillerMove
	// 3. �D����j��(PVS)(Principal Variation Search)
	// 4. ���v�ŪK(History Pruning)
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
		// �p�G������s�b�A�L�o���Ⲿ�ʡC�p�G�Ҧ������ⳣ�w�g�L�o�AThereIsKillers = 0;
		if( ThereIsKillers )
		{
			for(d=NextKiller; d<KillerTab.MoveNum; d++)
			{
				if( ThisMove==KillerTab.MoveList[d] )
				{
					if(d==NextKiller)
						NextKiller = d+1;		// �U�@�ӱ��⪺��m
					ThereIsKillers ^= 1<<d;		// �qThereIsKillers�лx���R���o�ӱ���
					d = -1;						// d < 0, ������e�����ʬO�ӱ���
					break;
				}
			}
			if( d<0 )							// ��e�����ʴ��g�O�ӱ���A���L���A�L�ݦA���i��j��
				continue;
		}
		
		MovePiece( ThisMove );										// �`�N�J���ʦZPlayer�w�g��ܹ��A�U�����P�_���n�X���C���޳o�˫ܧO��A���b�䥦�a��ܤ�K�A�ڥ����κޤF
			if( Checked(1-Player) )												// �ڤ�Q�N�x�A�U�@�B��訫�A�L�k���ϡA��^���Ѫ�����.�K���@�h�j��
				score = ply+1-WINSCORE;											// ���ӧQ�A�ڤ訫�F�@�B��ѡC�p�����ӭ����A�h�O�ۤv�e�W�����N�x(���Ӳέp�@�U�A�o�˪����ʾ��v�o�����ӫܧC�A�p���ֵL�Ī��B��)
			else
			{
				nChecking = Checking(Player) << 24;								// ���Q�N�x�A�ǰe��U�@�h
				StepRecords[nCurrentStep-1] |= nChecking;

				// ���v�ŪK(History Alpha-Beta Pruning)
				if(	    bHistoryPruning		//a. �ھڬɭ��ǰe���U�ѭ���H�η�e�����������M�w�O�_�ϥ�History Pruning
					 && nSearched >= 3		//b. HistoryMoves==3  ��v�ѻP����H�ѳq�Ϊ��`��
					 && depth>=3		    //c. HistoryDepth==3  depth-2>=1�A�O�һPNullMove���Ĭ�C���Y�W�h�ϥ�NullMove�A���h����ɡA�ݸ�������۪k�A����Zugswang�C
					 && !nChecked			//d. �ڤ�Q�N�x�ɡA�����\�ϥ�History Pruning
				 	 && !nChecking			//e. ���Q�N�x�ɡA�����\�ϥ�History Pruning
				  )
				{
					score = -AlphaBetaSearch(depth-2, bPruning, SubKillerTab, -alpha-1, -alpha);	// ��depth-2�i��j��
					if( score > alpha )																// �Yalpha+1==beta�Ascore>=beta
						score = -AlphaBetaSearch(depth-1, 0, SubKillerTab, -beta, -alpha);			// ��depth-1���alpha
				}
				// �D�зǪ�PVS�j��(Principal Variation Search)�A�зǪ�PVS�j���ݭn���score>alpha�A�YbFoundPV=true
				// �H�U�O����i����v�������A
				else if( nSearched )
				{
					score = -AlphaBetaSearch(depth-1, bPruning, SubKillerTab, -alpha-1, -alpha);
					if(score>alpha && score<beta)
						score = -AlphaBetaSearch(depth-1, 0, SubKillerTab, -beta, -alpha);
				}
				else //if( !nSearched )
					score = -AlphaBetaSearch(depth-1, 0, SubKillerTab, -beta, -alpha);
			}
		UndoMove();					// ��_���ʡA��_���ʤ�A��_�@��
		nNonCapNum = nNonCaptured;	// ��_��Ӫ��L���l�ѨB�ƥ�
		
		//********************************************************************************************************
		//Fail-Soft Alpha-Beta MainSearch�]Hash�MMTDf��������k�^
		if(score > nBestValue)
		{
			nBestValue = score;
			BestMove = ThisMove;

			if( score >= beta )				// Beta�ŪK�A�g���99%�H�W��beta�`�I���ͤ_w<6
			{
				if(w<4)
				nBetaNodes++;
				HashFlag = HashBeta;				
				break;
			}
			if( score > alpha )				// �Y�p���f�C
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


	//�Q�G�B���v�ҵo�B����ҵo�B�O��Hash��
CUT:
	if(HashFlag != HashAlpha)
	{
		if( HistoryRecord[BestMove] > 64000 )		// �y�X�l�a�J65535-64000=1535
			UpdateHistoryRecord(1);
		HistoryRecord[BestMove] += depth*depth;

		// �O�����Ⲿ��
		if(KillerTab.MoveNum < MaxKiller) 
		{
			// �h�����ƪ�����A�t�פj�P��10%
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
	
	// �m�����s�x��FailLow�ץ���k�J�a���b�y(�\�Ӱ�)---�q���H�Ѫ��]�p�P��{
	if( nBestValue<alpha && nHashValue==INT_MAX )	// FailLow���p�X�{(nBestValue<alpha); �m�������s�xHashMove(nHashValue==INT_MAX)
		BestMove = HashMove;						// Best����Ӹm������HashMove�A�Ӥ����ܭ�Ӫ����G
	m_Hash.RecordHash( BestMove, nBestValue, HashFlag, depth, ply, Player );

	return nBestValue;
}


// �I�R�j��(������㴣�ɴѤO�A�L�k�קK���S�⪺�i��)�C�D�n�޳N�J
// 1. �`���˴�----�_�h�|�X�{���N�F
// 2. �ŵ۱���----���Q�N�x�ɡA��������Ѧn�٬O���l��n�F
// 3. �Y�l���ʲ��;�----CapMoveGen()�F
// 4. �ѱN���ʲ��;�----CheckEvasionGen()�F
// 5. �_�w�ƧǴM��̤j��
// 6. ��ѽL�ֳt�N�x�˴��F
// 7. Fail Soft Alpha-Beta�j���C
int CSearch::QuiescenceSearch(int depth, int alpha, int beta)
{
	int score;
	const int ply = nCurrentStep - nStartStep;
	const int nChecked = (StepRecords[nCurrentStep-1] & 0xFF000000) >> 24;
	int nBestValue = ply-WINSCORE;		//�j�����f��^�����p���j��

	if( depth < 0 )
	{
		// �έp�I�R�j��������
		nQuiescNodes++;
		
		//�@�B�`���P�_
		//1.���ʤ�J�o�̧P�_�W�@�B�H�e�O�_�c���`���A�ھڭt�ȷ��j��z�A������^�t�ȡC	
		//2.�^�X�ơJ�J�M�P�_�W�@�B�A�۵Mply = ply-1�C���`�N�o�ӲӸ`��^�ȱN���|��T�C
		//2.�ޥ� 1�J�Y�e�������󥢱ѡA�{�����i��Z�����P�_�AnNonCapNum=0,1,2,3�ɡA���i��c���`���A�p���j�j��֤F�`���P�_�����ơC
		//3.�ޥ� 2�Jdepth==0�A�w�g�b�W�h�j���ɶi��F�P�_�A�L�ݭ��ơC
		if( nNonCapNum>=4 )
		{
			score = RepetitionDetect();
			if(score)
				return LoopValue(Player, ply, score);
		}

		// ���ѵ���
		if(nBestValue>=beta)
			return nBestValue;

		// �M�ѵ��šJ�ħ����褣�s�b�������L�e�������Ѥl
		if(!(BitPieces & BitAttackPieces))
			return m_Evalue.Evaluation(Player);

		/*
		//�T�BHash���� (�I�R�j�����R���v���C)
		CChessMove HashMove;
		score = m_Hash.ProbeHash(HashMove, alpha, beta, depth, ply, Player);
		if(score!=INT_MIN && score!=INT_MAX)	//Hash�R��
		{
			nLeafHashHit ++;
			return score;
		}
		*/
	}
	else
		nLeafNodes ++;


	//�G�B�Y�D�N�x�������A�����i��Null-Move���աC
	// �ت��O���F����Y�l�n�٬O���Y�l�n�A�p�G���Y�l�����Ƥw�g����ŪK�A�Y�l�Z�]���M�ŪK�C
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

	
	//�T�B���ͦY�l�����ʥH�θѱN������
	CChessMove ChessMove[32];					// �ϥΤF�ѱN���ʲ��;��Z�A�Ʋեi�H�]�o�p�ǡA32�N�����F�CMax=19

	// ����L�@�Ǵѧ��A�ݧ����i��N�x�X�i�ɡA�q���g�`���X���ۦӥ�l�C
	if( nChecked )
	{
		way = CheckEvasionGen(Player, nChecked, ChessMove);		// �ڤ�Q�N�x�A�i�}�Ҧ������k�A���ӥ����͸ѱN���B�k
		if(!way)
			return ply+1-MATEVALUE;								// �]���I�R�j���ä���T�A�Y�L�ѡA��^�D���Ѥ��ơA�����gHash��A���i�H�i��ŪK�C
	}
	else
		way = CapMoveGen(Player, ChessMove);					// ���ͩҦ����Y�l�۪k


	CChessMove ThisMove;

	//�|�BSoft Alpha-Beta�j��
	for(w=0; w<way; w++)
	{	
		BubbleSortMax(ChessMove, w, way);
		ThisMove = ChessMove[w] & 0xFFFF;

		assert(Board[ThisMove & 0xFF]!=16 && Board[ThisMove & 0xFF]!=32);		// �����ӥX�{�A�_�h�����~�A�z�L�N�x�˴��A���e�@�B�L�o�ճo�����k
		MovePiece( ThisMove );

		// �ӭt����: verify game		
		if(Checked(1-Player))					// �ڤ�Q�N�x�A�U�@�B��訫�A�L�k���ϡA��^���Ѫ�����.�K���@�h�j��
			score = ply+1-MATEVALUE;		    // ���ӧQ�A�ѩ�I�R�j���������ѨëD��T�A�G�Өϥ�MATEVALUE�A���ϥ�WINSCORE�C
		else								    // �o�ص����N���ۤ��⦹������@���ѡA�H�K��^�ɹ�Hash��y���v�T�A���S���|�v�T�ŪK�C
		{
			StepRecords[nCurrentStep-1] |= Checking(Player) << 24;
			score = -QuiescenceSearch(depth-1, -beta, -alpha);
		}

		UndoMove();					// ��_���ʡA��_���ʤ�A��_�@��
		nNonCapNum = nNonCaptured;	// ��_��Ӫ��L���l�ѨB�ƥ�


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
	

	//��s�ѽL
	Board[nSrc] = 0;
	Board[nDst] = nMovedChs;


	m_Hash.ZobristKey  ^= m_Hash.ZobristKeyTable[nMovedPiece][nSrc] ^ m_Hash.ZobristKeyTable[nMovedPiece][nDst];
	m_Hash.ZobristLock ^= m_Hash.ZobristLockTable[nMovedPiece][nSrc] ^ m_Hash.ZobristLockTable[nMovedPiece][nDst];

	
	//��s�Ѥl����
	Piece[nMovedChs] = nDst;
	Evalue[Player] += PositionValue[nMovedPiece][nDst] - PositionValue[nMovedPiece][nSrc];	// ��s����
	

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
		nNonCapNum ++;				// ����L���l���b�^�X��


	// �����_�InSrc�����P��C�A�m��0��
	xBitBoard[ nSrc >> 4 ]  ^= xBitMask[nSrc];
	yBitBoard[ nSrc & 0xF ] ^= yBitMask[nSrc];	

	// ��s���InDst�����P��C�A�m��1��
	xBitBoard[ nDst >> 4 ]  |= xBitMask[nDst];
	yBitBoard[ nDst & 0xF ] |= yBitMask[nDst];

	
	//�O����e������ZobristKey�A�Τ_�`�������B�N�x�˴�
	StepRecords[nCurrentStep] = move  | (nCaptured<<16);
	nZobristBoard[nCurrentStep] = m_Hash.ZobristKey;		// ��e����������
	nCurrentStep++;


	Player = 1 - Player;		// ���ܲ��ʤ�
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


	// ������_���ʤ�
	Player = 1 - Player;		
	//m_Hash.ZobristKey ^= m_Hash.ZobristKeyPlayer;
    //m_Hash.ZobristLock ^= m_Hash.ZobristLockPlayer;

	m_Hash.ZobristKey  ^= m_Hash.ZobristKeyTable[nMovedPiece][nSrc] ^ m_Hash.ZobristKeyTable[nMovedPiece][nDst];
	m_Hash.ZobristLock ^= m_Hash.ZobristLockTable[nMovedPiece][nSrc] ^ m_Hash.ZobristLockTable[nMovedPiece][nDst];

	//��s�ѽL�P�Ѥl
	Board[nSrc] = nMovedChs;
	Board[nDst] = nCaptured;
	Piece[nMovedChs] = nSrc;
	Evalue[Player] -= PositionValue[nMovedPiece][nDst] - PositionValue[nMovedPiece][nSrc];	// ��s����


	// ��_���P��C���_�l��mnSrc�A�ϥΡ�|���ާ@�Ÿm��1��
	xBitBoard[ nSrc >> 4 ]  |= xBitMask[nSrc];
	yBitBoard[ nSrc & 0xF ] |= yBitMask[nSrc];
	
	if( nCaptured )							//�Y�l����
	{
		int nKilledPiece = nPieceType[nCaptured];

		Piece[nCaptured] = nDst;			//��_�Q���Ѥl����m�C
		BitPieces ^= 1<<(nCaptured-16);

		m_Hash.ZobristKey  ^= m_Hash.ZobristKeyTable[nKilledPiece][nDst];
		m_Hash.ZobristLock ^= m_Hash.ZobristLockTable[nKilledPiece][nDst];

		Evalue[1-Player] += PositionValue[nKilledPiece][nDst] + BasicValues[nKilledPiece];
	}
	else									//�Y�O�D�Y�l���ʡA������R���m�m"0"
	{
		// �M�����P��C���_�l��mnDst�A�ϥΡ�^���ާ@�Ÿm��0��
		// �Ϥ��A�Y�O�Y�l���ʡA���I��m���ӴN�O"1"�A�ҥH���Ϋ�_�C
		xBitBoard[ nDst >> 4 ]  ^= xBitMask[nDst];
		yBitBoard[ nDst & 0xF ] ^= yBitMask[nDst];
	}

	//�M�����k���C
	nCurrentStep --;
	nNonCapNum --;
}

// �ھڴѤl��m�T���A��l�ƩҦ��ѽL�P�Ѥl���ƾ�
// �]�i�ϥδѽL�T���A�ݴ`��256���A�t�׵y�C�C
void CSearch::InitBitBoard(const int Player, const int nCurrentStep)
{
	int m,n,x,y;	
	int chess;
	
	// ��l�ơA�M�s
	BitPieces = 0;
	Evalue[0] = Evalue[1] = 0;
	m_Hash.ZobristKey  = 0;
	m_Hash.ZobristLock = 0;
	for(x=0; x<16; x++)
		xBitBoard[x] = 0;
	for(y=0; y<16; y++)
		yBitBoard[y] = 0;
	
	// �ھ�32���Ѥl��m��s
	for(n=16; n<48; n++)
	{
		m = Piece[n];													// �Ѥl��m
		if( m )															// �b�ѽL�W
		{
			chess               = nPieceType[n];						// �Ѥl�����J0��14
			BitPieces          |= 1<<(n-16);							// 32���Ѥl���
			xBitBoard[m >> 4 ] |= xBitMask[m];							// ���
			yBitBoard[m & 0xF] |= yBitMask[m];							// ��C
			m_Hash.ZobristKey  ^= m_Hash.ZobristKeyTable[chess][m];		// Hash��
			m_Hash.ZobristLock ^= m_Hash.ZobristLockTable[chess][m];	// Hash��
			
			if(n!=16 && n!=32)			
				Evalue[ (n-16)>>4 ] += PositionValue[chess][m] + BasicValues[chess];
		}
	}

	//m_Hash.InitZobristPiecesOnBoard( Piece );

	// �Τ_�`���˴�
	nZobristBoard[nCurrentStep-1] =  m_Hash.ZobristKey;

	// ��e���ʤ�O�_�Q�N�x�A�g�J���k���C
	StepRecords[nCurrentStep-1] &= 0xFF000000;
	StepRecords[nCurrentStep-1] |= Checking(Player) << 24;
}

//�����ѷӴѳW�@��ԲӪ��P�_�A���F�q�ΡA�̦n���ϥ�CString��
char *CSearch::GetStepName(CChessMove ChessMove, int *Board) const
{
	//�Ѥl�s��
	static char StepName[12];	// �������R�A�ܶq�A�_�h�����^

	static const char ChessName[14][4] = {"  ","  ","��","��","�H","�h","��", "  ","  ","��","  ","��","�K","�L"};

	static const char PostionName[2][16][4] = { {"", "", "", "��","��","��","��","��","��","��","��","��", "", "", "", ""}, 
	                                            {"", "", "", "�E","�K","�C","��","��","�|","�T","�G","�@", "", "", "", ""} };

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
	
	//�ˬd���Cx0�O�_�s�b�t�@�����諸�Ѥl.
	int y,chess;
	if( nPieceID[nMovedChs]!=0 && nPieceID[nMovedChs]!=4 && nPieceID[nMovedChs]!=5 )	// �N�B�h�B�H���ΰϤ�
	{
		for(y=3; y<13; y++)
		{
			chess = Board[ (y<<4) | x0 ];

			if( !chess || y==y0)														// �L�l�Ϊ̦P�@���Ѥl�A�~��j��
				continue;

			if( nPieceType[nMovedChs] == nPieceType[chess] )							// �J��t�@�ӬۦP���Ѥl
			{
				if( !Player )			// �¤l
				{
					if(y > y0)
						strcpy( StepName, "�e" );
					else
						strcpy( StepName, "�Z" );
				}
				else					// ���l
				{
					if(y < y0)
						strcpy( StepName, "�e" );
					else
						strcpy( StepName, "�Z" );
				}

				strcat( StepName, ChessName[nPieceType[nMovedChs]] );
				break;
			}
		}
	}

	int piece = nPieceID[nMovedChs];

	//�i, �h, ��
	if(y0==y1)
	{
		strcat( StepName, "��" );
		strcat( StepName, PostionName[Player][x1]);					// ���A����Ѥl���H�����m���
	}
	else if((!Player && y1>y0) || (Player && y1<y0))
	{
		strcat( StepName, "�i" );

		if(piece==3 || piece==4 || piece==5)						// ���B�H�B�h�ε����m���
			strcat( StepName, PostionName[Player][x1] );			
		else if(Player)												// �N�B���B���B�L�ά۹��m���
			strcat( StepName, PostionName[1][y1-y0+12] );			// ����
		else
			strcat( StepName, PostionName[0][y1-y0+2] );			// �¤�
	}
	else
	{
		strcat( StepName, "�h" );

		if(piece==3 || piece==4 || piece==5)						// ���B�H�B�h�ε����m���
			strcat( StepName, PostionName[Player][x1] );			
		else if(Player)												// �N�B���B���B�L�ά۹��m���
			strcat( StepName, PostionName[1][y0-y1+12] );			// ����
		else
			strcat( StepName, PostionName[0][y0-y1+2] );			// �¤�		
	}

	return(StepName);
}



// ��o�D����
// �ѩ�ϥΤFHash��A���ɥD����O���~���A���ݭץ��T�H�H�H
void CSearch::GetPvLine(void)
{
	CHashRecord *pHashIndex = m_Hash.pHashList[Player] + (m_Hash.ZobristKey & m_Hash.nHashMask);		//����e�ѽLZobrist������Hash����}

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
	
	// �Ыؤ��A�çR����Ӫ����e�C�Q�γo�خ榡�ƿ�X�����K�C
	FILE *out = fopen(szFileName, "w+");

	fprintf(out, "***************************�j���T��***************************\n\n");

	fprintf(out, "�j���`�סJ%d\n", MaxDepth);
	n = nTreeNodes + nLeafNodes + nQuiescNodes;
	fprintf(out, "TreeNodes : %u\n", n);
	fprintf(out, "TreeStruct: BranchNodes = %10u\n", nTreeNodes);
	fprintf(out, "            LeafNodes   = %10u\n", nLeafNodes);
	fprintf(out, "            QuiescNodes = %10u\n\n", nQuiescNodes);

	float TimeSpan = StartTimer/1000.0f;
	fprintf(out, "�j���ɶ�    :   %8.3f ��\n", TimeSpan);
	fprintf(out, "�K���j���t��:   %8.0f NPS\n", (nTreeNodes+nLeafNodes)/TimeSpan);
	fprintf(out, "����j���t��:   %8.0f NPS\n\n", n/TimeSpan);

	fprintf(out, "Hash��j�p: %d Bytes  =  %d M\n", m_Hash.nHashSize*2*sizeof(CHashRecord), m_Hash.nHashSize*2*sizeof(CHashRecord)/1024/1024);
	fprintf(out, "Hash�л\�v: %d / %d = %.2f%%\n\n", m_Hash.nHashCovers, m_Hash.nHashSize*2, m_Hash.nHashCovers/float(m_Hash.nHashSize*2.0f)*100.0f);

	unsigned int nHashHits = m_Hash.nHashAlpha+m_Hash.nHashExact+m_Hash.nHashBeta;
	fprintf(out, "Hash�R��: %d = %d(alpha:%.2f%%) + %d(exact:%.2f%%) +%d(beta:%.2f%%)\n", nHashHits, m_Hash.nHashAlpha, m_Hash.nHashAlpha/(float)nHashHits*100.0f, m_Hash.nHashExact, m_Hash.nHashExact/(float)nHashHits*100.0f, m_Hash.nHashBeta, m_Hash.nHashBeta/(float)nHashHits*100.0f);
	fprintf(out, "�R�����v: %.2f%%\n", nHashHits/float(nTreeNodes+nLeafNodes)*100.0f);
	fprintf(out, "��K�R��: %d / %d = %.2f%%\n", nTreeHashHit, nTreeNodes, nTreeHashHit/(float)nTreeNodes*100.0f);
	fprintf(out, "���l�R��: %d / %d = %.2f%%\n\n", nLeafHashHit, nLeafNodes, nLeafHashHit/(float)nLeafNodes*100.0f);

	fprintf(out, "NullMoveCuts   = %u\n", nNullMoveCuts);
	fprintf(out, "NullMoveNodes  = %u\n", nNullMoveNodes);
	fprintf(out, "NullMove�ŪK�v = %.2f%%\n\n", nNullMoveCuts/(float)nNullMoveNodes*100.0f);

	fprintf(out, "Hash�Ĭ�   : %d\n", m_Hash.nCollision);
	fprintf(out, "Null&Kill  : %d\n", m_Hash.nCollision-nHashMoves);
	fprintf(out, "HashMoves  : %d\n", nHashMoves);
	fprintf(out, "HashCuts   : %d\n", nHashCuts);
	fprintf(out, "Hash�ŪK�v : %.2f%%\n\n", nHashCuts/(float)nHashMoves*100.0f);

	fprintf(out, "���Ⲿ�� : \n");
	k = n = 0;
	for(m=0; m<MaxKiller; m++)
	{
		fprintf(out, "    Killer   %d : %8d /%8d = %.2f%%\n", m+1, nKillerCuts[m], nKillerNodes[m], nKillerCuts[m]/float(nKillerNodes[m]+0.001f)*100.0f);
		n += nKillerCuts[m];
		k += nKillerNodes[m];
	}
	fprintf(out, "    ����ŪK�v : %8d /%8d = %.2f%%\n\n", n, k, n/float(k+0.001f)*100.0f);


	fprintf(out, "�Y�l���ʰŪK�v = %d / %d = %.2f%%\n\n", nCapCuts, nCapMoves, nCapCuts/(float)nCapMoves*100.0f);


	m = nBetaNodes + nPvNodes + nAlphaNodes;
	fprintf(out, "�D�Y�l����: %d\n", m);	
	fprintf(out, "    BetaNodes: %10d  %4.2f%%\n", nBetaNodes, nBetaNodes/float(m)*100.0f);
	fprintf(out, "    PvNodes  : %10d  %4.2f%%\n", nPvNodes, nPvNodes/float(m)*100.0f);
	fprintf(out, "    AlphaNode: %10d  %4.2f%%\n\n", nAlphaNodes, nAlphaNodes/float(m)*100.0f);

	m += nNullMoveCuts + nHashMoves + nKillerNodes[0] + nKillerNodes[1] + nCapMoves;
	fprintf(out, "TotalTreeNodes: %d\n\n\n", m);

	n = nCheckCounts-nNonCheckCounts;
	fprintf(out, "�N�x����: %d\n", n);
	fprintf(out, "��������: %d\n", nCheckCounts);
	fprintf(out, "���\���v: %.2f%%\n\n", n/(float)nCheckCounts*100.0f);

	fprintf(out, "CheckEvasions = %d\n", nCheckEvasions);
	fprintf(out, "�ѱN / �N�x   = %d / %d = %.2f%%\n\n", nCheckEvasions, n, nCheckEvasions/float(n)*100.0f);


	// ��ܥD����
	int BoardStep[256];
	for(n=0; n<256; n++)
		BoardStep[n] = Board[n];

	static const char ChessName[14][4] = {"  ","  ","��","��","�H","�h","��", "  ","  ","��","  ","��","�K","�L"};

	fprintf(out, "\n�D����JPVLine***HashDepth**************************************\n");
	for(m=0; m<nPvLineNum; m++)
	{
		nSrc = (PvLine[m] & 0xFF00) >> 8;
		nDst = PvLine[m] & 0xFF;
		nCaptured = BoardStep[nDst];

		// �^�X�ƻP�ѨB�W��
		fprintf(out, "    %2d. %s", m+1, GetStepName( PvLine[m], BoardStep ));

		// �Y�l�۪k
		if( nCaptured )
			fprintf(out, " k-%s", ChessName[nPieceType[nCaptured]]);
		else
			fprintf(out, "     ");

		// �j���`��
		fprintf(out, "  depth = %2d", PvLine[m]>>16);

		// �N�x�лx
		nCaptured = (PvLine[m] & 0xFF0000) >> 16;
		if(nCaptured)
			fprintf(out, "   Check Extended 1 ply ");
		fprintf(out, "\n");

		BoardStep[nDst] = BoardStep[nSrc];
		BoardStep[nSrc] = 0;
	}

	fprintf(out, "\n\n***********************��%2d �^�X********************************\n\n", (nCurrentStep+1)/2);
	fprintf(out, "***********************�����ͦ��J%d �Ӳz�X�۪k**********************\n\n", nFirstLayerMoves);
	for(m=0; m<(unsigned int)nFirstLayerMoves; m++)
	{
		nSrc = (FirstLayerMoves[m] & 0xFF00) >> 8;
		nDst = FirstLayerMoves[m] & 0xFF;

		// �M��D����
		if(PvLine[0] == FirstLayerMoves[m])
		{
			fprintf(out, "*PVLINE=%d***********Nodes******History**************************\n", m+1);
			fprintf(out, "*%2d.  ", m+1);
		}
		else
			fprintf(out, "%3d.  ", m+1);

		//n = m==0 ? FirstLayerMoves[m].key : FirstLayerMoves[m].key-FirstLayerMoves[m-1].key;	// �έp����ƥ�
		n = FirstLayerMoves[m] >> 16;																// �έp����
		fprintf(out, "%s = %6d    hs = %6d\n", GetStepName(FirstLayerMoves[m], Board), n, HistoryRecord[FirstLayerMoves[m]&0xFFFF]);
	}
	
	fprintf(out, "\n\n********************�����L�o�J%d�ӸT��۪k********************************\n\n", nBanMoveNum);
	for(m=0; m<(unsigned int)nBanMoveNum; m++)
	{
		fprintf(out, "%3d. %s\n", m+1, GetStepName( BanMoveList[m], Board ));
	}

	fprintf(out, "\n\n***********************���v�O��********************************\n\n", (nCurrentStep+1)/2);
	
	int MoveStr; 
	for(m=0; m<=(int)nCurrentStep; m++)
	{
		MoveStr = Coord(StepRecords[m]);
		fprintf(out, "%3d. %s  %2d  %2d  %12u\n", m, &MoveStr, (StepRecords[m] & 0xFF0000)>>16, (StepRecords[m] & 0xFF000000)>>24, nZobristBoard[m]);
	}


	// �������
	fclose(out);
}

/*
int CSearch::Evaluation(int player)
{
	return Evalue[player] - Evalue[1-player] + nOffensiveValue;
}
*/

// �`���˴��J�z�L������v�O������zobrist��ȨӧP�_�C
// ��i��סJ�ϥηL��Hash��ALoopHash[zobrist & LoopMask] = zobrist  LoopMask=1023=0B1111111111  �i�H�٥h�˴��L�{�����`���P�_�C
// ��{�����誺��өΦh�ӴѤl�A�b��өΪ̨�ӥH�W����m�`�����ƹB�ʡC
// �ھڤ���H�Ѫ��ѳW�A����N�x�X�{�`���A���ܧ@�t�C�U���C�|���յ{���ɵo�{���N�x�`�������J
	// 5.         10101  01010  00100 10001 11111
	// 6.        001010
	// 7.       1001001
	// 8.      00001010
	// 9.     100000001  101000001  010001000 000001000
	//12. 1000000000001  0000001000000
// �p���ݨӡA�U�ئU�˪��`�����i��X�{�C�`�����Ĥ@�B�i�H�O�Y�l���ʡA�H�᪺�O�D�Y�l���ʡC
// �̤�5�B�ѥi�H�c���`���A�D�Y�l���ʪ��ּ̤ƥجO4�C5�`���O�̱`���������A6��120���`�������A�H�ѴѳW�èS���w�q�C
// �`���˴��A��ڤW�_��ŪK���@�ΡA�i�H��p�j���ƪ�����C�p�G���i��B�z�A�a�������{�����ɷ|�L�k�h�X�C
int CSearch::RepetitionDetect(int nRepeatNum)
{
	// 120�B(60�^�X)�L���l�A�����F��۵M���ۡA�P�w���M��
	// NaturalBou�]�i�H�O�䥦�ȡA�Ѭɭ��{���ӵo�e�A�p50�^�X�C
	// �g�b�o�̬O���n���A�H�K�����b�u�ժ����p�U�M�ѡC
	if(nNonCapNum >= NaturalBouts)
		return(-NaturalBouts);
	
	unsigned int m, n;
	unsigned long *pBoard = &nZobristBoard[nCurrentStep-1];
	
	for(m=4; m<=nNonCapNum; m++)	
	{
		if( *pBoard == *(pBoard-m) )		// Zobrist�ۦP�A�c���`��
		{
			// �έp�`�����X�{���N�x���ơC
			CChessMove *pMove = &StepRecords[nCurrentStep-1];
			int nOwnChecks = 1;
			int nOppChecks = 1;
			for(n=0; n<=m; n++)
			{
				if((*(pMove-n)) & 0xFF000000)
				{
					if( 1 & n )
					{
						if( nOppChecks )			// �Y���S���s�N�A�p�ƾ����W�[
							nOppChecks ++;			// �έp��誺�s�N����
					}
					else
					{
						if( nOwnChecks )			// �Y�ڤ�S���s�N�A�p�ƾ����W�[
							nOwnChecks ++;			// �έp�ڤ誺�s�N����
					}
				}
				else								// �@�����@���S���N�x�A�h����N���c���s�N
				{
					if( 1 & n )
						nOppChecks = 0;
					else
						nOwnChecks = 0;
				}

				// ���賣�S�����N�A�M�ѡA�����~���˴�
				if( !nOwnChecks && !nOppChecks )
					break;
			}

			// �d�ݴ`��������
			// �ڤ���N���, ���@�w�O���ʪ��Ѥl�N�x, ���ʦZ��L���Ѥl�N�x�]�ݤ_�����C
			if( nOwnChecks>=nRepeatNum && !nOppChecks )				// �p 10101
				return 1;

			// �����N�ڤ�A�D���v�b�ڤ�A�]���̫�@�B�ڤ貾�ʡC
			// �̫Ჾ�ʪ���m�A���ɬO�Q�����A���ɥi�H�ۤv�D�ʺc���`���A�y�����ǳW�C
			else if( nOppChecks>=nRepeatNum && !nOwnChecks )		// �p 01010
				return 2;
			
			// ��L���p�A�p�������A���Φ��N�x�A�����M�ѡC
			// ����H�Ѫ��ѳW�۷�����A�Ҧp���׹��Y���Ѥl���U�@�B�N�x�A���M�`���餺���c���N�x�A���Y�����׫h�����L�áC
			// �ھڴѳW�A�����p������褣�ܬ��t�C��{����k�۷�����C
			else
				return -int(m);
		}
	}

	return(0);
}


// �`������^��
// ��ѡJ - WINSCORE * 2	���O���_Hash��
// Ĺ�ѡJ + WINSCORE * 2	���O���_Hash��
// �M�ѡJ���� * �Ƶ��]�l    
int CSearch::LoopValue(int Player, int ply, int nLoopStyle)
{
	// �ڤ�`���A�O�ѩ���N���A�G�ӹ��ӧQ�C���O���bHash��C
	// �`���˴��W�@�h�����k�A���h�٨S�����ѡA�G�Өϥ�ply-1
	if( nLoopStyle == 2 )
		return (ply-1)-WINSCORE*2;

	// �����N�A�ڤ�ӧQ�A��^�ӧQ�����ơC���O���bHash��C
	// �`���˴��W�@�h�����k�A���h�٨S�����ѡA�G�Өϥ�ply-1
	else if( nLoopStyle == 1 )
		return WINSCORE*2-(ply-1);

	// �M�ѡJ���Ƶ��]�l��(ContemptFactor=1/8=0.125)�C
	// �u�ծɡAEvaluation(Player)�����j�_0�A��^�t�ơA�{���i�����e���`���A�M�䦸�n���۪k�A�קK�M�ѡC
	// �H�ծɡAEvaluation(Player)�����p�_0�A��^���ơA�{���̵M�l�D��e���`���A60�^�X���L���l�A�۵M�M�ѡC
	// ���ծɡAEvaluation(Player)����_0�A��^�ȧ󱵪�0�A�{���ܩ۪��i��ʫܤp�A�H�K�_�i�ӿ�ѡC
	else // nLoopStyle < 0 
		return -(m_Evalue.Evaluation(Player) >> 3);
	// ��H���@�k�O�ϥΤ@���Ƶ�����ContemptValue�A�u�լ��t�A��ܵ{���QĹ�ѡF�z�լ����A��ܵ{���Q�M�ѡC
	// �ھڦ�v�Ѫ��g��A�}����0.50�ӧL�A������0.25�ӧL�A�ݧ�����_0�C
}


// ���_�������
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
