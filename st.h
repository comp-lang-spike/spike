
#ifndef __spk_st_h__
#define __spk_st_h__


#include "obj.h"


typedef struct SpkSymbolNode SpkSymbolNode;
typedef struct SpkSTEntry SpkSTEntry;
typedef struct SpkContextClass SpkContextClass;
typedef struct SpkScope SpkScope;
typedef struct SpkSymbolTable SpkSymbolTable;


struct SpkSymbolNode {
    SpkObject base;
    struct SpkSTEntry *entry;
    SpkUnknown *sym;
};


struct SpkSTEntry {
    SpkObject base;
    SpkScope *scope;
    SpkSTEntry *nextInScope;
    SpkSTEntry *shadow;
    SpkSymbolNode *sym;
    struct SpkExpr *def;
};


struct SpkContextClass {
    SpkObject base;
    SpkScope *scope;
    unsigned int level;
    /*SpkOpcode*/ unsigned int pushOpcode, storeOpcode;
    unsigned int nDefs;
};


struct SpkScope {
    SpkObject base;
    SpkScope *outer;
    SpkSTEntry *entryList;
    SpkContextClass *context;
};


struct SpkSymbolTable {
    SpkObject base;
    SpkScope *currentScope;
    SpkUnknown *symbolNodes;
};


SpkSymbolNode *SpkSymbolNode_FromSymbol(SpkSymbolTable *st, SpkUnknown *sym);
SpkSymbolNode *SpkSymbolNode_FromCString(SpkSymbolTable *st, const char *str);
SpkSymbolTable *SpkSymbolTable_New(void);
void SpkSymbolTable_EnterScope(SpkSymbolTable *st, int enterNewContext);
void SpkSymbolTable_ExitScope(SpkSymbolTable *st);
SpkUnknown *SpkSymbolTable_Insert(SpkSymbolTable *st, struct SpkExpr *def,
                                  SpkUnknown *requestor);
struct SpkExpr *SpkSymbolTable_Lookup(SpkSymbolTable *st, SpkSymbolNode *sym);
SpkUnknown *SpkSymbolTable_Bind(SpkSymbolTable *st, struct SpkExpr *expr,
                                SpkUnknown *requestor);


extern struct SpkClassTmpl Spk_ClassSymbolNodeTmpl, Spk_ClassSTEntryTmpl,
    Spk_ClassContextClassTmpl, Spk_ClassScopeTmpl, Spk_ClassSymbolTableTmpl;


#endif /* __spk_st_h__ */
