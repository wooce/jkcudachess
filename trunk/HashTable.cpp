////////////////////////////////////////////////////////////////////////////////////////////////////////////
// �Y���JHashTable.cpp                                                                                  //
// *******************************************************************************************************//
// ����H�ѳq�Τ���----�L�e���|�A����m����H�ѳq�Τ�����ĳ�n(Universal Chinese Chess Protocol�A²��ucci) //
// �@�̡J �S �w �x                                                                                        //
// ���J �����l���Ǭ�s�|                                                                            //
// �l�c�J fan_de_jun@sina.com.cn                                                                          //
//  QQ �J 83021504                                                                                        //
// *******************************************************************************************************//
// �\��J                                                                                                 //
// 1. ���������oHash��A1��1024MB                                                                         //
// 2. Hash���� �P Hash�s�x                                                                                //
// 3. ��l�ƶ}���w                                                                                        //
// 4. �d��}���w�����۪k                                                                                  //
////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <stdlib.h>			// �H���Ƶo�;�rand()��ƪ��Y���
#include <stdio.h>			// ���ާ@
#include <time.h>			// ��l���H����
#include "HashTable.h"
#include "FenBoard.h"



CHashTable::CHashTable(void)
{
	//srand( (unsigned)time( 0 ) );		// ���ϥήɡA�C�������ͬۦP�������ȡA�H��K�ո�
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

//��Hash����o���s�J�H��[�J���s�˴��A�u�Ƥj�p�A�����\�h��^0
unsigned long CHashTable::NewHashTable(unsigned long nHashPower, unsigned long nBookPower)
{
	ZobristGen();

	nHashSize = 1<<(nHashPower-1); 
	nHashMask = nHashSize-1;						// �� & �N�� % �B��

	pHashList[0]  = new CHashRecord[nHashSize];		// �¤誺Hash��
	pHashList[1]  = new CHashRecord[nHashSize];		// ���誺Hash��
	
	nMaxBookPos = 1<<nBookPower;
	pBookList = new CBookRecord[nMaxBookPos];

	return nHashSize;
}

// ���񤺦s
void CHashTable::DeleteHashTable()
{
	// ����Hash�����s
	delete [] pHashList[0];
	delete [] pHashList[1];

	// ����}���w�����s
	delete [] pBookList;
}

// �M��Hash��A�åB��^Hash���л\�v
// style==0�A�����M��Hash�O���A�����ҰʩΪ̷s�شѧ��ɡA�ϥγo�ӰѼơC
// style!=0�A�������M���A�����ϥ�(Hash.depth - 2)�������A��_�U������z�J
// 1.���^�X�s�A2ply�ӥb�^�X�Z�A���쥻�訫�ѡC��e�j����MaxDepth-2�B�A�s�b�@���l��A�N�O�N�n�i�槹���j������K�A
// ��l����K���O�L�Ϊ��C�۹�_�s���j���Ө��A�����l��ַj���F��h�C���e��Hash�`�I�`�׭ȴ�2�A���n�����U�@�^�X
// MaxDepth-2���l��C�`�׭ȴ�2�H��A�MHash���edepth=1�Mdepth-2�����`�I�N���|�R���A���̵M������s���j���𴣨�
// HashMove�A�]�N�`�٤@�w���ɶ��C�ھڲ`���u���л\�����A�i��U�@�^�X�s���j���A���N�`�׭�depth<MaxDepth-2�H�e�A
// �j���𪺥D����|�����R���A�]���|�л\����Hash�`�I�F���Z�A�ѩ�`�׭Ȫ��W�[�A�N�|�v���л\�ۦP������Hash�`�I�C
// 2.�Y�ɭ��{�����}�Z�x��ҥ\��A�s���j���A�Y�ϦZ�x��ҨS���R���AHash���ֿn���״I�T���P�˯���Q�ΡC�b�W�w��
// �ɶ����A�����i�H�b����¦�W�j����`�C
// 3.��_���ѡAHash����ɤ��Ĩ��`���u�������A�MHash��ɡA�Ĩ������M������k�A�קK��^�Ȥ��ǽT�����D�C�Y�ϱ���
// �������]����Q�ΡAvalue<-MATEVALUE�ɡAvalue+=2�Fvalue>MATEVALUE�ɡAvalue-=2�C�Y�����W�۬��¹�ԡA�h���ӡ�1�C
// ���F�A�����P���j���Ҧ��A�W�[�w���ʡA�ĥΫO�I�����I---�����M���C
// 4.�M��Hash��A�O�ӤΨ�ӶO�ɶ����u�@�C�ҥH��ܤ�����XBestMove�Z�i��A�Y���Z�x�A�ȡ��C
// 5.���ժ���A�o�زM����ׯ���`��10��15%���j���ɶ��C
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
			// �p��Hash���л\�v
			//if(pHash0->flag)
			//	nHashCovers++;
			//if(pHash1->flag)
			//	nHashCovers++;

			// �J����ѡAHashFlag�m0�A��ܧ����M��
			if( (pHash0->value) < -MATEVALUE || (pHash0->value) > MATEVALUE )
				pHash0->flag = 0;
			if( (pHash1->value) < -MATEVALUE || (pHash1->value) > MATEVALUE )
				pHash1->flag = 0;

			// �`�׭ȴ� 2�A�U�@�^�X�٥i�H�Q��
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
	//�\�h��ƨϥΤW�����H���ơA0<=rand()<=32767, �u�ಣ��0��max(unsigned long)/2�������H���ơA��M�ܤ����áA�|�W�[�@�����Ĭ�X�v

	return ((unsigned long)(rand()+rand())<<16) ^ (unsigned long)(rand()+rand());	//��i�Z���ӬO���ä��G��
}


