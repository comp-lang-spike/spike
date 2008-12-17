
#ifndef __interp_h__
#define __interp_h__


#include "native.h"
#include "obj.h"
#include "oper.h"

#include <stddef.h>


/* XXX: This is arbitrary. */
#define LEAF_STACK_SPACE (4)


typedef struct Method Method;
typedef struct Context Context;
typedef struct Fiber Fiber;


typedef unsigned char opcode_t;

enum Opcode {
    OPCODE_NOP,
    OPCODE_PUSH_LOCAL,
    OPCODE_PUSH_INST_VAR,
    OPCODE_PUSH_GLOBAL,
    OPCODE_PUSH_SELF,
    OPCODE_PUSH_SUPER,
    OPCODE_PUSH_FALSE,
    OPCODE_PUSH_TRUE,
    OPCODE_PUSH_NULL,
    OPCODE_PUSH_VOID,
    OPCODE_PUSH_CONTEXT,
    OPCODE_DUP,
    OPCODE_DUP_N,
    OPCODE_PUSH_INT,
    OPCODE_STORE_LOCAL,
    OPCODE_STORE_INST_VAR,
    OPCODE_STORE_GLOBAL,
    OPCODE_POP,
    OPCODE_ROT,
    OPCODE_BRANCH_IF_FALSE,
    OPCODE_BRANCH_IF_TRUE,
    OPCODE_BRANCH_ALWAYS,
    OPCODE_ID,
    OPCODE_OPER,
    OPCODE_OPER_SUPER,
    OPCODE_CALL,
    OPCODE_CALL_VAR,
    OPCODE_CALL_SUPER,
    OPCODE_CALL_SUPER_VAR,
    OPCODE_ATTR,
    OPCODE_ATTR_SUPER,
    OPCODE_ATTR_VAR,
    OPCODE_ATTR_VAR_SUPER,
    OPCODE_SEND_MESSAGE,
    OPCODE_SEND_MESSAGE_SUPER,
    OPCODE_RET,
    OPCODE_RET_LEAF,
    OPCODE_RET_TRAMP,
    OPCODE_LEAF,
    OPCODE_SAVE,
    OPCODE_SAVE_VAR,
    OPCODE_RESTORE_SENDER,
    OPCODE_RESTORE_CALLER,
    OPCODE_THUNK,
    OPCODE_CALL_THUNK,
    OPCODE_CHECK_STACKP,
    
    NUM_OPCODES
};


typedef Object Null, Uninit, Void;


struct Context {
    VariableObject base;
    Context *caller; /* a.k.a. "sender" */
    opcode_t *pc;
    Object **stackp;
    Context *homeContext;
    union {
        struct /* MethodContext */ {
            Method *method;
            struct Behavior *methodClass;
            Object *receiver;
            Object **framep;
        } m;
        struct /* BlockContext */ {
            size_t nargs;
            opcode_t *startpc;
        } b;
    } u;
};

typedef struct ContextSubclass {
    Context base;
    Object *variables[1]; /* stretchy */
} ContextSubclass;

#define SpkContext_VARIABLES(op) ((Object **)SpkVariableObject_ITEM_BASE(op))


typedef struct Semaphore {
    Object base;
    Fiber *firstLink;
    Fiber *lastLink;
    int excessSignals;
} Semaphore;


struct Fiber {
    Object base;
    Fiber *nextLink;
    Context *suspendedContext;
    int priority;
    Semaphore *myList;
};


typedef struct ProcessorScheduler {
    Object base;
    Semaphore **quiescentFiberLists;
    Fiber *activeFiber;
} ProcessorScheduler;


struct Method {
    VariableObject base;
    SpkNativeCode nativeCode;
};

typedef struct MethodSubclass {
    Method base;
    Object *variables[1]; /* stretchy */
} MethodSubclass;

#define SpkMethod_OPCODES(op) ((opcode_t *)SpkVariableObject_ITEM_BASE(op))


typedef struct Thunk {
    Object base;
    Object *receiver;
    Method *method;
    struct Behavior *methodClass;
    opcode_t *pc;
} Thunk;

typedef struct ThunkSubclass {
    Thunk base;
    Object *variables[1]; /* stretchy */
} ThunkSubclass;


typedef struct Message {
    Object base;
    struct Symbol *messageSelector;
    struct VariableObject *argumentArray;
} Message;

typedef struct MessageSubclass {
    Message base;
    Object *variables[1]; /* stretchy */
} MessageSubclass;


typedef struct Interpreter {
    Object base;

    /* fibers */
    ProcessorScheduler *scheduler;
    Semaphore *theInterruptSemaphore;
    int interruptPending;

    /* contexts */
    Context *activeContext;
    Context *newContext;

    /* special objects */
    struct Symbol *selectorCannotReturn;
    struct Symbol *selectorDoesNotUnderstand;
    struct Symbol *selectorMustBeArray;
    struct Symbol *selectorMustBeBoolean;
    struct Symbol *selectorMustBeSymbol;
    struct Symbol *selectorNoRunnableFiber;
    struct Symbol *selectorUnknownOpcode;
    struct Symbol *selectorWrongNumberOfArguments;

    /* error handling */
    int printingStack;

} Interpreter;


extern Null *Spk_null;
extern Uninit *Spk_uninit;
extern Void *Spk_void;

extern struct Behavior *Spk_ClassMessage,*Spk_ClassMethod, *Spk_ClassThunk, *Spk_ClassContext, *Spk_ClassNull, *Spk_ClassUninit, *Spk_ClassVoid;
extern struct SpkClassTmpl Spk_ClassMessageTmpl, Spk_ClassMethodTmpl, Spk_ClassThunkTmpl, Spk_ClassContextTmpl, Spk_ClassNullTmpl, Spk_ClassUninitTmpl, Spk_ClassVoidTmpl;
extern Interpreter *theInterpreter; /* XXX */


Message *SpkMessage_new(void);
Method *SpkMethod_new(size_t);

Context *SpkContext_new(size_t);
Context *SpkContext_blockCopy(Context *, size_t, opcode_t *);
void SpkContext_init(Context *, Context *);
void SpkContext_initWithArg(Context *, Object *, Context *);

Semaphore *SpkSemaphore_new(void);
int SpkSemaphore_isEmpty(Semaphore *);
void SpkSemaphore_addLast(Semaphore *, Fiber *);
Fiber *SpkSemaphore_removeFirst(Semaphore *);

Object *SpkInterpreter_start(Object *);
void SpkInterpreter_init(Interpreter *, ProcessorScheduler *);
Object *SpkInterpreter_interpret(Interpreter *);
void SpkInterpreter_synchronousSignal(Interpreter *, Semaphore *);
void SpkInterpreter_transferTo(Interpreter *, Fiber *);
Fiber *SpkInterpreter_wakeHighestPriority(Interpreter *);
void SpkInterpreter_putToSleep(Interpreter *, Fiber *);
void SpkInterpreter_resume(Interpreter *, Fiber *);
void SpkInterpreter_printCallStack(Interpreter *);
Fiber *SpkInterpreter_trap(Interpreter *, struct Symbol *, Object *);
void SpkInterpreter_unknownOpcode(Interpreter *);
void SpkInterpreter_halt(Interpreter *, struct Symbol *, Object *);


#endif /* __interp_h__ */
