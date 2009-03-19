/*! \file pipe.h
@brief 管道通信模塊（第三方提供，遵循GPL許可）

　　作者訊息︰黃晨 複旦大學化學系表面化學實驗室 E-mail︰webmaster at elephantbase dot net 
*/

#ifdef _WIN32
  #include <windows.h>
#else
  #include <unistd.h>
#endif
#include "base.h"

#ifndef PIPE_H
#define PIPE_H

const int LINE_INPUT_MAX_CHAR = 4096;

///管道通信模塊（第三方提供，遵循GPL許可） 
/*!
　　我們直接使用UCCI開發人員提供的管道通訊模塊（遵循GPL許可）。

　　具體實現和功能說明參見︰http://www.elephantbase.net/protocol/cchess_ucci.htm。
*/
struct PipeStruct {
#ifdef _WIN32
  HANDLE hInput, hOutput;
  BOOL bConsole;
  int nBytesLeft;
#else
  int nInput, nOutput;
#endif
  int nReadEnd;
  char szBuffer[LINE_INPUT_MAX_CHAR];

  void Open(const char *szExecFile = NULL);
  void Close(void) const;
  void ReadInput(void);
  Bool CheckInput(void);
  Bool GetBuffer(char *szLineStr);
  Bool LineInput(char *szLineStr);
  void LineOutput(const char *szLineStr) const;
}; // pipe

#endif
