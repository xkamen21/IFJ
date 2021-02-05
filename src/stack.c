#include "stack.h"

int STACK_SIZE = MAX_STACK;


void copy_token2(Token *to, Token from){
    to->type = from.type;
    to->s_top = from.s_top;
    to->e_type = from.e_type;

	if (to->string.size != 0){
		to->string.str[0] = '\0';
		free_dyn_string(&(to->string));
		to->string.size = 0;
		//fprintf(stderr, "! ten co bol copy ale nebol prazdny %s\n", to->string.str);
	}

	//fprintf(stderr, "! FROM ======  %s\n", from.string.str);
	//fprintf(stderr, "! TO ======  %s\n", to->string.str);
    int i = 0;
    //stringy musia mat rovnaku dlzku!!!
    while (from.string.str[i]!= '\0'){
        dyn_string(&(to->string), from.string.str[i]);
        i++;
    }
	//fprintf(stderr, "! TO ======  %s\n", to->string.str);

    to->string.size = from.string.size;

}

void stackInit ( tStack* s ) {
/*
** Provede inicializaci zásobníku - nastaví vrchol zásobníku.
** Hodnoty ve statickém poli neupravujte - po inicializaci zásobníku
** jsou nedefinované.
*/
	if (s==NULL){
		return;
	}
 	s->top = -1;	//incializujem zasobnik - top = -1 lebo je prazdny
}

int stackEmpty ( const tStack* s ) {
/*  ----------
** Vrací nenulovou hodnotu, pokud je zásobník prázdný, jinak vrací hodnotu 0.
** Funkci implementujte jako jediný příkaz. Vyvarujte se zejména konstrukce
** typu "if ( true ) b=true else b=false".
*/
		return (s->top == -1); // s->top==-1 ked je zasobnik prazdny
}

int stackFull ( const tStack* s ) {
/*  ---------
** Vrací nenulovou hodnotu, je-li zásobník plný, jinak vrací hodnotu 0.
** Dejte si zde pozor na častou programátorskou chybu "o jedničku"
** a dobře se zamyslete, kdy je zásobník plný, jestliže může obsahovat
** nejvýše STACK_SIZE prkvů a první prvek je vložen na pozici 0.
*/
	return (s->top == STACK_SIZE-1); //stack size - 1 lebo indexovanie je od 0 - STACK_SIZE-1
}

int stackTop ( const tStack* s) {
/*
** Vrací znak z vrcholu zásobníku.
** Tato operace ale prvek z vrcholu zásobníku neodstraňuje.
*/
	if (stackEmpty(s)!=0){
		return -1;
	}

	int top = s->arr[s->top];	//ulozim do c = prvok ktory je na vrchu zasobniku
	return top;
}


int stackPop ( tStack* s ) {
/*
** Odstraní prvek z vrcholu zásobníku. Pro ověření, zda je zásobník prázdný,
** použijte dříve definovanou funkci stackEmpty.
*/

		int pop = s->arr[s->top];
		s->top = s->top - 1;
		return pop;
}


void stackPush ( tStack* s, int x ) {
/*   ---------
** Vloží znak na vrchol zásobníku. Pokus o vložení prvku do plného zásobníku
** je nekorektní a ošetřete ho voláním procedury stackError(SERR_PUSH).
*/
	if (stackFull(s)!=0){
			return;
	}
	s->top += 1;
	s->arr[s->top]= x;
}

//--------- ZASOBNIK NA TOKENY ----------


int stackTokInit ( tStackTok* s ) {
/*
** Provede inicializaci zásobníku - nastaví vrchol zásobníku.
** Hodnoty ve statickém poli neupravujte - po inicializaci zásobníku
** jsou nedefinované.
*/

	for(int i = 0; i<200; i++){
		s->a[i]= NULL;
	}
 	s->top = -1;	//incializujem zasobnik - top = -1 lebo je prazdny


	return 0;
}

int stackTokEmpty ( const tStackTok* s ) {
/*  ----------
** Vrací nenulovou hodnotu, pokud je zásobník prázdný, jinak vrací hodnotu 0.
** Funkci implementujte jako jediný příkaz. Vyvarujte se zejména konstrukce
** typu "if ( true ) b=true else b=false".
*/
		return (s->top == -1); // s->top==-1 ked je zasobnik prazdny
}

int stackTokFull ( const tStackTok* s ) {
/*  ---------
** Vrací nenulovou hodnotu, je-li zásobník plný, jinak vrací hodnotu 0.
** Dejte si zde pozor na častou programátorskou chybu "o jedničku"
** a dobře se zamyslete, kdy je zásobník plný, jestliže může obsahovat
** nejvýše STACK_SIZE prkvů a první prvek je vložen na pozici 0.
*/
	return (s->top == STACK_SIZE-1); //stack size - 1 lebo indexovanie je od 0 - STACK_SIZE-1
}

