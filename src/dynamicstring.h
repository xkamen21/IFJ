#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

#ifndef DEFINE_DYNSTRING_
#define DEFINE_DYNSTRING_

/***
*   Struktura reprezentujuca dynamicky string ktory sa sam realocuje
***/
typedef struct dynamic_string{
    char *str;
    int size;   //aktualna naalokovana velkost
} dynamic_string;

/***
*   Funkcia na ulozenie stringu
*   @param *s pointer na dynamicky string
*   @param c znak ktory ma byt ulozeny
*   @return 0 v pripade uspechu
*           INTERN_ERROR v pripade zlyhania malllocu
***/
int dyn_string(dynamic_string *s,char c);

void free_dyn_string(dynamic_string *s);

#endif //DEFINE_DYNSTRING_
