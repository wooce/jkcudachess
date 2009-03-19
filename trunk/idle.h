/*! \file idle.h
@brief CPU�����Ʃw�q�]�ĤT�责�ѡA��`GPL�\�i�^

�@�@�@�̰T���J���� �ƥ��j�ǤƾǨt���ƾǹ���� E-mail�Jwebmaster at elephantbase dot net 
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