//�ھڴѤl����m�T���A��l��ZobristKey�MZobristLock
//���}�@�ӷs���ѧ��Ϊ̸��J�ѧ��ɡA����եΦ����
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

// ����Hash��A�Y�����\��^"INT_MIN"�F�Y�o�ͽĬ�A�h��^HashMove�A�лx��"INT_MAX"
// ���C�k�J
// 1. �۰����C�k�С�h(x) = ZobristKey % MaxHashZize =ZobristKey & (MaxHashSize-1)  M = MaxHashSize = 1<<k = 1024*1204, k=20 
//    ��2������ܡA�i�H�Ⱓ�k�ܦ���(�P)�B��A�t�׫ܧ֡C���O�o���O���C���j�ҡA2^k-1=11111111111111111111(B),�ȶȥΨ�FZobristKey�C20��T���A��12��T�����O
//    �ѩ�ĥΤFZobristKey�w�g�ɧ��ä��G���H���ơA�۫H�۰����C�k�|�u�@�o�ܦn�C�ߤ@�����I�O�s�b�_���{�H�A�Yh(x) �P ZobristKey���_���ۦP�C
// 2. ����������C�k�С�h(x)=[M/W((x*x)%W)] = (x*x)>>(w-k) = (x*x)>>(32-20) = (x*x)>>12  �e��sx<sqrt(W/M)�M���H�sx=n*sqrt(W)������r�|�Ĭ�
//    ��x*x�k����12��A�ѤU����20��A�������0��M-1�������ơC
// 3. Fibonacci(���i�ǫ��ƦC)�ۭ����C�k, h(x) = (x*2654435769)>>(32-k)  2654435769�O�ӽ�ơA�B2654435769/2^32 = 0.618 ���������I�A�Y�ϳs����ȳ��ৡ�ä��G
//    2^16  40503
//    2^32  2654435769					�˼ơJ 340573321   a%W
//    2^64  11400714819323198485
// CHashRecord *pHashIndex = pHashList[player] + ((ZobristKey * 2654435769)>>(32 - nHashPower));		// Fibonacci(���i�ǫ��ƦC)�ۭ����C�k
// �g����A�۰����C�k��B�ɶ��̧֡CFibonacci�D�n�A�Ψ���������k�M�첾�B��A�G�ӳt�פϦӤ��p²�檺��k�C
int CHashTable::ProbeHash(CChessMove &hashmove, int alpha, int beta, int nDepth, int ply, int player)
{
	CHashRecord HashIndex = pHashList[player][ZobristKey & nHashMask];								//����e�ѽLZobrist������Hash����}
	
	if( HashIndex.zobristlock == ZobristLock )				//�w�g�s�bHash��, �O�P�@�ѧ�
	{	
		if( HashIndex.flag & HashExist )
		{
			// �ץ��N�x������Hash�ȡA�Y�ϳo�ˡA�q�����M������̵u���u�AHash��R���Z���B�k���h�ǽT�ʡC
			// ���O�Z���N�x�����ƬO���T���A�󬰥i�[���O�A�ݧ��ɷj���t�ק�30��40%.
			// �N�x���۪k�P�`�׵L���A�L�צb���h�N�x�A�u�n�L���ۦP�A�K�i�H�ե�Hash�����ȡC
			bool bMated = false;
			if( HashIndex.value > MATEVALUE )				// ��ӧ���
			{
				bMated = true;
				HashIndex.value -= ply;						// ��h��e���^�X�ơA�Yscore = WINSCORE-Hash(ply)-ply, �ݭn��h���B�Ƥ~����ӡA�p������o��̵u���N�x���u
			}
			else if( HashIndex.value < -MATEVALUE )			// ���ѧ���
			{
				bMated = true;
				HashIndex.value += ply;						// �[�W��e���^�X�ơA�Yscore = Hash(ply)+ply-WINSCORE, �p���q���|��������h���^�X�A�x�j��ܡA���ݤH���Ѥ⨫�X�S�ۦӰk�ױN�x
			}

			if( HashIndex.depth >= nDepth || bMated )		// �a��ڸ`�I���ȧ��T�A�`���u���F�N�x�����A���޲`�סA�u�n�ॿ�T��^�Z���ӱѪ��^�X�ƧY�i
			{
				//�t�X�`�׭��N�ɷ|�X�{���D�A�����C�����M��Hash��C
				//�]���ĥβ`���u���л\�A��a��ڸ`�I���ȷ��̺�T���C
				//���O�H�e���L�h�j�������ȬO����T���A�`���u�������ө߱�o�Ǥ��e�C
				if(HashIndex.flag & HashBeta)			
				{
					if (HashIndex.value >= beta)
					{
						nHashBeta++;					// 95-98%
						return HashIndex.value;
					}
				}
				else if(HashIndex.flag & HashAlpha)		// �]���֤@�����j���A�q�Ӹ`�ٮɶ�
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

				// �ѤU�ܤֶq������A�O�@�ǭ��i�}������(���ܵo�{�A��ƥػPHashAlpha�۪�)�Abeta=+32767, �W����������|����
			}
		}

		// ��_NullMove���ʩM���`�I�A�S��HashMove�C
		if( !HashIndex.move )
			return(INT_MIN);

		// Hash���R���A�������ۦP�A����HashMove�C
		// �]�i��O�W�@�^�X��d�U�Ӫ����ʡA�M��Hash��ɡA�u�O��flag=0�AHashMove���M�O�d�b�O�����C
		// ����o�{�J�H��h���j���P�@�����A�ɶ��|�V�ӶV�֡A�̲��ͤ_���šC
		if( HashIndex.depth > 0 )
		{
			nCollision ++;
			hashmove = HashIndex.move;
			return INT_MAX;		// ��ܬۦP����������Hash��
		}
	}

	return(INT_MIN);		// ���O�ۦP���ѧ�
}

