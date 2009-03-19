/*! \file ucci.cpp
@brief UCCI通訊模塊（第三方提供，遵循GPL許可）

　　作者訊息︰黃晨 複旦大學化學系表面化學實驗室 E-mail︰webmaster at elephantbase dot net 
*/

// ucci.h/ucci.cpp - Source Code for ElephantEye, Part I

#include <stdio.h>
#include <string.h>
#include "base.h"
#include "ucci.h"  
#include "pipe.h"

static int ReadDigit(char *&lpLinePtr, int nMaxValue) {
  int nRetValue;
  nRetValue = 0;
  for (; ; ) {
    if (*lpLinePtr >= '0' && *lpLinePtr <= '9') {
      nRetValue *= 10;
      nRetValue += *lpLinePtr - '0';
      lpLinePtr ++;
      if (nRetValue > nMaxValue) {
        nRetValue = nMaxValue;
      }
    } else {
      break;
    }
  }
  return nRetValue;
}

/* UCCI指令分析模塊由三各UCCI指令解釋器組成。
 *
 * 其中第一個解釋器"BootLine()"最簡單，只用來接收引擎啟動后的第一行指令
 * 輸入"ucci"時就返回"UCCI_COMM_UCCI"，否則一律返回"UCCI_COMM_NONE"
 * 前兩個解釋器都等待是否有輸入，如果沒有輸入則執行待機指令"Idle()"
 * 而第三個解釋器("BusyLine()"，只用在引擎思考時)則在沒有輸入時直接返回"UCCI_COMM_NONE"
 */
static PipeStruct pipeStdHandle;

UcciCommEnum BootLine(void) {
  char szLineStr[LINE_INPUT_MAX_CHAR];
  pipeStdHandle.Open();
  while (!pipeStdHandle.LineInput(szLineStr)) {
    Idle();
  }
  if (strcmp(szLineStr, "ucci") == 0) {
    return UCCI_COMM_UCCI;
  } else {
    return UCCI_COMM_NONE;
  }
}

static long dwCoordList[256];

