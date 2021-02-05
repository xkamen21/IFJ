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
#include <string.h>

#include "codegen.h"
//#include "error.h"
//#include "parser.h"

extern dynamic_string out_;
extern dynamic_string main_;

unsigned label_id = 1;	// jedinečné ID pre každý výraz a/alebo návestie

// komentáre o účele funkcíi sú v codegen.h

void codegen_send( dynamic_string *s, char* str ) {
    char c;

    for ( unsigned i = 0; i < strlen(str); i++ ) {	// po znaku posiela reťazec na dynamický reťazec
        c = str[i];
        dyn_string( s, c );
    }
}

void codegen_flush( dynamic_string *s ) {
    printf( "%s\n", s->str );	// vypíše dynamický reťazec na štandartný výstup
}

void codegen_symb( dynamic_string *s, char frame, char* symb ) {
	switch (frame) {	// podľa hodnoty <frame> určí typ
		case 'G': case 'L': case 'T':	// rámec premennej
			dyn_string( s, frame );
			codegen_send( s, "F@_" );
			codegen_send( s, symb );
			break;
		case 'I':	// integer
			codegen_send( s, "int@" );
			codegen_send( s, symb );
			break;
		case 'B':	// bool
			codegen_send( s, "bool@" );
			codegen_send( s, symb );
			break;
		case 'F':	// float
			codegen_send( s, "float@" );
			dynamic_string tosend1;					// float je potrebné skonvertovať na hexadecimálny floating point
			tosend1.size = 0;							// vytvorí sa dynamický string, a ten pošle funkcii, ktorá ho konvertuje
			codegen_machifloat( &tosend1, symb );		// veľkosť stringu závysí od konverzie, dynamický string sa po výpise uvoľní
			codegen_send( s, tosend1.str );
			free_dyn_string( &tosend1 );
			break;
		case 'S':	// string
			codegen_send( s, "string@" );
			dynamic_string tosend2;						// podobne ako pre float
			tosend2.size = 0;							// dyn. string pošle na konverznú funkciu
			codegen_machistr( &tosend2, symb );			// po výpise sa dyn. string uvoľní
			codegen_send( s, tosend2.str );
			free_dyn_string( &tosend2 );
			break;
		case 'N':	// none
			codegen_send( s, "nil@" );
			codegen_send( s, symb );
			break;
        default:
            fprintf( stderr, "Varovanie! Nepodporovaná hodnota pre <frame> - codegen_symb( dynamic_string *s, char frame, char* symb )\n");
            fprintf( stderr, "           Očakávaná hodnota 'G' / 'L' / 'F' / 'I' / 'B' / 'F' / 'S'          ...tu  ↑\n");
            break;
	}
}

void codegen_var ( dynamic_string *s, char frame, char* var ) {
	dyn_string( s, frame );		// podobná ako funkcia pre symbol
	codegen_send( s, "F@_" );	// ibaže je určená iba pre premenné
	codegen_send( s, var );
}

void codegen_machistr( dynamic_string* retval, char* string ) {
	char c;         // pomocná premenná pre načítanie znakov
	char esc[5];	// pomocná premenná pre escape

    for ( unsigned i = 0; i < strlen(string); i++ ) {
		c = string[i];

				// 0-32 = "biele znaky"
				// 35 = '#'
				// 92 = '\'
				// kontroluje ordinálne ASCII hodnoty znaku
        if ( ( ( (0) <= c ) && ( c <= (32) ) ) || ( c == (35) ) || ( c == (92) ) ) {
			sprintf( esc, "%c%03d", (92), c );	// prepíš v tvare \xxx
			codegen_send( retval, esc );		// uloží escape na zadaný dynamický string
		}
		else {
            dyn_string( retval, c );
		}
	}
}

void codegen_machifloat( dynamic_string* retval, char* string ) {
	char* ptr;		// pomocný pointer pre strtod
	char str[64];	// veľkosť 64 - pretože nemožno poznať veľkosť tvaru po konverzii
    sprintf( str, "%a", strtod( string, &ptr ) );	// strtod pretypuje zadaný string na číslo - berie aj exponent / cez %a sa zapíše na str

	for ( unsigned i = 0; str[i] != '\0'; i++ ) {	// str za po znaku vypíše na zadaný dynamický string
        dyn_string( retval, str[i] );
    }
}

/* ################################################################ Utility ################################################################ */
void genc_init( dynamic_string *s ) {
	codegen_send( s, ".IFJcode19\n" );
	codegen_send( s, "\n" );
	codegen_send( s, "DEFVAR GF@_res_express\n" );		// globálne premenné pre výrazy
    codegen_send( s, "DEFVAR GF@_control\n" );			// do ktorých sa ukladajú hodnoty pri výpočtoch
	codegen_send( s, "DEFVAR GF@_control-type\n" );
    codegen_send( s, "DEFVAR GF@_control2\n" );
    codegen_send( s, "DEFVAR GF@_control2-type\n" );
    codegen_send( s, "DEFVAR GF@_control-res1\n" );
	codegen_send( s, "DEFVAR GF@_control-res2\n" );
	codegen_send( s, "JUMP &main\n" );
	codegen_send( s, "\n" );
}

void genc_init_main( dynamic_string *s ){
    codegen_send( s, "LABEL &main\n" );
	codegen_send( s, "\n" );
}

void genc_comment( dynamic_string *s, char* comment ) {
    codegen_send( s, "# " );		// vypíše komentár
	codegen_send( s, comment );
    codegen_send( s, "\n" );
}

void genc_newline( dynamic_string *s ) {
	codegen_send( s, "\n" );		// vypíše nový riadok
}

/* ################################################################ Built-In ################################################################ */
void genc_inputs( dynamic_string *s ) {
	codegen_send( s, "\n# built-in inputs\n" );
	codegen_send( s, "LABEL &inputs\n" );
	codegen_send( s, "PUSHFRAME\n" );

	codegen_send( s, "DEFVAR LF@*returned\n" );			// funkcia definuje lokálnu premennú
	codegen_send( s, "READ LF@*returned string\n" );	// do ktorej uloží hodnotu zo štandartného vstupu - berie iba string
	codegen_send( s, "PUSHS LF@*returned\n" );			// a vloží ju na dátový zásobník

	codegen_send( s, "POPFRAME\n" );
	codegen_send( s, "RETURN\n\n" );
}

void genc_inputi( dynamic_string *s ) {
	codegen_send( s, "\n# built-in inputi\n" );
	codegen_send( s, "LABEL &inputi\n" );
	codegen_send( s, "PUSHFRAME\n" );

	codegen_send( s, "DEFVAR LF@*returned\n" );		// podobná ako inputs
	codegen_send( s, "READ LF@*returned int\n" );	// berie iba int
	codegen_send( s, "PUSHS LF@*returned\n" );

	codegen_send( s, "POPFRAME\n" );
	codegen_send( s, "RETURN\n\n" );
}

void genc_inputf( dynamic_string *s ) {
	codegen_send( s, "\n# built-in inputf\n" );
	codegen_send( s, "LABEL &inputf\n" );
	codegen_send( s, "PUSHFRAME\n" );

	codegen_send( s, "DEFVAR LF@*returned\n" );			// podobná ako inpts
	codegen_send( s, "READ LF@*returned float\n" );		// berie iba float
	codegen_send( s, "PUSHS LF@*returned\n" );

	codegen_send( s, "LABEL &inputs-return\n" );
	codegen_send( s, "POPFRAME\n" );
	codegen_send( s, "RETURN\n\n" );
}

void genc_print( dynamic_string *s ) {
	codegen_send( s, "\n# built-in print\n" );
	codegen_send( s, "LABEL &print\n" );
	codegen_send( s, "PUSHFRAME\n" );

	codegen_send( s, "DEFVAR LF@*print-counter\n" );	// počítadlo parametrov
	codegen_send( s, "POPS LF@*print-counter\n" );		// predpokladá, že na vrchu zásobníku bude počet parametrov

	codegen_send( s, "DEFVAR LF@*print-val\n" );		// definuje premennú, do ktorej sa bude ukladať vytláčaná hodnota

	codegen_send( s, "LABEL &print-while\n" );			// začiatok cyklu
	codegen_send( s, "POPS LF@*print-val\n" );			// popni zásobník
	codegen_send( s, "WRITE LF@*print-val\n" );			// vytlač hodnotu
	codegen_send( s, "SUB LF@*print-counter LF@*print-counter int@1\n" );	// dekrementácia
	codegen_send( s, "JUMPIFEQ &print-done LF@*print-counter int@0\n" );	// v prípade posledného parametru skoč na koniec
	codegen_send( s, "WRITE string@\\032\n" );								// inak vytlač medzeru
	codegen_send( s, "JUMP &print-while\n" );								// a opakuj cyklus

	codegen_send( s, "LABEL &print-done\n" );			// koniec cyklu
	codegen_send( s, "WRITE string@\\010\n" );			// vytlač odriadkovanie

	codegen_send( s, "POPFRAME\n" );
	codegen_send( s, "RETURN\n\n" );
}

