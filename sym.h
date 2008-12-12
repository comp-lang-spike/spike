
#ifndef __sym_h__
#define __sym_h__


#include "obj.h"


typedef struct Symbol {
    Object base;
    size_t hash;
    char str[1];
} Symbol;


extern struct Behavior *ClassSymbol;
extern struct SpkClassTmpl ClassSymbolTmpl;


Symbol *SpkSymbol_get(const char *str);


#endif /* __sym_h__ */
