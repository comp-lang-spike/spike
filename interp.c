
#include "interp.h"

#include "array.h"
#include "behavior.h"
#include "bool.h"
#include "class.h"
#include "dict.h"
#include "int.h"
#include "module.h"
#include "sym.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*** TODO ***/
#define XXX 0
#define NESTING XXX
#define HIGHEST_PRIORITY XXX


/* Context pseudo-fields */
#define sender caller


Null *Spk_null;
Uninit *Spk_uninit;
Void *Spk_void;

Behavior *Spk_ClassMessage, *Spk_ClassMethod, *Spk_ClassThunk, *Spk_ClassContext, *Spk_ClassMethodContext, *Spk_ClassBlockContext;
Behavior *Spk_ClassNull, *Spk_ClassUninit, *Spk_ClassVoid;
Interpreter *theInterpreter; /* XXX */



/*------------------------------------------------------------------------*/
/* class templates */

static SpkMethodTmpl MessageMethods[] = {
    { 0, 0, 0}
};

SpkClassTmpl Spk_ClassMessageTmpl = {
    "Message",
    offsetof(MessageSubclass, variables),
    sizeof(Message),
    0,
    0,
    MessageMethods
};


static SpkMethodTmpl MethodMethods[] = {
    { 0, 0, 0}
};

SpkClassTmpl Spk_ClassMethodTmpl = {
    "Method",
    offsetof(MethodSubclass, variables),
    sizeof(Method),
    sizeof(opcode_t),
    0,
    MethodMethods
};


static SpkMethodTmpl ThunkMethods[] = {
    { 0, 0, 0}
};

SpkClassTmpl Spk_ClassThunkTmpl = {
    "Thunk",
    offsetof(ThunkSubclass, variables),
    sizeof(Thunk),
    0,
    0,
    ThunkMethods
};


static Object *Context_blockCopy(Object *_self, Object *arg0, Object *arg1);

static SpkMethodTmpl ContextMethods[] = {
    { "blockCopy", SpkNativeCode_METH_ATTR | SpkNativeCode_ARGS_2, &Context_blockCopy },
    { 0, 0, 0}
};

SpkClassTmpl Spk_ClassContextTmpl = {
    "Context",
    offsetof(ContextSubclass, variables),
    sizeof(Context),
    sizeof(Object *),
    0,
    ContextMethods
};


static SpkMethodTmpl MethodContextMethods[] = {
    { 0, 0, 0}
};

SpkClassTmpl Spk_ClassMethodContextTmpl = {
    "MethodContext",
    offsetof(ContextSubclass, variables),
    sizeof(Context),
    sizeof(Object *),
    0,
    MethodContextMethods
};


static SpkMethodTmpl BlockContextMethods[] = {
    { 0, 0, 0}
};

SpkClassTmpl Spk_ClassBlockContextTmpl = {
    "BlockContext",
    offsetof(ContextSubclass, variables),
    sizeof(Context),
    sizeof(Object *),
    0,
    BlockContextMethods
};


static SpkMethodTmpl NullMethods[] = {
    { 0, 0, 0}
};

SpkClassTmpl Spk_ClassNullTmpl = {
    "Null",
    offsetof(ObjectSubclass, variables),
    sizeof(Null),
    0,
    0,
    NullMethods
};


static SpkMethodTmpl UninitMethods[] = {
    { 0, 0, 0}
};

SpkClassTmpl Spk_ClassUninitTmpl = {
    "Uninit",
    offsetof(ObjectSubclass, variables),
    sizeof(Uninit),
    0,
    0,
    UninitMethods
};


static SpkMethodTmpl VoidMethods[] = {
    { 0, 0, 0}
};

SpkClassTmpl Spk_ClassVoidTmpl = {
    "Void",
    offsetof(ObjectSubclass, variables),
    sizeof(Void),
    0,
    0,
    VoidMethods
};


/*------------------------------------------------------------------------*/
/* initialization */

Object *SpkInterpreter_start(Object *obj, Symbol *selector, Array *argumentArray) {
    Method *callThunk, *callBlock, *trampoline;
    Context *context;
    Fiber fiber;
    ProcessorScheduler scheduler;
    Interpreter interpreter;
    opcode_t *ip; size_t trampolineSize;
    
    callThunk = SpkMethod_new(1);
    SpkMethod_OPCODES(callThunk)[0] = OPCODE_CALL_THUNK;
    SpkBehavior_insertMethod(Spk_ClassThunk, METHOD_NAMESPACE_RVALUE, SpkSymbol_get("__apply__"), callThunk);
    
    callBlock = SpkMethod_new(1);
    SpkMethod_OPCODES(callBlock)[0] = OPCODE_CALL_BLOCK;
    SpkBehavior_insertMethod(Spk_ClassBlockContext, METHOD_NAMESPACE_RVALUE, SpkSymbol_get("__apply__"), callBlock);
    
    trampolineSize = 6;
    trampoline = SpkMethod_new(trampolineSize);
    ip = SpkMethod_OPCODES(trampoline);
    *ip++ = OPCODE_SEND_MESSAGE;
    *ip++ = OPCODE_CALL;
    *ip++ = (opcode_t)METHOD_NAMESPACE_RVALUE;
    *ip++ = (opcode_t)OPER_APPLY;
    *ip++ = (opcode_t)0;
    *ip++ = OPCODE_RET_TRAMP;
    
    context = SpkContext_new(4);

    context->caller = 0;
    context->pc = SpkMethod_OPCODES(trampoline);
    context->stackp = &SpkContext_VARIABLES(context)[4];
    context->homeContext = context;
    context->u.m.method = trampoline;
    context->u.m.methodClass = obj->klass;
    context->u.m.receiver = obj;
    context->u.m.framep = context->stackp;
    
    /* push arguments on the stack */
    *--context->stackp = obj;
    *--context->stackp = (Object *)selector;
    *--context->stackp = (Object *)argumentArray;
    assert(context->stackp >= &SpkContext_VARIABLES(context)[0]);
    
    fiber.nextLink = 0;
    fiber.suspendedContext = context;
    fiber.priority = 0;
    fiber.myList = 0;
    
    scheduler.quiescentFiberLists = 0;
    scheduler.activeFiber = &fiber;
    
    SpkInterpreter_init(&interpreter, &scheduler);
    theInterpreter = &interpreter; /* XXX */
    return SpkInterpreter_interpret(&interpreter);
}

