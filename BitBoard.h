// "BitBoard.h"  中國象棋位棋盤類

#pragma once

// Basic BitBoard Operations, including:
// 1. Assign Operations: Use Constructors
// 2. Comparison Operations: ! (Non-Zero), ==, !=
// 3. Bit Operations: ~, &, |, ^
// 4. Shift Operations: <<, >>

// Advanced CBitBoard Operations, including:
// 1. CheckSum (Fold) and Duplicate (Un-Fold)
// 2. MSB, LSB and Counter

class CBitBoard 
{
public:
	unsigned long Low, Mid, Hi;

public:
	CBitBoard() 
	{
	}

	CBitBoard(unsigned long Arg1, unsigned long Arg2 = 0, unsigned long Arg3 = 0) 
	{
		Low = Arg1;
		Mid = Arg2;
		Hi  = Arg3;
	}

	int operator !() const 
	{
		return !(Low || Mid || Hi);
	}

	int operator ==(const CBitBoard &Arg) const 
	{
		return Low == Arg.Low && Mid == Arg.Mid && Hi && Arg.Hi;
	}

	int operator !=(const CBitBoard &Arg) const 
	{
		return Low != Arg.Low || Mid != Arg.Mid || Hi && Arg.Hi;
	}

	CBitBoard operator ~() const 
	{
		return CBitBoard(~Low, ~Mid, ~Hi);
	}

	CBitBoard operator &(const CBitBoard &Arg) const 
	{
		return CBitBoard(Low & Arg.Low, Mid & Arg.Mid, Hi & Arg.Hi);
	}

	CBitBoard operator |(const CBitBoard &Arg) const 
	{
		return CBitBoard(Low | Arg.Low, Mid | Arg.Mid, Hi | Arg.Hi);
	}

	CBitBoard operator ^(const CBitBoard &Arg) const 
	{
		return CBitBoard(Low ^ Arg.Low, Mid ^ Arg.Mid, Hi ^ Arg.Hi);
	}

	CBitBoard &operator &=(const CBitBoard &Arg) 
	{
		Low &= Arg.Low;
		Mid &= Arg.Mid;
		Hi  &= Arg.Hi;
		return *this;
	}

	CBitBoard &operator |=(const CBitBoard &Arg) 
	{
		Low |= Arg.Low;
		Mid |= Arg.Mid;
		Hi  |= Arg.Hi;
		return *this;
	}

	CBitBoard &operator ^=(const CBitBoard &Arg) 
	{
		Low ^= Arg.Low;
		Mid ^= Arg.Mid;
		Hi  ^= Arg.Hi;
		return *this;
	}

	// Shift Operations

	CBitBoard operator <<(int Arg) const
	{
		if (Arg < 0)
			return *this >> -Arg;
		else if (Arg == 0)
			return *this;
		else if (Arg < 32)
			return CBitBoard(Low << Arg, Mid << Arg | Low >> (32 - Arg), Hi << Arg | Mid >> (32 - Arg));
		else if (Arg == 32)
			return CBitBoard(0, Low, Mid);
		else if (Arg < 64)
			return CBitBoard(0, Low << (Arg - 32), Mid << (Arg - 32) | Low >> (64 - Arg));
		else if (Arg == 64)
			return CBitBoard(0, 0, Low);
		else if (Arg < 96)
			return CBitBoard(0, 0, Low << (Arg - 64));
		else
			return CBitBoard(0, 0, 0);
	}

	CBitBoard operator >>(int Arg) const
	{
		if (Arg < 0)
			return *this << -Arg;
		else if (Arg == 0)
			return *this;
		else if (Arg < 32)
			return CBitBoard(Low >> Arg | Mid << (32 - Arg), Mid >> Arg || Hi << (32 - Arg), Hi >> Arg);
		else if (Arg == 32)
			return CBitBoard(Mid, Hi, 0);
		else if (Arg < 64)
			return CBitBoard(Mid >> (Arg - 32) | Hi << (64 - Arg), Hi >> (Arg - 32), 0);
		else if (Arg == 64)
			return CBitBoard(Hi, 0, 0);
		else if (Arg < 96)
			return CBitBoard(Hi >> (Arg - 64), 0, 0);
		else
			return CBitBoard(0, 0, 0);
	}

