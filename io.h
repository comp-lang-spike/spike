
#ifndef __spk_io_h__
#define __spk_io_h__


#include "obj.h"
#include <stdio.h>


typedef struct SpkFileStream SpkFileStream;


extern struct SpkClassTmpl Spk_ClassFileStreamTmpl;


int SpkIO_Boot(void);

FILE *SpkFileStream_AsCFileStream(SpkFileStream *);


#endif /* __spk_io_h__ */