void SpkInterpreter_init(Interpreter *self, ProcessorScheduler *aScheduler) {
    /* fibers */
    self->scheduler = aScheduler;
    self->theInterruptSemaphore = SpkSemaphore_new();
    self->interruptPending = 0;

    /* contexts */
    self->activeContext = self->scheduler->activeFiber->suspendedContext;
    self->newContext = 0;

    /* special objects */
    self->selectorCannotReenterBlock     = SpkSymbol_get("cannotReenterBlock");
    self->selectorCannotReturn           = SpkSymbol_get("cannotReturn");
    self->selectorDoesNotUnderstand      = SpkSymbol_get("doesNotUnderstand");
    self->selectorMustBeArray            = SpkSymbol_get("mustBeArray");
    self->selectorMustBeBoolean          = SpkSymbol_get("mustBeBoolean");
    self->selectorMustBeSymbol           = SpkSymbol_get("mustBeSymbol");
    self->selectorNoRunnableFiber        = SpkSymbol_get("noRunnableFiber");
    self->selectorUnknownOpcode          = SpkSymbol_get("unknownOpcode");
    self->selectorWrongNumberOfArguments = SpkSymbol_get("wrongNumberOfArguments");

    /* error handling */
    self->printingStack = 0;
}

Message *SpkMessage_new() {
    Message *newMessage;
    
    newMessage = (Message *)malloc(sizeof(Message));
    newMessage->base.klass = Spk_ClassMessage;
    return newMessage;
}

Method *SpkMethod_new(size_t size) {
    Method *newMethod;
    
    newMethod = (Method *)malloc(sizeof(Method) + size*sizeof(opcode_t));
    newMethod->base.base.klass = Spk_ClassMethod;
    newMethod->base.size = size;
    newMethod->nextInScope = 0;
    newMethod->nestedClassList.first = 0;
    newMethod->nestedClassList.last = 0;
    newMethod->nestedMethodList.first = 0;
    newMethod->nestedMethodList.last = 0;
    newMethod->nativeCode = 0;
    return newMethod;
}


/*------------------------------------------------------------------------*/
/* contexts */

Context *SpkContext_new(size_t size) {
    Context *newContext;
    Object **p;
    size_t count;
    
    newContext = (Context *)malloc(sizeof(Context) + size*sizeof(Object *));
    newContext->base.base.klass = Spk_ClassMethodContext;
    newContext->base.size = size;
    
    for (p = &newContext->leaf.arguments[0];
         p < &newContext->leaf.arguments[LEAF_MAX_ARGUMENT_COUNT];
         ++p)
        *p = Spk_uninit;
    for (p = &newContext->leaf.stack[0];
         p < &newContext->leaf.stack[LEAF_MAX_STACK_SIZE];
         ++p)
        *p = Spk_uninit;
    
    count = size;
    for (p = SpkContext_VARIABLES(newContext); count > 0; ++p, --count)
        *p = Spk_uninit;
    
    newContext->mark = 0;

    return newContext;
}

static Object *Context_blockCopy(Object *_self, Object *arg0, Object *arg1) {
    Context *self; Integer *arg;
    size_t index, numArgs;
    size_t size;
    Context *newContext;
    Object **p;
    
    self = (Context *)_self;
    assert(arg = Spk_CAST(Integer, arg0)); /* XXX */
    index = (size_t)SpkInteger_asLong(arg);
    assert(arg = Spk_CAST(Integer, arg1)); /* XXX */
    numArgs = (size_t)SpkInteger_asLong(arg);
    
    /* The compiler guarantees that the home context is at least as
       big as the biggest child block context needs to be. */
    size = self->homeContext->base.size;
    
    newContext = SpkContext_new(size);
    newContext->base.base.klass = Spk_ClassBlockContext;
    newContext->caller = 0;
    newContext->pc = 0;
    newContext->stackp = 0;
    newContext->homeContext = self->homeContext;
    newContext->u.b.index = index;
    newContext->u.b.nargs = numArgs;
    newContext->u.b.startpc = self->pc + 3; /* skip branch */
    
    return (Object *)newContext;
}

void SpkContext_init(Context *self, Context *activeContext) {
    if (self->u.b.nargs != 0) {
        assert(XXX);
    }
    self->pc = self->u.b.startpc;
    self->stackp = &SpkContext_VARIABLES(self)[self->homeContext->base.size];
    self->sender = activeContext;
}

void SpkContext_initWithArg(Context *self, Object *argument, Context *activeContext) {
    if (self->u.b.nargs != 1) {
        assert(XXX);
    }
    SpkContext_VARIABLES(self)[0] = argument;
    self->pc = self->u.b.startpc;
    self->stackp = &SpkContext_VARIABLES(self)[self->homeContext->base.size - 1];
    self->sender = activeContext;
}