	CBitBoard &operator <<=(int Arg)
	{
		if (Arg < 0) 
		{
			*this >>= -Arg;
		} 
		else if (Arg == 0) 
		{
		} 
		else if (Arg < 32) 
		{
			Hi <<= Arg;
			Hi |= Mid >> (32 - Arg);
			Mid <<= Arg;
			Mid |= Low >> (32 - Arg);
			Low <<= Arg;
		} 
		else if (Arg == 32) 
		{
			Hi = Mid;
			Mid = Low;
			Low = 0;
		} 
		else if (Arg < 64) 
		{
			Hi = Mid << (Arg - 32);
			Hi |= Low >> (64 - Arg);
			Mid = Low << (Arg - 32);
			Low = 0;
		} 
		else if (Arg == 64) 
		{
			Hi  = Low;
			Mid = 0;
			Low = 0;
		} 
		else if (Arg < 96) 
		{
			Hi = Low << (Arg - 64);
			Mid = 0;
			Low = 0;
		} 
		else 
		{
			Hi  = 0;
			Mid = 0;
			Low = 0;		
		}
		return *this;
	}

	CBitBoard &operator >>=(int Arg)
	{
		if (Arg < 0) 
		{
			*this <<= -Arg;
		} 
		else if (Arg == 0) 
		{
		} 
		else if (Arg < 32) 
		{
			Low >>= Arg;
			Low |= Mid << (32 - Arg);
			Mid >>= Arg;
			Mid |= Hi << (32 - Arg);
			Hi >>= Arg;
		} 
		else if (Arg == 32) 
		{
			Low = Mid;
			Mid = Hi;
			Hi = 0;
		} 
		else if (Arg < 64) 
		{
			Low = Mid >> (Arg - 32);
			Low |= Hi << (64 - Arg);
			Mid = Hi >> (Arg - 32);
			Hi = 0;
		} 
		else if (Arg == 64) 
		{
			Low = Hi;
			Mid = 0;
			Hi = 0;
		} 
		else if (Arg < 96) 
		{
			Low = Hi >> (Arg - 64);
			Mid = 0;
			Hi = 0;
		} 
		else 
		{
			Low = 0;
			Mid = 0;
			Hi = 0;
		}
		return *this;
	}
};

// CheckSum (Folder) and Duplicate (Un-Folder) Operations

//位棋盤的校驗和︰把棋盤的90個格子(CBitBoard中的96位)，經過三次摺疊，變為8位數
//可以表示256種棋盤狀況，馬腿和象眼的位置保證不會重疊，所以可以窮舉馬和象的各種走法
//CheckSum()還可以判斷CBitBoard是否為0和非0，所以必須用"|"操作符進行摺疊。不過速度稍慢，把后96位摺疊成一個32位數就可以了。
//"^"操作符對馬腿和象眼也是可行的。
//此函數在CPreMove類中不用，MoveGenerator()中會用到，但和Duplicate()相關，故而定義為全局函數。其它地方可以方便調用。
//此函數調用非常頻繁，inline能增加速度，差不多佔用思考時間的2～3%.倒不如把代碼直接複製到馬和象的走法地方,與inline作用相同
inline int CheckSum(const CBitBoard& board) 
{
	unsigned long  Temp32 = board.Low | board.Mid | board.Hi;
	unsigned short Temp16 = ((unsigned short *) &Temp32)[0] | ((unsigned short *) &Temp32)[1];
	return (int) (((unsigned char *) &Temp16)[0] | ((unsigned char *) &Temp16)[1]);
}


//展開校驗和，其作用與CheckSum()相反。其返回值是馬腿或象眼的位棋盤，AllLegsOrAllEyes其位置所有的馬腿或象眼
//這個函數只在馬和象的著法預產生函數種應用，其它地方很少用到，故而定義成私有的成員函數。
inline CBitBoard Duplicate(int CheckSum, const CBitBoard& AllLegsOrAllEyes) 
{
	unsigned short Temp16;
	unsigned long Temp32;

	((unsigned char *) &Temp16)[0] = ((unsigned char *) &CheckSum)[0];
	((unsigned char *) &Temp16)[1] = ((unsigned char *) &CheckSum)[0];
	((unsigned short *) &Temp32)[0] = Temp16;
	((unsigned short *) &Temp32)[1] = Temp16;

	return CBitBoard(Temp32, Temp32, Temp32) &  AllLegsOrAllEyes;	 //返回CheckSum代表的馬腿或者象眼的位棋盤
}


// MSB = Most Significant Bit
// It is a smart algorithm equivalent to BSR instruction in 80386 processor

inline int Msb32(unsigned long Arg) 
{
	int RetVal = 0;
	if (Arg & 0xffff0000) 
	{
		RetVal += 16;
		Arg &= 0xffff0000;
	}
	if (Arg & 0xff00ff00) 
	{
		RetVal += 8;
		Arg &= 0xff00ff00;
	}
	if (Arg & 0xf0f0f0f0) 
	{
		RetVal += 4;
		Arg &= 0xf0f0f0f0;
	}
	if (Arg & 0xcccccccc) 
	{
		RetVal += 2;
		Arg &= 0xcccccccc;
	}
	if (Arg & 0xaaaaaaaa) 
	{
		RetVal += 1;
	}

	return RetVal;
}

