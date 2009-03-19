////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 源文件︰PreMove.cpp                                                                                    //
// *******************************************************************************************************//
// 中國象棋通用引擎----兵河五四，支持《中國象棋通用引擎協議》(Universal Chinese Chess Protocol，簡稱ucci) //
// 作者︰ 范 德 軍                                                                                        //
// 單位︰ 中國原子能科學研究院                                                                            //
// 郵箱︰ fan_de_jun@sina.com.cn                                                                          //
//  QQ ︰ 83021504                                                                                        //
// *******************************************************************************************************//
// 功能︰                                                                                                 //
// 1. 初始化棋子（將車炮馬象士兵）的移動，產生偽合法的著法。                                              //
// 2. 初始化車炮位行與位列棋盤                                                                            //
//                                                                                                        //
// 注︰CPreMove類不包含實際的著法數組，初始化完成后，即可釋放此類，程式啟動后只需調用一次                 //
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreMove.h"


CPreMove::CPreMove()
{
}

CPreMove::~CPreMove()
{
}


// 將帥的移動，不包括雙王照面
// 初始化為將帥移動的目標格，nDst = *pMove
// 所有的非俘獲移動都合法，俘獲移動不一定合法，遇到吃自己的棋子應加以判別
void CPreMove::InitKingMoves(unsigned char KingMoves[][8])
{
	int i, m, n;
	unsigned char *pMove;
	int nKingMoveDir[4] = { -16, -1, 1, 16 };

	for(m=0x36; m<0xC9; m++)
	{
		if( nCityIndex[m] )										//在九宮內
		{
			pMove = KingMoves[m];
			for(i=0; i<4; i++)
			{
				n = m + nKingMoveDir[i];
				if( nCityIndex[n] )
					*(pMove++)  = unsigned char( n );
			}
			*pMove = 0;		// 表示不能再移動
		}
	}
}


// 初始化車的著法預產生數組︰
// 其值為行/列的相對位置x/y*16，計算目標格子的公式為︰nDst(y|x) = (nSrc & 0xF0) | x; 或者 nDst(y|x) = y | (nSrc & 0xF);  // x = *pMove; y = *pMove;
// 這一點與將、士、象、馬、兵的表示為棋子的絕對位置模式不同
// 所有的非俘獲移動都合法，俘獲移動不一定合法，遇到吃自己的棋子應加以判別
void CPreMove::InitRookMoves(unsigned char xRookMoves[][512][12], unsigned char yRookMoves[][1024][12], unsigned char xRookCapMoves[][512][4], unsigned char yRookCapMoves[][1024][4])
{
	int m, n, x, y;
	unsigned char *pMove, *pCapMove;

	// 初始化車的橫向移動
	for(x=3; x<12; x++)
	{
		for(m=0; m<512; m++)
		{
			pMove    = xRookMoves[x][m];
			pCapMove = xRookCapMoves[x][m];
			for(n=x-1; n>=3; n--)					// 向左搜索
			{
				*(pMove++) = unsigned char( n );
				if( m & ( 1 << (n-3) ) )			// 遇到棋子，產生俘獲
				{
					(*pCapMove++) = *(pMove-1);
					break;
				}
			}

			for(n=x+1; n<12; n++)					// 向右搜索
			{
				*(pMove++) = unsigned char( n );
				if( m & ( 1 << (n-3) ) )			// 遇到棋子，產生俘獲
				{
					(*pCapMove++) = *(pMove-1);
					break;
				}
			}

			*pMove    = 0;
			*pCapMove = 0;
		}
	}

	// 初始化車的縱向移動
	for(y=3; y<13; y++)
	{
		for(m=0; m<1024; m++)
		{
			pMove    = yRookMoves[y][m];
			pCapMove = yRookCapMoves[y][m];
			for(n=y-1; n>=3; n--)
			{
				*(pMove++) = unsigned char( n<<4 );
				if( m & ( 1 << (n-3) ) )			// 遇到棋子，產生俘獲
				{
					(*pCapMove++) = *(pMove-1);
					break;
				}
			}
			for(n=y+1; n<13; n++)
			{
				*(pMove++) = unsigned char( n<<4 );
				if( m & ( 1 << (n-3) ) )			// 遇到棋子，產生俘獲
				{
					(*pCapMove++) = *(pMove-1);
					break;
				}
			}
			
			*pMove    = 0;
			*pCapMove = 0;
		}
	}
}