/*------------------------------------------------------------------------*/
/* interpreter loop */

/* stackPointer */
#define POP_OBJECT() (*stackPointer++)
#define POP(nItems) (stackPointer += nItems)
#define UNPOP(nItems) (stackPointer -= nItems)
#define PUSH(object) (*--stackPointer = (Object *)(object))
#define STACK_TOP() (*stackPointer)

#define INSTANCE_VARS(op, mc) ((Object **)(((char *)op) + (mc)->instVarOffset))


#define DECODE_UINT(result) do { \
    opcode_t byte; \
    unsigned int shift = 0; \
    (result) = 0; \
    while (1) { \
        byte = *instructionPointer++; \
        (result) |= ((byte & 0x7F) << shift); \
        if ((byte & 0x80) == 0) { \
            break; \
        } \
        shift += 7; \
    } \
} while (0)

#define DECODE_SINT(result) do { \
    opcode_t byte; \
    unsigned int shift = 0; \
    (result) = 0; \
    do { \
        byte = *instructionPointer++; \
        (result) |= ((byte & 0x7F) << shift); \
        shift += 7; \
    } while (byte & 0x80); \
    if (shift < 8*sizeof((result)) && (byte & 0x40)) { \
        /* sign extend */ \
        (result) |= - (1 << shift); \
    } \
} while (0)

/* XXX: What about leaf methods? */
#define TRAP(selector, argument) \
    self->activeContext->pc = instructionPointer; \
    self->activeContext->stackp = stackPointer; \
    SpkInterpreter_trap(self, selector, argument)