inline int MSB(const CBitBoard &Arg) 
{
	if (Arg.Hi)
		return Msb32(Arg.Hi) + 64;
	else if (Arg.Mid)
		return Msb32(Arg.Mid) + 32;
	else if (Arg.Low)
		return Msb32(Arg.Low);
	else
		return -1;
}

// LSB = Least Significant Bit
// It is a smart algorithm equivalent to BSF instruction in 80386 processor

inline int Lsb32(unsigned long Arg) 
{
	int RetVal = 31;
	if (Arg & 0x0000ffff) 
	{
		RetVal -= 16;
		Arg &= 0x0000ffff;
	}
	if (Arg & 0x00ff00ff) 
	{
		RetVal -= 8;
		Arg &= 0x00ff00ff;
	}
	if (Arg & 0x0f0f0f0f) 
	{
		RetVal -= 4;
		Arg &= 0x0f0f0f0f;
	}
	if (Arg & 0x33333333) 
	{
		RetVal -= 2;
		Arg &= 0x33333333;
	}
	if (Arg & 0x55555555) 
	{
		RetVal -= 1;
	}

	return RetVal;
}

inline int LSB(const CBitBoard &Arg) 
{
	if (Arg.Low)
		return Lsb32(Arg.Low);
	else if (Arg.Mid)
		return Lsb32(Arg.Mid) + 32;
	else if (Arg.Hi)
		return Lsb32(Arg.Hi) + 64;
	else
		return -1;
}

// A smart algorithm to count set (1) bits

inline int Count32(unsigned long Arg)
{
	int t;

	t = ((Arg >> 1) & 0x55555555) + (Arg & 0x55555555);
	t = ((t >> 2) & 0x33333333) + (t & 0x33333333);
	t = ((t >> 4) & 0x0f0f0f0f) + (t & 0x0f0f0f0f);
	t = ((t >> 8) & 0x00ff00ff) + (t & 0x00ff00ff);
	return (t >> 16) + (t & 0x0000ffff);
}

inline int Count(const CBitBoard &Arg) 
{
  return Count32(Arg.Low) + Count32(Arg.Mid) + Count32(Arg.Hi);
}

/*
// 以下算法反而不快，MSB已經調試，剩下的還未調試
// 若想使用下面的函數，必須在適當的地方進行初始化:
// InitBit16(FirstOne16, LastOne16, Count16);

static unsigned char FirstOne16[65536];
static unsigned char LastOne16[65536];
static unsigned char Count16[65536];
inline void InitBit16(unsigned char *FirstOne16, unsigned char *LastOne16, unsigned char *Count16)
{
	int m,n;

	for(m=0; m<65536; m++)
	{
		for(n=15; n>=0; n--)
		{
			if(m & (1<<n))
			{
				FirstOne16[m] = n;
				break;
			}
		}

		for(n=0; n<16; n++)
		{
			if(m & (1<<n))
			{
				LastOne16[m] = n;
				break;
			}
		}

		Count16[m] = 0;
		for(n=0; n<16; n++)
		{
			if(m & (1<n))
				Count16[m] ++;
		}
	}
}

inline int MSB32(unsigned long Arg)
{
	int p = ((unsigned short *) &Arg)[1];
	if(p)
		return FirstOne16[p] + 16;
	else
		return FirstOne16[((unsigned short *) &Arg)[0]];
}

inline int LSB32(unsigned long Arg)
{
	if(((unsigned short *) &Arg)[0])
		return LastOne16[((unsigned short *) &Arg)[0]];
	else if(((unsigned short *) &Arg)[1])
		return LastOne16[((unsigned short *) &Arg)[1]] + 16;
	else
		return -1;
}

inline int COUNT32(unsigned long Arg)
{
	return Count16[((unsigned short *) &Arg)[0]] + Count16[((unsigned short *) &Arg)[1]];
}

inline int MSB96(const CBitBoard &Arg) 
{
	if (Arg.Hi)
		return MSB32(Arg.Hi) + 64;
	else if (Arg.Mid)
		return MSB32(Arg.Mid) + 32;
	else if (Arg.Low)
		return MSB32(Arg.Low);
	else
		return -1;
}

inline int LSB96(const CBitBoard &Arg) 
{
	if (Arg.Low)
		return LSB32(Arg.Low);
	else if (Arg.Mid)
		return LSB32(Arg.Mid) + 32;
	else if (Arg.Hi)
		return LSB32(Arg.Hi) + 64;
	else
		return -1;
}

inline int COUNT96(const CBitBoard &Arg)
{
	return COUNT32(Arg.Low) + COUNT32(Arg.Mid) + COUNT32(Arg.Hi);
}
*/