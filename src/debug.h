//
//  debug.h
//  tbce
//
//  Created by George Watson on 27/12/2023.
//

#ifndef debug_h
#define debug_h
#include "common.h"
#include <stdarg.h>

void InitDebug(void);
void DebugPrint(int x, int y, int vw, int vh, Color color, const char *string);
void DebugFormat(int x, int y, int vw, int vh, Color color, const char *fmt, ...);

#endif /* debug_h */
