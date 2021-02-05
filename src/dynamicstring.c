#include "dynamicstring.h"

int dyn_string(dynamic_string *s,char c){
    if (s->size == 0){
        s->str = malloc(8);
        if (s->str == NULL){
            fprintf(stderr, "Chyba pri alokovani dynamic_stringu\n");
            return INTERN_ERROR;
        }
        s->size = 8;
        s->str[0]='\0';
    }

    int len = strlen(s->str);

    if (s->size > len+1){
        s->str[len] = c;
        s->str[len+1]='\0';
    }
    else{
        s->str = realloc(s->str, 8 + s->size);
        if (s->str == NULL){
            fprintf(stderr, "Nastala chyba pri reallocovani stringu\n");
            return INTERN_ERROR;
        }
        s->str[len] = c;
        s->str[len+1]='\0';
        s->size += 8;
    }
    return 0;
}

void free_dyn_string(dynamic_string *s){
    if(s->str != NULL){
        free(s->str);
    }
    s->size = 0;
}
