
#ifndef __spk_heart_h__
#define __spk_heart_h__


#include "module.h"


typedef struct SpkHeart {
    SpkModule base;


    /*
     * class variables
     */

    struct SpkBehavior *Behavior;
    struct SpkBehavior *BlockContext;
    struct SpkBehavior *Char;
    struct SpkBehavior *Class;
    struct SpkBehavior *Context;
    struct SpkBehavior *ContextClass;
    struct SpkBehavior *Expr;
    struct SpkBehavior *Fiber;
    struct SpkBehavior *Interpreter;
    struct SpkBehavior *Message;
    struct SpkBehavior *Metaclass;
    struct SpkBehavior *Method;
    struct SpkBehavior *MethodContext;
    struct SpkBehavior *Module;
    struct SpkBehavior *Notifier;
    struct SpkBehavior *Null;
    struct SpkBehavior *Object;
    struct SpkBehavior *Parser;
    struct SpkBehavior *ProcessorScheduler;
    struct SpkBehavior *Scope;
    struct SpkBehavior *STEntry;
    struct SpkBehavior *Stmt;
    struct SpkBehavior *SymbolNode;
    struct SpkBehavior *SymbolTable;
    struct SpkBehavior *Thunk;
    struct SpkBehavior *Uninit;
    struct SpkBehavior *VariableObject;
    struct SpkBehavior *Void;

#ifdef MALTIPY
    struct SpkBehavior *PythonObject;
#else /* !MALTIPY */
    struct SpkBehavior *Array;
    struct SpkBehavior *Boolean;
    struct SpkBehavior *False;
    struct SpkBehavior *FileStream;
    struct SpkBehavior *Float;
    struct SpkBehavior *IdentityDictionary;
    struct SpkBehavior *Integer;
    struct SpkBehavior *String;
    struct SpkBehavior *Symbol;
    struct SpkBehavior *True;
#endif


    /*
     * global variables
     */
    
    struct SpkInterpreter *theInterpreter;
    SpkUnknown *xfalse, *xtrue;
    SpkUnknown *null, *uninit, *xvoid;
    
    /* sometimes std* are macros */
    struct SpkFileStream *xstdin, *xstdout, *xstderr;


} SpkHeart;


extern struct SpkBehaviorTmpl Spk_ModuleHeartTmpl;

extern SpkModule *Spk_heart;


#define Spk_GLOBAL(g) (((SpkHeart *)Spk_heart)->g)
#define Spk_LITERAL(l) (SpkModule_LITERALS(Spk_heart)[l])

#define Spk_CLASS(c) Spk_GLOBAL(c)

#define Spk_HEART_CLASS_TMPL(n, s) Spk_CLASS_TMPL(n, s, SpkHeart)


#endif /* __spk_heart_h__ */
