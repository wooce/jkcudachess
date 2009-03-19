////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 源文件︰FenBoard.cpp                                                                                   //
// *******************************************************************************************************//
// 中國象棋通用引擎----兵河五四，支持《中國象棋通用引擎協議》(Universal Chinese Chess Protocol，簡稱ucci) //
// 作者︰ 范 德 軍                                                                                        //
// 單位︰ 中國原子能科學研究院                                                                            //
// 郵箱︰ fan_de_jun@sina.com.cn                                                                          //
//  QQ ︰ 83021504                                                                                        //
// *******************************************************************************************************//
// 功能︰                                                                                                 //
// 1. 將fen串轉化為棋盤訊息                                                                               //
// 2. 將棋盤訊息轉化為fen串                                                                               //
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include "FenBoard.h"


static const char PieceChar[14] = { 'k', 'r', 'c', 'h', 'b', 'a', 'p', 'K', 'R', 'C', 'H', 'B', 'A', 'P' };


CFenBoard::CFenBoard(void)
{
	// 設定為初始局面，紅先手
	strcpy(FenStr, "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR r - - 0 1");
}

CFenBoard::~CFenBoard(void)
{
}


// 將棋子由字符行轉化為數字型
// 缺欠︰只能轉換大寫字符(紅色棋子)，若遇黑色棋子，可將其-32，小寫變為大寫
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

	// 保存棋盤
	for(y=3; y<13; y++)
	{
		m = 0;												//空格計數器
		for(x=3; x<12; x++)
		{
			n = Board[ (y<<4) | x ];
			if( n )
			{
				if(m > 0)									//插入空格數
					FenStr[p++] = char(m + '0');
				FenStr[p++] = PieceChar[nPieceType[n]];		//保存棋子字符
				m = 0;
			}
			else
				m ++;
		}

		if(m > 0)											//插入空格數
			FenStr[p++] = char(m + '0');		
		FenStr[p++] = '/';									//插入行分隔符
	}

	// 去掉最後一個'/'
	FenStr[--p] = '\0';

	// " 移動方 - - 無殺子半回合數 當前半回合數"
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
			x = (StepRecords[m] & 0xFF00) >> 8;		// 起始位置
			y =  StepRecords[m] & 0xFF;				// 終止位置

			FenStr[p++] = ' ';
			FenStr[p++] = char(x & 0xF) -  3 + 'a';
			FenStr[p++] = 12 - char(x >> 4 ) + '0';
			FenStr[p++] = char(y & 0xF) -  3 + 'a';
			FenStr[p++] = 12 - char(y >> 4 ) + '0';
		}

		// 結束
		FenStr[p] = '\0';
	}

	return FenStr;
}


// 將Fen串轉化為棋盤訊息︰Board[256], Piece[48], Player, nNonCapNum, nCurrentStep
// 注意︰為了加快營運速度，此函數未對棋盤訊息的合法性作任何檢驗，所以Fen串必須是合法的。
// 例如︰每行棋子數目超過9個、棋子數目錯誤、棋子位置非法等等。
int CFenBoard::FenToBoard(int *Board, int *Piece, int &Player, unsigned int &nNonCapNum, unsigned int &nCurrentStep, const char *FenStr)
{
	unsigned int m, n;
	int BlkPiece[7] = { 16, 17, 19, 21, 23, 25, 27 };
	int RedPiece[7] = { 32, 33, 35, 37, 39, 41, 43 };

	// 清空棋盤數組和棋子數組	
	for(m=0; m<256; m++)
		Board[m] = 0;
	for(m=0; m<48; m++)
		Piece[m] = 0;

	// 讀取棋子位置訊息，同時將其轉化為棋盤坐標Board[256]和棋子坐標Piece[48]
	int x = 3;
	int y = 3;
	char chess = *FenStr;
	while( chess != ' ')							// 串的分段標記
	{
		if(*FenStr == '/')							// 換行標記
		{
			x = 3;									// 從左開始
			y ++;									// 下一行
			if( y >= 13 )
				break;
		}
		else if(chess >= '1' && chess <= '9')		// 數字表示空格(無棋子)的數目
		{
			n = chess - '0';						// 連續無子的數目
			for(m=0; m<n; m++) 
			{
				if(x >= 12)
					break;
				x++;
			}
		} 
		else if (chess >= 'a' && chess <= 'z')		// 黑色棋子
		{
			m = FenToPiece( chess - 32 );			// 'A' - 'a' = -32, 目的就是將小寫字符轉化為大寫字符
			if(x < 12) 
			{
				n = BlkPiece[m];
				Board[ Piece[n] = (y<<4)|x ] = n;
				BlkPiece[m] ++;
			}
			x++;
		}
		else if(chess >= 'A' && chess <= 'Z')		// 紅色棋子
		{
			m = FenToPiece( chess );				// 此函數只能識別大寫字符
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

	// 讀取當前移動方Player: b-黑方， !black = white = red  紅方
	if(*(FenStr++) == '\0')
		return 1;
	Player = *(FenStr++) == 'b' ? 0:1;

	// Skip 2 Reserved Keys
	if(*(FenStr++) == '\0')    return 1;		// ' '
	if(*(FenStr++) == '\0')    return 1;      // '-'
	if(*(FenStr++) == '\0')    return 1;      // ' '
	if(*(FenStr++) == '\0')    return 1;      // '-'
	if(*(FenStr++) == '\0')    return 1;      // ' '
	

	// 若界面不傳送吃子以前的著法，完全可以
	// 解決棋局，可以使用下面的參數	
	nNonCapNum   = 0;
	nCurrentStep = 1;

	return 1;

/*
	// 雙方沒有吃子的走棋步數(半回合數), 通常該值達到120就要判和(六十回合自然限著)，一旦形成局面的上一步是吃子，這裡就標記“0”。 
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

	// 當前的回合數，在研究中局或排局時，作為研究對象的局面，這一項可以寫1，隨著局勢的發展逐漸增加。
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

	// 合法性檢驗
	if( nCurrentStep < 1 )					// 此數必須大于或者等于1。
		nCurrentStep = 1;	
	if( nNonCapNum >= nCurrentStep )		// 此數不能等于或者超過當前的回合數。
		nNonCapNum = nCurrentStep - 1;

	// Read Moves;
	// 這部分內容不必寫，主程式中CommPosition命令可以進行處理
	
*/
}