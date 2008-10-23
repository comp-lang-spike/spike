
%name SpkParser_Parse
%token_type {Token}
%token_prefix TOKEN_
%extra_argument { ParserState *parserState }

%include {
    #include <assert.h>
    #include "lexer.h"
    #include "parser.h"
}

start ::= statement_list(stmtList).                                             { parserState->root = stmtList.first; }

%type statement_list {StmtList}
statement_list(r) ::= statement(stmt).                                          { r.first = stmt; r.last = stmt; }
statement_list(r) ::= statement_list(stmtList) statement(stmt).                 { r = stmtList; r.last->next = stmt; r.last = stmt; }

%type statement {Stmt *}
statement(r) ::= open_statement(stmt).                                          { r = stmt; }
statement(r) ::= closed_statement(stmt).                                        { r = stmt; }

%type open_statement {Stmt *}
open_statement(r) ::= IF LPAREN expr(expr) RPAREN statement(ifTrue).            { r = SpkParser_NewStmt(STMT_IF_ELSE, expr, ifTrue, 0); }
open_statement(r) ::= IF LPAREN expr(expr) RPAREN closed_statement(ifTrue)
                      ELSE open_statement(ifFalse).                             { r = SpkParser_NewStmt(STMT_IF_ELSE, expr, ifTrue, ifFalse); }

%type closed_statement {Stmt *}
closed_statement(r) ::= IF LPAREN expr(expr) RPAREN closed_statement(ifTrue)
                        ELSE closed_statement(ifFalse).                         { r = SpkParser_NewStmt(STMT_IF_ELSE, expr, ifTrue, ifFalse); }
closed_statement(r) ::= SEMI.                                                   { r = SpkParser_NewStmt(STMT_EXPR, 0, 0, 0); }
closed_statement(r) ::= expr(expr) SEMI.                                        { r = SpkParser_NewStmt(STMT_EXPR, expr, 0, 0); }
closed_statement(r) ::= compound_statement(stmt).                               { r = stmt; }
closed_statement(r) ::= RETURN            SEMI.                                 { r = SpkParser_NewStmt(STMT_RETURN, 0, 0, 0); }
closed_statement(r) ::= RETURN expr(expr) SEMI.                                 { r = SpkParser_NewStmt(STMT_RETURN, expr, 0, 0); }
closed_statement(r) ::= expr(expr) compound_statement(stmt).                    { r = SpkParser_NewStmt(STMT_DEF, expr, stmt, 0); }
closed_statement(r) ::= CLASS TYPE_IDENTIFIER(name) compound_statement(stmt).   { r = SpkParser_NewClassDef(name.sym, 0, stmt); }
closed_statement(r) ::= CLASS TYPE_IDENTIFIER(name) COLON TYPE_IDENTIFIER(super) compound_statement(stmt).
                                                                                { r = SpkParser_NewClassDef(name.sym, super.sym, stmt); }

%type compound_statement {Stmt *}
compound_statement(r) ::= LCURLY                          RCURLY.               { r = SpkParser_NewStmt(STMT_COMPOUND, 0, 0, 0); }
compound_statement(r) ::= LCURLY statement_list(stmtList) RCURLY.               { r = SpkParser_NewStmt(STMT_COMPOUND, 0, stmtList.first, stmtList.last); }

%type expr {Expr *}
expr(r) ::= binary_expr(expr).                                                  { r = expr; }

%left EQ NE.
%left GT GE LT LE.
%left PLUS MINUS.
%left TIMES DIVIDE.

%type binary_expr {Expr *}
binary_expr(r) ::= binary_expr(left) EQ binary_expr(right).                     { r = SpkParser_NewExpr(EXPR_BINARY, OPER_EQ, 0, left, right); }
binary_expr(r) ::= binary_expr(left) NE binary_expr(right).                     { r = SpkParser_NewExpr(EXPR_BINARY, OPER_NE, 0, left, right); }
binary_expr(r) ::= binary_expr(left) GT binary_expr(right).                     { r = SpkParser_NewExpr(EXPR_BINARY, OPER_GT, 0, left, right); }
binary_expr(r) ::= binary_expr(left) GE binary_expr(right).                     { r = SpkParser_NewExpr(EXPR_BINARY, OPER_GE, 0, left, right); }
binary_expr(r) ::= binary_expr(left) LT binary_expr(right).                     { r = SpkParser_NewExpr(EXPR_BINARY, OPER_LT, 0, left, right); }
binary_expr(r) ::= binary_expr(left) LE binary_expr(right).                     { r = SpkParser_NewExpr(EXPR_BINARY, OPER_LE, 0, left, right); }
binary_expr(r) ::= binary_expr(left) PLUS binary_expr(right).                   { r = SpkParser_NewExpr(EXPR_BINARY, OPER_ADD, 0, left, right); }
binary_expr(r) ::= binary_expr(left) MINUS binary_expr(right).                  { r = SpkParser_NewExpr(EXPR_BINARY, OPER_SUB, 0, left, right); }
binary_expr(r) ::= binary_expr(left) TIMES binary_expr(right).                  { r = SpkParser_NewExpr(EXPR_BINARY, OPER_MUL, 0, left, right); }
binary_expr(r) ::= binary_expr(left) DIVIDE binary_expr(right).                 { r = SpkParser_NewExpr(EXPR_BINARY, OPER_DIV, 0, left, right); }
binary_expr(r) ::= postfix_expr(expr).                                          { r = expr; }

%type postfix_expr {Expr *}
postfix_expr(r) ::= postfix_expr(func) LPAREN RPAREN.                           { r = SpkParser_NewExpr(EXPR_POSTFIX, OPER_CALL, 0, func, 0); }
postfix_expr(r) ::= postfix_expr(func) LPAREN argument_expr_list(args) RPAREN.  { r = SpkParser_NewExpr(EXPR_POSTFIX, OPER_CALL, 0, func, args.first); }
postfix_expr(r) ::= postfix_expr(obj) DOT IDENTIFIER(attr).                     { r = SpkParser_NewExpr(EXPR_ATTR, 0, 0, obj, 0); r->sym = attr.sym; }
postfix_expr(r) ::= primary_expr(expr).                                         { r = expr; }

%type primary_expr {Expr *}
primary_expr(r) ::= IDENTIFIER(token).                                          { r = SpkParser_NewExpr(EXPR_NAME, 0, 0, 0, 0); r->sym = token.sym; }
primary_expr(r) ::= INT(token).                                                 { r = SpkParser_NewExpr(EXPR_INT, 0, 0, 0, 0); r->intValue = token.intValue; }
primary_expr(r) ::= LPAREN expr(expr) RPAREN.                                   { r = expr; }

%type argument_expr_list {ExprList}
argument_expr_list(r) ::= expr(arg).                                            { r.first = arg; r.last = arg; }
argument_expr_list(r) ::= argument_expr_list(args) COMMA expr(arg).             { r = args; r.last->next = arg; r.last = arg; }

%syntax_error {
    printf("syntax error! token %d, line %u\n", TOKEN.id, TOKEN.lineNo);
    parserState->error = 1;
}

%parse_accept {
    /*printf("parsing complete!\n");*/
}

%parse_failure {
    fprintf(stderr,"Giving up.  Parser is hopelessly lost...\n");
}
