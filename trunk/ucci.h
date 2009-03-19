/*! \file ucci.h
@brief UCCI�q�T�Ҷ��]�ĤT�责�ѡA��`GPL�\�i�^

�@�@�@�̰T���J���� �ƥ��j�ǤƾǨt���ƾǹ���� E-mail�Jwebmaster at elephantbase dot net 
*/

#ifndef UCCI_H
#define UCCI_H

#include "base.h"
const int UCCI_MAX_DEPTH = 32; // UCCI������Ҫ������`��

// �MUCCI���O������r�������ﶵ
enum UcciOptionEnum {
  UCCI_OPTION_NONE, UCCI_OPTION_BATCH, UCCI_OPTION_DEBUG, UCCI_OPTION_USEMILLISEC, UCCI_OPTION_BOOKFILES, UCCI_OPTION_HASHSIZE, UCCI_OPTION_THREADS, UCCI_OPTION_DRAWMOVES,
  UCCI_OPTION_REPETITION, UCCI_OPTION_PRUNING, UCCI_OPTION_KNOWLEDGE, UCCI_OPTION_SELECTIVITY, UCCI_OPTION_STYLE, UCCI_OPTION_LOADBOOK
}; // uo, ��"setoption"���w���ﶵ
enum UcciRepetEnum {
  UCCI_REPET_ALWAYSDRAW, UCCI_REPET_CHECKBAN, UCCI_REPET_ASIANRULE, UCCI_REPET_CHINESERULE
}; // ur
enum UcciGradeEnum {
  UCCI_GRADE_NONE, UCCI_GRADE_SMALL, UCCI_GRADE_MEDIUM, UCCI_GRADE_LARGE
}; // ug
enum UcciStyleEnum {
  UCCI_STYLE_SOLID, UCCI_STYLE_NORMAL, UCCI_STYLE_RISKY
}; // us, �ﶵstyle���]�w��
enum UcciTimeEnum {
  UCCI_TIME_DEPTH, UCCI_TIME_MOVE, UCCI_TIME_INC
}; // ut, ��"go"���O���w���ɶ��Ҧ��A���O�O�T�w�`��(�L���`�۷�_�`�׬�"c_MaxDepth")�B�ɬq�s(�X���������������X�B)�M�[�ɻs(�Ѿl�ɶ��h�֡A�����o�B�Z�[�X��)
enum UcciCommEnum {
  UCCI_COMM_NONE, UCCI_COMM_UCCI, UCCI_COMM_ISREADY, UCCI_COMM_PONDERHIT, UCCI_COMM_STOP, UCCI_COMM_SETOPTION, UCCI_COMM_POSITION, UCCI_COMM_BANMOVES, UCCI_COMM_GO, UCCI_COMM_GOPONDER, UCCI_COMM_QUIT
}; // uce, UCCI���O����

///UCCI�q�T�Ҷ��]�ĤT�责�ѡA��`GPL�\�i�^ 
/*!
�@�@�ڭ̪����ϥ�UCCI�}�o�H�����Ѫ��q�γq�T�Ҷ��]��`GPL�\�i�^�C

�@�@�����{�M�\�໡���Ѩ��Jhttp://www.elephantbase.net/protocol/cchess_ucci.htm�C
*/
// UCCI���O�i�H�������H�U�o�ө�H���[�c
union UcciCommStruct {

  /* �i�o�����T����UCCI���O�u���H�U4������
   *
   * 1. "setoption"���O�ǻ����T���A�A�X�_"UCCI_COMM_SETOPTION"���O����
   *    "setoption"���O�Ψӳ]�w�ﶵ�A�]�����������쪺�T�������ﶵ�������M���ﶵ�ȡ�
   *    �Ҧp�A"setoption batch on"�A�ﶵ�����N�O"UCCI_OPTION_DEBUG"�A��(Value.bCheck)�N�O"TRUE"
   */
  struct {
    UcciOptionEnum uoType;   // �ﶵ����
    union {                  // �ﶵ��
      int nSpin;             // "spin"�������ﶵ����
      Bool bCheck;           // "check"�������ﶵ����
      UcciRepetEnum urRepet; // "combo"�������ﶵ"repetition"����
      UcciGradeEnum ugGrade; // "combo"�������ﶵ"pruning"�B"knowledge"�M"selectivity"����
      UcciStyleEnum usStyle; // "combo"�������ﶵ"style"����
      const char *szString;  // "string"�������ﶵ����
    } Value;                 // "button"�������ﶵ�S����
  } Option;

  /* 2. "position"���O�ǻ����T���A�A�X�_"e_CommPosition"���O����
   *    "position"���O�Ψӳ]�m�����A�]�A��l�����s�P�Z��۪k�c��������
   *    �Ҧp�Aposition startpos moves h2e2 h9g8�AFEN��N�O"startpos"�N��FEN��A�۪k��(MoveNum)�N�O2
   */
  struct {
    const char *szFenStr; // FEN��A�S����(�p"startpos"��)�]�Ѹ������̲��ഫ��FEN��
    int nMoveNum;         // �Z��۪k��
    long *lpdwCoordList;  // �Z��۪k�A���V�{��"IdleLine()"�����@���R�A�ƲաA���i�H��"CoordList"�����ݦ��Ʋ�
  } Position;

  /* 3. "banmoves"���O�ǻ����T���A�A�X�_"e_CommBanMoves"���O����
   *    "banmoves"���O�Ψӳ]�m�T��۪k�A�ƾڬ[�c�������_"position"���O���Z��۪k�A���S��FEN��
   */
  struct {
    int nMoveNum;
    long *lpdwCoordList;
  } BanMoves;

  /* 4. "go"���O�ǻ����T���A�A�X�_"e_CommGo"��"e_CommGoPonder"���O����
   *    "go"���O���������(�j��)�A�P�ɳ]�w��ҼҦ��A�Y�T�w�`�סB�ɬq�s�٬O�[�ɻs
   */
  struct {
    UcciTimeEnum utMode; // ��ҼҦ�
    union {          
      int nDepth, nTime; 
    } DepthTime; // �p�G�O�T�w�`�סA�h��ܲ`��(�h)�A�p�G�O���w�ɶ��A�h��ܮɶ�(��)
    union {
      int nMovesToGo, nIncrement;
    } TimeMode;  // �p�G�O�[�ɻs�A�h���w�ɶ����n���h�֨B�ѡA�p�G�O�ɬq�s�A�h��ܨ����ӨB�Z���w�ɶ��[�h�֬�
  } Search;
}; // ucs

// �U���T�Ө�ƥΨӸ���UCCI���O�A���A�Τ_���P���X
UcciCommEnum BootLine(void);                                    // UCCI�����Ұʪ��Ĥ@�����O�A�u����"ucci"
UcciCommEnum IdleLine(UcciCommStruct &ucsCommand, Bool bDebug); // �����Ŷ��ɱ������O
UcciCommEnum BusyLine(Bool bDebug);                             // ������Үɱ������O�A�u���\����"stop"�M"ponderhit"
void ExitLine(void);

#endif
