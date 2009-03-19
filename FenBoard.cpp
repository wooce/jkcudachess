////////////////////////////////////////////////////////////////////////////////////////////////////////////
// �����JFenBoard.cpp                                                                                   //
// *******************************************************************************************************//
// ����H�ѳq�Τ���----�L�e���|�A����m����H�ѳq�Τ�����ĳ�n(Universal Chinese Chess Protocol�A²��ucci) //
// �@�̡J �S �w �x                                                                                        //
// ���J �����l���Ǭ�s�|                                                                            //
// �l�c�J fan_de_jun@sina.com.cn                                                                          //
//  QQ �J 83021504                                                                                        //
// *******************************************************************************************************//
// �\��J                                                                                                 //
// 1. �Nfen����Ƭ��ѽL�T��                                                                               //
// 2. �N�ѽL�T����Ƭ�fen��                                                                               //
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include "FenBoard.h"


static const char PieceChar[14] = { 'k', 'r', 'c', 'h', 'b', 'a', 'p', 'K', 'R', 'C', 'H', 'B', 'A', 'P' };


CFenBoard::CFenBoard(void)
{
	// �]�w����l�����A������
	strcpy(FenStr, "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR r - - 0 1");
}

CFenBoard::~CFenBoard(void)
{
}


// �N�Ѥl�Ѧr�Ŧ���Ƭ��Ʀr��
// �ʤ�J�u���ഫ�j�g�r��(����Ѥl)�A�Y�J�¦�Ѥl�A�i�N��-32�A�p�g�ܬ��j�g
int CFenBoard::FenToPiece(char fen) 
{
	if( fen>='a' && fen<='z' )
		fen -= 32;

	switch (fen) 
	{
		case 'K':			//King
			return 0;
		case 'R':			//Rook
			return 1;
		case 'C':			//Cannon
			return 2;
		case 'N':			//Knight
		case 'H':			//Horse
			return 3;
		case 'B':			//Bishop
		case 'E':			//Elephant
			return 4;
		case 'A':			//Advisor
		case 'G':			//Guard
			return 5;		
		default:
			return 6;		//Pawn
	}
}


char* CFenBoard::BoardToFen(const int *Board, int Player, const unsigned int nNonCapNum, const unsigned int nCurrentStep, unsigned int *StepRecords)
{
	int x,y;
	unsigned int m,n,p=0;

	strcpy(FenStr, "");
	//char *FenStr = "";

	// �O�s�ѽL
	for(y=3; y<13; y++)
	{
		m = 0;												//�Ů�p�ƾ�
		for(x=3; x<12; x++)
		{
			n = Board[ (y<<4) | x ];
			if( n )
			{
				if(m > 0)									//���J�Ů��
					FenStr[p++] = char(m + '0');
				FenStr[p++] = PieceChar[nPieceType[n]];		//�O�s�Ѥl�r��
				m = 0;
			}
			else
				m ++;
		}

		if(m > 0)											//���J�Ů��
			FenStr[p++] = char(m + '0');		
		FenStr[p++] = '/';									//���J����j��
	}

	// �h���̫�@��'/'
	FenStr[--p] = '\0';

	// " ���ʤ� - - �L���l�b�^�X�� ��e�b�^�X��"
	//strcat(FenStr, Player ? " r " : " b ");
	//strcat(FenStr, itoa(10, FenStr, nNonCapNum));
	//strcat(FenStr, " ");
	//strcat(FenStr, itoa(10, FenStr, nCurrentStep));
	char str[32];
	sprintf(str, " %c - - %u %u", Player?'r':'b', nNonCapNum, nCurrentStep);
	//FenStr += strlen(FenStr);
	strcat(FenStr, str);
	p = (unsigned int)strlen(FenStr);
	
	// Save Moves
	if(nCurrentStep>1)
	{
		strcat(FenStr, " moves");
		p += 6;

		for(m=1; m<nCurrentStep; m++)
		{
			x = (StepRecords[m] & 0xFF00) >> 8;		// �_�l��m
			y =  StepRecords[m] & 0xFF;				// �פ��m

			FenStr[p++] = ' ';
			FenStr[p++] = char(x & 0xF) -  3 + 'a';
			FenStr[p++] = 12 - char(x >> 4 ) + '0';
			FenStr[p++] = char(y & 0xF) -  3 + 'a';
			FenStr[p++] = 12 - char(y >> 4 ) + '0';
		}

		// ����
		FenStr[p] = '\0';
	}

	return FenStr;
}


