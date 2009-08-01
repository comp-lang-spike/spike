
#include "obj.h"

#include "bool.h"
#ifdef MALTIPY
#include "bridge.h"
#endif
#include "class.h"
#include "host.h"
#include "interp.h"
#include "native.h"
#include <stdlib.h>
#include <string.h>


SpkBehavior *Spk_ClassObject, *Spk_ClassVariableObject;


/*------------------------------------------------------------------------*/
/* methods */

static SpkUnknown *Object_eq(SpkUnknown *self, SpkUnknown *arg0, SpkUnknown *arg1) {
    SpkUnknown *result;

    result = (self == arg0 ? Spk_true : Spk_false);
    Spk_INCREF(result);
    return result;
}

static SpkUnknown *Object_ne(SpkUnknown *self, SpkUnknown *arg0, SpkUnknown *arg1) {
    SpkUnknown *temp, *result;
    
    temp = Spk_Oper(theInterpreter, self, Spk_OPER_EQ, arg0, 0);
    result = Spk_Oper(theInterpreter, temp, Spk_OPER_LNEG, 0);
    Spk_DECREF(temp);
    return result;
}


/*------------------------------------------------------------------------*/
/* meta-methods */

static SpkUnknown *ClassObject_new(SpkUnknown *_self, SpkUnknown *arg0, SpkUnknown *arg1) {
    /* Answer a new instance of the receiver. */
    SpkClass *self;
    SpkUnknown *args;
    
    self = (SpkClass *)_self;
    args = arg0;
    if (!Spk_IsArgs(args)) {
        Spk_Halt(Spk_HALT_TYPE_ERROR, "an argument list is required");
        return 0;
    }
    
    if (Spk_ArgsSize(args) != 0) {
        Spk_HaltWithFormat(Spk_HALT_TYPE_ERROR,
                           "method '%s::new' takes 0 arguments (%d given)",
                           SpkBehavior_Name((SpkBehavior *)self), Spk_ArgsSize(args));
        return 0;
    }
    
    return (SpkUnknown *)SpkObject_New((SpkBehavior *)self);
}

static SpkUnknown *ClassVariableObject_new(SpkUnknown *_self, SpkUnknown *arg0, SpkUnknown *arg1) {
    /* Answer a new instance of the receiver. */
    SpkClass *self;
    SpkUnknown *args;
    SpkUnknown *nItemsObj; long nItems;
    
    self = (SpkClass *)_self;
    args = arg0;
    if (!Spk_IsArgs(args)) {
        Spk_Halt(Spk_HALT_TYPE_ERROR, "an argument list is required");
        return 0;
    }
    
    if (Spk_ArgsSize(args) != 1) {
        Spk_HaltWithFormat(Spk_HALT_TYPE_ERROR,
                           "method '%s::new' takes 1 argument (%d given)",
                           SpkBehavior_Name((SpkBehavior *)self), Spk_ArgsSize(args));
        return 0;
    }
    if (self->base.itemSize == 0) {
        Spk_Halt(Spk_HALT_VALUE_ERROR, "bad item size in class object");
        return 0;
    }
    nItemsObj = Spk_GetArg(args, 0);
    if (!SpkHost_IsInteger(nItemsObj)) {
        Spk_DECREF(nItemsObj);
        Spk_Halt(Spk_HALT_TYPE_ERROR, "an integer object is required");
        return 0;
    }
    nItems = SpkHost_IntegerAsCLong(nItemsObj);
    Spk_DECREF(nItemsObj);
    if (nItems < 0) {
        Spk_Halt(Spk_HALT_VALUE_ERROR, "number of items cannot be negative");
        return 0;
    }
    
    return (SpkUnknown *)SpkObject_NewVar((SpkBehavior *)self, nItems);
}


/*------------------------------------------------------------------------*/
/* low-level hooks */

static void Object_zero(SpkObject *self) {
    SpkUnknown **variables;
    SpkBehavior *klass;
    size_t instVarTotal, i;
    
    klass = self->klass;
    
    variables = (SpkUnknown **)((char *)self + klass->instVarOffset);
    instVarTotal = klass->instVarBaseIndex + klass->instVarCount;
    
    for (i = 0; i < instVarTotal; ++i) {
        Spk_INCREF(Spk_uninit);
        variables[i] = Spk_uninit;
    }
}

static void Object_dealloc(SpkObject *self) {
    SpkUnknown **variables;
    SpkBehavior *klass;
    size_t instVarTotal, i;
    
    klass = self->klass;
    
    variables = (SpkUnknown **)((char *)self + klass->instVarOffset);
    instVarTotal = klass->instVarBaseIndex + klass->instVarCount;
    
    for (i = 0; i < instVarTotal; ++i) {
        Spk_DECREF(variables[i]);
        variables[i] = 0;
    }
}

