
#ifndef __spk_scheck_h__
#define __spk_scheck_h__


#include "obj.h"


struct SpkStmt;
struct SpkStmtList;
struct SpkSymbolTable;


SpkUnknown *SpkStaticChecker_DeclareBuiltIn(struct SpkSymbolTable *st,
                                            SpkUnknown *requestor);
SpkUnknown *SpkStaticChecker_Check(struct SpkStmt *tree,
                                   struct SpkSymbolTable *st,
                                   SpkUnknown *requestor);


extern int Spk_declareBuiltIn; /* XXX */
extern int Spk_declareObject;


#endif /* __spk_scheck_h__ */
