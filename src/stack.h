#include <stdio.h>
#include "scanner.h"



#define MAX_STACK 200

typedef struct {
	Token *a[MAX_STACK];					/* pole pro ulozeni tokenu */
	int top;										/* index prvku na vrcholu zasobniku */
} tStackTok;


 /* ADT zásobník implementovaný ve statickém poli */
typedef struct {
	int arr[MAX_STACK];                             /* pole pro uložení hodnot */
	int top;                                /* index prvku na vrcholu zásobníku */
} tStack;

void stackInit ( tStack* s );
int stackEmpty ( const tStack* s );
int stackFull ( const tStack* s );
int stackTop ( const tStack* s);
int stackPop ( tStack* s );
void stackPush ( tStack* s, int x );

int stackTokInit ( tStackTok* s );
int stackTokEmpty ( const tStackTok* s );
int stackTokFull ( const tStackTok* s );
void stackTokTop ( const tStackTok* s, Token *ret);
void stackTokPop ( tStackTok* s );
void stackTokPush ( tStackTok* s, Token x );
//void stackTokPush ( tStackTok* s, const char *string, type_token type, int s_top);
void insert_stop(tStackTok *s);
void copy_token2(Token *to, Token from);
