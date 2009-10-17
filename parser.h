
#ifndef __spk_parser_h__
#define __spk_parser_h__


#include "lexer.h"
#include "tree.h"

#include <stdio.h>


struct SpkSymbolNode;


typedef struct SpkParser {
    SpkObject base;
    SpkStmt *root;
    int error;
    struct SpkSymbolTable *st;
} SpkParser;


SpkStmt *SpkParser_NewClassDef(struct SpkToken *, struct SpkToken *, SpkStmt *, SpkStmt *,
                               struct SpkSymbolTable *);
SpkExpr *SpkParser_NewAttrExpr(SpkExpr *, struct SpkToken *, struct SpkToken *,
                               struct SpkSymbolTable *);
SpkExpr *SpkParser_NewClassAttrExpr(struct SpkToken *, struct SpkToken *, struct SpkToken *,
                                    struct SpkSymbolTable *);
SpkExpr *SpkParser_NewExpr(SpkExprKind, SpkOper, SpkExpr *, SpkExpr *, SpkExpr *, struct SpkToken *);
SpkExpr *SpkParser_NewBlock(SpkStmt *, SpkExpr *, struct SpkToken *);
SpkExpr *SpkParser_NewKeywordExpr(struct SpkToken *, SpkExpr *, struct SpkSymbolTable *);
SpkExpr *SpkParser_AppendKeyword(SpkExpr *, struct SpkToken *, SpkExpr *, struct SpkSymbolTable *);
SpkExpr *SpkParser_FreezeKeywords(SpkExpr *, struct SpkToken *, struct SpkSymbolTable *);
SpkExpr *SpkParser_NewCompoundExpr(SpkExpr *, struct SpkToken *);
SpkStmt *SpkParser_NewStmt(SpkStmtKind, SpkExpr *, SpkStmt *, SpkStmt *);
SpkStmt *SpkParser_NewForStmt(SpkExpr *, SpkExpr *, SpkExpr *, SpkStmt *);
SpkStmt *SpkParser_NewModuleDef(SpkStmt *);
SpkStmt *SpkParser_ParseFileStream(FILE *, struct SpkSymbolTable *);
SpkStmt *SpkParser_ParseString(const char *, struct SpkSymbolTable *);
void SpkParser_Source(SpkStmt **, SpkUnknown *);

void SpkParser_Parse(void *yyp, int yymajor, struct SpkToken yyminor, SpkParser *);
void *SpkParser_ParseAlloc(void *(*mallocProc)(size_t));
void SpkParser_ParseFree(void *p, void (*freeProc)(void*));
void SpkParser_ParseTrace(FILE *TraceFILE, char *zTracePrompt);


extern struct SpkClassTmpl Spk_ClassParserTmpl;


#endif /* __spk_parser_h__ */
