#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

//velkost tabulky - prvocislo - efektivita hashovania je vtedy najvyssia
#define SIZE 12289


typedef enum{
	T_UNDEF, //this is None
  T_STRING,
	T_NONE,
  T_BOOL,	// Bonusove body  a = 5
	T_INT,
	T_FLOAT,
} type_item;

typedef struct tData{
	type_item type;			// typ itemu alebo return funkcie
	bool def;		    	// ak je definovany tak true else false
	char *id;		        // id = key
	bool function;			// jedna se o funkci
	unsigned params_count;		//pocet parametru funkce
} tData;

typedef struct symtable_item{
	char *key;              // id
  tData data;
	struct symtable_item *next; /// ptr to next item
} symtable_item;


typedef symtable_item *symtable[SIZE];

/***
*	Hashovacia funkcia nasej tabulky
***/
unsigned int symtable_hash_function(const char *str);

/***
*	Inicializacia tabulky
***/
void symtable_init(symtable *htab);

/***
*	Update data v prvku pod zadanym klicem key
***/
void symtable_update_data(symtable *htab, char *key, tData data);

/***
*	Vyhledání prvku v htab podle zadaného klíče key.  Pokud je
*	daný prvek nalezen, vrací se ukazatel na daný prvek. Pokud prvek nalezen není,
*	vrací se hodnota NULL.
***/
symtable_item* symtable_search(symtable* htab, const char *key);

/***
*	Tato funkcia vkladá do tabulky htab položku s klúčom key a s daty
*	data. Ak sa tam dany prvok s rovnakym klucom uz nachadza - nerobi nic.
***/
int symtable_insert(symtable *htab, char *key, tData data);

/***
*	Tato funkcia zruší všechny položky tabulky, korektně uvolní prostor,
*	který tyto položky zabíraly, a uvede tabulku do počátečního stavu.
***/
void symtable_clear(symtable *htab);
