
#include "disasm.h"

#include "behavior.h"
#include "cgen.h"
#include "dict.h"
#include "interp.h"
#include "module.h"
#include "sym.h"


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


static void tab(unsigned int indent, FILE *out) {
    while (indent--)
        fprintf(out, "\t");
}

void SpkDisassembler_disassembleMethodOpcodes(Method *method, unsigned int indent, FILE *out) {
    opcode_t *begin, *ip, *instructionPointer, *end;
    
    begin = SpkMethod_OPCODES(method);
    end = begin + method->base.size;
    instructionPointer = begin;
    
    while (instructionPointer < end) {
        opcode_t opcode;
        long intValue = 0, *pIntValue = 0;
        char *keyword = 0;
        char *selector = 0;
        const char *mnemonic = "unk", *base = 0;
        size_t activation = 0, index = 0;
        ptrdiff_t displacement = 0;
        size_t argumentCount = 0, *pArgumentCount = 0;
        size_t displaySize = 0, variadic = 0, localCount = 0, stackSize = 0;
        size_t count = 0, *pCount = 0;
        unsigned int namespace = 0, operator = 0;
        size_t label = 0, *pLabel = 0;
        int i;
        
        ip = instructionPointer;
        opcode = *instructionPointer++;
        
        switch (opcode) {
            
        case OPCODE_NOP: mnemonic = "nop"; break;
            
        case OPCODE_PUSH_LOCAL:    mnemonic = "push"; base = "local";    goto push;
        case OPCODE_PUSH_INST_VAR: mnemonic = "push"; base = "receiver"; goto push;
        case OPCODE_PUSH_LITERAL:  mnemonic = "push"; base = "literal";  goto push;
 push:
            DECODE_UINT(index);
            break;
        case OPCODE_PUSH:
            mnemonic = "push";
            DECODE_UINT(activation);
            DECODE_UINT(index);
            break;
            
        case OPCODE_PUSH_SELF:     mnemonic = "push"; keyword = "self";        break;
        case OPCODE_PUSH_SUPER:    mnemonic = "push"; keyword = "super";       break;
        case OPCODE_PUSH_FALSE:    mnemonic = "push"; keyword = "false";       break;
        case OPCODE_PUSH_TRUE:     mnemonic = "push"; keyword = "true";        break;
        case OPCODE_PUSH_NULL:     mnemonic = "push"; keyword = "null";        break;
        case OPCODE_PUSH_VOID:     mnemonic = "push"; keyword = "void";        break;
        case OPCODE_PUSH_CONTEXT:  mnemonic = "push"; keyword = "thisContext"; break;
            
        case OPCODE_DUP_N:
            DECODE_UINT(count);
            pCount = &count;
        case OPCODE_DUP:
            mnemonic = "dup";
            break;

        case OPCODE_PUSH_INT:
            mnemonic = "push";
            DECODE_SINT(intValue);
            pIntValue = &intValue;
            break;
            
        case OPCODE_STORE_LOCAL:    mnemonic = "store"; base = "local";    goto store;
        case OPCODE_STORE_INST_VAR: mnemonic = "store"; base = "receiver"; goto store;
 store:
            DECODE_UINT(index);
            break;
        case OPCODE_STORE:
            mnemonic = "store";
            DECODE_UINT(activation);
            DECODE_UINT(index);
            break;
            
        case OPCODE_POP: mnemonic = "pop"; break;
            
        case OPCODE_ROT:
            mnemonic = "rot";
            DECODE_UINT(count);
            pCount = &count;
            break;

        case OPCODE_BRANCH_IF_FALSE: mnemonic = "brf"; goto branch;
        case OPCODE_BRANCH_IF_TRUE:  mnemonic = "brt"; goto branch;
        case OPCODE_BRANCH_ALWAYS:   mnemonic = "bra"; goto branch;
 branch:
            DECODE_SINT(displacement);
            label = (ip + displacement) - begin;
            pLabel = &label;
            break;
            
        case OPCODE_ID:         mnemonic = "id";    break;
            
        case OPCODE_OPER:       mnemonic = "oper";  goto oper;
        case OPCODE_OPER_SUPER: mnemonic = "soper"; goto oper;
 oper:
            operator = (unsigned int)(*instructionPointer++);
            selector = Spk_operSelectors[operator].messageSelectorStr;
            break;
            
        case OPCODE_CALL:           mnemonic = "call";   goto call;
        case OPCODE_CALL_VAR:       mnemonic = "callv";  goto call;
        case OPCODE_CALL_SUPER:     mnemonic = "scall";  goto call;
        case OPCODE_CALL_SUPER_VAR: mnemonic = "scallv"; goto call;
 call:
            namespace = (unsigned int)(*instructionPointer++);
            operator = (unsigned int)(*instructionPointer++);
            selector = Spk_operCallSelectors[operator].messageSelectorStr;
            DECODE_UINT(argumentCount);
            pArgumentCount = &argumentCount;
            break;

        case OPCODE_GET_ATTR:       mnemonic = "gattr";  goto attr;
        case OPCODE_GET_ATTR_SUPER: mnemonic = "gsattr"; goto attr;
        case OPCODE_SET_ATTR:       mnemonic = "sattr";  goto attr;
        case OPCODE_SET_ATTR_SUPER: mnemonic = "ssattr"; goto attr;
 attr:
            base = "literal";
            DECODE_UINT(index);
            break;
            
        case OPCODE_GET_ATTR_VAR:        mnemonic = "gattrv";   break;
        case OPCODE_GET_ATTR_VAR_SUPER:  mnemonic = "gsattrv";  break;
        case OPCODE_SET_ATTR_VAR:        mnemonic = "sattrv";   break;
        case OPCODE_SET_ATTR_VAR_SUPER:  mnemonic = "ssattrv";  break;

        case OPCODE_RET:       mnemonic = "ret";   break;
        case OPCODE_RET_LEAF:  mnemonic = "retl";  break;
        case OPCODE_RET_TRAMP: mnemonic = "rett";  break;
            
        case OPCODE_LEAF:
            mnemonic = "leaf";
            DECODE_UINT(argumentCount);
            break;
            
        case OPCODE_SAVE:     mnemonic = "save";  goto save;
        case OPCODE_SAVE_VAR: mnemonic = "savev"; goto save;
 save:
            DECODE_UINT(displaySize);
            DECODE_UINT(argumentCount);
            DECODE_UINT(localCount);
            DECODE_UINT(stackSize);
            break;
            
        case OPCODE_RESTORE_SENDER: mnemonic = "restore"; keyword = "sender"; break;
        case OPCODE_RESTORE_CALLER: mnemonic = "restore"; keyword = "caller"; break;
        case OPCODE_THUNK:          mnemonic = "thunk"; break;
        case OPCODE_CALL_THUNK:     mnemonic = "ct";    break;
            
        case OPCODE_CHECK_STACKP:
            mnemonic = "check";
            base = "stackp";
            DECODE_UINT(index);
            break;
        }
        
        tab(indent, out);
        fprintf(out, "%04x", ip - begin);
        for (i = 0; i < 3; ++i, ++ip) {
            if (ip < instructionPointer) {
                fprintf(out, " %02x", *ip);
            } else {
                fprintf(out, "   ");
            }
        }
        
        fprintf(out, "\t%s", mnemonic);
        if (pIntValue) {
            fprintf(out, "\t%ld", *pIntValue);
        } else if (pCount) {
            fprintf(out, "\t%lu", *pCount);
        } else if (keyword) {
            fprintf(out, "\t%s", keyword);
        } else if (pArgumentCount) {
            fprintf(out, "\t[%u].%s %u", namespace, selector, *pArgumentCount);
        } else if (selector) {
            fprintf(out, "\t$%s", selector);
        } else if (pLabel) {
            fprintf(out, "\t%04x", *pLabel);
        } else if (base) {
            fprintf(out, "\t%s[%u]", base, index);
        } else {
            switch (opcode) {
            case OPCODE_PUSH:
            case OPCODE_STORE:
                fprintf(out, "\t%u[%u]", activation, index);
                break;
            case OPCODE_LEAF:
                fprintf(out, "\t%lu", argumentCount);
                break;
            case OPCODE_SAVE:
            case OPCODE_SAVE_VAR:
                fprintf(out, "\t%lu,%lu,%lu,%lu", displaySize, argumentCount, localCount, stackSize);
                break;
            }
        }
        fprintf(out, "\n");
    }
}

