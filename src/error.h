#define LEX_ERROR 1
#define SYNTAX_ERROR 2
#define SEM_UNDEF_ERROR 3
#define SEM_TYPE_ERROR 4
#define SEM_PARAM_ERROR 5
#define SEM_OTHER_ERROR 6
#define DIV_0_ERROR 9
#define INTERN_ERROR 99
/*
•1 - chyba v programu v rámci lexikální analýzy (chybná struktura aktuálního lexému,špatný počet mezer odsazení).
•2 - chyba v programu v rámci syntaktické analýzy (chybná syntaxe programu, neo-čekávaná změna úrovně odsazení).
•3 - sémantická chyba v programu – nedefinovaná funkce/proměnná, pokus o redefi-nici funkce/proměnné, atp.
•4 - sémantická/běhová chyba typové kompatibility v aritmetických, řetězcových arelačních výrazech.
•5 - sémantická chyba v programu – špatný počet parametrů u volání funkce.
•6 - ostatní sémantické chyby.
•9 - běhová chyba dělení nulou.
•99 - interní chyba překladače tj. neovlivněná vstupním programem (např. chyba alo-kace paměti, atd.).
*/