void genc_len( dynamic_string *s ) {
	codegen_send( s, "\n# built-in length\n" );
	codegen_send( s, "LABEL &len\n" );
	codegen_send( s, "PUSHFRAME\n" );

	codegen_send( s, "DEFVAR LF@*returned\n" );		// premenná pre uloženie výslednej hodnoty
	codegen_send( s, "DEFVAR LF@*param1\n" );		// premenná pre meraný reťazec
	codegen_send( s, "POPS LF@*param1\n" );			// vezmi hodnotu zo zásobníku

	codegen_send( s, "DEFVAR LF@*param1-type\n" );				// premenná pre typ
	codegen_send( s, "TYPE LF@*param1-type LF@*param1\n" );		// ulož dátový typ meraného výrazu

	codegen_send( s, "JUMPIFNEQ &len-exit LF@*param1-type string@string\n" );	// ak meraný výraz nie je reťazec, ukonči
	codegen_send( s, "STRLEN LF@*returned LF@*param1\n" );						// ak je, tak zmeraj jeho veľkosť
	codegen_send( s, "PUSHS LF@*returned\n" );					// a ulož ju na zásobník
	codegen_send( s, "JUMP &len-return\n" );

	codegen_send( s, "LABEL &len-exit\n" );			// ukončí Error 4
	codegen_send( s, "EXIT int@4\n" );

	codegen_send( s, "LABEL &len-return\n" );		// ukonči normálne

	codegen_send( s, "POPFRAME\n" );
	codegen_send( s, "RETURN\n\n" );
}

void genc_substr( dynamic_string *s ) {
	codegen_send( s, "\n# built-in substr\n" );
	codegen_send( s, "LABEL &substr\n" );
	codegen_send( s, "PUSHFRAME\n" );
	codegen_send( s, "DEFVAR LF@*returned\n" );		// premenná pre návratovú hodnotu
	codegen_send( s, "DEFVAR LF@*param1\n" );		// parameter 1 - reťazec, v ktorom hľadáme podreťazec
	codegen_send( s, "DEFVAR LF@*param2\n" );		// 2 - index začiatku podreťazca
	codegen_send( s, "DEFVAR LF@*param3\n" );		// 3 - dĺžka podreťazca

	codegen_send( s, "POPS LF@*param1\n" );			// parametre sú predávané zprava-do-ľava
	codegen_send( s, "POPS LF@*param2\n" );
	codegen_send( s, "POPS LF@*param3\n" );

	codegen_send( s, "DEFVAR LF@*param1-type\n" );	// premenné pre dátový typ parametrov
	codegen_send( s, "DEFVAR LF@*param2-type\n" );
	codegen_send( s, "DEFVAR LF@*param3-type\n" );
	codegen_send( s, "TYPE LF@*param1-type LF@*param1\n" );	// ulož do nich dátové typy jednotlivých parametrov
	codegen_send( s, "TYPE LF@*param2-type LF@*param2\n" );
	codegen_send( s, "TYPE LF@*param3-type LF@*param3\n" );

	codegen_send( s, "JUMPIFNEQ &substr-exit LF@*param1-type string@string\n" );	// ak sa dátové typy nerovnajú požadovanému typu (podľa inštrukcie) skoč na koniec error 4
	codegen_send( s, "JUMPIFNEQ &substr-exit LF@*param2-type string@int\n" );
	codegen_send( s, "JUMPIFNEQ &substr-exit LF@*param3-type string@int\n" );

	codegen_send( s, "MOVE LF@*returned string@\n" );	// do návratovej hodnoty presuň prázdny reťazec
	codegen_send( s, "DEFVAR LF@*helpstr\n" );			// pomocná premenná pre znak
	codegen_send( s, "DEFVAR LF@*result\n" );			// pomocná premenná pre podmienku

	// funkcia v skutočnosti nemeria dĺžku podreťazca, ale cyklicky posúva index do prava "dĺžka"-krát

	codegen_send( s, "LABEL &substr-while\n" );								// začiatok cyklu
	codegen_send( s, "GT LF@*result LF@*param3 int@0\n" );					// vráť TRUE ak je dĺžka ešte stále väčšia ako 0
	codegen_send( s, "JUMPIFNEQ &substr-return LF@*result bool@true\n" );	// ak je už dĺžka 0, skoč na koniec (hotovo)
	codegen_send( s, "GETCHAR LF@*helpstr LF@*param1 LF@*param2\n" );		// vezme hodnotu znaku z param1 na indexe param2
	codegen_send( s, "ADD LF@*param2 LF@*param2 int@1\n" );					// inkrementuje index začiatku podreťazca
	codegen_send( s, "SUB LF@*param3 LF@*param3 int@1\n" );					// dekrementuje dĺžku podreťazca
	codegen_send( s, "CONCAT LF@*returned LF@*returned LF@*helpstr\n" );	// konkatenuje návratovú hodnotu so podreťazcom
	codegen_send( s, "JUMP &substr-while\n" );								// skoč na začiatok cyklu

	codegen_send( s, "LABEL &substr-exit\n" );			// error 4
	codegen_send( s, "EXIT int@4\n" );

	codegen_send( s, "LABEL &substr-return\n" );		// koniec
	
	codegen_send( s, "PUSHS LF@*returned\n" );

	codegen_send( s, "POPFRAME\n" );
	codegen_send( s, "RETURN\n\n" );
}

void genc_ord( dynamic_string *s ) {
	codegen_send( s, "\n# built-in ord\n" );
	codegen_send( s, "LABEL &ord\n" );
	codegen_send( s, "PUSHFRAME\n" );

	codegen_send( s, "DEFVAR LF@*returned\n" );
	codegen_send( s, "DEFVAR LF@*param1\n" );	// parameter 1:(string) a 2:(int)
	codegen_send( s, "DEFVAR LF@*param2\n" );	// dáta sú na zásobník posúvané zľava-do-prava
	codegen_send( s, "POPS LF@*param1\n" );		// čo znamená, že musíme POP-ovať od konca
	codegen_send( s, "POPS LF@*param2\n" );

	codegen_send( s, "DEFVAR LF@*param1-type\n" );				// dátové typy...
	codegen_send( s, "DEFVAR LF@*param2-type\n" );
	codegen_send( s, "TYPE LF@*param1-type LF@*param1\n" );
	codegen_send( s, "TYPE LF@*param2-type LF@*param2\n" );

	codegen_send( s, "JUMPIFNEQ &ord-exit LF@*param1-type string@string\n" );	// nesprávne dátové typy = exit
	codegen_send( s, "JUMPIFNEQ &ord-exit LF@*param2-type string@int\n" );
	codegen_send( s, "STRI2INT LF@*returned LF@*param1 LF@*param2\n" );		// vráti ordinálne číslo znaku na zadanom indexe param2 v stringu param1
	codegen_send( s, "PUSHS LF@*returned\n" );								// vloží hodnotu na dátový zásobník
	codegen_send( s, "JUMP &ord-return\n" );

	codegen_send( s, "LABEL &ord-exit\n" );		// error 4
	codegen_send( s, "EXIT int@4\n" );

	codegen_send( s, "LABEL &ord-return\n" );	// normálny koniec

	codegen_send( s, "POPFRAME\n" );
	codegen_send( s, "RETURN\n\n" );
}

void genc_chr( dynamic_string *s ) {
	codegen_send( s, "\n# built-in chr\n" );
	codegen_send( s, "LABEL &chr\n" );
	codegen_send( s, "PUSHFRAME\n" );

	codegen_send( s, "DEFVAR LF@*returned\n" );
	codegen_send( s, "DEFVAR LF@*param1\n" );	// parameter 1:(int)
	codegen_send( s, "POPS LF@*param1\n" );

	codegen_send( s, "DEFVAR LF@*param1-type\n" );				// dátový typ...
	codegen_send( s, "TYPE LF@*param1-type LF@*param1\n" );

	codegen_send( s, "JUMPIFNEQ &chr-exit LF@*param1-type string@int\n" );	// parameter nie je int = error 4
	codegen_send( s, "INT2CHAR LF@*returned LF@*param1\n" );				// prekonvertuje číslo na znak ordinálnej hodnoty ekvivalentnej tomu číslu
	codegen_send( s, "PUSHS LF@*returned\n" );						// vloží výsledok na dátový zásobník
	codegen_send( s, "JUMP &chr-return\n" );

	codegen_send( s, "LABEL &chr-exit\n" );			// error 4
	codegen_send( s, "EXIT int@4\n" );

	codegen_send( s, "LABEL &chr-return\n" );		// normálny koniec

	codegen_send( s, "POPFRAME\n" );
	codegen_send( s, "RETURN\n\n" );
}

