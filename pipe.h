/*! \file pipe.h
@brief �޹D�q�H�Ҷ��]�ĤT�责�ѡA��`GPL�\�i�^

�@�@�@�̰T���J���� �ƥ��j�ǤƾǨt���ƾǹ���� E-mail�Jwebmaster at elephantbase dot net 
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

///�޹D�q�H�Ҷ��]�ĤT�责�ѡA��`GPL�\�i�^ 
/*!
�@�@�ڭ̪����ϥ�UCCI�}�o�H�����Ѫ��޹D�q�T�Ҷ��]��`GPL�\�i�^�C

�@�@�����{�M�\�໡���Ѩ��Jhttp://www.elephantbase.net/protocol/cchess_ucci.htm�C
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