Object *SpkInterpreter_interpret(Interpreter *self) {

    /* context registers */
    Context *homeContext;
    Object *receiver;
    Method *method;
    Behavior *methodClass;
    register opcode_t *instructionPointer;
    opcode_t *linkRegister;
    register Object **stackPointer;
    register Object **framePointer;
    register Object **instVarPointer;
    register Object **globalPointer;
    register Object **literalPointer;
    Object ***display;

    size_t activation, index;

    /* message sending */
    MethodNamespace namespace;
    Symbol *messageSelector = 0;
    size_t argumentCount = 0, varArg = 0, variadic = 0;
    unsigned int operator;
    opcode_t *oldIP;
    
    /* unwinding */
    int mark = 666;
    
    self->activeContext->mark = &mark;
    assert(!self->activeContext->sender || self->activeContext->sender->mark); /* XXX */
    
 fetchContextRegisters:
    homeContext = self->activeContext->homeContext;
    receiver = homeContext->u.m.receiver;
    method = homeContext->u.m.method;
    methodClass = homeContext->u.m.methodClass;
    instructionPointer = self->activeContext->pc;
    linkRegister = 0;
    display = (Object ***)SpkContext_VARIABLES(self->activeContext);
    stackPointer = self->activeContext->stackp;
    framePointer = homeContext->u.m.framep;
    instVarPointer = INSTANCE_VARS(receiver, methodClass);
    globalPointer = SpkModule_VARIABLES(methodClass->module);
    literalPointer = SpkModule_LITERALS(methodClass->module);
    self->newContext = 0;

 checkForInterrupts:
    if (self->interruptPending) {
        self->interruptPending = 0;
        SpkInterpreter_synchronousSignal(self, self->theInterruptSemaphore);
        if (self->newContext) {
            self->activeContext = self->newContext;
            goto fetchContextRegisters;
        }
    }

 loop:
    while (1) {
        
        switch (*instructionPointer++) {
            
        default:
            --instructionPointer;
            SpkInterpreter_unknownOpcode(self);
            break;
            
        case OPCODE_NOP:
            break;

/*** push opcodes ***/
        case OPCODE_PUSH_LOCAL:
            DECODE_UINT(index);
            PUSH(framePointer[index]);
            break;
        case OPCODE_PUSH_INST_VAR:
            DECODE_UINT(index);
            PUSH(instVarPointer[index]);
            break;
        case OPCODE_PUSH_GLOBAL:
            DECODE_UINT(index);
            PUSH(globalPointer[index]);
            break;
        case OPCODE_PUSH:
            DECODE_UINT(activation);
            DECODE_UINT(index);
            PUSH(display[activation][index]);
            break;
        case OPCODE_PUSH_LITERAL:
            DECODE_UINT(index);
            PUSH(literalPointer[index]);
            break;
        case OPCODE_PUSH_SUPER:
            /* OPCODE_PUSH_SUPER is a pseudo-op equivalent to
             * OPCODE_PUSH_SELF.  The receiver is always pushed onto
             * the stack so that the stack clean-up machinery doesn't
             * have to distinguish between sends and super-sends.
             */
        case OPCODE_PUSH_SELF:     PUSH(receiver);             break;
        case OPCODE_PUSH_FALSE:    PUSH(Spk_false);            break;
        case OPCODE_PUSH_TRUE:     PUSH(Spk_true);             break;
        case OPCODE_PUSH_NULL:     PUSH(Spk_null);             break;
        case OPCODE_PUSH_VOID:     PUSH(Spk_void);             break;
        case OPCODE_PUSH_CONTEXT:  PUSH(self->activeContext);  break;
        case OPCODE_PUSH_INT: {
            long value;
            DECODE_SINT(value);
            PUSH(SpkInteger_fromLong(value));
            break; }
                
/*** store opcodes ***/
        case OPCODE_STORE_LOCAL:
            DECODE_UINT(index);
            framePointer[index] = STACK_TOP();
            break;
        case OPCODE_STORE_INST_VAR:
            DECODE_UINT(index);
            instVarPointer[index] = STACK_TOP();
            break;
        case OPCODE_STORE_GLOBAL:
            DECODE_UINT(index);
            globalPointer[index] = STACK_TOP();
            break;
        case OPCODE_STORE:
            DECODE_UINT(activation);
            DECODE_UINT(index);
            display[activation][index] = STACK_TOP();
            break;
            
/*** additional stack opcodes ***/
        case OPCODE_DUP: {
            Object *temp;
            temp = STACK_TOP();
            PUSH(temp);
            break; }
        case OPCODE_DUP_N: {
            Object **s;
            size_t n;
            DECODE_UINT(n);
            s = stackPointer + n;
            while (n--)
                *--stackPointer = *--s;
            break; }
        case OPCODE_POP:
            POP(1);
            break;
        case OPCODE_ROT: {
            Object **s, **end, *temp;
            size_t n;
            DECODE_UINT(n);
            if (n > 1) {
                temp = STACK_TOP();
                s = stackPointer;
                end = stackPointer + n - 1;
                while (s < end) {
                    s[0] = s[1];
                    ++s;
                }
                *s = temp;
            }
            break; }

/*** branch opcodes ***/
        case OPCODE_BRANCH_IF_FALSE: {
            ptrdiff_t displacement;
            Object *x, *o, *boolean;
            x = (Object *)Spk_false;
            o = (Object *)Spk_true;
            goto branch;
        case OPCODE_BRANCH_IF_TRUE:
            x = (Object *)Spk_true;
            o = (Object *)Spk_false;
 branch:
            boolean = POP_OBJECT();
            if (boolean == x) {
                opcode_t *base;
            case OPCODE_BRANCH_ALWAYS:
                base = instructionPointer - 1;
                DECODE_SINT(displacement);
                instructionPointer = base + displacement;
                if (displacement < 0) {
                    goto checkForInterrupts;
                }
            } else if (boolean != o) {
                --instructionPointer;
                UNPOP(1);
                TRAP(self->selectorMustBeBoolean, 0);
            } else {
                DECODE_SINT(displacement);
            }
            break; }
            
/* identity comparison opcode */
        case OPCODE_ID: {
            Object *x, *y;
            x = POP_OBJECT();
            y = POP_OBJECT();
            PUSH(x == y ? Spk_true : Spk_false);
            break; }

/*** send opcodes -- operators ***/
        case OPCODE_OPER:
            operator = (unsigned int)(*instructionPointer++);
            argumentCount = Spk_operSelectors[operator].argumentCount;
            varArg = 0;
            receiver = stackPointer[argumentCount];
            methodClass = receiver->klass;
            goto oper;
        case OPCODE_OPER_SUPER:
            operator = (unsigned int)(*instructionPointer++);
            argumentCount = Spk_operSelectors[operator].argumentCount;
            varArg = 0;
            methodClass = methodClass->superclass;
 oper:
            method = methodClass->ns[METHOD_NAMESPACE_RVALUE].operTable[operator].method;
            if (method) {
                methodClass = methodClass->ns[METHOD_NAMESPACE_RVALUE].operTable[operator].methodClass;
                goto callNewMethod;
            }
            instructionPointer -= 2;
            TRAP(self->selectorDoesNotUnderstand, (Object *)Spk_operSelectors[operator].messageSelector);
            break;
        case OPCODE_CALL:
            oldIP = instructionPointer - 1;
            namespace = (unsigned int)(*instructionPointer++);
            operator = (unsigned int)(*instructionPointer++);
            DECODE_UINT(argumentCount);
            varArg = 0;
            receiver = stackPointer[argumentCount];
            methodClass = receiver->klass;
            goto call;
        case OPCODE_CALL_VAR:
            oldIP = instructionPointer - 1;
            namespace = (unsigned int)(*instructionPointer++);
            operator = (unsigned int)(*instructionPointer++);
            DECODE_UINT(argumentCount);
            varArg = 1;
            receiver = stackPointer[argumentCount + 1];
            methodClass = receiver->klass;
            goto call;
        case OPCODE_CALL_SUPER:
            oldIP = instructionPointer - 1;
            namespace = (unsigned int)(*instructionPointer++);
            operator = (unsigned int)(*instructionPointer++);
            DECODE_UINT(argumentCount);
            varArg = 0;
            methodClass = methodClass->superclass;
            goto call;
        case OPCODE_CALL_SUPER_VAR:
            oldIP = instructionPointer - 1;
            operator = (unsigned int)(*instructionPointer++);
            namespace = (unsigned int)(*instructionPointer++);
            DECODE_UINT(argumentCount);
            varArg = 1;
            methodClass = methodClass->superclass;
 call:
            method = methodClass->ns[namespace].operCallTable[operator].method;
            if (method) {
                methodClass = methodClass->ns[namespace].operCallTable[operator].methodClass;
                goto callNewMethod;
            }
            instructionPointer = oldIP;
            TRAP(self->selectorDoesNotUnderstand, (Object *)Spk_operCallSelectors[operator].messageSelector);
            break;

/*** send opcodes -- "obj.attr" ***/
        case OPCODE_SET_ATTR:
            namespace = METHOD_NAMESPACE_LVALUE;
            argumentCount = 1;
            varArg = 0;
            goto attr;
        case OPCODE_GET_ATTR:
            namespace = METHOD_NAMESPACE_RVALUE;
            argumentCount = varArg = 0;
 attr:
            oldIP = instructionPointer - 1;
            receiver = stackPointer[argumentCount];
            methodClass = receiver->klass;
            DECODE_UINT(index);
            messageSelector = (Symbol *)(literalPointer[index]);
            goto lookupMethodInClass;
        case OPCODE_SET_ATTR_SUPER:
            namespace = METHOD_NAMESPACE_LVALUE;
            argumentCount = 1;
            varArg = 0;
            goto superAttr;
        case OPCODE_GET_ATTR_SUPER:
            namespace = METHOD_NAMESPACE_RVALUE;
            argumentCount = varArg = 0;
 superAttr:
            oldIP = instructionPointer - 1;
            methodClass = methodClass->superclass;
            DECODE_UINT(index);
            messageSelector = (Symbol *)(literalPointer[index]);
 lookupMethodInClass:
            for ( ; methodClass; methodClass = methodClass->superclass) {
                method = SpkBehavior_lookupMethod(methodClass, namespace, messageSelector);
                if (method) {
 callNewMethod:
                    /* call (jmpl -- jump and link) */
                    linkRegister = instructionPointer;
                    instructionPointer = SpkMethod_OPCODES(method);
 jump:
                    framePointer = stackPointer;
                    instVarPointer = INSTANCE_VARS(receiver, methodClass);
                    globalPointer = SpkModule_VARIABLES(methodClass->module);
                    literalPointer = SpkModule_LITERALS(methodClass->module);
                    goto loop;
                }
            }
            do { /* createActualMessage */
                Message *message;
                Array *varArgArray;
                
                if (varArg) {
                    varArgArray = Spk_CAST(Array, stackPointer[0]);
                    if (!varArgArray) {
                        TRAP(self->selectorMustBeArray, 0);
                    }
                } else {
                    varArgArray = 0;
                }
                message = SpkMessage_new();
                message->messageSelector = messageSelector;
                message->argumentArray = SpkArray_withArguments(stackPointer + varArg, argumentCount,
                                                                varArgArray, 0);
                instructionPointer = oldIP;
                TRAP(self->selectorDoesNotUnderstand, (Object *)message);
            } while (0);
            break;
            
/*** send opcodes -- "obj.*attr" ***/
        case OPCODE_SET_ATTR_VAR:
            namespace = METHOD_NAMESPACE_LVALUE;
            argumentCount = 1;
            varArg = 0;
            goto attrVar;
        case OPCODE_GET_ATTR_VAR:
            namespace = METHOD_NAMESPACE_RVALUE;
            argumentCount = varArg = 0;
 attrVar:
            receiver = stackPointer[argumentCount + 1];
            methodClass = receiver->klass;
 perform:
            messageSelector = Spk_CAST(Symbol, stackPointer[argumentCount]);
            if (!messageSelector) {
                --instructionPointer;
                TRAP(self->selectorMustBeSymbol, 0);
            }
            if (argumentCount == 1) {
                stackPointer[1] = stackPointer[0];
            }
            POP(1);
            oldIP = instructionPointer - 1;
            goto lookupMethodInClass;
        case OPCODE_SET_ATTR_VAR_SUPER:
            namespace = METHOD_NAMESPACE_LVALUE;
            argumentCount = 1;
            varArg = 0;
            goto superSetAttr;
        case OPCODE_GET_ATTR_VAR_SUPER:
            namespace = METHOD_NAMESPACE_RVALUE;
            argumentCount = varArg = 0;
 superSetAttr:
            methodClass = methodClass->superclass;
            goto perform;

/*** send opcodes -- generic ***/
        case OPCODE_SEND_MESSAGE:
            receiver = stackPointer[2];
            methodClass = receiver->klass;
 send:
            namespace = METHOD_NAMESPACE_RVALUE; /* XXX */
            messageSelector = Spk_CAST(Symbol, stackPointer[1]);
            if (!messageSelector) {
                --instructionPointer;
                TRAP(self->selectorMustBeSymbol, 0);
            }
            stackPointer[1] = stackPointer[0];
            POP(1);
            oldIP = instructionPointer - 1;
            argumentCount = 0;
            varArg = 1;
            goto lookupMethodInClass;
        case OPCODE_SEND_MESSAGE_SUPER:
            methodClass = methodClass->superclass;
            goto send;

/*** save/restore/return opcodes ***/
        case OPCODE_RET_LEAF: {
            /* return from leaf method */
            Object *result = POP_OBJECT();
            stackPointer = self->activeContext->stackp;
            POP(varArg + argumentCount + 1);
            PUSH(result); }
        case OPCODE_RET:
 ret:       /* ret/retl (blr) */
            instructionPointer = linkRegister;
            receiver = homeContext->u.m.receiver;
            method = homeContext->u.m.method;
            methodClass = homeContext->u.m.methodClass;
            display = (Object ***)SpkContext_VARIABLES(self->activeContext);
            framePointer = homeContext->u.m.framep;
            instVarPointer = INSTANCE_VARS(receiver, methodClass);
            globalPointer = SpkModule_VARIABLES(methodClass->module);
            literalPointer = SpkModule_LITERALS(methodClass->module);
            if (self->activeContext->mark != &mark) {
                /* suspend */
                self->activeContext->pc = linkRegister;
                self->activeContext->stackp = stackPointer;
                return 0; /* unwind */
            }
            break;
        case OPCODE_RET_TRAMP: {
            /* return from trampoline */
            Object *result = POP_OBJECT();
            self->activeContext->pc = instructionPointer;
            self->activeContext->stackp = stackPointer;
            return result; }
            
        case OPCODE_LEAF: {
            size_t fixedArgumentCount, varArgArraySize;
            Object **p;
            size_t i;
            
            DECODE_UINT(fixedArgumentCount);
            
            /* process arguments */
            if (varArg) {
                Array *varArgArray;
                
                varArgArray = Spk_CAST(Array, stackPointer[0]);
                varArgArraySize = varArgArray->size;
                if (!varArgArray) {
                    TRAP(self->selectorMustBeArray, 0);
                }
                if (argumentCount + varArgArraySize != fixedArgumentCount) {
                    TRAP(self->selectorWrongNumberOfArguments, 0);
                }
                
                /* copy & reverse arguments from stack */
                p = &self->activeContext->leaf.arguments[0];
                framePointer = p;
                for (i = 0; i < argumentCount; ++p, ++i) {
                    *p = stackPointer[1 + argumentCount - i - 1];
                }
                
                /* copy arguments from array */
                for (i = 0; i < varArgArraySize; ++p, ++i) {
                    *p = SpkArray_item(varArgArray, (long)i);
                }
                
            } else {
                varArgArraySize = 0;
                if (argumentCount != fixedArgumentCount) {
                    TRAP(self->selectorWrongNumberOfArguments, 0);
                }
                
                /* copy & reverse arguments from stack */
                p = &self->activeContext->leaf.arguments[0];
                framePointer = p;
                for (i = 0; i < fixedArgumentCount; ++p, ++i) {
                    *p = stackPointer[argumentCount - i - 1];
                }
            }
            
            for ( ; i < LEAF_MAX_ARGUMENT_COUNT; ++p, ++i) {
                *p = Spk_uninit;
            }
            
            /* initialize the stack */
            p = &self->activeContext->leaf.stack[0];
            for (i = 0; i < LEAF_MAX_STACK_SIZE; ++p, ++i) {
                *p = Spk_uninit;
            }
            self->activeContext->stackp = stackPointer;
            stackPointer = p;
            
            if (method->nativeCode) {
                Object *result, *arg1 = 0, *arg2 = 0;
                switch (argumentCount + varArgArraySize) {
                case 2: arg2 = framePointer[1];
                case 1: arg1 = framePointer[0];
                case 0: break;
                default: assert(XXX);
                }
                result = (*method->nativeCode)(receiver, arg1, arg2);
                assert(result);
                PUSH(result);
            }
            break; }
            
        case OPCODE_SAVE_VAR:
            variadic = 1;
            goto save;
        case OPCODE_SAVE: {
            /* save */
            size_t displaySize, fixedArgumentCount, localCount, stackSize;
            size_t contextSize, i, j, varArgArraySize;
            size_t excessStackArgCount, consumedArrayArgCount;
            Context *newContext;
            Array *varArgArray;
            Object **p;
            
            variadic = 0;
 save:
            /* Create a new context for the currently
             * executing method (cf. activateNewMethod).
             */
            DECODE_UINT(displaySize);
            DECODE_UINT(fixedArgumentCount);
            DECODE_UINT(localCount);
            DECODE_UINT(stackSize);
            
            contextSize = /*displaySize +*/ stackSize + fixedArgumentCount + variadic + localCount;
            newContext = SpkContext_new(contextSize);

            newContext->sender = self->activeContext;
            newContext->pc = instructionPointer;
            newContext->homeContext = newContext;
            newContext->u.m.method = method;
            newContext->u.m.methodClass = methodClass;
            newContext->u.m.receiver = receiver;
            newContext->mark = &mark;
            
            if (NESTING) {
                /* build the display */
                i = 0;
                display = (Object ***)SpkContext_VARIABLES(newContext);
                display[i++] = (Object **)(display + displaySize) + stackSize; /* frame pointer */
                display[i++] = INSTANCE_VARS(receiver, methodClass);
            
                if (i < displaySize) {
                    Object *outer;
                    Behavior *outerClass;
                
                    outerClass = receiver->klass->outerClass;
                    outer = receiver->klass->outer;
                    while (outerClass) {
                        assert(i < displaySize);
                        display[i++] = INSTANCE_VARS(outer, outerClass);
                        outerClass = outer->klass->outerClass;
                        outer = outer->klass->outer;
                    }
                
                    if (i < displaySize) {
                        /* copy the rest from the outer context */
                        Context *outerContext = Spk_CAST(Context, outer);
                        Object ***outerDisplay = (Object ***)SpkContext_VARIABLES(outerContext);
                        for (j = 0; i < displaySize; ++i, ++j) {
                            display[i] = outerDisplay[j];
                        }
                    }
                }
            }
            
            p = SpkContext_VARIABLES(newContext) + stackSize;
            newContext->stackp = newContext->u.m.framep = p;
            
            /* process arguments */
            if (varArg) {
                varArgArray = Spk_CAST(Array, stackPointer[0]);
                if (!varArgArray) {
                    TRAP(self->selectorMustBeArray, 0);
                }
                varArgArraySize = varArgArray->size;
            } else {
                varArgArray = 0;
                varArgArraySize = 0;
            }
                
            if (variadic
                ? argumentCount + varArgArraySize < fixedArgumentCount
                : argumentCount + varArgArraySize != fixedArgumentCount) {
                TRAP(self->selectorWrongNumberOfArguments, 0);
            }
            if (fixedArgumentCount > argumentCount) {
                /* copy & reverse fixed arguments from stack */
                for (i = 0; i < argumentCount; ++p, ++i) {
                    *p = stackPointer[varArg + argumentCount - i - 1];
                }
                excessStackArgCount = 0;
                /* copy fixed arguments from array */
                consumedArrayArgCount = fixedArgumentCount - argumentCount;
                for (i = 0; i < consumedArrayArgCount; ++p, ++i) {
                    *p = SpkArray_item(varArgArray, (long)i);
                }
            } else {
                /* copy & reverse fixed arguments from stack */
                for (i = 0; i < fixedArgumentCount; ++p, ++i) {
                    *p = stackPointer[varArg + argumentCount - i - 1];
                }
                excessStackArgCount = argumentCount - fixedArgumentCount;
                consumedArrayArgCount = 0;
            }
            if (variadic) {
                /* initialize the argument array variable */
                *p++ = (Object *)SpkArray_withArguments(stackPointer + varArg, excessStackArgCount,
                                                        varArgArray, consumedArrayArgCount);
            }
            
            /* clean up the caller's stack */
            POP(varArg + argumentCount + 1);
            
            self->activeContext->pc = linkRegister;
            self->activeContext->stackp = stackPointer;
            
            stackPointer = newContext->stackp;
            framePointer = newContext->u.m.framep;
            self->activeContext = homeContext = newContext;
            
            if (method->nativeCode) {
                Object *result, *arg1 = 0, *arg2 = 0;
                if (variadic) {
                    arg1 = framePointer[0];
                } else switch (argumentCount) {
                case 2: arg2 = framePointer[1];
                case 1: arg1 = framePointer[0];
                case 0: break;
                default: assert(XXX);
                }
                result = (*method->nativeCode)(receiver, arg1, arg2);
                if (result)
                    PUSH(result);
                else if (self->activeContext->mark != &mark)
                    return 0; /* unwind */
                else /* unwinding is done, and the result is already
                        on the stack */
                    goto fetchContextRegisters;
            }
            break; }
            
        case OPCODE_RESTORE_SENDER: {
            Context *thisCntx, *caller;
            /* restore sender */
            self->newContext = homeContext->sender;
            if (!self->newContext || !self->newContext->pc) {
                --instructionPointer;
                TRAP(self->selectorCannotReturn, 0);
                break;
            }
            thisCntx = self->activeContext;
            for ( ; thisCntx != self->newContext; thisCntx = caller) {
                caller = thisCntx->caller;
                thisCntx->caller = 0;
                thisCntx->pc = 0;
            }
            goto restore; }
        case OPCODE_RESTORE_CALLER: {
            Object *result;
            /* restore caller */
            self->newContext = self->activeContext->caller;
            self->activeContext->caller = 0;
            self->activeContext->pc = 0;
 restore:
            result = POP_OBJECT();

            self->activeContext = self->newContext; self->newContext = 0;
            homeContext = self->activeContext->homeContext;

            linkRegister = self->activeContext->pc;
            stackPointer = self->activeContext->stackp;
            framePointer = 0;
            display = 0;
            
            PUSH(result);
            break; }
            
/*** thunk opcodes ***/
        case OPCODE_THUNK: {
            /* new thunk */
            Thunk *thunk;
            if (argumentCount != 0) {
                TRAP(self->selectorWrongNumberOfArguments, 0);
            }
            if (varArg) {
                Array *varArgArray = Spk_CAST(Array, stackPointer[0]);
                if (!varArgArray) {
                    TRAP(self->selectorMustBeArray, 0);
                }
                if (varArgArray->size != 0) {
                    TRAP(self->selectorWrongNumberOfArguments, 0);
                }
                POP(1);
            }
            thunk = (Thunk *)malloc(sizeof(Thunk));
            thunk->base.klass = Spk_ClassThunk;
            thunk->receiver = receiver;
            thunk->method = method;
            thunk->methodClass = methodClass;
            thunk->pc = instructionPointer;
            stackPointer[0] = (Object *)thunk; /* replace receiver */
            goto ret; }
        case OPCODE_CALL_THUNK: {
            Thunk *thunk = (Thunk *)(receiver);
            receiver = thunk->receiver;
            method = thunk->method;
            methodClass = thunk->methodClass;
            instructionPointer = thunk->pc;
            goto jump; }
        
/*** block context opcodes ***/
        case OPCODE_CALL_BLOCK: {
            Context *blockContext;
            Object **p;
            size_t i;
            
            blockContext = (Context *)(receiver);
            
            if (blockContext->pc) {
                TRAP(self->selectorCannotReenterBlock, 0);
            }
            
            /* process arguments */
            if (varArg) {
                Array *varArgArray;
                
                varArgArray = Spk_CAST(Array, stackPointer[0]);
                if (!varArgArray) {
                    TRAP(self->selectorMustBeArray, 0);
                }
                if (argumentCount + varArgArray->size != blockContext->u.b.nargs) {
                    TRAP(self->selectorWrongNumberOfArguments, 0);
                }
                
                /* copy & reverse arguments from stack */
                p = &blockContext->homeContext->u.m.framep[blockContext->u.b.index];
                for (i = 0; i < argumentCount; ++p, ++i) {
                    *p = stackPointer[1 + argumentCount - i - 1];
                }
                
                /* copy arguments from array */
                for (i = 0; i < varArgArray->size; ++p, ++i) {
                    *p = SpkArray_item(varArgArray, (long)i);
                }
                
            } else {
                if (argumentCount != blockContext->u.b.nargs) {
                    TRAP(self->selectorWrongNumberOfArguments, 0);
                }
                
                /* copy & reverse arguments from stack */
                p = &blockContext->homeContext->u.m.framep[blockContext->u.b.index];
                for (i = 0; i < argumentCount; ++p, ++i) {
                    *p = stackPointer[argumentCount - i - 1];
                }
                
            }
            
            /* clean up the caller's stack */
            POP(varArg + argumentCount + 1);
            
            /* suspend the caller */
            self->activeContext->pc = linkRegister;
            self->activeContext->stackp = stackPointer;
            
            blockContext->caller = self->activeContext;
            blockContext->pc = blockContext->u.b.startpc;
            blockContext->stackp = &SpkContext_VARIABLES(blockContext)[blockContext->base.size];
            blockContext->mark = &mark;
            self->activeContext = blockContext;
            goto fetchContextRegisters; }
            
/*** debugging ***/
        case OPCODE_CHECK_STACKP: {
            size_t offset;
            DECODE_UINT(offset);
            assert(method != self->activeContext->u.m.method || /* in leaf routine */
                   /* XXX: What about BlockContexts? */
                   stackPointer == homeContext->u.m.framep - offset);
            break; }
        }
    }

}