/* ################################################################ Zásobník ################################################################ */
void genc_pushs( dynamic_string *s, char frame, char* symb ) {
	codegen_send( s, "PUSHS " );		// vlož symbol na dátový zásobník
	codegen_symb( s, frame, symb );
	codegen_send( s, "\n");
}

void genc_pops( dynamic_string *s, char frame, char* var ) {
	codegen_send( s, "POPS " );			// vezmi z dátového zásobníku a ulož to na premennú
	codegen_var ( s, frame, var );
	codegen_send( s, "\n");
}

void genc_clears( dynamic_string *s ) {
	codegen_send( s, "CLEARS\n" );		// vyčistí dátový zásobník
}

/* ################################################################ Funkcie a rámce ################################################################ */
void genc_call( dynamic_string *s, char* label ) {
	codegen_send( s, "CREATEFRAME\n" );		// vytvorí rámec a zavolá funkciu
    codegen_send( s, "CALL &" );
    codegen_send( s, label );
    codegen_send( s, "\n" );
}

void genc_return( dynamic_string *s ) {
	codegen_send( s, "RETURN\n" );			// návrat na miesto, kde bola funkcia povolaná
}

void genc_label( dynamic_string *s, char* label ) {
    codegen_send( s, "LABEL &" );		// návestie
    codegen_send( s, label);
    codegen_send( s, "\n" );
}

void genc_begin( dynamic_string *s ) {
	codegen_send( s, "PUSHFRAME\n" );				// začiatok rámca (indent)
    codegen_send( s, "DEFVAR LF@_ret_value\n");
}

void genc_end( dynamic_string *s ) {
	codegen_send( s, "POPFRAME\n" );		// koniec rámca (dedent)
}

/* ################################################################ Aritmetika ################################################################ */
void genc_add( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 ) {

	// x = a + b
	char label[64];
	sprintf( label, "addlabel%d", label_id++ );		// vytvorenie jedinečného návestia

	codegen_send( s, "MOVE GF@_control " );
	codegen_symb( s, frameS1, symb1 );									// ulož zadaný parameter do globálnej premennej pre výrazy
	codegen_send( s, "\nTYPE GF@_control-type GF@_control\n" );			// a zisti jej dátový typ

	codegen_send( s, "MOVE GF@_control2 " );							// parameter 2 - ulož & zisti DT
	codegen_symb( s, frameS2, symb2 );
	codegen_send( s, "\nTYPE GF@_control2-type GF@_control2\n" );

	codegen_send( s, "JUMPIFNEQ &" );									// ak sa dátové typy nerovnajú, skoč na <label>-neq
	codegen_send( s, label );
	codegen_send( s, "-neq GF@_control-type GF@_control2-type\n" );

	codegen_send( s, "JUMPIFEQ &" );									// v tejto chvíli vieme, že sú oba operandy rovnakého typu
	codegen_send( s, label );											// takže stačí porovnávať iba jeden z nich
	codegen_send( s, "-exit GF@_control-type string@bool\n" );			// bool = exit

	codegen_send( s, "JUMPIFEQ &" );
	codegen_send( s, label );											// ak je typu string, tak skoč na konkatenáciu, CONCAT
	codegen_send( s, "-string GF@_control-type string@string\n" );

	codegen_send( s, "JUMPIFEQ &" );									// nil = exit
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@nil\n" );

	codegen_send( s, "JUMP &" );
	codegen_send( s, label );											// int alebo float, skoč na výpočet
	codegen_send( s, "-ok\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );											// v tejto chvíli sú operandy rôzneho typu
	codegen_send( s, "-neq\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );											// Ak sa prvý operand nerovná typu integer..
	codegen_send( s, "-bint? GF@_control-type string@int\n" );			// zisti či sa druhý rovná integer

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );											// prvý sa rovná typu integer, tak pozri či sa druhý rovná float
	codegen_send( s, "-exit GF@_control2-type string@float\n" );		// ak nie, error 4

	codegen_send( s, "INT2FLOAT GF@_control GF@_control\n" );			// ak áno, pretypuj prvý operand(int) na float

	codegen_send( s, "JUMP &" );
	codegen_send( s, label );											// pretypované
	codegen_send( s, "-ok\n" );											// skoč na výpočet

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );											// prvý operand nie je int, je ním druhý?
	codegen_send( s, "-bint?\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );											// ak ani druhý operandu nie je int, ukonči error 4
	codegen_send( s, "-exit GF@_control2-type string@int\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );											// ak je druhý operand int, tak je prvý float?
	codegen_send( s, "-exit GF@_control2-type string@float\n" );

	codegen_send( s, "INT2FLOAT GF@_control2 GF@_control2\n" );			// pretypuj druhý(int) na float

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );											// label pre výpočet
	codegen_send( s, "-ok\n" );

	codegen_send( s, "ADD " );
	codegen_var ( s, frameV, var );										// súčet
	codegen_send( s, " GF@_control GF@_control2\n" );

	codegen_send( s, "JUMP &" );
	codegen_send( s, label );											// vypočítane
	codegen_send( s, "-return\n" );										// ukonči

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );											// sem skáču v prípade oboch operandov typu string
	codegen_send( s, "-string\n" );

	codegen_send( s, "CONCAT " );
	codegen_var ( s, frameV, var );										// súčet reťazcov - konkatenácia
	codegen_send( s, " GF@_control GF@_control2\n" );

	codegen_send( s, "JUMP &" );
	codegen_send( s, label );											// vypočítane
	codegen_send( s, "-return\n" );										// ukonči

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );											// sem skáču v prípade špatného dátového typu pre operáciu
	codegen_send( s, "-exit\n" );

	codegen_send( s, "EXIT int@4\n" );									// EXIT int@4 - Error 4

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );											// návestie konca výpočtu
	codegen_send( s, "-return\n" );
}

