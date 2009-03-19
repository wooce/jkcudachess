#ifdef _WIN32
  // #include <windows.h>
  // skip pre-compiling <windows.h>
  #if !defined(_INC_WINBASE) && !defined(_WINBASE_H)
    extern "C" __declspec(dllimport) void __stdcall Sleep(unsigned long);
  #endif
#else
  #include <unistd.h>
#endif
#include <sys/timeb.h>
#include "x86asm.h"

#ifndef BASE_H
#define BASE_H

#ifdef _WIN32
  inline void Idle(void) {
    Sleep(1);
  }
#else
  inline void Idle(void) {
    usleep(1000);
  }
#endif

typedef int Bool;            // b
typedef sint8 BoolChar;      // bc
typedef sint16 BoolShort;    // bs
typedef sint32 BoolLong;     // bl
typedef sint64 BoolLongLong; // bll

#ifndef FALSE
  const Bool FALSE = 0;
#endif

#ifndef TRUE
  const Bool TRUE = 1;
#endif

inline Bool EQV(Bool bArg1, Bool bArg2) {
  return bArg1 ? bArg2 : !bArg2;
}

inline Bool XOR(Bool bArg1, Bool bArg2) {
  return bArg1 ? !bArg2 : bArg2;
}

template <typename T> inline T MIN(T Arg1, T Arg2) {
  return Arg1 < Arg2 ? Arg1 : Arg2;
}

template <typename T> inline T MAX(T Arg1, T Arg2) {
  return Arg1 > Arg2 ? Arg1 : Arg2;
}

template <typename T> inline T ABS(T Arg) {
  return Arg < 0 ? -Arg : Arg;
}

template <typename T> inline T SQR(T Arg) {
  return Arg * Arg;
}

template <typename T> inline void SWAP(T &Arg1, T &Arg2) {
  T Temp;
  Temp = Arg1;
  Arg1 = Arg2;
  Arg2 = Temp;
}

/* Note: "MutexAssign(bLock, TRUE)" is equivalent to "Lock(bLock)":
 *
 * inline Bool Lock(volatile Bool &bLock) {
 *   return MutexAssign(bLock, TRUE);
 * }
 */
inline Bool MutexAssign(volatile int &nDst, int nSrc) {
  return Exchange(&nDst, nSrc) != nSrc;
}

inline Bool MutexAssignEqv(volatile int &nDst, int nSrc, int nComp) {
  return CompareExchange(&nDst, nSrc, nComp) == nComp;
}

inline int MutexAdd(volatile int &nDst, int nSrc) {
  return ExchangeAdd(&nDst, nSrc) + nSrc;
}

inline int MutexIncr(volatile int &nDst) {
  return ExchangeAdd(&nDst, 1) + 1;
}

inline int MutexDecr(volatile int &nDst) {
  return ExchangeAdd(&nDst, -1) - 1;
}

struct TimerStruct {
  timeb tbStart;
  void Init(void) {
    ftime(&tbStart);
  }  
  int GetTimer(void) {
    timeb tb;
    ftime(&tb);
    return (tb.time - tbStart.time) * 1000 + tb.millitm - tbStart.millitm;
  }
}; // tb

/* Here is the random number algorithm issued by Lewis, Goodman and Miller in 1969:
 * Multiplier = 7 ^ 5;
 * Divisor = 2 ^ 31 - 1; // which is a prime number
 * Seed *= Multiplier;
 * Seed %= Divisor;
 */
inline uint32 LongRand(uint32 &dwSeed, uint32 dwMultiplier = 16807) {
  dwSeed = LongMulMod(dwSeed, dwMultiplier, 0x7fffffff);
  return dwSeed;
}

inline uint32 GenLongSeed(void) {
  return (uint32) TimeStampCounter() & 0x7fffffff;
}

#endif
