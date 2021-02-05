#include "symtable.h"
#include "error.h"

unsigned int symtable_hash_function(const char *str){

    unsigned int h=0;
    const unsigned char *p;
    for(p=(const unsigned char*)str; *p!='\0'; p++)
    {
    	h = 65599*h + *p;
    }
    return h % SIZE;
}

void symtable_init(symtable *htab){

    for(unsigned int i = 0; i < SIZE; i++){
        (*htab)[i] = NULL;
    }
}

symtable_item* symtable_search(symtable* htab, const char *key) {

		int index = symtable_hash_function(key);
		symtable_item *found = (*htab)[index];
		while (found!=NULL){
			if (strcmp(found->key, key) == 0){
				return found;
			}
			found = found->next;
		}

		return NULL;
}

void symtable_update_data(symtable *htab, char *key, tData data){
  symtable_item *element;

  element = symtable_search(htab, key);
  if(element!=NULL)
  {
    element->data.type = data.type;
    element->data.def = data.def;
    element->data.params_count = data.params_count;
    element->data.function = data.function;
    element->data.id = data.id;
    element->next = NULL;
  }

}

int symtable_insert(symtable *htab, char *key, tData data){
    int index = symtable_hash_function(key);
    symtable_item *element;

    element = symtable_search(htab, key);
    if (element == NULL){
        symtable_item *new = malloc(sizeof(symtable_item));
        if (new == NULL){
            return INTERN_ERROR;
        }
        new->data.id = malloc(sizeof(strlen(data.id) + 1));
        new->key = malloc(sizeof(strlen(key) + 1));
        strcpy(new->key, key);
        new->data.type = data.type;
        new->data.def = data.def;
        new->data.params_count = data.params_count;
        new->data.function = data.function;
        strcpy(new->data.id, data.id);
        new->next = NULL;

        //ak existuju synonyma pridam na zaciatok
        if ((*htab)[index]!=NULL){
            new->next = (*htab)[index];
        }

        //novy prvok zarad do tabulky
        (*htab)[index] = new;
        return 0;
    }

        //ak sa tam nachadza prvok s rovnakym klucom
        //nerob nic

    return 1;
}


void symtable_clear ( symtable* htab ) {

		symtable_item *elem;
		symtable_item *elem_del;
		for(int i = 0; i < SIZE; i++){
			if ((*htab)[i] == NULL){
				continue;
			}
			else{
				elem = (*htab)[i];
				while (elem!=NULL){
					elem_del = elem;
					elem = elem->next;
					free(elem_del);
				} //while
				(*htab)[i]= NULL;
			} //else
		} //for

}
