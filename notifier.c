
#include "notifier.h"

#include "class.h"
#include "heart.h"
#include "host.h"
#include "native.h"
#include "rodata.h"
#include "st.h"
#include "tree.h"

#include <stdio.h>
#include <stdlib.h>


struct Notifier {
    Object base;
    Unknown *stream;
    FILE *cStream;
    unsigned int errorTally;
    Unknown *source;
};


/*------------------------------------------------------------------------*/
/* methods */

static Unknown *Notifier_init(Unknown *_self, Unknown *stream, Unknown *arg1) {
    Notifier *self;
    
    self = (Notifier *)_self;
    if (!Host_IsFileStream(stream)) {
        Halt(HALT_TYPE_ERROR, "a stream is required");
        return 0;
    }
    
    self->stream = stream; INCREF(stream);
    self->cStream = Host_FileStreamAsCFileStream(stream);
    self->errorTally = 0;
    
    INCREF(_self);
    return _self;
}

static Unknown *Notifier_badExpr(Unknown *_self, Unknown *arg0, Unknown *arg1) {
    Notifier *self;
    Expr *expr;
    const char *source, *desc;
    
    self = (Notifier *)_self;
    expr = CAST(XExpr, arg0);
    if (!expr) {
        Halt(HALT_TYPE_ERROR, "an expression node is required");
        return 0;
    }
    if (!Host_IsString(arg1)) {
        Halt(HALT_TYPE_ERROR, "a string is required");
        return 0;
    }
    desc = Host_StringAsCString(arg1);
    
    source = self->source
             ? Host_StringAsCString(self->source)
             : "<unknown>";
    fprintf(stderr, "%s:%u: %s\n",
            source, expr->lineNo, desc);
    ++self->errorTally;
    
    INCREF(GLOBAL(xvoid));
    return GLOBAL(xvoid);
}

static Unknown *Notifier_redefinedSymbol(Unknown *_self, Unknown *arg0, Unknown *arg1) {
    Notifier *self;
    Expr *expr;
    SymbolNode *sym;
    const char *source, *name, *format;
    
    self = (Notifier *)_self;
    expr = CAST(XExpr, arg0);
    if (!expr) {
        Halt(HALT_TYPE_ERROR, "an expression node is required");
        return 0;
    }
    
    source = self->source
             ? Host_StringAsCString(self->source)
             : "<unknown>";
    sym = expr->sym;
    name = Host_SymbolAsCString(sym->sym);
    format = (sym->entry && sym->entry->scope->context->level == 0)
             ? "%s:%u: cannot redefine built-in name '%s'\n"
             : "%s:%u: symbol '%s' multiply defined\n";
    fprintf(stderr, format,
            source, expr->lineNo,
            name);
    ++self->errorTally;
    
    INCREF(GLOBAL(xvoid));
    return GLOBAL(xvoid);
}

static Unknown *Notifier_undefinedSymbol(Unknown *_self, Unknown *arg0, Unknown *arg1) {
    Notifier *self;
    Expr *expr;
    const char *source;
    
    self = (Notifier *)_self;
    expr = CAST(XExpr, arg0);
    if (!expr) {
        Halt(HALT_TYPE_ERROR, "an expression node is required");
        return 0;
    }
    
    source = self->source
                     ? Host_StringAsCString(self->source)
                     : "<unknown>";
    fprintf(self->cStream, "%s:%u: symbol '%s' undefined\n",
            source, expr->lineNo,
            Host_SymbolAsCString(expr->sym->sym));
    ++self->errorTally;
    
    INCREF(GLOBAL(xvoid));
    return GLOBAL(xvoid);
}

static Unknown *Notifier_failOnError(Unknown *_self, Unknown *arg0, Unknown *arg1) {
    Notifier *self;
    
    self = (Notifier *)_self;
    if (self->errorTally > 0) {
        exit(1);
        return 0;
    }
    INCREF(GLOBAL(xvoid));
    return GLOBAL(xvoid);
}


/*------------------------------------------------------------------------*/
/* meta-methods */

static Unknown *ClassNotifier_new(Unknown *self, Unknown *arg0, Unknown *arg1) {
    Unknown *newNotifier, *tmp;
    
    newNotifier = Send(GLOBAL(theInterpreter), SUPER, new, 0);
    if (!newNotifier)
        return 0;
    tmp = newNotifier;
    newNotifier = Send(GLOBAL(theInterpreter), newNotifier, init, arg0, 0);
    DECREF(tmp);
    return newNotifier;
}


/*------------------------------------------------------------------------*/
/* low-level hooks */

static void Notifier_zero(Object *_self) {
    Notifier *self = (Notifier *)_self;
    (*CLASS(XNotifier)->superclass->zero)(_self);
    self->stream = 0;
    self->cStream = 0;
    self->source = 0;
}

static void Notifier_dealloc(Object *_self, Unknown **l) {
    Notifier *self = (Notifier *)_self;
    LDECREF(self->stream, l);
    XLDECREF(self->source, l);
    (*CLASS(XNotifier)->superclass->dealloc)(_self, l);
}


/*------------------------------------------------------------------------*/
/* class tmpl */

typedef struct NotifierSubclass {
    Notifier base;
    Unknown *variables[1]; /* stretchy */
} NotifierSubclass;

static AccessorTmpl accessors[] = {
    { "source", T_OBJECT, offsetof(Notifier, source),
      Accessor_READ | Accessor_WRITE },
    { 0 }
};

static MethodTmpl methods[] = {
    { "init", NativeCode_ARGS_1, &Notifier_init },
    { "badExpr:", NativeCode_ARGS_2, &Notifier_badExpr },
    { "redefinedSymbol:", NativeCode_ARGS_1, &Notifier_redefinedSymbol },
    { "undefinedSymbol:", NativeCode_ARGS_1, &Notifier_undefinedSymbol },
    { "failOnError", NativeCode_ARGS_0, &Notifier_failOnError },
    { 0 }
};

static MethodTmpl metaMethods[] = {
    { "new", NativeCode_ARGS_1, &ClassNotifier_new },
    { 0 }
};

ClassTmpl ClassXNotifierTmpl = {
    HEART_CLASS_TMPL(XNotifier, Object), {
        accessors,
        methods,
        /*lvalueMethods*/ 0,
        offsetof(NotifierSubclass, variables),
        /*itemSize*/ 0,
        &Notifier_zero,
        &Notifier_dealloc
    }, /*meta*/ {
        /*accessors*/ 0,
        metaMethods,
        /*lvalueMethods*/ 0
    }
};