/*------------------------------------------------------------------------*/
/* fibers */

Semaphore *SpkSemaphore_new() {
    return (Semaphore *)malloc(sizeof(Semaphore));
}

int SpkSemaphore_isEmpty(Semaphore *self) {
    return !self->firstLink;
}

void SpkSemaphore_addLast(Semaphore *self, Fiber *link) {
    if (SpkSemaphore_isEmpty(self)) {
        self->firstLink = link;
    } else {
        self->lastLink->nextLink = link;
    }
    self->lastLink = link;
}

Fiber *SpkSemaphore_removeFirst(Semaphore *self) {
    Fiber *first = self->firstLink;
    if (first == self->lastLink) {
        self->firstLink = self->lastLink = 0;
    } else {
        self->firstLink = first->nextLink;
    }
    first->nextLink = 0;
    return first;
}

void SpkInterpreter_synchronousSignal(Interpreter *self, Semaphore *aSemaphore) {
    if (SpkSemaphore_isEmpty(aSemaphore)) {
        ++aSemaphore->excessSignals;
    } else {
        SpkInterpreter_resume(self, SpkSemaphore_removeFirst(aSemaphore));
    }
}

void SpkInterpreter_transferTo(Interpreter *self, Fiber *newFiber) {
    Fiber *oldFiber = self->scheduler->activeFiber;
    oldFiber->suspendedContext = self->activeContext;
    self->scheduler->activeFiber = newFiber;
    self->newContext = newFiber->suspendedContext;
}

