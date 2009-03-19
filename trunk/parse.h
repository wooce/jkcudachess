#include <stdio.h>
#include <string.h>
#include "base.h"

#ifndef PARSE_H
#define PARSE_H

#ifdef _WIN32
  #if !defined(_INC_SHLWAPI) && !defined(_SHLWAPI_H)
    extern "C" __declspec(dllimport) char *__stdcall StrStrIA(const char *, const char *); // skip pre-compiling <windows.h>
  #endif
  inline char *strcasestr(const char *sz1, const char *sz2) {
    return StrStrIA(sz1, sz2);
  }
#endif

#ifdef _MSC_VER
  inline int strncasecmp(const char *sz1, const char *sz2, size_t n) {
    return strnicmp(sz1, sz2, n);
  }
#endif

inline void StrCutCrLf(char *sz) {
  char *lpsz;
  lpsz = strchr(sz, '\r');
  if (lpsz != NULL) {
    *lpsz = '\0';
  }
  lpsz = strchr(sz, '\n');
  if (lpsz != NULL) {
    *lpsz = '\0';
  }
}

inline Bool StrEqv(const char *sz1, const char *sz2) {
  return strncasecmp(sz1, sz2, strlen(sz2)) == 0;
}

inline Bool StrEqvSkip(const char *&sz1, const char *sz2) {
  if (strncasecmp(sz1, sz2, strlen(sz2)) == 0) {
    sz1 += strlen(sz2);
    return TRUE;
  } else {
    return FALSE;
  }
}

inline Bool StrEqvSkip(char *&sz1, const char *sz2) {
  if (strncasecmp(sz1, sz2, strlen(sz2)) == 0) {
    sz1 += strlen(sz2);
    return TRUE;
  } else {
    return FALSE;
  }
}

inline Bool StrScan(const char *sz1, const char *sz2) {
  return strcasestr(sz1, sz2) != NULL;
}

inline Bool StrScanSkip(const char *&sz1, const char *sz2) {
  const char *lpsz;
  lpsz = strcasestr(sz1, sz2);
  if (lpsz == NULL) {
    return FALSE;
  } else {
    sz1 = lpsz + strlen(sz2);
    return TRUE;
  }
}

inline Bool StrScanSkip(char *&sz1, const char *sz2) {
  char *lpsz;
  lpsz = strcasestr(sz1, sz2);
  if (lpsz == NULL) {
    return FALSE;
  } else {
    sz1 = lpsz + strlen(sz2);
    return TRUE;
  }
}

inline Bool StrSplitSkip(const char *&szSrc, int nSeperator, char *szDst = NULL) {
  const char *lpsz;
  lpsz = strchr(szSrc, nSeperator);
  if (lpsz == NULL) {
    if (szDst != NULL) {
      strcpy(szDst, szSrc);
    }
    szSrc += strlen(szSrc);
    return FALSE;
  } else {
    if (szDst != NULL) {
      strncpy(szDst, szSrc, lpsz - szSrc);
      szDst[lpsz - szSrc] = '\0';
    }
    szSrc = lpsz + 1;
    return TRUE;
  }
}

inline Bool StrSplitSkip(char *&szSrc, int nSeperator, char *szDst = NULL) {
  char *lpsz;
  lpsz = strchr(szSrc, nSeperator);
  if (lpsz == NULL) {
    if (szDst != NULL) {
      strcpy(szDst, szSrc);
    }
    szSrc += strlen(szSrc);
    return FALSE;
  } else {
    if (szDst != NULL) {
      strncpy(szDst, szSrc, lpsz - szSrc);
      szDst[lpsz - szSrc] = '\0';
    }
    szSrc = lpsz + 1;
    return TRUE;
  }
}

inline int Str2Digit(const char *sz, int nMin, int nMax) {
  int nRet;
  if (sscanf(sz, "%d", &nRet) > 0) {
    return MIN(MAX(nRet, nMin), nMax);
  } else {
    return nMin;
  }
}

#ifdef _WIN32

const int PATH_SEPERATOR = '\\';

inline Bool AbsolutePath(const char *sz) {
  return sz[0] == '\\' || (((sz[0] >= 'A' && sz[0] <= 'Z') || (sz[0] >= 'a' && sz[0] <= 'z')) && sz[1] == ':');
}

#else

const int PATH_SEPERATOR = '/';

inline Bool AbsolutePath(const char *sz) {
  return sz[0] == '/' || (sz[0] == '~' && sz[1] == '/');
}

#endif

inline void LocatePath(char *szDst, const char *szSrc, const char *szPath) {
  char *lpSeperator;
  lpSeperator = strrchr((char *)szSrc, PATH_SEPERATOR);
  if (lpSeperator == NULL) {
    strcpy(szDst, szPath);
  } else {
    strncpy(szDst, szSrc, lpSeperator + 1 - szSrc);
    strcpy(szDst + (lpSeperator + 1 - szSrc), szPath);
  }
}

inline void AppendPath(char *szDst, const char *szSrc, const char *szPath) {
  int nLen;
  nLen = strlen(szSrc);
  if (nLen == 0) {
    strcpy(szDst, szPath);
  } else {
    if (szDst != szSrc) {
      strncpy(szDst, szSrc, nLen);
    }
    if (szDst[nLen - 1] != PATH_SEPERATOR) {
      szDst[nLen] = PATH_SEPERATOR;
      nLen ++;
    }
    strcpy(szDst + nLen, szPath);
  }
}

#endif