void genc_sub( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 ) {

	// x = a - b
	char label[64];
	sprintf( label, "sublabel%d", label_id++ );		// vytvorenie jedinečného návestia

	codegen_send( s, "MOVE GF@_control " );								// operand1 - hodnota & DT
	codegen_symb( s, frameS1, symb1 );
	codegen_send( s, "\nTYPE GF@_control-type GF@_control\n" );

	codegen_send( s, "MOVE GF@_control2 " );							// operand2 - hodnota & DT
	codegen_symb( s, frameS2, symb2 );
	codegen_send( s, "\nTYPE GF@_control2-type GF@_control2\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );											// kontrola dátových typov operandov
	codegen_send( s, "-neq GF@_control-type GF@_control2-type\n" );		// ak sa nerovnajú, skočí na <label>-neq

	codegen_send( s, "JUMPIFEQ &" );									// v tejto chvíli vieme, že sú oba operandy rovnakého typu
	codegen_send( s, label );											// takže stačí porovnávať iba jeden z nich
	codegen_send( s, "-exit GF@_control-type string@bool\n" );			// bool = exit

	codegen_send( s, "JUMPIFEQ &" );
	codegen_send( s, label );											// string = exit
	codegen_send( s, "-exit GF@_control-type string@string\n" );

	codegen_send( s, "JUMPIFEQ &" );									// nil = exit
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@nil\n" );

	codegen_send( s, "JUMP &" );
	codegen_send( s, label );											// skoč na výpočet
	codegen_send( s, "-ok\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );											// v tejto chvíli sú operandy rôzneho typu
	codegen_send( s, "-neq\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );											// Ak sa prvý operand nerovná typu integer..
	codegen_send( s, "-bint? GF@_control-type string@int\n" );			// pozri, či sa druhý rovná int

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );											// prvý sa rovná typu integer, tak pozri či sa druhý rovná float
	codegen_send( s, "-exit GF@_control2-type string@float\n" );		// ak nie = error 4

	codegen_send( s, "INT2FLOAT GF@_control GF@_control\n" );			// ak áno - pretypuj operand1(int) na float

	codegen_send( s, "JUMP &" );
	codegen_send( s, label );											// skoč na výpočet
	codegen_send( s, "-ok\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );											// prvý operand nie je int, je ním druhý?
	codegen_send( s, "-bint?\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );											// ak ani druhý operand nie je int, ukonči výpočet - error 4
	codegen_send( s, "-exit GF@_control2-type string@int\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );											// ak je druhý operand int, tak je prvý float?
	codegen_send( s, "-exit GF@_control2-type string@float\n" );		// ak nie - error 4

	codegen_send( s, "INT2FLOAT GF@_control2 GF@_control2\n" );			// ak áno, pretypuj druhý(int) na float

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );											// lnávestie pre výpočet
	codegen_send( s, "-ok\n" );

	codegen_send( s, "SUB " );
	codegen_var ( s, frameV, var );										// rozdiel
	codegen_send( s, " GF@_control GF@_control2\n" );

	codegen_send( s, "JUMP &" );
	codegen_send( s, label );											// vypočítane
	codegen_send( s, "-return\n" );										// ukonči

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );											// sem skáču v prípade špatného dátového typu pre operáciu
	codegen_send( s, "-exit\n" );

	codegen_send( s, "EXIT int@4\n" );									// EXIT int@4 - Error 4

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );											// návestie konca výpočtu
	codegen_send( s, "-return\n" );
}

void genc_mul( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 ) {

	// x = a - b
	char label[64];
	sprintf( label, "mullabel%d", label_id++ );		// jedinečné návestie

	codegen_send( s, "MOVE GF@_control " );								// ďalšie komentáre si netreba všímať
	codegen_symb( s, frameS1, symb1 );									// zastaralé a prakticky také isté ako pre ADD a SUB
	codegen_send( s, "\nTYPE GF@_control-type GF@_control\n" );			// výraz je súčin

	codegen_send( s, "MOVE GF@_control2 " );
	codegen_symb( s, frameS2, symb2 );
	codegen_send( s, "\nTYPE GF@_control2-type GF@_control2\n" );

	codegen_send( s, "JUMPIFNEQ &" );									// JUMPIFNEQ &LxLaplusLb-neq LF@_a-type LF@_b-type
	codegen_send( s, label );											// kontrola dátových typov operandov
	codegen_send( s, "-neq GF@_control-type GF@_control2-type\n" );		// ak sa nerovnajú, skočí na <label>-neq

	codegen_send( s, "JUMPIFEQ &" );									// JUMPIFEQ &LxLaplusLb-exit LF@_a-type string@bool
	codegen_send( s, label );											// v tejto chvíli vieme, že sú oba operandy rovnakého typu
	codegen_send( s, "-exit GF@_control-type string@bool\n" );			// takže stačí porovnávať iba jeden z nich

	codegen_send( s, "JUMPIFEQ &" );									// JUMPIFEQ &LxLaplusLb-string LF@_a-type string@string
	codegen_send( s, label );											// ak je typu string, tak skoč na konkatenáciu, CONCAT
	codegen_send( s, "-exit GF@_control-type string@string\n" );

	codegen_send( s, "JUMPIFEQ &" );									// JUMPIFEQ &LxLaplusLb-exit LF@_a-type string@nil
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@nil\n" );

	codegen_send( s, "JUMP &" );										// JUMP &LxLaplusLb-return
	codegen_send( s, label );											// spočítané, skoč na return
	codegen_send( s, "-ok\n" );

	codegen_send( s, "LABEL &" );										// LABEL &LxLaplusLb-neq
	codegen_send( s, label );											// v tejto chvíli sú operandy rôzneho typu
	codegen_send( s, "-neq\n" );

	codegen_send( s, "JUMPIFNEQ &" );									// JUMPIFNEQ &LxLaplusLb-bint? LF@_a-type string@int
	codegen_send( s, label );											// Ak sa prvý operand nerovná typu integer..
	codegen_send( s, "-bint? GF@_control-type string@int\n" );

	codegen_send( s, "JUMPIFNEQ &" );									// JUMPIFNEQ &LxLaplusLb-exit LF@_b-type string@float
	codegen_send( s, label );											// prvý sa rovná typu integer, tak pozri či sa druhý rovná float
	codegen_send( s, "-exit GF@_control2-type string@float\n" );

	codegen_send( s, "INT2FLOAT GF@_control GF@_control\n" );			// INT2FLOAT

	codegen_send( s, "JUMP &" );										// JUMP &LxLaplusLb-return
	codegen_send( s, label );											// vypočítane
	codegen_send( s, "-ok\n" );											// ukonči

	codegen_send( s, "LABEL &" );										// LABEL &LxLaplusLb-bint?
	codegen_send( s, label );											// prvý operand nie je int, je ním druhý?
	codegen_send( s, "-bint?\n" );

	codegen_send( s, "JUMPIFNEQ &" );									// JUMPIFNEQ &LxLaplusLb-exit LF@_b-type string@int
	codegen_send( s, label );											// ak ani druhý operandu nie je int, ukonči výpočet
	codegen_send( s, "-exit GF@_control2-type string@int\n" );

	codegen_send( s, "JUMPIFNEQ &" );									// JUMPIFNEQ &LxLaplusLb-exit LF@_a-type string@float
	codegen_send( s, label );											// ak je druhý operand int, tak je prvý float?
	codegen_send( s, "-exit GF@_control2-type string@float\n" );

	codegen_send( s, "INT2FLOAT GF@_control2 GF@_control2\n" );			// INT2FLOAT LF@_a LF@_a

	codegen_send( s, "LABEL &" );										// LABEL &LxLaplusLb-string
	codegen_send( s, label );											// sem skáču v prípade oboch operandov
	codegen_send( s, "-ok\n" );

	codegen_send( s, "MUL " );											// ADD LF@_x LF@_a LF@_b
	codegen_var ( s, frameV, var );										// normálny súčet
	codegen_send( s, " GF@_control GF@_control2\n" );

	codegen_send( s, "JUMP &" );										// JUMP &LxLaplusLb-return
	codegen_send( s, label );											// vypočítane
	codegen_send( s, "-return\n" );										// ukonči

	codegen_send( s, "LABEL &" );										// LABEL &LxLaplusLb-exit
	codegen_send( s, label );											// sem skáču v prípade špatného dátového typu pre operáciu
	codegen_send( s, "-exit\n" );

	codegen_send( s, "EXIT int@4\n" );									// EXIT int@4 - Error 4

	codegen_send( s, "LABEL &" );										// LABEL &LxLaplusLb-return
	codegen_send( s, label );											// návestie konca výpočtu
	codegen_send( s, "-return\n" );
}

void genc_div( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 ) {

	// x = a / b
	char label[64];
	sprintf( label, "divlabel%d", label_id++ );		// jedinečne návestie

	codegen_send( s, "MOVE GF@_control " );
	codegen_symb( s, frameS1, symb1 );
	codegen_send( s, "\nTYPE GF@_control-type v\n" );

	codegen_send( s, "MOVE GF@_control2 " );
	codegen_symb( s, frameS2, symb2 );
	codegen_send( s, "\nTYPE GF@_control2-type GF@_control2\n" );

	codegen_send( s, "JUMPIFNEQ &" );									// operand 1 = int - pretypuj ho na float
	codegen_send( s, label );
	codegen_send( s, "-type1-false GF@_control-type string@int\n" );

	codegen_send( s, "INT2FLOAT GF@_control GF@_control\n" );

	codegen_send( s, "MOVE GF@_control-type string@float" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-type1-false\n" );

	codegen_send( s, "JUMPIFNEQ &" );									// operand 2 = int - pretypuj ho na float
	codegen_send( s, label );
	codegen_send( s, "-type2-false GF@_control2-type string@int\n" );

	codegen_send( s, "INT2FLOAT GF@_control2 GF@_control2\n" );

	codegen_send( s, "JUMPIFEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control2 float@0x0p+0\n" );

	codegen_send( s, "MOVE GF@_control2-type string@float" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-type2-false\n" );

	codegen_send( s, "JUMPIFNEQ &" );									// ak operandy nie sú rovnakého typu - error 4
	codegen_send( s, label );											// pretože v tejto chvíli by mali byť obi dva buď typu float (int je vždy pretypovaný)
	codegen_send( s, "-exit GF@_control-type GF@_control2-type\n" );	// alebo nepodporovaného typu

	codegen_send( s, "JUMPIFNEQ &" );									// po kontrole, či sú oba rovnakého typu
	codegen_send( s, label );											// skontroluj, či sa naozaj rovnajú typu float
	codegen_send( s, "-exit GF@_control-type string@float\n" );

	codegen_send( s, "JUMPIFEQ &" );
	codegen_send( s, label );											// kontrola pri delení float 0
	codegen_send( s, "-exit GF@_control2 float@0x0p+0\n" );				// error 4

	codegen_send( s, "DIV " );							// podiel bezo zvyšku
	codegen_symb( s, frameV, var );
	codegen_send( s, " GF@_control GF@_control2\n" );

	codegen_send( s, "JUMP &" );			// ukonči
	codegen_send( s, label );
	codegen_send( s, "-return\n" );

	codegen_send( s, "LABEL &" );			// návestie exit
	codegen_send( s, label );
	codegen_send( s, "-exit\n" );

	codegen_send( s, "EXIT int@4\n" );		// error 4

	codegen_send( s, "LABEL &" );			// koniec
	codegen_send( s, label );
	codegen_send( s, "-return\n" );
}

void genc_idiv( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 ) {

	// x = a / b
	char label[64];
	sprintf( label, "idivlabel%d", label_id++ );

	codegen_send( s, "MOVE GF@_control " );
	codegen_symb( s, frameS1, symb1 );
	codegen_send( s, "\nTYPE GF@_control-type v\n" );

	codegen_send( s, "MOVE GF@_control2 " );
	codegen_symb( s, frameS2, symb2 );
	codegen_send( s, "\nTYPE GF@_control2-type GF@_control2\n" );

	codegen_send( s, "JUMPIFNEQ &" );									// musia byť rovnakého typu
	codegen_send( s, label );											// inak error 4
	codegen_send( s, "-exit GF@_control-type GF@_control2-type\n" );

	codegen_send( s, "JUMPIFNEQ &" );									// a zároveň oba typu int
	codegen_send( s, label );											// inak error 4
	codegen_send( s, "-exit GF@_control-type string@int\n" );			// implicítna konverzia float-na-int nie je povolená!

	codegen_send( s, "JUMPIFEQ &" );						// kontrola delenia int 0
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control2 int@0\n" );

	codegen_send( s, "IDIV " );								// celočíselné delenie
	codegen_symb( s, frameV, var );
	codegen_send( s, " GF@_control GF@_control2\n" );

	codegen_send( s, "JUMP &" );			// skoč na koniec
	codegen_send( s, label );
	codegen_send( s, "-return\n" );

	codegen_send( s, "LABEL &" );			// návestie exit
	codegen_send( s, label );
	codegen_send( s, "-exit\n" );

	codegen_send( s, "EXIT int@4\n" );		// error 4

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );				// koniec
	codegen_send( s, "-return\n" );
}

/* Zásobníkové verzie */
void genc_adds( dynamic_string *s ) {

	char label[64];
	sprintf( label, "addslabel%d", label_id++ );	// jedinečne návestie

	codegen_send( s, "POPS GF@_control2\n" );							// zásobníková verzia genc_add - veľmi podobná
	codegen_send( s, "TYPE GF@_control2-type GF@_control2\n" );			// operandy berie zo zásobníka, skontroluje DT a vráti naspäť na zásobník pred výpočtom
	codegen_send( s, "POPS GF@_control\n" );
	codegen_send( s, "TYPE GF@_control-type GF@_control\n" );

	codegen_send( s, "JUMPIFNEQ &" );
    codegen_send( s, label );
    codegen_send( s, "-neq GF@_control-type GF@_control2-type\n" );		// kontrola dátových typov
	codegen_send( s, "JUMPIFEQ &" );
    codegen_send( s, label );
    codegen_send( s, "-concats GF@_control-type string@string\n" );		// ak sú vhodné, pokračuj k výpočtu
	codegen_send( s, "JUMPIFEQ &" );									// string, bool, nil = error 4
    codegen_send( s, label );
    codegen_send( s, "-exit GF@_control-type string@bool\n" );
	codegen_send( s, "JUMPIFEQ &" );
    codegen_send( s, label );
    codegen_send( s, "-exit GF@_control-type string@nil\n" );
	codegen_send( s, "JUMP &" );
    codegen_send( s, label );
    codegen_send( s, "-ok\n" );

	codegen_send( s, "LABEL &");
    codegen_send( s, label );
    codegen_send( s, "-neq\n" );
	codegen_send( s, "JUMPIFNEQ &" );
    codegen_send( s, label );
    codegen_send( s, "-op2int GF@_control-type string@int\n" );			// prvý operand je int a druhý float
	codegen_send( s, "JUMPIFNEQ &" );
    codegen_send( s, label );
    codegen_send( s, "-exit GF@_control2-type string@float\n" );		// pretypuj int a pokračuj k výpočtu
	codegen_send( s, "INT2FLOAT GF@_control GF@_control\n" );
	codegen_send( s, "JUMP &" );
    codegen_send( s, label );
    codegen_send( s, "-ok\n" );

	codegen_send( s, "LABEL &" );
    codegen_send( s, label );
    codegen_send( s, "-op2int\n" );
	codegen_send( s, "JUMPIFNEQ &");
    codegen_send( s, label );
    codegen_send( s, "-exit GF@_control2-type string@int\n" );			// druhý operand je int a druhý float
	codegen_send( s, "JUMPIFNEQ &");
    codegen_send( s, label );
    codegen_send( s, "-exit GF@_control-type string@float\n" );			// pretypuj int a pokračuj k výpočtu
	codegen_send( s, "INT2FLOAT GF@_control2 GF@_control2\n" );
	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-ok\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
    codegen_send( s, "-concats\n" );
	codegen_send( s, "CONCAT GF@_control GF@_control GF@_control2\n" );
	codegen_send( s, "PUSHS GF@_control\n" );
	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );
	
	codegen_send( s, "LABEL &" );				// v prípade nesprávneho dátového typu
    codegen_send( s, label );
    codegen_send( s, "-exit\n" );
	codegen_send( s, "EXIT int@4\n" );						// ukonči - error 4

	codegen_send( s, "LABEL &" );
    codegen_send( s, label );
    codegen_send( s, "-ok\n" );
	codegen_send( s, "PUSHS GF@_control2\n" );				// výpočet a koniec
	codegen_send( s, "PUSHS GF@_control\n" );
	codegen_send( s, "ADDS\n" );
	
	codegen_send( s, "LABEL &" );
    codegen_send( s, label );
    codegen_send( s, "-return\n" );
}

void genc_subs( dynamic_string *s ) {
	char label[64];
	sprintf( label, "subslabel%d", label_id++ );			// taká istá ako genc_adds - ale výraz je SUBS

	codegen_send( s, "POPS GF@_control\n" );
	codegen_send( s, "TYPE GF@_control-type GF@_control\n" );
	codegen_send( s, "POPS GF@_control2\n" );
	codegen_send( s, "TYPE GF@_control2-type GF@_control2\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-neq GF@_control-type GF@_control2-type\n" );
	codegen_send( s, "JUMPIFEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@string\n" );
	codegen_send( s, "JUMPIFEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@bool\n" );
	codegen_send( s, "JUMPIFEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@nil\n" );
	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-ok\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-neq\n" );
	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-op2int GF@_control-type string@int\n" );
	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control2-type string@float\n" );
	codegen_send( s, "INT2FLOAT GF@_control GF@_control\n" );
	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-ok\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-op2int\n" );
	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control2-type string@int\n" );
	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@float\n" );
	codegen_send( s, "INT2FLOAT GF@_control2 GF@_control2\n" );
	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-ok\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-exit\n" );
	codegen_send( s, "EXIT int@4\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-ok\n" );
	codegen_send( s, "PUSHS GF@_control2\n" );
	codegen_send( s, "PUSHS GF@_control\n" );
	codegen_send( s, "SUBS\n" );
}

void genc_muls( dynamic_string *s) {
	char label[64];
	sprintf( label, "subslabel%d", label_id++ );			// ekvivalentná s genc_adds - ale výraz je MULS

	codegen_send( s, "POPS GF@_control\n" );
	codegen_send( s, "TYPE GF@_control-type GF@_control\n" );
	codegen_send( s, "POPS GF@_control2\n" );
	codegen_send( s, "TYPE GF@_control2-type GF@_control2\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-neq GF@_control-type GF@_control2-type\n" );
	codegen_send( s, "JUMPIFEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@string\n" );
	codegen_send( s, "JUMPIFEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@bool\n" );
	codegen_send( s, "JUMPIFEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@nil\n" );
	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-ok\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-neq\n" );
	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-op2int GF@_control-type string@int\n" );
	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control2-type string@float\n" );
	codegen_send( s, "INT2FLOAT GF@_control GF@_control\n" );
	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-ok\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-op2int\n" );
	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control2-type string@int\n" );
	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@float\n" );
	codegen_send( s, "INT2FLOAT GF@_control2 GF@_control2\n" );

	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-ok\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-exit\n" );
	codegen_send( s, "EXIT int@4\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-ok\n" );
	codegen_send( s, "PUSHS GF@_control2\n" );
	codegen_send( s, "PUSHS GF@_control\n" );
	codegen_send( s, "MULS\n" );
}

void genc_divs( dynamic_string *s) {
	char label[64];
	sprintf( label, "divslabel%d", label_id++ );	// jedinečné navestie

	codegen_send( s, "POPS GF@_control\n" );
	codegen_send( s, "TYPE GF@_control-type GF@_control\n" );
	codegen_send( s, "POPS GF@_control2\n" );
	codegen_send( s, "TYPE GF@_control2-type GF@_control2\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-1false GF@_control-type string@int\n" );				// ak sa rovná typu int, pretypuj na float - inak pokračuj
	codegen_send( s, "INT2FLOAT GF@_control GF@_control\n" );
	codegen_send( s, "MOVE GF@_control-type string@float\n" );
	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-1false\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-2false GF@_control2-type string@int\n" );			// ak sa rovná typu int, pretypuj na float - inak pokračuj
	codegen_send( s, "INT2FLOAT GF@_control2 GF@_control2\n" );
	codegen_send( s, "MOVE GF@_control-type string@float\n" );
	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-2false\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type GF@_control2-type\n" );		// v tomto bode by sa obe hodnoty mali rovnať typu float - ak nie, ukonči error 4
	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@float\n" );				// kontrola, že sa ozaj jedná o float
	codegen_send( s, "JUMPIFEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control2 float@0x0p+0\n" );

	codegen_send( s, "PUSHS GF@_control2\n" );
	codegen_send( s, "PUSHS GF@_control\n" );
	codegen_send( s, "DIVS\n" );						// výpočet
	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-exit\n" );
	codegen_send( s, "EXIT int@4\n" );					// error 4
	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );
}

void genc_idivs( dynamic_string *s) {
	char label[64];
	sprintf( label, "idivslabel%d", label_id++ );

	codegen_send( s, "POPS GF@_control\n" );						// funkcia je síce zásobníková
	codegen_send( s, "TYPE GF@_control-type GF@_control\n" );		// ale aj tak treba skontrolovať správnosť dátového typu
	codegen_send( s, "POPS GF@_control2\n" );
	codegen_send( s, "TYPE GF@_control2-type GF@_control2\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type GF@_control2-type\n" );		// kontrola azda sú obe operandy rovnakého typu
	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@int\n" );				// v tejto chvíli vieme, že sú obi dva rovnakého typu - ak nie sú int, error 4
	codegen_send( s, "JUMPIFEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control2 int@0\n" );						// delenie nulou = error 4

	codegen_send( s, "PUSHS GF@_control2\n" );		// vráť hodnotu späť na dátový zásobník tak ako boli predávané pred kontrolou DT
	codegen_send( s, "PUSHS GF@_control\n" );
	codegen_send( s, "IDIVS\n" );					// výpočet
	codegen_send( s, "JUMP &" );
	codegen_send( s, label );						// konči
	codegen_send( s, "-return\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-exit\n" );
	codegen_send( s, "EXIT int@4\n" );				// error 4

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );
}