Fiber *SpkInterpreter_wakeHighestPriority(Interpreter *self) {
    int p = HIGHEST_PRIORITY - 1;
    Semaphore **fiberLists = self->scheduler->quiescentFiberLists;
    Semaphore *fiberList = fiberLists[p];
    while (SpkSemaphore_isEmpty(fiberList)) {
        --p;
        if (p < 0) {
            return SpkInterpreter_trap(self, self->selectorNoRunnableFiber, 0);
        }
        fiberList = fiberLists[p];
    }
    return SpkSemaphore_removeFirst(fiberList);
}

/* ----- private ----- */

void SpkInterpreter_putToSleep(Interpreter *self, Fiber *aFiber) {
    Semaphore **fiberLists = self->scheduler->quiescentFiberLists;
    Semaphore *fiberList = fiberLists[aFiber->priority - 1];
    SpkSemaphore_addLast(fiberList, aFiber);
    aFiber->myList = fiberList;
}

void SpkInterpreter_resume(Interpreter *self, Fiber *aFiber) {
    Fiber *activeFiber = self->scheduler->activeFiber;
    if (aFiber->priority > activeFiber->priority) {
        SpkInterpreter_putToSleep(self, activeFiber);
        SpkInterpreter_transferTo(self, aFiber);
    } else {
        SpkInterpreter_putToSleep(self, aFiber);
    }
}


