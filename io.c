
#include "io.h"

#include "array.h"
#include "bool.h"
#include "char.h"
#include "class.h"
#include "float.h"
#include "int.h"
#include "native.h"
#include "str.h"

#include <stdlib.h>
#include <string.h>


SpkBehavior *Spk_ClassFileStream;
SpkFileStream *Spk_stdin, *Spk_stdout, *Spk_stderr;


/*------------------------------------------------------------------------*/
/* attributes */

static SpkUnknown *FileStream_eof(SpkUnknown *_self, SpkUnknown *arg0, SpkUnknown *arg1) {
    SpkFileStream *self;
    SpkUnknown *result;
    
    self = (SpkFileStream *)_self;
    result = self->stream
             ? (feof(self->stream) ? Spk_true : Spk_false)
             : Spk_true;
    Spk_INCREF(result);
    return result;
}


/*------------------------------------------------------------------------*/
/* methods */

static SpkUnknown *FileStream_close(SpkUnknown *_self, SpkUnknown *arg0, SpkUnknown *arg1) {
    SpkFileStream *self;
    
    self = (SpkFileStream *)_self;
    if (self->stream) {
        fclose(self->stream);
        self->stream = 0;
    }
    Spk_INCREF(Spk_void);
    return Spk_void;
}

static SpkUnknown *FileStream_flush(SpkUnknown *_self, SpkUnknown *arg0, SpkUnknown *arg1) {
    SpkFileStream *self;
    
    self = (SpkFileStream *)_self;
    if (self->stream)
        fflush(self->stream);
    Spk_INCREF(Spk_void);
    return Spk_void;
}

static SpkUnknown *FileStream_getc(SpkUnknown *_self, SpkUnknown *arg0, SpkUnknown *arg1) {
    SpkFileStream *self;
    int c;
    
    self = (SpkFileStream *)_self;
    if (!self->stream) {
        Spk_INCREF(Spk_null);
        return Spk_null;
    }
    c = fgetc(self->stream);
    if (c == EOF) {
        Spk_INCREF(Spk_null);
        return Spk_null;
    }
    return (SpkUnknown *)SpkChar_FromChar((char)c);
}

static SpkUnknown *FileStream_gets(SpkUnknown *_self, SpkUnknown *arg0, SpkUnknown *arg1) {
    SpkFileStream *self;
    SpkInteger *size;
    SpkString *s;
    SpkUnknown *result;
    
    self = (SpkFileStream *)_self;
    size = Spk_CAST(Integer, arg0);
    if (!size) {
        Spk_Halt(Spk_HALT_TYPE_ERROR, "an integer is required");
        return 0;
    }
    if (!self->stream) {
        Spk_INCREF(Spk_null);
        return Spk_null;
    }
    s = SpkString_fromStream(self->stream, (size_t)SpkInteger_asLong(size));
    result = s ? (SpkUnknown *)s : Spk_null;
    Spk_INCREF(result);
    return result;
}

