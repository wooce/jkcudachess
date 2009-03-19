////////////////////////////////////////////////////////////////////////////////////////////////////////////
// �Y���JHashTable.h                                                                                    //
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
#include "FenBoard.h"

#pragma once


const int WINSCORE = 10000;				// �ӧQ������WINSCORE�A���Ѭ�-WINSCORE
const int MATEVALUE = 9000;				// score<-MATEVALUE || score>+MATEVALUE, �����O�ӱN�x������
//const int INT_MIN = -2147483648L;
//const int INT_MAX = 2147483647L;
//const int SHRT_MAX = 32767;

// �`�q
const int BookUnique = 1;
const int BookMulti = 2;
const int BookExist = BookUnique | BookMulti;
const int HashAlpha = 4;
const int HashBeta = 8;
const int HashPv = 16;
const int HashExist = HashAlpha | HashBeta | HashPv;
const int MaxBookMove = 15;


// �}���w�۪k�[�c
struct CBookRecord
{
	int MoveNum;
	CChessMove MoveList[MaxBookMove];
};

// �����O���[�c
struct CHashRecord					// 14Byte --> 16Byte
{
	unsigned __int64   zobristlock;	// 8 byte  64�����
	unsigned char      flag;        // 1 byte  flag==0, Hash�ȳQ�M��
	char			   depth;		// 1 byte  �j���`��
	short			   value;       // 2 byte  ����
	unsigned short     move;		// 2 byte
};


// Hash����
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

	unsigned long  nHashSize;			// Hash����ڤj�p
	unsigned long  nHashMask;			// nHashMask = nHashSize-1;
	CHashRecord    *pHashList[2];		// Hash��A�¬�����U�Τ@�ӡA�H�K�o�ͽĬ�A�o�˸ѨM�̹���
										// �ѦҤ���K�ۡmPC�C���s�{----�H���ի�.pdf�n

	unsigned int   nMaxBookPos;
	unsigned int   nBookPosNum;
	CBookRecord    *pBookList;


// �ոհT��
public:
	unsigned long nCollision;	//Hash�Ĭ�p�ƾ�
	unsigned long nHashAlpha, nHashExact, nHashBeta;
	unsigned long nHashCovers;


public:
	// ��Hash����o�M�}���w���o���s
	unsigned long NewHashTable(unsigned long nHashPower, unsigned long nBookPower);

	// �M��Hash��M�}���w
	void DeleteHashTable();

	// Hash��M�s�A�åB��^Hash���л\�v
	float ClearHashTable(int style=0);

	// ��l�ƻPHash��������Ѥl�ƾ�
	void InitZobristPiecesOnBoard(int *Piece);

	// Hash����
	int ProbeHash(CChessMove &hashmove, int alpha, int beta, int nDepth, int ply, int player);

	// Hash�s�x
	void RecordHash(const CChessMove hashmove, int score, int nFlag, int nDepth, int ply, int player);
	
	// ��l�ƶ}���w
	int LoadBook(const char *BookFile);

	// �b�}���w���M��۪k
	int ProbeOpeningBook(CChessMove &BookMove, int Player);

private:
	// 32���H���Ƶo�;�
	unsigned long Rand32U();

	// ��l��ZobristKeyTable[14][256]�MZobristLockTable[14][256]�A�ᤩ32��M64���H�Y��
	// �u�ݤ@���A�bCHashTable()�c�y��Ƥ��۰ʶi��A�L�ݤ����ե�
	// ���D���s�Ұʵ{���A�_�h�H�Y�ƱN���|����
	void ZobristGen(void);
};