UcciCommEnum IdleLine(UcciCommStruct &ucsCommand, Bool bDebug) {
  char szLineStr[LINE_INPUT_MAX_CHAR];
  int i;
  char *lpLineChar;
  UcciCommEnum uceReturnValue;

  while (!pipeStdHandle.LineInput(szLineStr)) {
    Idle();
  }
  lpLineChar = szLineStr;
  if (bDebug) {
    printf("info string %s\n", lpLineChar);
    fflush(stdout);
  }
  // "IdleLine()"是最複雜的UCCI指令解釋器，大多數的UCCI指令都由它來解釋，包括︰

  // 1. "isready"指令
  if (strcmp(lpLineChar, "isready") == 0) {
    return UCCI_COMM_ISREADY;

  // 2. "setoption <option> [<arguments>]"指令
  } else if (strncmp(lpLineChar, "setoption ", 10) == 0) {
    lpLineChar += 10;

    // (i) "batch"選項
    if (strncmp(lpLineChar, "batch ", 6) == 0) {
      lpLineChar += 6;
      ucsCommand.Option.uoType = UCCI_OPTION_BATCH;
      if (strncmp(lpLineChar, "on", 2) == 0) {
        ucsCommand.Option.Value.bCheck = TRUE;
      } else if (strncmp(lpLineChar, "true", 4) == 0) {
        ucsCommand.Option.Value.bCheck = TRUE;
      } else {
        ucsCommand.Option.Value.bCheck = FALSE;
      } // 由於"batch"選項默認是關閉的，所以只有設定"on"或"true"時才打開，下同

    // (ii) "debug"選項
    } else if (strncmp(lpLineChar, "debug ", 6) == 0) {
      lpLineChar += 6;
      ucsCommand.Option.uoType = UCCI_OPTION_DEBUG;
      if (strncmp(lpLineChar, "on", 2) == 0) {
        ucsCommand.Option.Value.bCheck = TRUE;
      } else if (strncmp(lpLineChar, "true", 4) == 0) {
        ucsCommand.Option.Value.bCheck = TRUE;
      } else {
        ucsCommand.Option.Value.bCheck = FALSE;
      }

    // (iii) "usemillisec"選項
    } else if (strncmp(lpLineChar, "usemillisec ", 12) == 0) {
      lpLineChar += 12;
      ucsCommand.Option.uoType = UCCI_OPTION_USEMILLISEC;
      if (strncmp(lpLineChar, "on", 2) == 0) {
        ucsCommand.Option.Value.bCheck = TRUE;
      } else if (strncmp(lpLineChar, "true", 4) == 0) {
        ucsCommand.Option.Value.bCheck = TRUE;
      } else {
        ucsCommand.Option.Value.bCheck = FALSE;
      }

    // (iv) "bookfiles"選項
    } else if (strncmp(lpLineChar, "bookfiles ", 10) == 0) {
      ucsCommand.Option.uoType = UCCI_OPTION_BOOKFILES;
      ucsCommand.Option.Value.szString = lpLineChar + 10;

    // (v) "hashsize"選項
    } else if (strncmp(lpLineChar, "hashsize ", 9) == 0) {
      lpLineChar += 9;
      ucsCommand.Option.uoType = UCCI_OPTION_HASHSIZE;
      ucsCommand.Option.Value.nSpin = ReadDigit(lpLineChar, 1024);

    // (vi) "threads"選項
    } else if (strncmp(lpLineChar, "threads ", 8) == 0) {
      lpLineChar += 8;
      ucsCommand.Option.uoType = UCCI_OPTION_THREADS;
      ucsCommand.Option.Value.nSpin = ReadDigit(lpLineChar, 32);

    // (vii) "drawmoves"選項
    } else if (strncmp(lpLineChar, "drawmoves ", 10) == 0) {
      lpLineChar += 10;
      ucsCommand.Option.uoType = UCCI_OPTION_DRAWMOVES;
      ucsCommand.Option.Value.nSpin = ReadDigit(lpLineChar, 200);

    // (viii) "repetition"選項
    } else if (strncmp(lpLineChar, "repetition ", 11) == 0) {
      lpLineChar += 11;
      ucsCommand.Option.uoType = UCCI_OPTION_REPETITION;
      if (strncmp(lpLineChar, "alwaysdraw", 10) == 0) {
        ucsCommand.Option.Value.urRepet = UCCI_REPET_ALWAYSDRAW;
      } else if (strncmp(lpLineChar, "checkban", 8) == 0) {
        ucsCommand.Option.Value.urRepet = UCCI_REPET_CHECKBAN;
      } else if (strncmp(lpLineChar, "asianrule", 9) == 0) {
        ucsCommand.Option.Value.urRepet = UCCI_REPET_ASIANRULE;
      } else if (strncmp(lpLineChar, "chineserule", 11) == 0) {
        ucsCommand.Option.Value.urRepet = UCCI_REPET_CHINESERULE;
      } else {
        ucsCommand.Option.Value.urRepet = UCCI_REPET_CHINESERULE;
      }

    // (ix) "pruning"選項
    } else if (strncmp(lpLineChar, "pruning ", 8) == 0) {
      lpLineChar += 8;
      ucsCommand.Option.uoType = UCCI_OPTION_PRUNING;
      if (strncmp(lpLineChar, "none", 4) == 0) {
        ucsCommand.Option.Value.ugGrade = UCCI_GRADE_NONE;
      } else if (strncmp(lpLineChar, "small", 5) == 0) {
        ucsCommand.Option.Value.ugGrade = UCCI_GRADE_SMALL;
      } else if (strncmp(lpLineChar, "medium", 6) == 0) {
        ucsCommand.Option.Value.ugGrade = UCCI_GRADE_MEDIUM;
      } else if (strncmp(lpLineChar, "large", 5) == 0) {
        ucsCommand.Option.Value.ugGrade = UCCI_GRADE_LARGE;
      } else {
        ucsCommand.Option.Value.ugGrade = UCCI_GRADE_LARGE;
      }

    // (x) "knowledge"選項
    } else if (strncmp(lpLineChar, "knowledge ", 10) == 0) {
      lpLineChar += 10;
      ucsCommand.Option.uoType = UCCI_OPTION_KNOWLEDGE;
      if (strncmp(lpLineChar, "none", 4) == 0) {
        ucsCommand.Option.Value.ugGrade = UCCI_GRADE_NONE;
      } else if (strncmp(lpLineChar, "small", 5) == 0) {
        ucsCommand.Option.Value.ugGrade = UCCI_GRADE_SMALL;
      } else if (strncmp(lpLineChar, "medium", 6) == 0) {
        ucsCommand.Option.Value.ugGrade = UCCI_GRADE_MEDIUM;
      } else if (strncmp(lpLineChar, "large", 5) == 0) {
        ucsCommand.Option.Value.ugGrade = UCCI_GRADE_LARGE;
      } else {
        ucsCommand.Option.Value.ugGrade = UCCI_GRADE_LARGE;
      }

    // (xi) "selectivity"選項
    } else if (strncmp(lpLineChar, "selectivity ", 12) == 0) {
      lpLineChar += 12;
      ucsCommand.Option.uoType = UCCI_OPTION_SELECTIVITY;
      if (strncmp(lpLineChar, "none", 4) == 0) {
        ucsCommand.Option.Value.ugGrade = UCCI_GRADE_NONE;
      } else if (strncmp(lpLineChar, "small", 5) == 0) {
        ucsCommand.Option.Value.ugGrade = UCCI_GRADE_SMALL;
      } else if (strncmp(lpLineChar, "medium", 6) == 0) {
        ucsCommand.Option.Value.ugGrade = UCCI_GRADE_MEDIUM;
      } else if (strncmp(lpLineChar, "large", 5) == 0) {
        ucsCommand.Option.Value.ugGrade = UCCI_GRADE_LARGE;
      } else {
        ucsCommand.Option.Value.ugGrade = UCCI_GRADE_NONE;
      }

    // (xii) "style"選項
    } else if (strncmp(lpLineChar, "style ", 6) == 0) {
      lpLineChar += 6;
      ucsCommand.Option.uoType = UCCI_OPTION_STYLE;
      if (strncmp(lpLineChar, "solid", 5) == 0) {
        ucsCommand.Option.Value.usStyle = UCCI_STYLE_SOLID;
      } else if (strncmp(lpLineChar, "normal", 6) == 0) {
        ucsCommand.Option.Value.usStyle = UCCI_STYLE_NORMAL;
      } else if (strncmp(lpLineChar, "risky", 5) == 0) {
        ucsCommand.Option.Value.usStyle = UCCI_STYLE_RISKY;
      } else {
        ucsCommand.Option.Value.usStyle = UCCI_STYLE_NORMAL;
      }

    // (xiii) "loadbook"選項
    } else if (strncmp(lpLineChar, "loadbook", 8) == 0) {
      ucsCommand.Option.uoType = UCCI_OPTION_LOADBOOK;

    // (xiv) 無法識別的選項，有擴充的餘地
    } else {
      ucsCommand.Option.uoType = UCCI_OPTION_NONE;
    }
    return UCCI_COMM_SETOPTION;

  // 3. "position {<special_position> | fen <fen_string>} [moves <move_list>]"指令
  } else if (strncmp(lpLineChar, "position ", 9) == 0) {            
    lpLineChar += 9;
    // 首先判斷是否是特殊局面(這裡規定了5種)，是特殊局面就直接轉換成對應的FEN串
    if (strncmp(lpLineChar, "startpos", 8) == 0) {
      ucsCommand.Position.szFenStr = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1";
    } else if (strncmp(lpLineChar, "midgamepos", 10) == 0) {
      ucsCommand.Position.szFenStr = "2bakab1r/6r2/1cn4c1/p1p1p3p/9/2P3p2/PC2P1n1P/2N1B1NC1/R4R3/3AKAB2 w - - 0 1";
    } else if (strncmp(lpLineChar, "checkmatepos", 12) == 0) {
      ucsCommand.Position.szFenStr = "4kar2/4a2rn/4bc3/RN1c5/2bC5/9/4p4/9/4p4/3p1K3 w - - 0 1";
    } else if (strncmp(lpLineChar, "zugzwangpos", 11) == 0) {
      ucsCommand.Position.szFenStr = "3k5/4PP3/4r4/3P5/9/9/9/9/9/5K3 w - - 0 1";
    } else if (strncmp(lpLineChar, "endgamepos", 10) == 0) {
      ucsCommand.Position.szFenStr = "4k4/4a4/4P4/9/9/9/9/4B4/9/4K4 w - - 0 1";
    // 然後判斷是否指定了FEN串
    } else if (strncmp(lpLineChar, "fen ", 4) == 0) {
      ucsCommand.Position.szFenStr = lpLineChar + 4;
    // 如果兩者都不是，就立即返回
    } else {
      return UCCI_COMM_NONE;
    }
    // 然後尋找是否指定了后續著法，即是否有"moves"關鍵字
    lpLineChar = strstr(lpLineChar, " moves ");
    ucsCommand.Position.nMoveNum = 0;
    if (lpLineChar != NULL) {
      lpLineChar += 7;
      ucsCommand.Position.nMoveNum = ((strlen(lpLineChar) + 1) / 5); // 提示︰"moves"后面的每個著法都是4個字符和1個空格
      for (i = 0; i < ucsCommand.Position.nMoveNum; i ++) {
        dwCoordList[i] = *(long *) lpLineChar; // 4個字符可轉換為一個"long"，存儲和處理起來方便
        lpLineChar += 5;
      }
      ucsCommand.Position.lpdwCoordList = dwCoordList;
    }
    return UCCI_COMM_POSITION;

  // 4. "banmoves <move_list>"指令，處理起來和"position ... moves ..."是一樣的
  } else if (strncmp(lpLineChar, "banmoves ", 9) == 0) {
    lpLineChar += 9;
    ucsCommand.BanMoves.nMoveNum = ((strlen(lpLineChar) + 1) / 5);
    for (i = 0; i < ucsCommand.Position.nMoveNum; i ++) {
      dwCoordList[i] = *(long *) lpLineChar;
      lpLineChar += 5;
    }
    ucsCommand.BanMoves.lpdwCoordList = dwCoordList;
    return UCCI_COMM_BANMOVES;

  // 5. "go [ponder] {infinite | depth <depth> | time <time> [movestogo <moves_to_go> | increment <inc_time>]}"指令
  } else if (strncmp(lpLineChar, "go ", 3) == 0) {
    lpLineChar += 3;
    // 首先判斷到底是"go"還是"go ponder"，因為兩者解釋成不同的指令
    if (strncmp(lpLineChar, "ponder ", 7) == 0) {
      lpLineChar += 7;
      uceReturnValue = UCCI_COMM_GOPONDER;
    } else {
      uceReturnValue = UCCI_COMM_GO;
    }
    // 然後判斷到底是固定深度還是設定時限
    if (strncmp(lpLineChar, "time ", 5) == 0) {
      lpLineChar += 5;
      ucsCommand.Search.DepthTime.nTime = ReadDigit(lpLineChar, 36000);
      // 如果是設定時限，就要判斷是時段製還是加時製
      if (strncmp(lpLineChar, " movestogo ", 11) == 0) {
        lpLineChar += 11;
        ucsCommand.Search.utMode = UCCI_TIME_MOVE;
        ucsCommand.Search.TimeMode.nMovesToGo = ReadDigit(lpLineChar, 100);
        if (ucsCommand.Search.TimeMode.nMovesToGo < 1) {
          ucsCommand.Search.TimeMode.nMovesToGo = 1;
        }
      } else if (strncmp(lpLineChar, " increment ", 11) == 0) {
        lpLineChar += 11;
        ucsCommand.Search.utMode = UCCI_TIME_INC;
        ucsCommand.Search.TimeMode.nIncrement = ReadDigit(lpLineChar, 600);
      // 如果沒有說明是時段製還是加時製，就設定為步數是1的時段製
      } else {
        ucsCommand.Search.utMode = UCCI_TIME_MOVE;
        ucsCommand.Search.TimeMode.nMovesToGo = 1;
      }
    } else if (strncmp(lpLineChar, "depth ", 6) == 0) {
      lpLineChar += 6;
      ucsCommand.Search.utMode = UCCI_TIME_DEPTH;
      ucsCommand.Search.DepthTime.nDepth = ReadDigit(lpLineChar, UCCI_MAX_DEPTH - 1);
    // 如果沒有說明是固定深度還是設定時限，就固定深度為"UCCI_MAX_DEPTH"
    } else {
      ucsCommand.Search.utMode = UCCI_TIME_DEPTH;
      ucsCommand.Search.DepthTime.nDepth = UCCI_MAX_DEPTH - 1;
    }
    return uceReturnValue;

  // 5. "stop"指令
  } else if (strcmp(lpLineChar, "stop") == 0) {
    return UCCI_COMM_STOP;

  // 6. "quit"指令
  } else if (strcmp(lpLineChar, "quit") == 0) {
    return UCCI_COMM_QUIT;

  // 7. 無法識別的指令
  } else {
    return UCCI_COMM_NONE;
  }
}

UcciCommEnum BusyLine(Bool bDebug) {
  char szLineStr[LINE_INPUT_MAX_CHAR];
  if (pipeStdHandle.LineInput(szLineStr)) {
    if (bDebug) {
      printf("info string %s\n", szLineStr);
      fflush(stdout);
    }
    // "BusyLine"只能接收"isready"、"ponderhit"和"stop"這三條指令
    if (strcmp(szLineStr, "isready") == 0) {
      return UCCI_COMM_ISREADY;
    } else if (strcmp(szLineStr, "ponderhit") == 0) {
      return UCCI_COMM_PONDERHIT;
    } else if (strcmp(szLineStr, "stop") == 0) {
      return UCCI_COMM_STOP;
    } else {
      return UCCI_COMM_NONE;
    }
  } else {
    return UCCI_COMM_NONE;
  }
}