void genc_concats( dynamic_string *s ) {
	char label[64];
	sprintf( label, "concatslabel%d", label_id++ );		// jedinečné návestie

	codegen_send( s, "POPS GF@_control2\n" );
	codegen_send( s, "TYPE GF@_control2-type GF@_control2\n" );				// prakticky sme spravili zásobníkovú verziu CONCAT
	codegen_send( s, "POPS GF@_control\n" );
	codegen_send( s, "TYPE GF@_control-type GF@_control\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type GF@_control2-type\n" );		// kontrola dátových typov
	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@string\n" );			// ak sa nerovanjú reťazcu = error 4
	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-ok\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-exit\n" );				// v prípade nesprávneho dátového typu
	codegen_send( s, "EXIT int@4\n" );			// ukonči - error 4

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-ok\n" );
	codegen_send( s, "CONCAT GF@_control GF@_control GF@_control2\n" );		// konkatenuj
	codegen_send( s, "PUSHS GF@_control\n" );								// vlož výsledok na zásobník
}

/* ################################################################ Relácia ################################################################ */
void genc_gt( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 ) {	// greater than

	char label[64];
	sprintf( label, "gtlabel%d", label_id++ );		// jedinečné návestie

	codegen_send( s, "MOVE GF@_control " );
	codegen_symb( s, frameS1, symb1 );									// ulož zadaný parameter do globálnej premennej pre výrazy
	codegen_send( s, "\nTYPE GF@_control-type GF@_control\n" );			// a zisti jej dátový typ

	codegen_send( s, "MOVE GF@_control2 " );							// parameter 2 - ulož & zisti DT
	codegen_symb( s, frameS2, symb2 );
	codegen_send( s, "\nTYPE GF@_control2-type GF@_control2\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type GF@_control2-type\n" );	// kontrola DT

	codegen_send( s, "JUMPIFEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@nil\n" );

	codegen_send( s, "GT " );											// väčší než - >
	codegen_var ( s, frameV, var );
	codegen_send( s, " GF@_control GF@_control2\n" );
	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-exit\n" );			// error 4
	codegen_send( s, "EXIT int@4\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );				// koniec
	codegen_send( s, "-return\n" );
}

void genc_lt( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 ) {	// lesser than

	char label[64];
	sprintf( label, "ltlabel%d", label_id++ );

	codegen_send( s, "MOVE GF@_control " );
	codegen_symb( s, frameS1, symb1 );									// ulož zadaný parameter do globálnej premennej pre výrazy
	codegen_send( s, "\nTYPE GF@_control-type GF@_control\n" );			// a zisti jej dátový typ

	codegen_send( s, "MOVE GF@_control2 " );							// parameter 2 - ulož & zisti DT
	codegen_symb( s, frameS2, symb2 );
	codegen_send( s, "\nTYPE GF@_control2-type GF@_control2\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type GF@_control2-type\n" );		// kontrola DT

	codegen_send( s, "JUMPIFEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@nil\n" );

	codegen_send( s, "LT " );							// menší než - <
	codegen_var ( s, frameV, var );
	codegen_send( s, " GF@_control GF@_control2\n" );
	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-exit\n" );
	codegen_send( s, "EXIT int@4\n" );				// eror 4

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );						// koniec
	codegen_send( s, "-return\n" );
}