/*------------------------------------------------------------------------*/
/* debug support */

void SpkInterpreter_printCallStack(Interpreter *self) {
    Context *ctxt, *home;
    Method *method;
    Behavior *methodClass;
    Symbol *methodSel, *call;
    
    call = SpkSymbol_get("__apply__");
    for (ctxt = self->activeContext; ctxt; ctxt = ctxt->sender) {
        home = ctxt->homeContext;
        method = home->u.m.method;
        methodClass = home->u.m.methodClass;
        methodSel = SpkBehavior_findSelectorOfMethod(methodClass, method);
        printf("%04x %p%s",
               ctxt->pc - SpkMethod_OPCODES(method), ctxt, (ctxt == home ? " " : " [] in "));
        if (methodSel == call) {
            printf("%s\n", SpkBehavior_name(methodClass));
        } else {
            printf("%s::%s\n",
                   SpkBehavior_name(methodClass),
                   methodSel ? methodSel->str : "<unknown>");
        }
    }
}


/*------------------------------------------------------------------------*/
/* error handling */

Fiber *SpkInterpreter_trap(Interpreter *self, Symbol *selector, Object *argument) {
    /* XXX */
    SpkInterpreter_halt(self, selector, argument);
    return 0;
}

void SpkInterpreter_unknownOpcode(Interpreter *self) {
    SpkInterpreter_trap(self, self->selectorUnknownOpcode, 0);
}

void SpkInterpreter_halt(Interpreter *self, Symbol *selector, Object *argument) {
    if (argument) {
        Symbol *symbol; Message *message;
        symbol = Spk_CAST(Symbol, argument);
        message = Spk_CAST(Message, argument);
        if (symbol) {
            printf("\n%s '%s'", selector->str, symbol->str);
        } else if (message) {
            printf("\n%s '%s'", selector->str, message->messageSelector->str);
        } else {
            printf("\n%s", selector->str);
        }
    } else {
        printf("\n%s", selector->str);
    }
    printf("\n\n");
    if (!self->printingStack) {
        self->printingStack = 1;
        SpkInterpreter_printCallStack(self);
    }
    abort();
}