void CHashTable::RecordHash(const CChessMove hashmove, int score, int nFlag, int nDepth, int ply, int player)
{
	//����e�ѽLZobrist������Hash����}
	CHashRecord *pHashIndex = pHashList[player] + (ZobristKey & nHashMask);

	// �L�o�A�`���u����h
	if((pHashIndex->flag & HashExist) && pHashIndex->depth > nDepth)
		return;

	// �L�o�`�������C�`����������g�JHash��T
	//     ���p�Y�����u�s�b�`���A���\�j�����^��`������l�����ɡA���u���C�ӧ������N�Q�s�JHash��A���OĹ�ѴN�O��ѡC
	// �A���j���ɡA�S�J��o�����u�W���ѧ��A���S����������`�����u�A�z�פW�N���c���`���C���{���|�qHash���o�{�A���o��
	// ���u�|Ĺ�ѩΪ̿�ѡC�p�G�O��ѡA�{���h�|�Q�H�e�������~�ˡA��������W�o�����C��ڤW�A���⦸�O�i�H���A�����ਫ�T���A
	// �]���T���N�c���F�`���C
	//    �ݧ��ɡA�o�ر��p�𨣤��A�C���ǧ����A�{�����o�ظ��u�|Ĺ�o�ӧQ�F�z�ծɥi�H�x�j�ϧܡC
	//    �o�S�A�Ψ�`������^�Ȱ��D�C��ڤW�A��_���O�M�Ѫ������A�`�����۪k�q�Ӥ��O�̹x�j������Ҧ��C�Y�Ϯz�ձ��p�A
	// ���ϥδ`������~�i�H�����o��[�C��_�o�ӭ�z�A�`�������i�H��^�p�_-WINSCORE�Ϊ̤j�_+WINSCORE���ȡC��^��j����
	// ���W�@�h�ARecordHash()��ƴN�i�H�Q�γo�ӭȨӧP�_�X�{�F�`���������C�_�O���O���bHash���A�u�O²�檺��^�C��Z��
	// �j�����|�y���v�T�C
	if(score<-WINSCORE || score>+WINSCORE)
		return;

	// �ץ��N�x������Hash�ȡA�i�H��o�̵u���N�x���u�C
	if( score > MATEVALUE )
		score += ply;
	else if( score < -MATEVALUE )
		score -= ply;

	// �O��Hash��
	pHashIndex->zobristlock = ZobristLock;
	pHashIndex->flag    	= (unsigned char)nFlag;
	pHashIndex->depth   	= (char)nDepth;
	pHashIndex->value       = (short)score;
	pHashIndex->move        = (unsigned short)hashmove;		//�ȫO�s�Z16�쪺�T��
}

