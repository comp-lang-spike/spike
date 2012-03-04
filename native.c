
#include "native.h"

#include "class.h"
#include "host.h"
#include "interp.h"
#include "obj.h"
#include "rodata.h"
#include <assert.h>
#include <stdarg.h>


Method *NewNativeMethod(NativeCodeFlags flags, NativeCode nativeCode) {
    Method *newMethod;
    size_t argumentCount, variadic;
    size_t size;
    Opcode *ip;
    
    size = 0;
    if (flags & NativeCode_LEAF) {
        size += 5; /* leaf, arg, native */
    } else {
        size += 13; /* save, ... rett */
    }
    size += 2; /* restore, ret */
    
    variadic = 0;
    switch (flags & NativeCode_SIGNATURE_MASK) {
    case NativeCode_ARGS_0: argumentCount = 0; break;
    case NativeCode_ARGS_1: argumentCount = 1; break;
    case NativeCode_ARGS_2: argumentCount = 2; break;
        
    case NativeCode_ARGS_ARRAY:
        argumentCount = 0;
        variadic = 1;
        break;
        
    default: assert(0); /* XXX */
    }
    
    newMethod = Method_New(size);
    newMethod->nativeCode = nativeCode;
    
    ip = Method_OPCODES(newMethod);
    if (flags & NativeCode_LEAF) {
        assert(!variadic && "NativeCode_ARGS_ARRAY cannot be combined with NativeCode_LEAF");
        *ip++ = OPCODE_LEAF;
        *ip++ = OPCODE_ARG;
        *ip++ = (Opcode)argumentCount;
        *ip++ = (Opcode)argumentCount;
        *ip++ = OPCODE_NATIVE;
    } else {
        size_t stackSize = 4;
        size_t contextSize =
            stackSize +
            argumentCount + variadic;
        *ip++ = OPCODE_SAVE;
        *ip++ = (Opcode)contextSize;
        *ip++ = (Opcode)stackSize;
        *ip++ = variadic ? OPCODE_ARG_VA : OPCODE_ARG;
        *ip++ = (Opcode)argumentCount;
        *ip++ = (Opcode)argumentCount;
        *ip++ = OPCODE_NATIVE;
        
        /* skip trampoline code */
        *ip++ = OPCODE_BRANCH_ALWAYS;
        *ip++ = 6;
        
        /* trampolines for re-entering interpreted code */
        *ip++ = OPCODE_SEND_MESSAGE_NS_VAR_VA;
        *ip++ = OPCODE_RET_TRAMP;
        *ip++ = OPCODE_SEND_MESSAGE_SUPER_NS_VAR_VA;
        *ip++ = OPCODE_RET_TRAMP;
    }
    
    *ip++ = OPCODE_RESTORE_SENDER;
    *ip++ = OPCODE_RET;
    
    return newMethod;
}


/*------------------------------------------------------------------------*/
/* routines to send messages from native code */

Unknown *SendMessage(Interpreter *interpreter,
                            Unknown *obj,
                            unsigned int ns,
                            Unknown *selector,
                            Unknown *argumentArray)
{
    return Interpreter_SendMessage(interpreter, obj, ns, selector, argumentArray);
}

static Unknown *vSendMessage(Interpreter *interpreter,
                                Unknown *obj, unsigned int ns, Unknown *selector, va_list ap)
{
    Unknown *argumentList;

    argumentList = Host_ArgsFromVAList(ap);
    return SendMessage(interpreter, obj, ns, selector, argumentList);
}

static Unknown *sendMessage(Interpreter *interpreter,
                               Unknown *obj, unsigned int ns, Unknown *selector, ...)
{
    Unknown *result;
    va_list ap;
    
    va_start(ap, selector);
    result = vSendMessage(interpreter, obj, ns, selector, ap);
    va_end(ap);
    return result;
}

Unknown *SendOper(Interpreter *interpreter, Unknown *obj, Oper oper, ...) {
    Unknown *result;
    va_list ap;
    
    va_start(ap, oper);
    result = VSendOper(interpreter, obj, oper, ap);
    va_end(ap);
    return result;
}

Unknown *VSendOper(Interpreter *interpreter, Unknown *obj, Oper oper, va_list ap) {
    return vSendMessage(interpreter, obj, METHOD_NAMESPACE_RVALUE, *operSelectors[oper].selector, ap);
}

Unknown *Call(Interpreter *interpreter, Unknown *obj, CallOper oper, ...) {
    Unknown *result;
    va_list ap;
    
    va_start(ap, oper);
    result = VCall(interpreter, obj, oper, ap);
    va_end(ap);
    return result;
}

Unknown *VCall(Interpreter *interpreter, Unknown *obj, CallOper oper, va_list ap) {
    return vSendMessage(interpreter, obj, METHOD_NAMESPACE_RVALUE, *operCallSelectors[oper].selector, ap);
}

Unknown *Attr(Interpreter *interpreter, Unknown *obj, Unknown *name) {
    return SendMessage(interpreter, obj, METHOD_NAMESPACE_RVALUE, name, emptyArgs);
}

Unknown *SetAttr(Interpreter *interpreter, Unknown *obj, Unknown *name, Unknown *value) {
    return sendMessage(interpreter, obj, METHOD_NAMESPACE_LVALUE, name, value, 0);
}

Unknown *Send(Interpreter *interpreter, Unknown *obj, Unknown *selector, ...) {
    Unknown *result;
    va_list ap;
    
    va_start(ap, selector);
    result = VSend(interpreter, obj, selector, ap);
    va_end(ap);
    return result;
}

Unknown *VSend(Interpreter *interpreter, Unknown *obj, Unknown *selector, va_list ap) {
    return vSendMessage(interpreter, obj, METHOD_NAMESPACE_RVALUE, selector, ap);
}

Unknown *SendWithArguments(Interpreter *interpreter, Unknown *obj, Unknown *name,
                                  Unknown *argumentArray)
{
    return SendMessage(interpreter,
                           obj,
                           METHOD_NAMESPACE_RVALUE,
                           name,
                           argumentArray);
}


/*------------------------------------------------------------------------*/
/* halting */

void Halt(int code, const char *message) {
    Host_Halt(code, message);
}

void HaltWithFormat(int code, const char *format, ...) {
    va_list args;
    
    va_start(args, format);
    Host_VHaltWithFormat(code, format, args);
    va_end(args);
}

void HaltWithString(int code, Unknown *message) {
    Host_HaltWithString(code, message);
}


/*------------------------------------------------------------------------*/
/* argument processing */

int IsArgs(Unknown *op) {
    return Host_IsArgs(op);
}

size_t ArgsSize(Unknown *args) {
    return Host_ArgsSize(args);
}

Unknown *GetArg(Unknown *args, size_t index) {
    return Host_GetArg(args, index);
}
