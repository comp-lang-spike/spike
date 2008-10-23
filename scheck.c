
#include "scheck.h"

#include "interp.h"
#include "st.h"
#include <assert.h>
#include <stdio.h>


typedef struct StaticChecker {
    SymbolTable *st;
    Symbol *self, *super;
} StaticChecker;


static void checkExpr(Expr *expr, Stmt *stmt, StaticChecker *checker, unsigned int pass) {
    Expr *arg;
    
    switch (expr->kind) {
    case EXPR_NAME:
        switch (pass) {
        case 1:
            if (0 && stmt->kind == STMT_DEF) {
                SpkSymbolTable_Insert(checker->st, expr);
            }
            break;
        case 2:
            if (stmt->kind != STMT_DEF) {
                if (expr->sym->sym == checker->self) {
                    expr->kind = EXPR_SELF;
                } else if (expr->sym->sym == checker->super) {
                    expr->kind = EXPR_SUPER;
                } else {
                    SpkSymbolTable_Bind(checker->st, expr);
                }
            }
            break;
        }
        break;
    case EXPR_POSTFIX:
        switch (expr->oper) {
        case OPER_CALL:
            checkExpr(expr->left, stmt, checker, pass);
            if (stmt->kind == STMT_DEF) {
                if (pass == 1) {
                    assert(expr->left->kind == EXPR_NAME);
                    stmt->u.method.name = expr->left->sym;
                    stmt->u.method.argList = expr->right;
                }
            } else {
                for (arg = expr->right; arg; arg = arg->next) {
                    checkExpr(arg, stmt, checker, pass);
                }
            }
            break;
        }
        break;
    case EXPR_ATTR:
        checkExpr(expr->left, stmt, checker, pass);
        break;
    case EXPR_BINARY:
        checkExpr(expr->left, stmt, checker, pass);
        checkExpr(expr->right, stmt, checker, pass);
        break;
    }
}

static void checkStmt(Stmt *stmt, Stmt *outer, StaticChecker *checker, unsigned int outerPass) {
    Stmt *s;
    SymbolNode *sym;
    Expr *arg;
    unsigned int innerPass;
    int enterNewContext;
    
    switch (stmt->kind) {
    case STMT_COMPOUND:
        if (outerPass == 2) {
            enterNewContext = outer &&
                              (outer->kind == STMT_DEF ||
                               outer->kind == STMT_DEF_CLASS);
            SpkSymbolTable_EnterScope(checker->st, enterNewContext);
            if (outer && outer->kind == STMT_DEF) {
                /* declare function arguments */
                for (arg = outer->u.method.argList; arg; arg = arg->next) {
                    assert(arg->kind == EXPR_NAME);
                    SpkSymbolTable_Insert(checker->st, arg);
                    ++outer->u.method.argumentCount;
                }
            }
            for (innerPass = 1; innerPass < 3; ++innerPass) {
                for (s = stmt->top; s; s = s->next) {
                    checkStmt(s, stmt, checker, innerPass);
                }
            }
            SpkSymbolTable_ExitScope(checker->st);
        }
        break;
    case STMT_DEF:
        checkExpr(stmt->expr, stmt, checker, outerPass);
        checkStmt(stmt->top, stmt, checker, outerPass);
        break;
    case STMT_DEF_CLASS:
        if (outerPass == 1) {
            assert(stmt->expr->kind == EXPR_NAME);
            SpkSymbolTable_Insert(checker->st, stmt->expr);
        }
        if (stmt->u.klass.super) {
            checkExpr(stmt->u.klass.super, stmt, checker, outerPass);
        }
        checkStmt(stmt->top, stmt, checker, outerPass);
        break;
    case STMT_EXPR:
        checkExpr(stmt->expr, stmt, checker, outerPass);
        break;
    case STMT_IF_ELSE:
        checkExpr(stmt->expr, stmt, checker, outerPass);
        checkStmt(stmt->top, stmt, checker, outerPass);
        if (stmt->bottom) {
            checkStmt(stmt->bottom, stmt, checker, outerPass);
        }
        break;
    case STMT_RETURN:
        /* return */
        if (stmt->expr) {
            checkExpr(stmt->expr, stmt, checker, outerPass);
        }
        break;
    }
}

int SpkStaticChecker_Check(Stmt *tree, unsigned int *pDataSize) {
    Stmt *s;
    StaticChecker checker;
    unsigned int pass;
    
    checker.st = SpkSymbolTable_New();
    checker.self = SpkSymbol_get("self");
    checker.super = SpkSymbol_get("super");
    SpkSymbolTable_EnterScope(checker.st, 1);
    
    for (pass = 1; pass < 3; ++pass) {
        for (s = tree; s; s = s->next) {
            checkStmt(s, 0, &checker, pass);
        }
    }
    
    *pDataSize = checker.st->currentScope->context->nDefs;
    
    SpkSymbolTable_ExitScope(checker.st);
    
    if (checker.st->errorList) {
        SymbolNode *sym;
        
        fprintf(stderr, "errors!\n");
        for (sym = checker.st->errorList; sym; sym = sym->nextError) {
            if (sym->multipleDefList) {
                fprintf(stderr, "symbol '%s' multiply defined\n",
                        sym->sym->str);
            }
            if (sym->undefList) {
                fprintf(stderr, "symbol '%s' undefined\n",
                        sym->sym->str);
            }
        }
        return -1;
    }
    
    return 0;
}
