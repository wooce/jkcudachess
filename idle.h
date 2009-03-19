/*! \file idle.h
@brief CPU空轉函數定義（第三方提供，遵循GPL許可）

　　作者訊息︰黃晨 複旦大學化學系表面化學實驗室 E-mail︰webmaster at elephantbase dot net 
*/

#ifndef IDLE_H
#define IDLE_H

#ifdef _WIN32
  #include <windows.h>
  inline void Idle(void) {
    Sleep(1);
  }
#else
  #include <unistd.h>
  inline void Idle(void) {
    usleep(1000);
  }
#endif

#endif
