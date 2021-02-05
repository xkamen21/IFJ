/* * *
 * Projekt IFJ - časť: Generovanie kódu
 * autori: Tomáš Lisický (xlisic01), Ondrej Kondek (xkonde04)
 * tím: 116			ročník: 2019/20
 * * * * * * * * * * * * * * * * * * * * * * * */

/* * *
 *	Zoznam:
 *	1.) Utility
 *	2.) Built-In
 *	3.) Funkcie a rámce
 *	4.) Aritmetika
 *	5.) Relácia
 *	6.) Logika
 * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>

#include "dynamicstring.h" // dynamicstring.h -> error.h
//#include "error.h"


/* Posiela po znakoch inštrukcie do dynamického stringu.
 */
void codegen_send( dynamic_string *s, char* str );


/* Výpis obsahu dynamického stringu na stdout-
 */
void codegen_flush( dynamic_string *s );


/* Vypíše výraz v tvare:
 * <frame>F@_<symb> alebo <frame>@<symb> bez nového riadku;
 * tvar výrazú závysí od <frame> - či je to FRAME alebo TYPE (I, B, F, S)
 */
void codegen_symb( dynamic_string *s, char frame, char* symb );


/* Vypíše výraz v tvare <frame>F@_<var> bez nového riadku-
 */
void codegen_var ( dynamic_string *s, char frame, char* var );


/* Funkcia spracuje pridelený reťazec znakov, nahradí biele znaky a znaky '#' '\' escape sekvenciou.
 * tvar escape sekv. je '\xxx' pričom xxx je ordinálna hodnota znaku na 3 cifrách
 * funkcia vráti ukazateľ na dynamický string, KTORÝ JE POTREBNĚ UVOĽNIŤ
 */
void codegen_machistr( dynamic_string* retval, char* string );


/* Funkcia spracuje číslo (float) v desiatkovej sústave a prepíše ho ako hexadecimálne
 * funkcia vráti ukazateľ na dynamický string, KTORÝ JE POTREBNĚ UVOĽNIŤ
 */
void codegen_machifloat( dynamic_string* retval, char* string );


/* █████████████████████████████████████████████████████████████████████ Utility █████████████████████████████████████████████████████████████████████ */

/* Pošle hlavičku výsledného kódu na dyn. string.
 * ########! Musí byť vždy na začiatku kódu. !########
 */
void genc_init( dynamic_string *s );

void genc_init_main( dynamic_string *s );


/* Pošle na dyn.string komentár v tvare "# <comment> \n".
 */
void genc_comment( dynamic_string *s, char* comment );


/* Pošle nový riadok na dyn. string.
 */
void genc_newline( dynamic_string *s );


/* █████████████████████████████████████████████████████████████████████ Built-In █████████████████████████████████████████████████████████████████████ */

/* inputX - funkcia číta hodnotu zo vstupu kde X je požadovaný typ hodnoty. Potom ju prenesie na zásobník.
 * string / int / float
 */
void genc_inputs( dynamic_string *s );
void genc_inputi( dynamic_string *s );
void genc_inputf( dynamic_string *s );


/* Vstavané funkcie na cyklické vypisovanie argumentov.
 */
void genc_print( dynamic_string *s );


/* Vstavaná funkcia na spočítanie dĺžky reťazca.
 */
void genc_len( dynamic_string *s );


/* Vstavaná funkcia, ktorá vracia reŤazec na zadanej adrese a o zadanej dĺžky
 */
void genc_substr( dynamic_string *s );


/* Funkcii sú predávané parametre cez zásobník (PUSHS - POPS), ona si ich prevezme a výsledok vráti tiež na zásobník.
 */
void genc_ord( dynamic_string *s );


/* Premení zadaný parameter (int) a znak podľa ASCII.
 */
void genc_chr( dynamic_string *s );

/* █████████████████████████████████████████████████████████████████████ Zásobník █████████████████████████████████████████████████████████████████████ */

/* Vygeneruje PUSHS <symb> \n - pushne hodnotu na dátový zásobník.
 */
void genc_pushs( dynamic_string *s, char frame, char* symb );


/* Vygeneruje POPS <var> \n - pop hodnotu zo zásobníka do premennej.
 */
void genc_pops( dynamic_string *s, char frame, char* var );


/* Vyčistí dátový zásobník.
 */
void genc_clears( dynamic_string *s );


/* █████████████████████████████████████████████████████████████████████ Funkcie a rámce █████████████████████████████████████████████████████████████████████ */

/* Pošle volanie funkcie "CALL <label> \n" na dyn. string.
 */
void genc_call( dynamic_string *s, char* label );


/* Pošle návratovú funkciu "RETURN \n".
 */
void genc_return( dynamic_string *s );


/* Pošle návestie "LABEL <label> \n" na dyn. string.
 */
void genc_label( dynamic_string *s, char* label );


/* Pošle begin "PUSHFRAME \n".
 */
void genc_begin( dynamic_string *s );


/* Pošle end "POPFRAME \n".
 */
void genc_end( dynamic_string *s );


/* █████████████████████████████████████████████████████████████████████ Aritmetika █████████████████████████████████████████████████████████████████████ */

/* Funkcia, ktorá spracuje výraz "x = a + b"
 * V prípade stringov -> konkatenácia.
 * V prípade int + float, pretypuje int -> float + float.
 */
void genc_add( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 );


/* "x = a - b"
 * V prípade int - float, pretypuje int -> float - float.
 */