// 初始化炮的著法預產生數組︰
// 其值為行/列的相對位置x/y*16，計算目標格子的公式為︰nDst(y|x) = (nSrc & 0xF0) | x; 或者 nDst(y|x) = y | (nSrc & 0xF);  // x = *pMove; y = *pMove;
// 這一點與將、士、象、馬、兵的表示為棋子的絕對位置模式不同
// 所有的非俘獲移動都合法，俘獲移動不一定合法，遇到吃自己的棋子應加以判別
void CPreMove::InitCannonMoves(unsigned char xCannonMoves[][512][12], unsigned char yCannonMoves[][1024][12], unsigned char xCannonCapMoves[][512][4], unsigned char yCannonCapMoves[][1024][4])
{
	int m, n, x, y;
	unsigned char *pMove, *pCapMove;

	//初始化炮的橫向移動
	for(x=3; x<12; x++)
	{
		for(m=0; m<512; m++)
		{
			pMove    = xCannonMoves[x][m];
			pCapMove = xCannonCapMoves[x][m];
			for(n=x-1; n>=3; n--)
			{
				if( m & ( 1 << (n-3) ) )			// 遇到炮架子
				{
					n--;							// 跨越炮架子
					break;
				}
				*(pMove++) = unsigned char( n );
			}
			for(; n>=3; n--)
			{
				if( m & ( 1 << (n-3) ) )			// 遇到棋子才移動
				{
					*(pMove++) = unsigned char( n );
					*(pCapMove++) = *(pMove-1);
					break;
				}
			}

			for(n=x+1; n<12; n++)
			{
				if( m & ( 1 << (n-3) ) )			// 遇到炮架子
				{
					n++;							// 跨越炮架子
					break;
				}
				*(pMove++) = unsigned char( n );
			}
			for(; n<12; n++)
			{
				if( m & ( 1 << (n-3) ) )			// 遇到棋子才移動
				{
					*(pMove++) = unsigned char( n );
					*(pCapMove++) = *(pMove-1);
					break;
				}
			}

			*pMove    = 0;
			*pCapMove = 0;	
		}
	}

	//初始化炮的縱向移動
	for(y=3; y<13; y++)
	{
		for(m=0; m<1024; m++)
		{
			pMove    = yCannonMoves[y][m];
			pCapMove = yCannonCapMoves[y][m];
			for(n=y-1; n>=3; n--)
			{
				if( m & ( 1 << (n-3) ) )			//遇到炮架子
				{
					n--;							//跨越炮架子
					break;
				}
				*(pMove++) = unsigned char( n<<4 );
			}
			for(; n>=3; n--)
			{
				if( m & ( 1 << (n-3) ) )			//遇到棋子才移動
				{
					*(pMove++) = unsigned char( n<<4 );
					*(pCapMove++) = *(pMove-1);
					break;
				}
			}

			for(n=y+1; n<13; n++)
			{
				if( m & ( 1 << (n-3) ) )			//遇到炮架子
				{
					n++;							//跨越炮架子
					break;
				}
				*(pMove++) = unsigned char( n<<4 );
			}
			for(; n<13; n++)
			{
				if( m & ( 1 << (n-3) ) )			//遇到棋子才移動
				{
					*(pMove++) = unsigned char( n<<4 );
					*(pCapMove++) = *(pMove-1);
					break;
				}
			}

			*pMove    = 0;
			*pCapMove = 0;
		}
	}
}


// 初始化馬的著法預產生數組︰
// 其值初始化為馬移動的絕對位置。
// 不合法移動︰絆馬腿的移動，吃己方棋子的移動，必須檢測。
void CPreMove::InitKnightMoves(unsigned char KnightMoves[][12])
{
	int i, m, n;
	unsigned char *pMove;
	int nHorseMoveDir[] = { -33, -31, -18, -14, 14, 18, 31, 33 };
	
	for(m=0x33; m<0xCC; m++)
	{
		if( nBoardIndex[m] )
		{
			pMove = KnightMoves[m];
			for(i=0; i<8; i++)
			{	
				n = m + nHorseMoveDir[i];
				if( nBoardIndex[n] )						// 馬在棋盤內	
					*(pMove++)  = unsigned char( n );
			}
			*pMove = 0;
		}
	}
}

// 初始化象的著法預產生數組︰
// 其值初始化為象移動的絕對位置。
// 不合法移動︰塞象眼的移動，吃己方棋子的移動，必須檢測。
void CPreMove::InitBishopMoves(unsigned char BishopMoves[][8])
{
	int i, m, n;
	unsigned char *pMove;
	int nBishopMoveDir[] = { -34, -30, 30, 34 };
	
	for(m=0x33; m<0xCC; m++)
	{
		if( nBoardIndex[m] )
		{
			pMove = BishopMoves[m];
			for(i=0; i<4; i++)
			{	
				n = m + nBishopMoveDir[i];
				if( nBoardIndex[n] && (m^n)<128 )			// (m^n)<128, 保證象不會過河
					*(pMove++)  = unsigned char( n );
			}
			*pMove = 0;
		}
	}
}


