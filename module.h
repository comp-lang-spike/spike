
#ifndef __spk_module_h__
#define __spk_module_h__


#include "obj.h"


typedef struct SpkModule {
    SpkObject base;
    size_t literalCount;
    SpkUnknown **literals;
    struct SpkBehavior *firstClass;
} SpkModule;


#define SpkModule_VARIABLES(op) ((SpkUnknown **)((char *)(op) + ((SpkObject *)(op))->klass->instVarOffset) + Spk_CLASS(Module)->instVarBaseIndex)
#define SpkModule_LITERALS(op) (((SpkModule *)(op))->literals)


extern struct SpkClassTmpl Spk_ClassModuleTmpl;


SpkModule *SpkModule_New(struct SpkBehavior *);


#endif /* __spk_module_h__ */