void genc_sub( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 );


/* "x = a * b"
 * V prípade int * float, pretypuje int -> float * float.
 */
void genc_mul( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 );


/* "x = a / b"
 * int pretypuje na float
 */
void genc_div( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 );


/* "x = a / b"
 * float pretypuje na int
 */
void genc_idiv( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 );


/* Zásobníkové verzie - vezmú si vstupy od vrchu zásobníku a vrátia výsledok na vrchol
 */
void genc_adds( dynamic_string *s);
void genc_subs( dynamic_string *s);
void genc_muls( dynamic_string *s);
void genc_divs( dynamic_string *s);
void genc_idivs( dynamic_string *s);
void genc_concats( dynamic_string *s );


/* █████████████████████████████████████████████████████████████████████ Relácia █████████████████████████████████████████████████████████████████████ */

/* Funkcia vráti do prvého operandu bool@true v prípade, že operand1 je väčší ako operand2, inak false
 */
void genc_gt( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 );


/* Funkcia vráti do prvého operandu bool@true v prípade, že operand1 je menší ako operand2, inak false
 */
void genc_lt( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 );


/* Funkcia vráti do prvého operandu bool@true v prípade, že operand1 je rovný  operand2, inak false
 */
void genc_eq( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 );


/* Funkcia vráti do prvého operandu bool@true v prípade, že operand1 je väčší alebo rovný operand2, inak false
 */
void genc_ge( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 );


/* Funkcia vráti do prvého operandu bool@true v prípade, že operand1 je menší alebo rovný operand2, inak false
 */
void genc_le( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 );


/* Funkcia vráti do prvého operandu bool@true v prípade, že operand1 je nerovný operand2, inak false
 */
void genc_neq( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 );


/* Zásobníkové verzie, vezmú dve hodnoty zo zásobníku a vrátia na vrch výsledok relácie
 */
void genc_gts( dynamic_string *s );		// > - väčší než
void genc_lts( dynamic_string *s );		// < - menší než
void genc_eqs( dynamic_string *s );		// = - ekvivalentný s

void genc_ges( dynamic_string *s );		// >= - väčší alebo rovný
void genc_les( dynamic_string *s );		// <= - menší alebo rovný
void genc_neqs( dynamic_string *s );	// <> - nerovný


/* █████████████████████████████████████████████████████████████████████ Logika █████████████████████████████████████████████████████████████████████ */

/* Logický súcin - do prvého parametru vráti hodnotu bool@true ak sa operand1 a zároveň operand2 rovnajú true
 */
void genc_and( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 );


/* Logický súcet - do prvého parametru vráti hodnotu bool@true ak sa operand1 alebo operand2 rovnajú true
 */
void genc_or( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 );


/* Logická negácia - do prvého parametru vráti inverznú hodnotu operandu1
 */
void genc_not( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1 );


/* zásobníkové verzie
 */
void genc_ands( dynamic_string *s );
void genc_ors( dynamic_string *s );
void genc_nots( dynamic_string *s );


/* █████████████████████████████████████████████████████████████████████ Konverzia █████████████████████████████████████████████████████████████████████ */

/* frame0 -> kam	- musí byť vždy premenná (i.e. frame0 = {G, L, T}
 * frame1 -> co		- musí byť buď premenná (frame1 = {G, L, T}, alebo konštanta typu integer (v prípade konštatny frame1 musí byť I - a.k.a. iny )
 */
void genc_i2f( dynamic_string *s, char frameV, char* var, char frameS, char* symb );


/* frame0 -> kam	- musí byť vždy premenná (i.e. frame0 = {G, L, T}
 * frame1 -> co		- musí byť buď premenná (frame1 = {G, L, T}, alebo konštanta typu float (v prípade konštatny frame1 musí byť F - a.k.a. float )
 */
void genc_f2i( dynamic_string *s, char frameV, char* var, char frameS, char* symb );


/* frame0 -> kam	- musí byť vždy premenná (i.e. frame0 = {G, L, T}
 * frame1 -> co		- musí byť buď premenná (frame1 = {G, L, T}, alebo konštanta typu integer (v prípade konštatny frame1 musí byť S - a.k.a. int )
 */
void genc_i2c( dynamic_string *s, char frameV, char* var, char frameS, char* symb );


/* frame0 -> kam	- musí byť vždy premenná (i.e. frame0 = {G, L, T}
 * frame1 -> co		- musí byť buď premenná (frame1 = {G, L, T}, alebo konštanta typu string (v prípade konštatny frame1 musí byť S - a.k.a. string )
 */
void genc_s2i( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 );


/* myšlienka: Ak hodnota "result" je pravdivá, skočí na "label".
 * ########! Porovnanie podmienky sa koná pred touto funkciou !########
 * resp. porovnáva výsledok toho porovnávania s bool@true.
 */
void genc_ifeq( dynamic_string *s, char* label, char* result );


/* Podobne ako predošlá funkcia, ale ak výraz NEplatí.
 * ########! Porovnanie podmienky sa koná pred touto funkciou !########
 * resp. porovnáva výsledok toho porovnávania s bool@true.
 */
void genc_ifNeq( dynamic_string *s, char* label, char* result );

void genc_define( dynamic_string *s, char frame, char* var );
void genc_move( dynamic_string *s, char frameV, char* var, char frameS, char* symb );
void genc_jump( dynamic_string *s, char* label );
