/*! \file ucci.h
@brief UCCI通訊模塊（第三方提供，遵循GPL許可）

　　作者訊息︰黃晨 複旦大學化學系表面化學實驗室 E-mail︰webmaster at elephantbase dot net 
*/

#ifndef UCCI_H
#define UCCI_H

#include "base.h"
const int UCCI_MAX_DEPTH = 32; // UCCI引擎思考的極限深度

// 和UCCI指令中關鍵字有關的選項
enum UcciOptionEnum {
  UCCI_OPTION_NONE, UCCI_OPTION_BATCH, UCCI_OPTION_DEBUG, UCCI_OPTION_USEMILLISEC, UCCI_OPTION_BOOKFILES, UCCI_OPTION_HASHSIZE, UCCI_OPTION_THREADS, UCCI_OPTION_DRAWMOVES,
  UCCI_OPTION_REPETITION, UCCI_OPTION_PRUNING, UCCI_OPTION_KNOWLEDGE, UCCI_OPTION_SELECTIVITY, UCCI_OPTION_STYLE, UCCI_OPTION_LOADBOOK
}; // uo, 由"setoption"指定的選項
enum UcciRepetEnum {
  UCCI_REPET_ALWAYSDRAW, UCCI_REPET_CHECKBAN, UCCI_REPET_ASIANRULE, UCCI_REPET_CHINESERULE
}; // ur
enum UcciGradeEnum {
  UCCI_GRADE_NONE, UCCI_GRADE_SMALL, UCCI_GRADE_MEDIUM, UCCI_GRADE_LARGE
}; // ug
enum UcciStyleEnum {
  UCCI_STYLE_SOLID, UCCI_STYLE_NORMAL, UCCI_STYLE_RISKY
}; // us, 選項style的設定值
enum UcciTimeEnum {
  UCCI_TIME_DEPTH, UCCI_TIME_MOVE, UCCI_TIME_INC
}; // ut, 由"go"指令指定的時間模式，分別是固定深度(無限深相當于深度為"c_MaxDepth")、時段製(幾秒鐘內必須完成幾步)和加時製(剩餘時間多少，走完這步后加幾秒)
enum UcciCommEnum {
  UCCI_COMM_NONE, UCCI_COMM_UCCI, UCCI_COMM_ISREADY, UCCI_COMM_PONDERHIT, UCCI_COMM_STOP, UCCI_COMM_SETOPTION, UCCI_COMM_POSITION, UCCI_COMM_BANMOVES, UCCI_COMM_GO, UCCI_COMM_GOPONDER, UCCI_COMM_QUIT
}; // uce, UCCI指令類型

///UCCI通訊模塊（第三方提供，遵循GPL許可） 
/*!
　　我們直接使用UCCI開發人員提供的通用通訊模塊（遵循GPL許可）。

　　具體實現和功能說明參見︰http://www.elephantbase.net/protocol/cchess_ucci.htm。
*/
// UCCI指令可以解釋成以下這個抽象的架構
union UcciCommStruct {

  /* 可得到具體訊息的UCCI指令只有以下4種類型
   *
   * 1. "setoption"指令傳遞的訊息，適合于"UCCI_COMM_SETOPTION"指令類型
   *    "setoption"指令用來設定選項，因此引擎接受到的訊息有“選項類型”和“選項值”
   *    例如，"setoption batch on"，選項類型就是"UCCI_OPTION_DEBUG"，值(Value.bCheck)就是"TRUE"
   */
  struct {
    UcciOptionEnum uoType;   // 選項類型
    union {                  // 選項值
      int nSpin;             // "spin"類型的選項的值
      Bool bCheck;           // "check"類型的選項的值
      UcciRepetEnum urRepet; // "combo"類型的選項"repetition"的值
      UcciGradeEnum ugGrade; // "combo"類型的選項"pruning"、"knowledge"和"selectivity"的值
      UcciStyleEnum usStyle; // "combo"類型的選項"style"的值
      const char *szString;  // "string"類型的選項的值
    } Value;                 // "button"類型的選項沒有值
  } Option;

  /* 2. "position"指令傳遞的訊息，適合于"e_CommPosition"指令類型
   *    "position"指令用來設置局面，包括初始局面連同后續著法構成的局面
   *    例如，position startpos moves h2e2 h9g8，FEN串就是"startpos"代表的FEN串，著法數(MoveNum)就是2
   */
  struct {
    const char *szFenStr; // FEN串，特殊局面(如"startpos"等)也由解釋器最終轉換成FEN串
    int nMoveNum;         // 后續著法數
    long *lpdwCoordList;  // 后續著法，指向程式"IdleLine()"中的一個靜態數組，但可以把"CoordList"本身看成數組
  } Position;

  /* 3. "banmoves"指令傳遞的訊息，適合于"e_CommBanMoves"指令類型
   *    "banmoves"指令用來設置禁止著法，數據架構時類似于"position"指令的后續著法，但沒有FEN串
   */
  struct {
    int nMoveNum;
    long *lpdwCoordList;
  } BanMoves;

  /* 4. "go"指令傳遞的訊息，適合于"e_CommGo"或"e_CommGoPonder"指令類型
   *    "go"指令讓引擎思考(搜索)，同時設定思考模式，即固定深度、時段製還是加時製
   */
  struct {
    UcciTimeEnum utMode; // 思考模式
    union {          
      int nDepth, nTime; 
    } DepthTime; // 如果是固定深度，則表示深度(層)，如果是限定時間，則表示時間(秒)
    union {
      int nMovesToGo, nIncrement;
    } TimeMode;  // 如果是加時製，則限定時間內要走多少步棋，如果是時段製，則表示走完該步后限定時間加多少秒
  } Search;
}; // ucs

// 下面三個函數用來解釋UCCI指令，但適用于不同場合
UcciCommEnum BootLine(void);                                    // UCCI引擎啟動的第一條指令，只接收"ucci"
UcciCommEnum IdleLine(UcciCommStruct &ucsCommand, Bool bDebug); // 引擎空閒時接收指令
UcciCommEnum BusyLine(Bool bDebug);                             // 引擎思考時接收指令，只允許接收"stop"和"ponderhit"
void ExitLine(void);

#endif