// �}���w�榡�J
// ���� �v�� �����A��b2e2 5895 rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1
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

	// �U��������{���ܶq�ΨӫO�s�ª��ѧ��A��Ƶ����ɥΨӫ�_�F
	// �]����z�}���w�ɡA�|����Hash����e�������A�_�h���������_��Ӫ������C
	unsigned long    Old_ZobristKey = ZobristKey;
	unsigned __int64 Old_ZobristLock = ZobristLock;

	if(!BookFile)
		return 0;

	// �H�uŪ�奻�Ҧ����}�}���w
	if((FilePtr = fopen(BookFile, "rt"))==0)
		return 0;										// �����\�A��^0

	nBookPosNum = 0;
	LineStr[254] = LineStr[255] = '\0';
	
	// �q�}���w��Ū��254�Ӧr��
	while(fgets(LineStr, 254, FilePtr) != 0)
	{
		// ����
		LinePtr = LineStr;
		BookMove = Move(*(long *) LinePtr);				// �N���ʱq�r�ū���Ƭ��Ʀr���ABookMove�C16�즳��

		if( BookMove )
		{
			// �v��
			LinePtr += 5;								// ���L�۪k��4�Ӧr�ũM1�ӪŮ�
			temp = 0;
			while(*LinePtr >= '0' && *LinePtr <= '9')
			{
				temp *= 10;
				temp += *LinePtr - '0';
				LinePtr ++;
			}
			
			BookMove |= temp<<16;						// �N�����ʪ��v��(�o��)�O�s�bBook����16��A�C16��O���ʪ��۪k

			// ����
			LinePtr ++;														// ���L�Ů�
			fen.FenToBoard(Board, Piece, Player, nNonCapNum, nCurrentStep, LinePtr);	// ��LinePtr�r������Ƭ��ѽL�ƾ�
			InitZobristPiecesOnBoard( Piece );								// �ھڴѽL�W���Ѥl�p��ZobristKey�MZobristLock

			if( Board[(BookMove & 0xFF00)>>8] )								// ����m���Ѥl�s�b
			{
				TempHash = pHashList[Player][ZobristKey & nHashMask];
				if(TempHash.flag)											// Hash�������e
				{
					if(TempHash.zobristlock == ZobristLock)					// �ӥB�O�ۦP������
					{
						if(TempHash.flag & BookUnique)						// �}���w���O�ߤ@�۪k
						{
							if(nBookPosNum < nMaxBookPos)					// �S���W�X�}���w���d��
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
						else															// �}���w������ӥH�W���ܩ� 
						{
							if(pBookList[TempHash.value].MoveNum < MaxBookMove)
							{
								pBookList[TempHash.value].MoveList[pBookList[TempHash.value].MoveNum] = BookMove;
								pBookList[TempHash.value].MoveNum ++;
							}
						}
					}
				} 
				else					// Hash���S����e�������A�g�JBestMove
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

	// ��_�ѧ��A�����i�H���ۭ�Ӫ��ѧ��~���١C
	ZobristKey = Old_ZobristKey;
	ZobristLock = Old_ZobristLock;

	return 1;
}



int CHashTable::ProbeOpeningBook(CChessMove &BookMove, int Player)
{
	CHashRecord TempHash = pHashList[Player][ZobristKey & nHashMask];
	
	if((TempHash.flag & BookExist) && TempHash.zobristlock == ZobristLock)
	{
		if(TempHash.flag & BookUnique)			// �}���w���s�b�ߤ@���۪k�A�R���C
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