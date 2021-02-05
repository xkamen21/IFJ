#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "dynamicstring.h"

//globalny stack na identy a dedenty
//tStack *stack;


typedef enum type_token{
    ID,             // 0 IDENTIFIKATOR
    KW,             // 1  KLUCOVE SLOVO - KEYWORD
    COMMENT,        // 2 Komentar

    STRING,         // 3
    INT,            // 4
    FLOAT,          // 5
    STRING_BLOCK,   // 6

    //OPERATORY
    PLUS,           // 7 +
    MINUS,          // 8 -
    MULTIPLY,       // 9 *
    DIVIDE,         // 10 /
    ASSIGN,         // 11 =    11
    NEGATION,       // 12 !
    DIV,            // 13 // div

    EQUAL,          // 14 ==
    GR_OR_EQ,       // 15 >=
    LE_OR_EQ,       // 16 <=
    LESS,           // 17 <
    GREATER,        // 18 >
    NOT_EQUAL,      // 19 !=


    RBRACKET,       // 20 )
    LBRACKET,       // 21 (
    QUOTE,          // 22 "
    COMMA,          // 23 ,
    COLON,          // 24 :

    EOL,            // 25
    EOF_T,          // 26

    INDENT,         // 27
    DEDENT,         // 28

    E_TOK,          // 29
    STOP,           // 30
} type_token;


/***
*   Struktura reprezentujuca token
***/
typedef struct Token{
    type_token type;
    dynamic_string string;
    int s_top;
    type_token e_type;
} Token;


/***
*   Funkcia na ziskanie tokenu
*   @param Token
*   @return 0 v pripade uspechu
*           LEX_ERROR v pripade chyby lexikalnej
*           alebo 99 v pripade internej
***/
int get_token(Token *t);

/***
*   Funkcia na overenie ci id nie je klucove slovo
*   @param nacitany string
*   @return -1 ak string nie je kw
*           0 ak string je kw = def
*           1 ak string je kw = else
*           2 ak string je kw = if
*           3 ak string je kw = None
*           4 ak string je kw = pass
*           5 ak string je kw = return
*           6 ak string je kw = while
***/
int is_id_kw(char *s);

/***
*   Funkcia na ziskanie tokenu
*   @param Token
*   @param type_token
*   @param string ktory ma byt priradeny tokenu
*   @param s_top - vrchol zasobniku
*   @return void
***/
void set_token(Token *t, type_token type, char *str, int s_top);

/***
*   Funkcia na priradenie operatora do tokenu
*   @param Token
*   @param char ktory je nacitany - v ktorom sa rozhodne ktory je to operator
*   @return void
***/
void choose_op(Token *t, char c);

/***
*   Funkcia ktora zisti ci je cislo v stringu float alebo int
*   @param str string v ktorom je ulozene cislo
*   @return 0 ak je int, 1 ak je float, -1 ak je zly format teda dve dvojbodky
***/
int float_or_int(char *str);