void genc_eq( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 ) {	// equal to

	char label[64];
	sprintf( label, "eqlabel%d", label_id++ );

	codegen_send( s, "MOVE GF@_control " );
	codegen_symb( s, frameS1, symb1 );									// ulož zadaný parameter do globálnej premennej pre výrazy
	codegen_send( s, "\nTYPE GF@_control-type GF@_control\n" );			// a zisti jej dátový typ

	codegen_send( s, "MOVE GF@_control2 " );							// parameter 2 - ulož & zisti DT
	codegen_symb( s, frameS2, symb2 );
	codegen_send( s, "\nTYPE GF@_control2-type GF@_control2\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type GF@_control2-type\n" );	// kontrola DT

	codegen_send( s, "EQ " );							// rovný - =
	codegen_var ( s, frameV, var );
	codegen_send( s, " GF@_control GF@_control2\n" );
	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-exit\n" );			// error 4
	codegen_send( s, "EXIT int@4\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );				// koniec
	codegen_send( s, "-return\n" );
}

void genc_ge( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 ) {	// greater or equal

	char label[64];
	sprintf( label, "gelabel%d", label_id++ );

	codegen_send( s, "MOVE GF@_control " );
	codegen_symb( s, frameS1, symb1 );									// ulož zadaný parameter do globálnej premennej pre výrazy
	codegen_send( s, "\nTYPE GF@_control-type GF@_control\n" );			// a zisti jej dátový typ

	codegen_send( s, "MOVE GF@_control2 " );							// parameter 2 - ulož & zisti DT
	codegen_symb( s, frameS2, symb2 );
	codegen_send( s, "\nTYPE GF@_control2-type GF@_control2\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type GF@_control2-type\n" );	// kontrola DT
	codegen_send( s, "JUMPIFEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@nil\n" );

	codegen_send( s, "GT GF@_control-res1 GF@_control GF@_control2\n" );	// väčší - >

	codegen_send( s, "EQ GF@_control-res2 GF@_control GF@_control2\n" );	// rovný - =

	codegen_send( s, "OR " );							// väčší alebo rovný - >=
	codegen_var ( s, frameV, var );
	codegen_send( s, " GF@_control-res1 GF@_control-res2\n" );

	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-exit\n" );			// error 4
	codegen_send( s, "EXIT int@4\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );				// koniec
	codegen_send( s, "-return\n" );
}

void genc_le( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 ) {	// lesser or equal

	char label[64];
	sprintf( label, "lelabel%d", label_id++ );		// taká istá ako genc_ge - ale výraz je menší alebo rovný <=

	codegen_send( s, "MOVE GF@_control " );
	codegen_symb( s, frameS1, symb1 );									// ulož zadaný parameter do globálnej premennej pre výrazy
	codegen_send( s, "\nTYPE GF@_control-type GF@_control\n" );			// a zisti jej dátový typ

	codegen_send( s, "MOVE GF@_control2 " );							// parameter 2 - ulož & zisti DT
	codegen_symb( s, frameS2, symb2 );
	codegen_send( s, "\nTYPE GF@_control2-type GF@_control2\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type GF@_control2-type\n" );	// kontrola DT
	codegen_send( s, "JUMPIFEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@nil\n" );

	codegen_send( s, "LT GF@_control-res1 GF@_control GF@_control2\n" );

	codegen_send( s, "EQ GF@_control-res2 GF@_control GF@_control2\n" );

	codegen_send( s, "OR " );
	codegen_var ( s, frameV, var );
	codegen_send( s, " GF@_control-res1 GF@_control-res2\n" );

	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-exit\n" );
	codegen_send( s, "EXIT int@4\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );
}

void genc_neq( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 ) {	// not equal

	char label[64];
	sprintf( label, "neqlabel%d", label_id++ );

	codegen_send( s, "MOVE GF@_control " );
	codegen_symb( s, frameS1, symb1 );									// ulož zadaný parameter do globálnej premennej pre výrazy
	codegen_send( s, "\nTYPE GF@_control-type GF@_control\n" );			// a zisti jej dátový typ

	codegen_send( s, "MOVE GF@_control2 " );							// parameter 2 - ulož & zisti DT
	codegen_symb( s, frameS2, symb2 );
	codegen_send( s, "\nTYPE GF@_control2-type GF@_control2\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type GF@_control2-type\n" );	// kontrola DT

	codegen_send( s, "EQ GF@_control-res1 GF@_control GF@_control2\n" );

	codegen_send( s, "NOT " );				// negácia výrazu a = b -> NOT(a=b)
	codegen_var ( s, frameV, var );
	codegen_send( s, " GF@_control-res1\n" );

	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-exit\n" );				// error 4
	codegen_send( s, "EXIT int@4\n" );

	codegen_send( s, "LABEL &" );				// koniec
	codegen_send( s, label );
	codegen_send( s, "-return\n" );
}

/* zásobníkové verzie */
void genc_gts( dynamic_string *s ) {

	char label[64];
	sprintf( label, "gtslabel%d", label_id++ );

	codegen_send( s, "POPS GF@_control2\n" );							// popni, zisti DT
	codegen_send( s, "POPS GF@_control\n" );
	codegen_send( s, "TYPE GF@_control-type GF@_control\n" );
	codegen_send( s, "TYPE GF@_control2-type GF@_control2\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type GF@_control2-type\n" );	// kontrola DT
	codegen_send( s, "JUMPIFEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@nil\n" );
	codegen_send( s, "PUSHS GF@_control\n" );							// vráť na zásobník
	codegen_send( s, "PUSHS GF@_control2\n" );
	codegen_send( s, "GTS\n" );						// väčší než - >
	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-exit\n" );			// error 4
	codegen_send( s, "EXIT int@4\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );				// koniec
	codegen_send( s, "-return\n" );
}

void genc_lts( dynamic_string *s ) {

	char label[64];
	sprintf( label, "ltslabel%d", label_id++ );

	codegen_send( s, "POPS GF@_control2\n" );								// popni, zisti DT
	codegen_send( s, "POPS GF@_control\n" );
	codegen_send( s, "TYPE GF@_control-type GF@_control\n" );
	codegen_send( s, "TYPE GF@_control2-type GF@_control2\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type GF@_control2-type\n" );		// kontrola DT
	codegen_send( s, "JUMPIFEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@nil\n" );
	codegen_send( s, "PUSHS GF@_control\n" );								// vráť na dátový zásobník
	codegen_send( s, "PUSHS GF@_control2\n" );
	codegen_send( s, "LTS\n" );							// menší než - <
	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-exit\n" );
	codegen_send( s, "EXIT int@4\n" );				// eror 4

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );						// koniec
	codegen_send( s, "-return\n" );
}

void genc_eqs( dynamic_string *s ) {

	char label[64];
	sprintf( label, "eqslabel%d", label_id++ );

	codegen_send( s, "POPS GF@_control2\n" );
	codegen_send( s, "POPS GF@_control\n" );							// POP, DT
	codegen_send( s, "TYPE GF@_control-type GF@_control\n" );
	codegen_send( s, "TYPE GF@_control2-type GF@_control2\n" );

	codegen_send( s, "JUMPIFNEQ &" );
    codegen_send( s, label );
    codegen_send( s, "-neq GF@_control-type GF@_control2-type\n" );		// kontrola dátových typov
	codegen_send( s, "JUMP &" );
    codegen_send( s, label );
    codegen_send( s, "-ok\n" );

	codegen_send( s, "LABEL &");
    codegen_send( s, label );
    codegen_send( s, "-neq\n" );
	codegen_send( s, "JUMPIFNEQ &" );
    codegen_send( s, label );
    codegen_send( s, "-op2nil GF@_control-type string@nil\n" );			// prvý operand None?
	codegen_send( s, "JUMP &" );
    codegen_send( s, label );
    codegen_send( s, "-ok\n" );

	codegen_send( s, "LABEL &" );
    codegen_send( s, label );
    codegen_send( s, "-op2nil\n" );
	codegen_send( s, "JUMPIFNEQ &");
    codegen_send( s, label );
    codegen_send( s, "-exit GF@_control2-type string@nil\n" );			// ak druhý nie je None tak error 4

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-ok\n" );
	
	codegen_send( s, "PUSHS GF@_control\n" );
	codegen_send( s, "PUSHS GF@_control2\n" );
	codegen_send( s, "EQS\n" );						// rovný - =
	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-exit\n" );			// error 4
	codegen_send( s, "EXIT int@4\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );				// koniec
	codegen_send( s, "-return\n" );
}

void genc_ges( dynamic_string *s ) {

	char label[64];
	sprintf( label, "geslabel%d", label_id++ );

	codegen_send( s, "POPS GF@_control2\n" );
	codegen_send( s, "POPS GF@_control\n" );							// POP, DT
	codegen_send( s, "TYPE GF@_control-type GF@_control\n" );
	codegen_send( s, "TYPE GF@_control2-type GF@_control2\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type GF@_control2-type\n" );	// kontrola DT
	codegen_send( s, "JUMPIFEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@nil\n" );

	codegen_send( s, "GT GF@_control-res1 GF@_control GF@_control2\n" );	// väčší - >

	codegen_send( s, "EQ GF@_control-res2 GF@_control GF@_control2\n" );	// rovný - =

	codegen_send( s, "OR GF@_control-res1 GF@_control-res1 GF@_control-res2\n" );

	codegen_send( s, "PUSHS GF@_control-res1\n" );

	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-exit\n" );			// error 4
	codegen_send( s, "EXIT int@4\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );				// koniec
	codegen_send( s, "-return\n" );
}

void genc_les( dynamic_string *s ) {

	char label[64];
	sprintf( label, "leslabel%d", label_id++ );		// taká istá ako genc_ges - ale výraz je menší alebo rovný <=

	codegen_send( s, "POPS GF@_control2\n" );
	codegen_send( s, "POPS GF@_control\n" );
	codegen_send( s, "TYPE GF@_control-type GF@_control\n" );
	codegen_send( s, "TYPE GF@_control2-type GF@_control2\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type GF@_control2-type\n" );
	codegen_send( s, "JUMPIFEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@nil\n" );

	codegen_send( s, "LT GF@_control-res1 GF@_control GF@_control2\n" );

	codegen_send( s, "EQ GF@_control-res2 GF@_control GF@_control2\n" );

	codegen_send( s, "OR GGF@_control-res1 GF@_control-res1 GF@_control-res2\n" );

	codegen_send( s, "PUSHS GF@_control-res1\n" );

	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-exit\n" );
	codegen_send( s, "EXIT int@4\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );
}

void genc_neqs( dynamic_string *s ) {

	char label[64];
	sprintf( label, "neqslabel%d", label_id++ );

	codegen_send( s, "POPS GF@_control2\n" );
	codegen_send( s, "POPS GF@_control\n" );							// POP, DT
	codegen_send( s, "TYPE GF@_control-type GF@_control\n" );
	codegen_send( s, "TYPE GF@_control2-type GF@_control2\n" );

	codegen_send( s, "JUMPIFNEQ &" );
    codegen_send( s, label );
    codegen_send( s, "-neq GF@_control-type GF@_control2-type\n" );		// kontrola dátových typov
	codegen_send( s, "JUMP &" );
    codegen_send( s, label );
    codegen_send( s, "-ok\n" );

	codegen_send( s, "LABEL &");
    codegen_send( s, label );
    codegen_send( s, "-neq\n" );
	codegen_send( s, "JUMPIFNEQ &" );
    codegen_send( s, label );
    codegen_send( s, "-op2nil GF@_control-type string@nil\n" );			// prvý operand je int a druhý float
	codegen_send( s, "JUMP &" );
    codegen_send( s, label );
    codegen_send( s, "-ok\n" );

	codegen_send( s, "LABEL &" );
    codegen_send( s, label );
    codegen_send( s, "-op2nil\n" );
	codegen_send( s, "JUMPIFNEQ &");
    codegen_send( s, label );
    codegen_send( s, "-exit GF@_control2-type string@nil\n" );			// druhý operand je int a druhý float

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-ok\n" );
	
	codegen_send( s, "EQ GF@_control-res1 GF@_control GF@_control2\n" );

	codegen_send( s, "NOT GF@_control-res1 GF@_control-res1\n" );

	codegen_send( s, "PUSHS GF@_control-res1\n" );

	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-exit\n" );				// error 4
	codegen_send( s, "EXIT int@4\n" );

	codegen_send( s, "LABEL &" );				// koniec
	codegen_send( s, label );
	codegen_send( s, "-return\n" );
}

/* ################################################################ Logika ################################################################ */

void genc_and( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 ) {

	char label[64];
	sprintf( label, "andlabel%d", label_id++ );

	codegen_send( s, "MOVE GF@_control " );
	codegen_symb( s, frameS1, symb1 );									// ulož zadaný parameter do globálnej premennej pre výrazy
	codegen_send( s, "\nTYPE GF@_control-type GF@_control\n" );			// a zisti jej dátový typ

	codegen_send( s, "MOVE GF@_control2 " );							// parameter 2 - ulož & zisti DT
	codegen_symb( s, frameS2, symb2 );
	codegen_send( s, "\nTYPE GF@_control2-type GF@_control2\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type GF@_control2-type\n" );	// kontrola DT
	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@bool\n" );

	codegen_send( s, "AND " );			// logický súčin
	codegen_var ( s, frameV, var );
	codegen_send( s, " GF@_control GF@_control2\n" );
	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-exit\n" );			// error 4
	codegen_send( s, "EXIT int@4\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );				// koniec
	codegen_send( s, "-return\n" );
}

void genc_or( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 ) {

	char label[64];
	sprintf( label, "orlabel%d", label_id++ );			// podobné ako genc_ands - ale výraz je logický súčet OR

	codegen_send( s, "MOVE GF@_control " );
	codegen_symb( s, frameS1, symb1 );									// ulož zadaný parameter do globálnej premennej pre výrazy
	codegen_send( s, "\nTYPE GF@_control-type GF@_control\n" );			// a zisti jej dátový typ

	codegen_send( s, "MOVE GF@_control2 " );							// parameter 2 - ulož & zisti DT
	codegen_symb( s, frameS2, symb2 );
	codegen_send( s, "\nTYPE GF@_control2-type GF@_control2\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type GF@_control2-type\n" );	// kontrola DT
	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@bool\n" );

	codegen_send( s, "OR " );			// logický súčet
	codegen_var ( s, frameV, var );
	codegen_send( s, " GF@_control GF@_control2\n" );
	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-exit\n" );
	codegen_send( s, "EXIT int@4\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );
}

void genc_not( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1 ) {

	char label[64];
	sprintf( label, "notslabel%d", label_id++ );		// logická negácia NOT - operandy iba typu bool

	codegen_send( s, "MOVE GF@_control " );
	codegen_symb( s, frameS1, symb1 );									// ulož zadaný parameter do globálnej premennej pre výrazy
	codegen_send( s, "\nTYPE GF@_control-type GF@_control\n" );			// a zisti jej dátový typ

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@bool\n" );

	codegen_send( s, "NOT " );			// logický súčin
	codegen_var ( s, frameV, var );
	codegen_send( s, " GF@_control\n" );
	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-exit\n" );
	codegen_send( s, "EXIT int@4\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );
}

/* Zásobníkové verzie */
void genc_ands( dynamic_string *s ) {

	char label[64];
	sprintf( label, "andslabel%d", label_id++ );

	codegen_send( s, "POPS GF@_control2\n" );
	codegen_send( s, "POPS GF@_control\n" );							// POP, DT
	codegen_send( s, "TYPE GF@_control-type GF@_control\n" );
	codegen_send( s, "TYPE GF@_control2-type GF@_control2\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type GF@_control2-type\n" );	// kontrola DT
	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@bool\n" );

	codegen_send( s, "PUSHS GF@_control\n" );			// vráť na zásobník
	codegen_send( s, "PUSHS GF@_control2\n" );

	codegen_send( s, "ANDS\n" );			// logický súčin
	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-exit\n" );			// error 4
	codegen_send( s, "EXIT int@4\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );				// koniec
	codegen_send( s, "-return\n" );
}

void genc_ors( dynamic_string *s ) {

	char label[64];
	sprintf( label, "orslabel%d", label_id++ );			// podobné ako genc_ands - ale výraz je logický súčet OR

	codegen_send( s, "POPS GF@_control2\n" );
	codegen_send( s, "POPS GF@_control\n" );
	codegen_send( s, "TYPE GF@_control-type GF@_control\n" );
	codegen_send( s, "TYPE GF@_control2-type GF@_control2\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type GF@_control2-type\n" );
	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@bool\n" );

	codegen_send( s, "PUSHS GF@_control\n" );
	codegen_send( s, "PUSHS GF@_control2\n" );

	codegen_send( s, "ORS\n" );
	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-exit\n" );
	codegen_send( s, "EXIT int@4\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );
}

void genc_nots( dynamic_string *s ) {

	char label[64];
	sprintf( label, "notslabel%d", label_id++ );		// logická negácia NOT - operandy iba typu bool

	codegen_send( s, "POPS GF@_control\n" );
	codegen_send( s, "TYPE GF@_control-type GF@_control\n" );

	codegen_send( s, "JUMPIFNEQ &" );
	codegen_send( s, label );
	codegen_send( s, "-exit GF@_control-type string@bool\n" );

	codegen_send( s, "PUSHS GF@_control\n" );

	codegen_send( s, "NOTS\n" );
	codegen_send( s, "JUMP &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-exit\n" );
	codegen_send( s, "EXIT int@4\n" );

	codegen_send( s, "LABEL &" );
	codegen_send( s, label );
	codegen_send( s, "-return\n" );
}

/* ################################################################ Práca s premennými ################################################################ */
void genc_define( dynamic_string *s, char frame, char* var ) {
	codegen_send( s, "DEFVAR " );			// definícia premennej
	codegen_var ( s, frame, var );
	codegen_send( s, "\n" );
}

void genc_move( dynamic_string *s, char frameV, char* var, char frameS, char* symb ) {
	codegen_send( s, "MOVE " );				// priradenie hodnoty do premennej
	codegen_var ( s, frameV, var );			// hodnota môže byť premenná alebo konštanta
	codegen_send( s, " " );
	codegen_symb( s, frameS, symb );
	codegen_send( s, "\n" );
}

/* ################################################################ Konverzia ################################################################ */
void genc_i2f( dynamic_string *s, char frameV, char* var, char frameS, char* symb ) {
	codegen_send( s, "INT2FLOAT " );	// pretypuj int na float
	codegen_var ( s, frameV, var );
	codegen_send( s, " " );
	codegen_symb( s, frameS, symb );
	codegen_send( s, "\n" );
}

void genc_f2i( dynamic_string *s, char frameV, char* var, char frameS, char* symb ) {
	codegen_send( s, "FLOAT2INT " );		// pretypuj float na int
	codegen_var ( s, frameV, var );
	codegen_send( s, " " );
	codegen_symb( s, frameS, symb );
	codegen_send( s, "\n" );
}

void genc_i2c( dynamic_string *s, char frameV, char* var, char frameS, char* symb ) {
	codegen_send( s, "INT2CHAR " );			// pretypuj celé číslo na znak rovný ordinálnej hodnote toho čísla
	codegen_var ( s, frameV, var );
	codegen_send( s, " " );
	codegen_symb( s, frameS, symb );
	codegen_send( s, "\n" );
}

void genc_s2i( dynamic_string *s, char frameV, char* var, char frameS1, char* symb1, char frameS2, char* symb2 ) {
	codegen_send( s, "STRI2INT " );			// vezme ordinálnu hodnotu znaku na indexe symb2 v reŤazci symb1
	codegen_var ( s, frameV, var );
	codegen_send( s, " " );
	codegen_symb( s, frameS1, symb1 );
	codegen_send( s, " " );
	codegen_symb( s, frameS2, symb2 );
	codegen_send( s, "\n" );
}

void genc_ifeq( dynamic_string *s, char* label, char* result ) {
    codegen_send( s, "JUMPIFEQ &" );		// porovná azda je zadaný výraz pravdivý - ak je, skočí na návestie
    codegen_send( s, "&");
    codegen_send( s, label );
    codegen_send( s, " ");
    codegen_send( s, result );
    codegen_send( s, " bool@true\n");
}

void genc_ifNeq( dynamic_string *s, char* label, char* result ) {
    codegen_send( s, "JUMPIFNEQ " );		// porovná azda je daný výraz nepravdivý - ak je, skočí
    codegen_send( s, "&");
    codegen_send( s, label );
    codegen_send( s, " ");
    codegen_send( s, result );
    codegen_send( s, " bool@true\n");
}

void genc_jump( dynamic_string *s, char* label ){
    codegen_send( s,"JUMP ");		// skoč na návestie
    codegen_send( s, "&");
    codegen_send( s, label );
    codegen_send( s, "\n" );
}