// 初始化士的著法預產生數組︰
// 其值初始化為士移動的絕對位置。
// 不合法移動︰吃己方棋子的移動，必須檢測。
void CPreMove::InitGuardMoves(unsigned char GuardMoves[][8])
{
	int i, m, n;
	unsigned char *pMove;
	int nGuardMoveDir[] = { -17, -15, 15, 17 };

	for(m=0x36; m<0xC9; m++)
	{
		if( nCityIndex[m] )										// 在九宮內
		{
			pMove = GuardMoves[m];
			for(i=0; i<4; i++)
			{
				n = m + nGuardMoveDir[i];
				if( nCityIndex[n] )								// 在九宮內
					*(pMove++)  = unsigned char( n );
			}
			*pMove = 0;		// 表示不能再移動
		}
	}
}


// 初始化兵卒的著法預產生數組︰
// 其值初始化為移動的絕對位置。
// 不合法移動︰吃己方棋子的移動，必須檢測。
void CPreMove::InitPawnMoves(unsigned char PawnMoves[][256][4])
{
	int i, m, n, Player;
	unsigned char *pMove;
	int PawnMoveDir[2][3] = { {16, -1, 1}, {-16, -1, 1} };							// 黑︰下左右；  紅︰上左右

	for(Player=0; Player<=1; Player++)												//Player=0, 黑卒； Player=1, 紅兵
	{
		for(m=0x33; m<0xCC; m++)
		{
			if( nBoardIndex[m] )
			{
				pMove = PawnMoves[Player][m];
				for(i=0; i<3; i++)
				{
					if( i>0 && ((!Player && m<128) || (Player && m>=128)) )			//未過河的兵不能左右晃動
						break;

					n = m + PawnMoveDir[Player][i];
					if( nBoardIndex[n] )										   //在棋盤範圍內
						*(pMove++)  = unsigned char( n );
				}
				*pMove = 0;
			}
		}
	}	
}


// *****************************************************************************************************************
// *****************************************************************************************************************

// 車的位行與位列棋盤︰包含吃子移動和非吃子移動
void CPreMove::InitBitRookMove( unsigned short xBitRookMove[][512], unsigned short yBitRookMove[][1024])
{
	int x, y, m, n, index;

	// 車的橫向移動
	for(x=3; x<12; x++)
	{
		for(m=0; m<512; m++)
		{
			xBitRookMove[x][m] = 0;

			// 向左移動
			for(n=x-1; n>=3; n--)
			{
				index = 1<<(n-3);
				xBitRookMove[x][m] |= index;
				if( m & index )						// 遇到棋子，產生俘獲
					break;
			}

			// 向右移動
			for(n=x+1; n<12; n++)
			{
				index = 1<<(n-3);
				xBitRookMove[x][m] |= index;
				if( m & index )						// 遇到棋子，產生俘獲
					break;
			}
		}
	}

	// 車的縱向移動
	for(y=3; y<13; y++)
	{
		for(m=0; m<1024; m++)
		{
			yBitRookMove[y][m] = 0;

			// 向上移動
			for(n=y-1; n>=3; n--)
			{
				index = 1<<(n-3);
				yBitRookMove[y][m] |= index;
				if( m & index )						// 遇到棋子，產生俘獲
					break;
			}

			// 向下移動
			for(n=y+1; n<13; n++)
			{
				index = 1<<(n-3);
				yBitRookMove[y][m] |= index;
				if( m & index )						// 遇到棋子，產生俘獲
					break;
			}
		}
	}
}