void SpkDisassembler_disassembleMethod(Method *method, unsigned int indent, FILE *out) {
    Behavior *nestedClass;
    Method *nestedMethod;
    
    for (nestedClass = method->nestedClassList.first; nestedClass; nestedClass = nestedClass->nextInScope) {
        SpkDisassembler_disassembleClass(nestedClass, indent + 1, out);
        fprintf(out, "\n");
    }
    for (nestedMethod = method->nestedMethodList.first; nestedMethod; nestedMethod = nestedMethod->nextInScope) {
        tab(indent + 1, out);
        fprintf(out, "method\n");
        SpkDisassembler_disassembleMethod(nestedMethod, indent + 1, out);
        fprintf(out, "\n");
    }
    SpkDisassembler_disassembleMethodOpcodes(method, indent + 1, out);
}

static void disassembleMethodDict(IdentityDictionary *methodDict,
                                  MethodNamespace namespace,
                                  unsigned int indent,
                                  FILE *out)
{
    Method *method;
    Symbol *methodName;
    size_t i;
    
    for (i = 0; i < methodDict->size; ++i) {
        method = (Method *)methodDict->valueArray[i];
        if (method) {
            methodName = (Symbol *)methodDict->keyArray[i];
            tab(indent, out);
            fprintf(out, "%d.%s\n", namespace, methodName->str);
            SpkDisassembler_disassembleMethod(method, indent, out);
            fprintf(out, "\n");
        }
    }
}

void SpkDisassembler_disassembleClass(Behavior *aClass, unsigned int indent, FILE *out) {
    Behavior *nestedClass;
    MethodNamespace namespace;
    IdentityDictionary *methodDict;
    
    tab(indent, out);
    fprintf(out, "class %s\n", SpkBehavior_name(aClass));
    for (nestedClass = aClass->nestedClassList.first; nestedClass; nestedClass = nestedClass->nextInScope) {
        SpkDisassembler_disassembleClass(nestedClass, indent + 1, out);
        fprintf(out, "\n");
    }
    for (namespace = 0; namespace < NUM_METHOD_NAMESPACES; ++namespace) {
        disassembleMethodDict(aClass->ns[namespace].methodDict, namespace, indent + 1, out);
    }
    return;
}

void SpkDisassembler_disassembleModule(Module *module, FILE *out) {
    Behavior *nestedClass;
    MethodNamespace namespace;
    
    for (nestedClass = module->base.klass->nestedClassList.first; nestedClass; nestedClass = nestedClass->nextInScope) {
        SpkDisassembler_disassembleClass(nestedClass, 0, out);
        fprintf(out, "\n");
    }
    for (namespace = 0; namespace < NUM_METHOD_NAMESPACES; ++namespace) {
        disassembleMethodDict(module->base.klass->ns[namespace].methodDict, namespace, 0, out);
    }
}
