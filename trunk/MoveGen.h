////////////////////////////////////////////////////////////////////////////////////////////////////////////
// �Y���JMoveGen.h                                                                                      //
// *******************************************************************************************************//
// ����H�ѳq�Τ���----�L�e���|�A����m����H�ѳq�Τ�����ĳ�n(Universal Chinese Chess Protocol�A²��ucci) //
// �@�̡J �S �w �x                                                                                        //
// ���J �����l���Ǭ�s�|                                                                            //
// �l�c�J fan_de_jun@sina.com.cn                                                                          //
//  QQ �J 83021504                                                                                        //
// *******************************************************************************************************//
// �\��J                                                                                                 //
// 1. ��¦����CMoveGen, CSearch�l���~�Ӥ��C�ѽL�B�Ѥl�B���B��C�B�۪k���ƾڦb�o�������Q�w�q�C           //
// 2. �q�β��ʲ��;�                                                                                      //
// 3. �Y�l���ʲ��;�                                                                                      //
// 4. �N�x�k�ײ��ʲ��;�                                                                                  //
// 5. ���Ⲿ�ʦX�k������                                                                                  //
// 6. �N�x�˴�Checked(Player), Checking(1-Player)                                                         //
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "FenBoard.h"

#pragma once


// �ѽL�ƲթM�Ѥl�Ʋ�
extern int Board[256];								// �ѽL�ƲաA��ܴѤl�Ǹ��J0��15�A�L�l; 16��31,�¤l; 32��47, ���l�F
extern int Piece[48];								// �Ѥl�ƲաA��ܴѽL��m�J0, ���b�ѽL�W; 0x33��0xCC, �����ѽL��m�F

// ���P��C�ѽL�Ʋ�
extern unsigned int xBitBoard[16];					// 16�Ӧ��A���ͨ�������V���ʡA�e12�즳��
extern unsigned int yBitBoard[16];					// 16�Ӧ�C�A���ͨ������a�V���ʡA�e13�즳��

// ���P��C�ѽL����
extern const int xBitMask[256];
extern const int yBitMask[256];

// ������V�P�a�V���ʪ�16��ѽL�A�u�Τ_���Ⲿ�ʦX�k������B�N�x�˴��M�N�x�k��   							          
extern unsigned short xBitRookMove[12][512];		//  12288 Bytes, �������ѽL
extern unsigned short yBitRookMove[13][1024];		//  26624 Bytes  ������C�ѽL
extern unsigned short xBitCannonMove[12][512];		//  12288 Bytes  �������ѽL
extern unsigned short yBitCannonMove[13][1024];	    //  26624 Bytes  ������C�ѽL
									      // Total: //  77824 Bytes =  76K
extern unsigned short HistoryRecord[65535];		// ���v�ҵo�A�ƲդU�Ь�: move = (nSrc<<8)|nDst;

extern const char nHorseLegTab[512];
extern const char nDirection[512];

class CMoveGen
{
public:
	CMoveGen(void);
	~CMoveGen(void);

	//unsigned short HistoryRecord[65535];		// ���v�ҵo�A�ƲդU�Ь�: move = (nSrc<<8)|nDst;

// �ոհT��
public:
	unsigned int nCheckCounts;
	unsigned int nNonCheckCounts;
	unsigned int nCheckEvasions;

// ��k
public:	
	// ��s���v�O���A�M�s�Ϊ̰I��
	void UpdateHistoryRecord(unsigned int nMode=0);

	// �q�β��ʲ��;�
	int MoveGenerator(const int player, CChessMove* pGenMove);

	// �Y�l���ʲ��;�
	int CapMoveGen(const int player, CChessMove* pGenMove);

	// �N�x�k�ײ��ʲ��;�
	int CheckEvasionGen(const int Player, int checkers, CChessMove* pGenMove);
	
	// ���Ⲿ�ʦX�k������
	int IsLegalKillerMove(int Player, const CChessMove KillerMove);

	// �N�x�˴��A�ߧY��^�O�_�A�Τ_�ڤ�O�_�Q�N�x
	int Checked(int player);

	// �N�x�˴��A��^�N�x�����A�Τ_���O�_�Q�N�x
	int Checking(int Player);

	// �O�@�P�_
	int Protected(int Player, int from, int nDst);

private:
	// ����Ѥlpiece�O�_����qnSrc���ʨ�nDst�A�Y���\�[�J�쨫�k���CChessMove��
	int AddLegalMove(const int piece, const int nSrc, const int nDst, CChessMove *ChessMove);
};
