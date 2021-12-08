#ifndef _OPF_H_

#define _OPF_H_

#include <winsock2.h>
#include <time.h>
#include <sys/timeb.h>


#include "subgrafico.c"
#include "common.c"
#include "set.c"
#include "realheap.c"

#include "realheap.h"
#include "subgrafico.h"
#include "common.H"
#include "set.h"




/*--------- Common definições --------- */
#define opf_MAXARCW			100000.0
#define opf_PROTOTYPE		1


typedef float (*opf_ArcWeightFun)(float *f1, float *f2, int n);

extern opf_ArcWeightFun opf_ArcWeight;

#endif