void stackTokTop ( const tStackTok* s, Token *ret) {
/*
** Vrací znak z vrcholu zásobníku.
** Tato operace ale prvek z vrcholu zásobníku neodstraňuje.
*/

	//if (stackTokEmpty(s)!=0){
	//	fprintf(stderr, "Zasobnikje prazdny\n" );

//	}
	//else{

//	fprintf(stderr, ">>>>> %s <<<<<<< \n", s->a[s->top].string.str);


	//ret = malloc(sizeof(Token));
	//if (ret==NULL){
	//	exit(INTERN_ERROR);
	//}
	ret->type = s->a[s->top]->type;
	ret->s_top = s->a[s->top]->s_top;
	ret->e_type = s->a[s->top]->e_type;

	//ret->string.size = 0;

	int i = 0;


	if (ret->string.size != 0){
		ret->string.str[0] = '\0';
		free_dyn_string(&(ret->string));
		ret->string.size = 0;
		//fprintf(stderr, "! UVOLNENIE RETU V TOPE %s\n", ret->string.str);
	}


	while (s->a[s->top]->string.str[i]!= '\0'){
		dyn_string(&(ret->string), s->a[s->top]->string.str[i]);
		i++;
	}

	//printf(">>> TOP >>>>> %s\n", ret->string.str);


//	ret->type = s->a[s->top]->type;



	return;
		///return_token = s->a[s->top];
		//return return_token;
	//}
}

void stackTokPop ( tStackTok* s ) {
/*
** Odstraní prvek z vrcholu zásobníku.
*/

	if (stackTokEmpty(s)==0){

		//while (s->a[s->top].string.size != 0){
		//	s->a[s->top].string.str[s->a[s->top].string.size] = '\0';
		//	s->a[s->top].string.size --;
		//}
		//fprintf(stderr, " POPUJEM --->  %s\n",s->a[s->top]->string.str );
		s->top = s->top - 1;
	}
	else{
		fprintf(stderr, "Pop nad prazdnym tokenovym zasobnikem!\n" );
	}
	//	return pop;
}
/*
void stackTokPush ( tStackTok* s, const char *string, type_token type, int s_top){
*/
/*   ---------
** Vloží znak na vrchol zásobníku. Pokus o vložení prvku do plného zásobníku
** je nekorektní a ošetřete ho voláním procedury stackError(SERR_PUSH).
*/
	//if (stackTokFull(s)!=0){
		//fprintf(stderr, "Tokenovy zasobnik je plny!\n" );
			//return;
	//}
/*
	s->top += 1;

	Token on_stack;



		on_stack.string = malloc(sizeof(strlen(string))+1);
		on_stack.string[0] = '\0';
		strcpy(on_stack.string,string);


	on_stack.s_top = s_top;
	on_stack.type = type;

	s->a[s->top]= on_stack;
}
*/
void stackTokPush ( tStackTok* s, Token x ) {
/*   ---------
** Vloží znak na vrchol zásobníku. Pokus o vložení prvku do plného zásobníku
** je nekorektní a ošetřete ho voláním procedury stackError(SERR_PUSH).
*/

	if (stackTokFull(s)){
		fprintf(stderr, "Tokenovy zasobnik je plny!\n" );
			return;
	}

	Token *new;
	new = malloc(sizeof(Token));
	new->string.size = 0;


	copy_token2(new, x);

	s->top += 1;
	s->a[s->top]= new;
	//printf(">>> PUSH >>>>> %s\n", new->string.str);


}

void insert_stop(tStackTok *s){
//printf("Jdu pushovat\n" );
	Token on_top;
	on_top.string.size = 0;
	stackTokTop(s, &on_top);
	Token stop_token;
	stop_token.string.size = 0;

	char str[2] = "$";

	int i = 0;
	while (str[i]!= '\0'){
		dyn_string(&(stop_token.string), str[i]);
		i++;
	}

	stop_token.type = STOP;
	stop_token.s_top = 0;

	//Pokud je na vrcholu E_TOK
	if (on_top.type == E_TOK) {
 //printf("Push s E_TOK\n" );
		stackTokPop(s);	// Oddelam vrchni neterminal
		stackTokPush(s,stop_token);	//nahraju stop token
		//printf(">>> STOPPUSH -- STOPTOKEN!! >>>>> %s\n", stop_token.string.str);
		stackTokPush(s,on_top);	// Vratime E_tok
		//printf(">>> STOPPUSH -- STOPTOKEN!! >>>>> %s\n", stop_token.string.str);

	}
	//Vkladam rovnou STOP token
	else{
//printf("Push bez E_TOK\n" );
		stackTokPush(s,stop_token);	// Vratime E_tok
	}

	if (stackTokFull(s)!=0){
		fprintf(stderr, "Tokenovy zasobnik je plny po nahrani posleniho tokenu!\n" );
			exit(1);
	}

}