static void VariableObject_zero(SpkObject *_self) {
    SpkVariableObject *self = (SpkVariableObject *)_self;
    (*Spk_ClassVariableObject->superclass->zero)(_self);
    memset(SpkVariableObject_ITEM_BASE(self), 0,
           self->size * self->base.klass->itemSize);
}


/*------------------------------------------------------------------------*/
/* memory layout of instances */

static void Object_traverse_init(SpkObject *self) {
#ifndef MALTIPY
    self->base.refCount = 0;
#endif /* !MALTIPY */
}

static SpkUnknown **Object_traverse_current(SpkObject *self) {
#ifdef MALTIPY
    return 0;
#else
    if (self->base.refCount >= self->klass->instVarCount)
        return 0;
    return (SpkUnknown **)(self->klass->instVarOffset + self->base.refCount * sizeof(SpkObject *));
#endif /* !MALTIPY */
}

static void Object_traverse_next(SpkObject *self) {
#ifndef MALTIPY
    ++self->base.refCount;
#endif /* !MALTIPY */
}


/*------------------------------------------------------------------------*/
/* class templates */

static SpkAccessorTmpl ObjectAccessors[] = {
    { "class", Spk_T_OBJECT, offsetof(SpkObject, klass), SpkAccessor_READ },
    { 0, 0, 0, 0 }
};

static SpkMethodTmpl ObjectMethods[] = {
    { "__eq__", SpkNativeCode_BINARY_OPER | SpkNativeCode_LEAF, &Object_eq },
    { "__ne__", SpkNativeCode_BINARY_OPER, &Object_ne },
    { 0, 0, 0 }
};

static SpkMethodTmpl ClassObjectMethods[] = {
    { "new", SpkNativeCode_METH_ATTR | SpkNativeCode_ARGS_ARRAY, &ClassObject_new },
    { 0, 0, 0}
};

static SpkTraverse ObjectTraverse = {
    &Object_traverse_init,
    &Object_traverse_current,
    &Object_traverse_next,
};

SpkClassTmpl Spk_ClassObjectTmpl = {
    "Object", {
        ObjectAccessors,
        ObjectMethods,
        /*lvalueMethods*/ 0,
        offsetof(SpkObjectSubclass, variables),
        /*itemSize*/ 0,
        &Object_zero,
        &Object_dealloc,
        &ObjectTraverse
    }, /*meta*/ {
        /*accessors*/ 0,
        ClassObjectMethods,
        /*lvalueMethods*/ 0
    }
};


static SpkMethodTmpl VariableObjectMethods[] = {
    { 0, 0, 0 }
};

static SpkMethodTmpl ClassVariableObjectMethods[] = {
    { "new", SpkNativeCode_METH_ATTR | SpkNativeCode_ARGS_ARRAY, &ClassVariableObject_new },
    { 0, 0, 0}
};

SpkClassTmpl Spk_ClassVariableObjectTmpl = {
    "VariableObject", {
        /*accessors*/ 0,
        VariableObjectMethods,
        /*lvalueMethods*/ 0,
        offsetof(SpkVariableObjectSubclass, variables),
        /*itemSize*/ 0,
        &VariableObject_zero
    }, /*meta*/ {
        /*accessors*/ 0,
        ClassVariableObjectMethods,
        /*lvalueMethods*/ 0
    }
};


/*------------------------------------------------------------------------*/
/* casting */

SpkObject *Spk_Cast(SpkBehavior *target, SpkUnknown *unk) {
    SpkObject *op;
    SpkBehavior *c;
    
#ifdef MALTIPY
    if (!PyObject_TypeCheck(unk, &SpkSpikeObject_Type)) {
        return 0;
    }
#endif /* MALTIPY */
    op = (SpkObject *)unk;
    for (c = op->klass; c; c = c->superclass) {
        if (c == target) {
            return (SpkObject *)op;
        }
    }
    return 0;
}


/*------------------------------------------------------------------------*/
/* object memory */

SpkObject *SpkObject_New(SpkBehavior *klass) {
    SpkObject *newObject;
    
    newObject = (SpkObject *)SpkObjMem_Alloc(klass->instanceSize);
    if (!newObject) {
        return 0;
    }
    newObject->klass = klass;  Spk_INCREF(klass);
    (*klass->zero)(newObject);
    return newObject;
}

SpkObject *SpkObject_NewVar(SpkBehavior *klass, size_t size) {
    SpkObject *newObject;
    
    newObject = (SpkObject *)SpkObjMem_Alloc(klass->instanceSize + size * klass->itemSize);
    if (!newObject) {
        return 0;
    }
    newObject->klass = klass;  Spk_INCREF(klass);
    ((SpkVariableObject *)newObject)->size = size;
    (*klass->zero)(newObject);
    return newObject;
}
