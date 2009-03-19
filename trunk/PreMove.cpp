////////////////////////////////////////////////////////////////////////////////////////////////////////////
// �����JPreMove.cpp                                                                                    //
// *******************************************************************************************************//
// ����H�ѳq�Τ���----�L�e���|�A����m����H�ѳq�Τ�����ĳ�n(Universal Chinese Chess Protocol�A²��ucci) //
// �@�̡J �S �w �x                                                                                        //
// ���J �����l���Ǭ�s�|                                                                            //
// �l�c�J fan_de_jun@sina.com.cn                                                                          //
//  QQ �J 83021504                                                                                        //
// *******************************************************************************************************//
// �\��J                                                                                                 //
// 1. ��l�ƴѤl�]�N�������H�h�L�^�����ʡA���Ͱ��X�k���۪k�C                                              //
// 2. ��l�ƨ������P��C�ѽL                                                                            //
//                                                                                                        //
// �`�JCPreMove�����]�t��ڪ��۪k�ƲաA��l�Ƨ����Z�A�Y�i�������A�{���ҰʦZ�u�ݽեΤ@��                 //
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreMove.h"


CPreMove::CPreMove()
{
}

CPreMove::~CPreMove()
{
}


// �N�Ӫ����ʡA���]�A�����ӭ�
// ��l�Ƭ��N�Ӳ��ʪ��ؼЮ�AnDst = *pMove
// �Ҧ����D�R�򲾰ʳ��X�k�A�R�򲾰ʤ��@�w�X�k�A�J��Y�ۤv���Ѥl���[�H�P�O
void CPreMove::InitKingMoves(unsigned char KingMoves[][8])
{
	int i, m, n;
	unsigned char *pMove;
	int nKingMoveDir[4] = { -16, -1, 1, 16 };

	for(m=0x36; m<0xC9; m++)
	{
		if( nCityIndex[m] )										//�b�E�c��
		{
			pMove = KingMoves[m];
			for(i=0; i<4; i++)
			{
				n = m + nKingMoveDir[i];
				if( nCityIndex[n] )
					*(pMove++)  = unsigned char( n );
			}
			*pMove = 0;		// ��ܤ���A����
		}
	}
}


