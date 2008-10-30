
#include "module.h"

#include "behavior.h"
#include <stdio.h>
#include <stdlib.h>


Behavior *ClassModule;
Module *builtInModule;


/*------------------------------------------------------------------------*/
/* methods */

static Object *Module_print(Object *self, Object *arg0, Object *arg1) {
    printf("<Module object at %p>", self);
    return Spk_void;
}


/*------------------------------------------------------------------------*/
/* class template */

static SpkMethodTmpl methods[] = {
    { "print", SpkNativeCode_ARGS_0 | SpkNativeCode_CALLABLE, &Module_print },
    { 0, 0, 0}
};

SpkClassTmpl ClassModuleTmpl = {
    offsetof(ModuleSubclass, variables),
    sizeof(Module),
    0,
    methods
};


/*------------------------------------------------------------------------*/
/* C API */

Module *SpkModule_new(unsigned int nGlobals) {
    Module *newModule;
    
    newModule = (Module *)malloc(ClassModule->instVarOffset + nGlobals*sizeof(Object *));
    newModule->base.klass = ClassModule;
    return newModule;
}
