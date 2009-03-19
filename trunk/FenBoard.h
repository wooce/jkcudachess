////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 頭文件︰FenBoard.h                                                                                     //
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


#pragma once

#define  CChessMove  unsigned int


const int nPieceType[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// 無子
	                       0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,  6,  6,  6,		// 黑子︰帥車炮馬象士卒
					       7,  8,  8,  9,  9, 10, 10, 11, 11, 12, 12, 13, 13, 13, 13, 13  };	// 紅子︰將車炮馬相仕兵

const int nPieceID[] = {  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// 無子
	                       0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,  6,  6,  6,		// 黑子︰帥車炮馬象士卒
					       0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,  6,  6,  6  };	// 紅子︰將車炮馬相仕兵

class CFenBoard
{
public:
	CFenBoard(void);
	~CFenBoard(void);

public:	
	// 傳遞棋局的字元串。字元串必須足夠長，否則當回合數太大時，moves超出，與棋盤相互串換時會發生死循環。
	char FenStr[2048];

public:
	// 將fen串轉化為棋局訊息，返回成功或失敗的標誌
	int FenToBoard(int *Board, int *Piece, int &Player, unsigned int &nNonCapNum, unsigned int &nCurrentStep, const char *FenStr);

	// 將當前棋局轉化為fen串，返回串的指針
	char *BoardToFen(const int *Board, int Player, const unsigned int nNonCapNum=0, const unsigned int nCurrentStep=1, CChessMove *StepRecords=0);

private:
	// 將fen字符轉化為棋子序號
	int FenToPiece(char fen);	
};


inline unsigned int Coord(const CChessMove move)
{
    unsigned char RetVal[4];
	unsigned int  src = (move & 0xFF00) >> 8;
	unsigned int  dst = move & 0xFF;

	RetVal[0] = unsigned char(src & 0xF) -  3 + 'a';
	RetVal[1] = 12 - unsigned char(src >> 4 ) + '0';
	RetVal[2] = unsigned char(dst & 0xF) -  3 + 'a';
	RetVal[3] = 12 - unsigned char(dst >> 4 ) + '0';

	return *(unsigned int *) RetVal;
}

inline CChessMove Move(const unsigned int MoveStr) 
{
	unsigned char *ArgPtr = (unsigned char *) &MoveStr;
	unsigned int src = ((12-ArgPtr[1]+'0')<<4) + ArgPtr[0]-'a'+3;	// y0x0
	unsigned int dst = ((12-ArgPtr[3]+'0')<<4) + ArgPtr[2]-'a'+3;	// y1x1
	return ( src << 8 ) | dst;										// y0x0y1x1
}