static SpkUnknown *FileStream_printf(SpkUnknown *_self, SpkUnknown *arg0, SpkUnknown *arg1) {
    SpkFileStream *self;
    SpkArray *args; size_t nArgs, argIndex;
    SpkUnknown *formatObj = 0; SpkString *formatString; char *format = 0; size_t formatSize;
    char c, convOp, *f, *chunk;
    SpkUnknown *arg = 0; SpkChar *charArg; SpkInteger *intArg; SpkFloat *floatArg; SpkString *strArg;
    static char *convOps = "cdeEfgGinopsuxX%";
    
    self = (SpkFileStream *)_self;
    args = Spk_CAST(Array, arg0);
    if (!args) {
        Spk_Halt(Spk_HALT_TYPE_ERROR, "an array is required");
        goto unwind;
    }
    nArgs = SpkArray_size(args);
    if (nArgs == 0) {
        Spk_Halt(Spk_HALT_TYPE_ERROR, "wrong number of arguments");
        goto unwind;
    }
    formatObj = SpkArray_item(args, 0);
    formatString = Spk_CAST(String, formatObj);
    if (!formatString) {
        Spk_Halt(Spk_HALT_TYPE_ERROR, "a string is required");
        goto unwind;
    }
    formatSize = SpkString_size(formatString);
    format = (char *)malloc(formatSize);
    memcpy(format, SpkString_asString(formatString), formatSize);
    
    argIndex = 1;
    f = chunk = format;
    c = *f;
    
    while (c) {
        while (c && c != '%')
            c = *++f;
        if (chunk < f) {
            *f = 0;
            fputs(chunk, self->stream);
            *f = c;
        }
        if (!c) break;
        
        /* found a conversion specification */
        chunk = f;
        do
            c = *++f;
        while (c && !strchr(convOps, c));
        if (!c) {
            Spk_Halt(Spk_HALT_VALUE_ERROR, "invalid conversion specification");
            goto unwind;
        }
        
        convOp = c;
        c = *++f;
        if (convOp == '%') {
            if (f - chunk != 2) {
                Spk_Halt(Spk_HALT_VALUE_ERROR, "invalid conversion specification");
                goto unwind;
            }
            fputc('%', self->stream);
            chunk = f;
            continue;
        }
        *f = 0;
        
        /* consume an argument */
        if (argIndex >= nArgs) {
            Spk_Halt(Spk_HALT_TYPE_ERROR, "too few arguments");
            goto unwind;
        }
        Spk_XDECREF(arg);
        arg = SpkArray_item(args, argIndex++);
        
        switch (convOp) {
        case 'c':
            charArg = Spk_CAST(Char, arg);
            if (!charArg) {
                Spk_Halt(Spk_HALT_TYPE_ERROR, "character expected");
                goto unwind;
            }
            fprintf(self->stream, chunk, (int)SpkChar_AsChar(charArg));
            break;
        case 'd': case 'i': case 'o': case 'u': case 'x':
            intArg = Spk_CAST(Integer, arg);
            if (!intArg) {
                Spk_Halt(Spk_HALT_TYPE_ERROR, "integer expected");
                goto unwind;
            }
            fprintf(self->stream, chunk, SpkInteger_asLong(intArg));
            break;
        case 'e': case 'E': case 'f': case 'g': case 'G':
            floatArg = Spk_CAST(Float, arg);
            if (!floatArg) {
                Spk_Halt(Spk_HALT_TYPE_ERROR, "float expected");
                goto unwind;
            }
            fprintf(self->stream, chunk, SpkFloat_asDouble(floatArg));
            break;
        case 's':
            strArg = Spk_CAST(String, arg);
            if (!strArg) {
                Spk_Halt(Spk_HALT_TYPE_ERROR, "string expected");
                goto unwind;
            }
            fprintf(self->stream, chunk, SpkString_asString(strArg));
            break;
        default:
            Spk_Halt(Spk_HALT_ASSERTION_ERROR, "conversion letter not implemented");
            goto unwind;
        }
        
        *f = c;
        chunk = f;
    }
    
    if (argIndex != nArgs) {
        Spk_Halt(Spk_HALT_TYPE_ERROR, "too many arguments");
        goto unwind;
    }
    
    free(format);
    Spk_DECREF(formatObj);
    Spk_XDECREF(arg);
    Spk_INCREF(Spk_void);
    return Spk_void;
    
 unwind:
    free(format);
    Spk_XDECREF(formatObj);
    Spk_XDECREF(arg);
    return 0;
}

static SpkUnknown *FileStream_putc(SpkUnknown *_self, SpkUnknown *arg0, SpkUnknown *arg1) {
    SpkFileStream *self;
    SpkChar *c;
    
    self = (SpkFileStream *)_self;
    c = Spk_CAST(Char, arg0);
    if (!c) {
        Spk_Halt(Spk_HALT_TYPE_ERROR, "a character is required");
        return 0;
    }
    if (self->stream)
        fputc(SpkChar_AsChar(c), self->stream);
    Spk_INCREF(Spk_void);
    return Spk_void;
}

