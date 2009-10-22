
#ifndef __QSORT_H_INCLUDED
#define __QSORT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

void quicksort(	void *base, long num, long width, const void * args,
    			long ( *comp)(const void *, const void *, const void *));


#ifdef __cplusplus
};
#endif

#endif