// �NFen����Ƭ��ѽL�T���JBoard[256], Piece[48], Player, nNonCapNum, nCurrentStep
// �`�N�J���F�[����B�t�סA����ƥ���ѽL�T�����X�k�ʧ@��������A�ҥHFen�ꥲ���O�X�k���C
// �Ҧp�J�C��Ѥl�ƥضW�L9�ӡB�Ѥl�ƥؿ��~�B�Ѥl��m�D�k�����C
int CFenBoard::FenToBoard(int *Board, int *Piece, int &Player, unsigned int &nNonCapNum, unsigned int &nCurrentStep, const char *FenStr)
{
	unsigned int m, n;
	int BlkPiece[7] = { 16, 17, 19, 21, 23, 25, 27 };
	int RedPiece[7] = { 32, 33, 35, 37, 39, 41, 43 };

	// �M�ŴѽL�ƲթM�Ѥl�Ʋ�	
	for(m=0; m<256; m++)
		Board[m] = 0;
	for(m=0; m<48; m++)
		Piece[m] = 0;

	// Ū���Ѥl��m�T���A�P�ɱN����Ƭ��ѽL����Board[256]�M�Ѥl����Piece[48]
	int x = 3;
	int y = 3;
	char chess = *FenStr;
	while( chess != ' ')							// �ꪺ���q�аO
	{
		if(*FenStr == '/')							// ����аO
		{
			x = 3;									// �q���}�l
			y ++;									// �U�@��
			if( y >= 13 )
				break;
		}
		else if(chess >= '1' && chess <= '9')		// �Ʀr��ܪŮ�(�L�Ѥl)���ƥ�
		{
			n = chess - '0';						// �s��L�l���ƥ�
			for(m=0; m<n; m++) 
			{
				if(x >= 12)
					break;
				x++;
			}
		} 
		else if (chess >= 'a' && chess <= 'z')		// �¦�Ѥl
		{
			m = FenToPiece( chess - 32 );			// 'A' - 'a' = -32, �ت��N�O�N�p�g�r����Ƭ��j�g�r��
			if(x < 12) 
			{
				n = BlkPiece[m];
				Board[ Piece[n] = (y<<4)|x ] = n;
				BlkPiece[m] ++;
			}
			x++;
		}
		else if(chess >= 'A' && chess <= 'Z')		// ����Ѥl
		{
			m = FenToPiece( chess );				// ����ƥu���ѧO�j�g�r��
			if(x < 12) 
			{
				n = RedPiece[m];
				Board[ Piece[n] = (y<<4)|x ] = n;
				RedPiece[m] ++;
			}
			x++;
		}
		
		// Next Char
		chess = *(++FenStr);
		if( chess == '\0' )
			return 0;
	}

	// Ū����e���ʤ�Player: b-�¤�A !black = white = red  ����
	if(*(FenStr++) == '\0')
		return 1;
	Player = *(FenStr++) == 'b' ? 0:1;

	// Skip 2 Reserved Keys
	if(*(FenStr++) == '\0')    return 1;		// ' '
	if(*(FenStr++) == '\0')    return 1;      // '-'
	if(*(FenStr++) == '\0')    return 1;      // ' '
	if(*(FenStr++) == '\0')    return 1;      // '-'
	if(*(FenStr++) == '\0')    return 1;      // ' '
	

	// �Y�ɭ����ǰe�Y�l�H�e���۪k�A�����i�H
	// �ѨM�ѧ��A�i�H�ϥΤU�����Ѽ�	
	nNonCapNum   = 0;
	nCurrentStep = 1;

	return 1;

/*
	// ����S���Y�l�����ѨB��(�b�^�X��), �q�`�ӭȹF��120�N�n�P�M(���Q�^�X�۵M����)�A�@���Φ��������W�@�B�O�Y�l�A�o�̴N�аO��0���C 
	m = 0;
	while(*FenStr != ' ') 
	{
		if (*FenStr >= '0' && *FenStr <= '9') 
		{
			m *= 10;
			m += *FenStr - '0';
		}
		nNonCapNum = m;
		if(*(FenStr++) == '\0')    
			return;
	}
	if(*(FenStr++) == '\0')    
		return;      // ' '

	// ��e���^�X�ơA�b��s�����αƧ��ɡA�@����s��H�������A�o�@���i�H�g1�A�H�ۧ��ժ��o�i�v���W�[�C
	m = 0;
	while (*FenStr != ' ') 
	{
		if (*FenStr >= '0' && *FenStr <= '9') 
		{
			m *= 10;
			m += *FenStr - '0';
		}
		nCurrentStep = m;

		if(*(FenStr++) == '\0')    
			return;
	}

	// �X�k������
	if( nCurrentStep < 1 )					// ���ƥ����j�_�Ϊ̵��_1�C
		nCurrentStep = 1;	
	if( nNonCapNum >= nCurrentStep )		// ���Ƥ��൥�_�Ϊ̶W�L��e���^�X�ơC
		nNonCapNum = nCurrentStep - 1;

	// Read Moves;
	// �o�������e�����g�A�D�{����CommPosition�R�O�i�H�i��B�z
	
*/
}