static SpkUnknown *FileStream_puts(SpkUnknown *_self, SpkUnknown *arg0, SpkUnknown *arg1) {
    SpkFileStream *self;
    SpkString *s;
    
    self = (SpkFileStream *)_self;
    s = Spk_CAST(String, arg0);
    if (!s) {
        Spk_Halt(Spk_HALT_TYPE_ERROR, "a string is required");
        return 0;
    }
    if (self->stream)
        fputs(SpkString_asString(s), self->stream);
    Spk_INCREF(Spk_void);
    return Spk_void;
}

static SpkUnknown *FileStream_reopen(SpkUnknown *_self, SpkUnknown *arg0, SpkUnknown *arg1) {
    /* XXX: "open" should be a class method */
    SpkFileStream *self;
    SpkString *pathnameString, *modeString;
    char *pathname, *mode;
    SpkUnknown *result;
    
    self = (SpkFileStream *)_self;
    pathnameString = Spk_CAST(String, arg0);
    if (!pathnameString) {
        Spk_Halt(Spk_HALT_TYPE_ERROR, "a string is required");
        return 0;
    }
    modeString = Spk_CAST(String, arg1);
    if (!modeString) {
        Spk_Halt(Spk_HALT_TYPE_ERROR, "a string is required");
        return 0;
    }
    pathname = SpkString_asString(pathnameString);
    mode = SpkString_asString(modeString);
    if (self->stream)
        self->stream = freopen(pathname, mode, self->stream);
    else
        self->stream = fopen(pathname, mode);
    result = self->stream ? (SpkUnknown *)self : Spk_null;
    Spk_INCREF(result);
    return result;
}

static SpkUnknown *FileStream_rewind(SpkUnknown *_self, SpkUnknown *arg0, SpkUnknown *arg1) {
    SpkFileStream *self;
    
    self = (SpkFileStream *)_self;
    if (self->stream)
        rewind(self->stream);
    Spk_INCREF(Spk_void);
    return Spk_void;
}


/*------------------------------------------------------------------------*/
/* low-level hooks */

static void FileStream_zero(SpkObject *_self) {
    SpkFileStream *self = (SpkFileStream *)_self;
    (*Spk_ClassFileStream->superclass->zero)(_self);
    self->stream = 0;
}

static void FileStream_dealloc(SpkObject *_self) {
    SpkFileStream *self = (SpkFileStream *)_self;
    if (self->stream) {
        fclose(self->stream);
        self->stream = 0;
    }
    (*Spk_ClassFileStream->superclass->dealloc)(_self);
}


/*------------------------------------------------------------------------*/
/* class template */

static SpkMethodTmpl methods[] = {
    /* attributes */
    { "eof", SpkNativeCode_ARGS_0, &FileStream_eof },
    /* methods */
    { "close",  SpkNativeCode_METH_ATTR | SpkNativeCode_ARGS_0, &FileStream_close },
    { "flush",  SpkNativeCode_METH_ATTR | SpkNativeCode_ARGS_0, &FileStream_flush },
    { "getc",   SpkNativeCode_METH_ATTR | SpkNativeCode_ARGS_0, &FileStream_getc },
    { "gets",   SpkNativeCode_METH_ATTR | SpkNativeCode_ARGS_1, &FileStream_gets },
    { "open",   SpkNativeCode_METH_ATTR | SpkNativeCode_ARGS_2, &FileStream_reopen },
    { "printf", SpkNativeCode_METH_ATTR | SpkNativeCode_ARGS_ARRAY, &FileStream_printf },
    { "putc",   SpkNativeCode_METH_ATTR | SpkNativeCode_ARGS_1, &FileStream_putc },
    { "puts",   SpkNativeCode_METH_ATTR | SpkNativeCode_ARGS_1, &FileStream_puts },
    { "reopen", SpkNativeCode_METH_ATTR | SpkNativeCode_ARGS_2, &FileStream_reopen },
    { "rewind", SpkNativeCode_METH_ATTR | SpkNativeCode_ARGS_0, &FileStream_rewind },
    { 0, 0, 0}
};

SpkClassTmpl Spk_ClassFileStreamTmpl = {
    "FileStream", {
        /*accessors*/ 0,
        methods,
        /*lvalueMethods*/ 0,
        offsetof(SpkFileStreamSubclass, variables),
        0,
        &FileStream_zero,
        &FileStream_dealloc
    }, /*meta*/ {
    }
};