// 炮的位行與位列棋盤︰包含吃子移動和非吃子移動
void CPreMove::InitBitCannonMove( unsigned short xBitCannonMove[][512], unsigned short yBitCannonMove[][1024] )
{
	int x, y, m, n, index;

	// 炮的橫向移動
	for(x=3; x<12; x++)
	{
		for(m=0; m<512; m++)
		{
			xBitCannonMove[x][m] = 0;

			// 向左移動
			for(n=x-1; n>=3; n--)				// 非吃子移動
			{
				index = 1<<(n-3);
				if( m & index )					// 遇到炮架子
				{
					n--;						// 跳躍跑架子
					break;
				}
				xBitCannonMove[x][m] |= index;
			}
			for( ; n>=3; n--)					// 吃子移動
			{
				index = 1<<(n-3);
				if( m & index )					// 遇到棋子，產生俘獲
				{
					xBitCannonMove[x][m] |= index;
					break;
				}				
			}

			// 向右移動
			for(n=x+1; n<12; n++)				// 非吃子移動
			{
				index = 1<<(n-3);
				if( m & index )					// 遇到炮架子
				{
					n++;						// 跳躍跑架子
					break;
				}
				xBitCannonMove[x][m] |= index;
			}
			for( ; n<12; n++)					// 吃子移動
			{
				index = 1<<(n-3);
				if( m & index )					// 遇到棋子，產生俘獲
				{
					xBitCannonMove[x][m] |= index;
					break;
				}				
			}
		}
	}

	// 炮的縱向移動
	for(y=3; y<13; y++)
	{
		for(m=0; m<1024; m++)
		{
			yBitCannonMove[y][m] = 0;

			// 向上移動
			for(n=y-1; n>=3; n--)				// 非吃子移動
			{
				index = 1<<(n-3);
				if( m & index )					// 遇到炮架子
				{
					n--;						// 跳躍跑架子
					break;
				}
				yBitCannonMove[y][m] |= index;
			}
			for( ; n>=3; n--)					// 吃子移動
			{
				index = 1<<(n-3);
				if( m & index )					// 遇到棋子，產生俘獲
				{
					yBitCannonMove[y][m] |= index;
					break;
				}				
			}

			// 向下移動
			for(n=y+1; n<13; n++)				// 非吃子移動
			{
				index = 1<<(n-3);
				if( m & index )					// 遇到炮架子
				{
					n++;						// 跳躍跑架子
					break;
				}
				yBitCannonMove[y][m] |= index;
			}
			for( ; n<13; n++)					// 吃子移動
			{
				index = 1<<(n-3);
				if( m & index )					// 遇到棋子，產生俘獲
				{
					yBitCannonMove[y][m] |= index;
					break;
				}				
			}
		}
	}
}

void CPreMove::InitBitSupperCannon( unsigned short xBitSupperCannon[][512], unsigned short yBitSupperCannon[][1024] )
{
	int x, y, m, n, index;

	// 炮的橫向移動
	for(x=3; x<12; x++)
	{
		for(m=0; m<512; m++)
		{
			xBitSupperCannon[x][m] = 0;

			// 向左移動
			index = 0;
			for(n=x-1; n>=3; n--)				// 非吃子移動
			{
				if( m & (1<<(n-3)) )			// 遇到炮架子
				{
					index ++;
					if( index >= 2 )			// 跳躍兩個炮架子
					{
						n --;
						break;
					}
				}
			}
			for( ; n>=3; n--)					// 吃子移動
			{
				index = 1<<(n-3);
				if( m & index )					// 遇到棋子，產生俘獲
				{
					xBitSupperCannon[x][m] |= index;
					break;
				}				
			}

			// 向右移動
			index = 0;
			for(n=x+1; n<12; n++)				// 非吃子移動
			{
				if( m & (1<<(n-3)) )			// 遇到炮架子
				{
					index ++;
					if( index >= 2 )			// 跳躍兩個炮架子
					{
						n ++;
						break;
					}
				}
			}
			for( ; n<12; n++)					// 吃子移動
			{
				index = 1<<(n-3);
				if( m & index )					// 遇到棋子，產生俘獲
				{
					xBitSupperCannon[x][m] |= index;
					break;
				}				
			}
		}
	}

	// 炮的縱向移動
	for(y=3; y<13; y++)
	{
		for(m=0; m<1024; m++)
		{
			yBitSupperCannon[y][m] = 0;

			// 向上移動
			index = 0;
			for(n=y-1; n>=3; n--)				// 非吃子移動
			{
				if( m & (1<<(n-3)) )			// 遇到炮架子
				{
					index ++;
					if( index >= 2 )			// 跳躍兩個炮架子
					{
						n --;
						break;
					}
				}
			}
			for( ; n>=3; n--)					// 吃子移動
			{
				index = 1<<(n-3);
				if( m & index )					// 遇到棋子，產生俘獲
				{
					yBitSupperCannon[y][m] |= index;
					break;
				}				
			}

			// 向下移動
			index = 0;
			for(n=y+1; n<13; n++)				// 非吃子移動
			{
				if( m & (1<<(n-3)) )			// 遇到炮架子
				{
					index ++;
					if( index >= 2 )			// 跳躍兩個炮架子
					{
						n ++;
						break;
					}
				}
			}
			for( ; n<13; n++)					// 吃子移動
			{
				index = 1<<(n-3);
				if( m & index )					// 遇到棋子，產生俘獲
				{
					yBitSupperCannon[y][m] |= index;
					break;
				}				
			}
		}
	}
}