// ��l�ƨ����۪k�w���ͼƲաJ
// ��Ȭ���/�C���۹��mx/y*16�A�p��ؼЮ�l���������JnDst(y|x) = (nSrc & 0xF0) | x; �Ϊ� nDst(y|x) = y | (nSrc & 0xF);  // x = *pMove; y = *pMove;
// �o�@�I�P�N�B�h�B�H�B���B�L����ܬ��Ѥl�������m�Ҧ����P
// �Ҧ����D�R�򲾰ʳ��X�k�A�R�򲾰ʤ��@�w�X�k�A�J��Y�ۤv���Ѥl���[�H�P�O
void CPreMove::InitRookMoves(unsigned char xRookMoves[][512][12], unsigned char yRookMoves[][1024][12], unsigned char xRookCapMoves[][512][4], unsigned char yRookCapMoves[][1024][4])
{
	int m, n, x, y;
	unsigned char *pMove, *pCapMove;

	// ��l�ƨ�����V����
	for(x=3; x<12; x++)
	{
		for(m=0; m<512; m++)
		{
			pMove    = xRookMoves[x][m];
			pCapMove = xRookCapMoves[x][m];
			for(n=x-1; n>=3; n--)					// �V���j��
			{
				*(pMove++) = unsigned char( n );
				if( m & ( 1 << (n-3) ) )			// �J��Ѥl�A���ͫR��
				{
					(*pCapMove++) = *(pMove-1);
					break;
				}
			}

			for(n=x+1; n<12; n++)					// �V�k�j��
			{
				*(pMove++) = unsigned char( n );
				if( m & ( 1 << (n-3) ) )			// �J��Ѥl�A���ͫR��
				{
					(*pCapMove++) = *(pMove-1);
					break;
				}
			}

			*pMove    = 0;
			*pCapMove = 0;
		}
	}

	// ��l�ƨ����a�V����
	for(y=3; y<13; y++)
	{
		for(m=0; m<1024; m++)
		{
			pMove    = yRookMoves[y][m];
			pCapMove = yRookCapMoves[y][m];
			for(n=y-1; n>=3; n--)
			{
				*(pMove++) = unsigned char( n<<4 );
				if( m & ( 1 << (n-3) ) )			// �J��Ѥl�A���ͫR��
				{
					(*pCapMove++) = *(pMove-1);
					break;
				}
			}
			for(n=y+1; n<13; n++)
			{
				*(pMove++) = unsigned char( n<<4 );
				if( m & ( 1 << (n-3) ) )			// �J��Ѥl�A���ͫR��
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


// ��l�Ƭ����۪k�w���ͼƲաJ
// ��Ȭ���/�C���۹��mx/y*16�A�p��ؼЮ�l���������JnDst(y|x) = (nSrc & 0xF0) | x; �Ϊ� nDst(y|x) = y | (nSrc & 0xF);  // x = *pMove; y = *pMove;
// �o�@�I�P�N�B�h�B�H�B���B�L����ܬ��Ѥl�������m�Ҧ����P
// �Ҧ����D�R�򲾰ʳ��X�k�A�R�򲾰ʤ��@�w�X�k�A�J��Y�ۤv���Ѥl���[�H�P�O
void CPreMove::InitCannonMoves(unsigned char xCannonMoves[][512][12], unsigned char yCannonMoves[][1024][12], unsigned char xCannonCapMoves[][512][4], unsigned char yCannonCapMoves[][1024][4])
{
	int m, n, x, y;
	unsigned char *pMove, *pCapMove;

	//��l�Ƭ�����V����
	for(x=3; x<12; x++)
	{
		for(m=0; m<512; m++)
		{
			pMove    = xCannonMoves[x][m];
			pCapMove = xCannonCapMoves[x][m];
			for(n=x-1; n>=3; n--)
			{
				if( m & ( 1 << (n-3) ) )			// �J�쬶�[�l
				{
					n--;							// ��V���[�l
					break;
				}
				*(pMove++) = unsigned char( n );
			}
			for(; n>=3; n--)
			{
				if( m & ( 1 << (n-3) ) )			// �J��Ѥl�~����
				{
					*(pMove++) = unsigned char( n );
					*(pCapMove++) = *(pMove-1);
					break;
				}
			}

			for(n=x+1; n<12; n++)
			{
				if( m & ( 1 << (n-3) ) )			// �J�쬶�[�l
				{
					n++;							// ��V���[�l
					break;
				}
				*(pMove++) = unsigned char( n );
			}
			for(; n<12; n++)
			{
				if( m & ( 1 << (n-3) ) )			// �J��Ѥl�~����
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

	//��l�Ƭ����a�V����
	for(y=3; y<13; y++)
	{
		for(m=0; m<1024; m++)
		{
			pMove    = yCannonMoves[y][m];
			pCapMove = yCannonCapMoves[y][m];
			for(n=y-1; n>=3; n--)
			{
				if( m & ( 1 << (n-3) ) )			//�J�쬶�[�l
				{
					n--;							//��V���[�l
					break;
				}
				*(pMove++) = unsigned char( n<<4 );
			}
			for(; n>=3; n--)
			{
				if( m & ( 1 << (n-3) ) )			//�J��Ѥl�~����
				{
					*(pMove++) = unsigned char( n<<4 );
					*(pCapMove++) = *(pMove-1);
					break;
				}
			}

			for(n=y+1; n<13; n++)
			{
				if( m & ( 1 << (n-3) ) )			//�J�쬶�[�l
				{
					n++;							//��V���[�l
					break;
				}
				*(pMove++) = unsigned char( n<<4 );
			}
			for(; n<13; n++)
			{
				if( m & ( 1 << (n-3) ) )			//�J��Ѥl�~����
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


// ��l�ư����۪k�w���ͼƲաJ
// ��Ȫ�l�Ƭ������ʪ������m�C
// ���X�k���ʡJ�̰��L�����ʡA�Y�v��Ѥl�����ʡA�����˴��C
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
				if( nBoardIndex[n] )						// ���b�ѽL��	
					*(pMove++)  = unsigned char( n );
			}
			*pMove = 0;
		}
	}
}

// ��l�ƶH���۪k�w���ͼƲաJ
// ��Ȫ�l�Ƭ��H���ʪ������m�C
// ���X�k���ʡJ��H�������ʡA�Y�v��Ѥl�����ʡA�����˴��C
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
				if( nBoardIndex[n] && (m^n)<128 )			// (m^n)<128, �O�ҶH���|�L�e
					*(pMove++)  = unsigned char( n );
			}
			*pMove = 0;
		}
	}
}


// ��l�Ƥh���۪k�w���ͼƲաJ
// ��Ȫ�l�Ƭ��h���ʪ������m�C
// ���X�k���ʡJ�Y�v��Ѥl�����ʡA�����˴��C
void CPreMove::InitGuardMoves(unsigned char GuardMoves[][8])
{
	int i, m, n;
	unsigned char *pMove;
	int nGuardMoveDir[] = { -17, -15, 15, 17 };

	for(m=0x36; m<0xC9; m++)
	{
		if( nCityIndex[m] )										// �b�E�c��
		{
			pMove = GuardMoves[m];
			for(i=0; i<4; i++)
			{
				n = m + nGuardMoveDir[i];
				if( nCityIndex[n] )								// �b�E�c��
					*(pMove++)  = unsigned char( n );
			}
			*pMove = 0;		// ��ܤ���A����
		}
	}
}


// ��l�ƧL�򪺵۪k�w���ͼƲաJ
// ��Ȫ�l�Ƭ����ʪ������m�C
// ���X�k���ʡJ�Y�v��Ѥl�����ʡA�����˴��C
void CPreMove::InitPawnMoves(unsigned char PawnMoves[][256][4])
{
	int i, m, n, Player;
	unsigned char *pMove;
	int PawnMoveDir[2][3] = { {16, -1, 1}, {-16, -1, 1} };							// �¡J�U���k�F  ���J�W���k

	for(Player=0; Player<=1; Player++)												//Player=0, �¨�F Player=1, ���L
	{
		for(m=0x33; m<0xCC; m++)
		{
			if( nBoardIndex[m] )
			{
				pMove = PawnMoves[Player][m];
				for(i=0; i<3; i++)
				{
					if( i>0 && ((!Player && m<128) || (Player && m>=128)) )			//���L�e���L���४�k�̰�
						break;

					n = m + PawnMoveDir[Player][i];
					if( nBoardIndex[n] )										   //�b�ѽL�d��
						*(pMove++)  = unsigned char( n );
				}
				*pMove = 0;
			}
		}
	}	
}


// *****************************************************************************************************************
// *****************************************************************************************************************

// �������P��C�ѽL�J�]�t�Y�l���ʩM�D�Y�l����
void CPreMove::InitBitRookMove( unsigned short xBitRookMove[][512], unsigned short yBitRookMove[][1024])
{
	int x, y, m, n, index;

	// ������V����
	for(x=3; x<12; x++)
	{
		for(m=0; m<512; m++)
		{
			xBitRookMove[x][m] = 0;

			// �V������
			for(n=x-1; n>=3; n--)
			{
				index = 1<<(n-3);
				xBitRookMove[x][m] |= index;
				if( m & index )						// �J��Ѥl�A���ͫR��
					break;
			}

			// �V�k����
			for(n=x+1; n<12; n++)
			{
				index = 1<<(n-3);
				xBitRookMove[x][m] |= index;
				if( m & index )						// �J��Ѥl�A���ͫR��
					break;
			}
		}
	}

	// �����a�V����
	for(y=3; y<13; y++)
	{
		for(m=0; m<1024; m++)
		{
			yBitRookMove[y][m] = 0;

			// �V�W����
			for(n=y-1; n>=3; n--)
			{
				index = 1<<(n-3);
				yBitRookMove[y][m] |= index;
				if( m & index )						// �J��Ѥl�A���ͫR��
					break;
			}

			// �V�U����
			for(n=y+1; n<13; n++)
			{
				index = 1<<(n-3);
				yBitRookMove[y][m] |= index;
				if( m & index )						// �J��Ѥl�A���ͫR��
					break;
			}
		}
	}
}


// �������P��C�ѽL�J�]�t�Y�l���ʩM�D�Y�l����
void CPreMove::InitBitCannonMove( unsigned short xBitCannonMove[][512], unsigned short yBitCannonMove[][1024] )
{
	int x, y, m, n, index;

	// ������V����
	for(x=3; x<12; x++)
	{
		for(m=0; m<512; m++)
		{
			xBitCannonMove[x][m] = 0;

			// �V������
			for(n=x-1; n>=3; n--)				// �D�Y�l����
			{
				index = 1<<(n-3);
				if( m & index )					// �J�쬶�[�l
				{
					n--;						// ���D�]�[�l
					break;
				}
				xBitCannonMove[x][m] |= index;
			}
			for( ; n>=3; n--)					// �Y�l����
			{
				index = 1<<(n-3);
				if( m & index )					// �J��Ѥl�A���ͫR��
				{
					xBitCannonMove[x][m] |= index;
					break;
				}				
			}

			// �V�k����
			for(n=x+1; n<12; n++)				// �D�Y�l����
			{
				index = 1<<(n-3);
				if( m & index )					// �J�쬶�[�l
				{
					n++;						// ���D�]�[�l
					break;
				}
				xBitCannonMove[x][m] |= index;
			}
			for( ; n<12; n++)					// �Y�l����
			{
				index = 1<<(n-3);
				if( m & index )					// �J��Ѥl�A���ͫR��
				{
					xBitCannonMove[x][m] |= index;
					break;
				}				
			}
		}
	}

	// �����a�V����
	for(y=3; y<13; y++)
	{
		for(m=0; m<1024; m++)
		{
			yBitCannonMove[y][m] = 0;

			// �V�W����
			for(n=y-1; n>=3; n--)				// �D�Y�l����
			{
				index = 1<<(n-3);
				if( m & index )					// �J�쬶�[�l
				{
					n--;						// ���D�]�[�l
					break;
				}
				yBitCannonMove[y][m] |= index;
			}
			for( ; n>=3; n--)					// �Y�l����
			{
				index = 1<<(n-3);
				if( m & index )					// �J��Ѥl�A���ͫR��
				{
					yBitCannonMove[y][m] |= index;
					break;
				}				
			}

			// �V�U����
			for(n=y+1; n<13; n++)				// �D�Y�l����
			{
				index = 1<<(n-3);
				if( m & index )					// �J�쬶�[�l
				{
					n++;						// ���D�]�[�l
					break;
				}
				yBitCannonMove[y][m] |= index;
			}
			for( ; n<13; n++)					// �Y�l����
			{
				index = 1<<(n-3);
				if( m & index )					// �J��Ѥl�A���ͫR��
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

	// ������V����
	for(x=3; x<12; x++)
	{
		for(m=0; m<512; m++)
		{
			xBitSupperCannon[x][m] = 0;

			// �V������
			index = 0;
			for(n=x-1; n>=3; n--)				// �D�Y�l����
			{
				if( m & (1<<(n-3)) )			// �J�쬶�[�l
				{
					index ++;
					if( index >= 2 )			// ���D��Ӭ��[�l
					{
						n --;
						break;
					}
				}
			}
			for( ; n>=3; n--)					// �Y�l����
			{
				index = 1<<(n-3);
				if( m & index )					// �J��Ѥl�A���ͫR��
				{
					xBitSupperCannon[x][m] |= index;
					break;
				}				
			}

			// �V�k����
			index = 0;
			for(n=x+1; n<12; n++)				// �D�Y�l����
			{
				if( m & (1<<(n-3)) )			// �J�쬶�[�l
				{
					index ++;
					if( index >= 2 )			// ���D��Ӭ��[�l
					{
						n ++;
						break;
					}
				}
			}
			for( ; n<12; n++)					// �Y�l����
			{
				index = 1<<(n-3);
				if( m & index )					// �J��Ѥl�A���ͫR��
				{
					xBitSupperCannon[x][m] |= index;
					break;
				}				
			}
		}
	}

	// �����a�V����
	for(y=3; y<13; y++)
	{
		for(m=0; m<1024; m++)
		{
			yBitSupperCannon[y][m] = 0;

			// �V�W����
			index = 0;
			for(n=y-1; n>=3; n--)				// �D�Y�l����
			{
				if( m & (1<<(n-3)) )			// �J�쬶�[�l
				{
					index ++;
					if( index >= 2 )			// ���D��Ӭ��[�l
					{
						n --;
						break;
					}
				}
			}
			for( ; n>=3; n--)					// �Y�l����
			{
				index = 1<<(n-3);
				if( m & index )					// �J��Ѥl�A���ͫR��
				{
					yBitSupperCannon[y][m] |= index;
					break;
				}				
			}

			// �V�U����
			index = 0;
			for(n=y+1; n<13; n++)				// �D�Y�l����
			{
				if( m & (1<<(n-3)) )			// �J�쬶�[�l
				{
					index ++;
					if( index >= 2 )			// ���D��Ӭ��[�l
					{
						n ++;
						break;
					}
				}
			}
			for( ; n<13; n++)					// �Y�l����
			{
				index = 1<<(n-3);
				if( m & index )					// �J��Ѥl�A���ͫR��
				{
					yBitSupperCannon[y][m] |= index;
					break;
				}				
			}
		}
	}
}