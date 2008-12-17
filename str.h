
#ifndef __str_h__
#define __str_h__


#include "obj.h"


typedef VariableObject String;
typedef VariableObjectSubclass StringSubclass;


extern struct Behavior *Spk_ClassString;
extern struct SpkClassTmpl Spk_ClassStringTmpl;


String *SpkString_fromLiteral(char *, size_t);
char *SpkString_asString(String *);


#endif /* __str_h__ */
