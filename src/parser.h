#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "symtable.h"
#include "stack.h" // stack.h -> scanner.h

#include "codegen.h" // codegen -> dynamicstring.h -> error.h


extern dynamic_string out_;
extern dynamic_string main_;
extern tStackTok *ParamsStack;

#define SYNTAX_IS_FINE 0

typedef enum{
  IFWHILE, //in if or while cycle ending with :,
  ID_EXP, //after id or just standing alone
  FUNC_EXP, //in function like param ending with , or )
} prec_type;

typedef struct pData{
  Token token; //uloznea struktura token, muzu dohledat info o tokenu

  symtable global; //hashovaci tabulka pro ulozeni promennych a funkci
  symtable local;

  char *remeber_function; //token pro ulozeni nazvu funkce k zpetnemu urceni jejiho datoveho typu
  char *remeber_variable; //token pro ulozeni nazvu promenne k zpetnemu urceni jejiho datoveho typu

  unsigned params_count; //counter pro parametry funkce

  type_item type_function; //ulozi datovy typ funkce
  type_item type_id; //ulozi datovy typ promenne

  bool in_func; //jsem ve funkci, muzu dat return_value

  prec_type prec_item; //urceni pro precedent table kde se nachazi

  int if_counter; //counter pro sekvenci prikazu v if a while
} pData;

int realloc_and_copy_string(char *string, char *value);
int body(pData *data);
int body_after_indent(pData *data);
int after_id(pData *data);
int return_value(pData *data);
int params(pData *data);
int params_n(pData *data);
int def_params(pData *data);
int def_params_n(pData *data);

//********prectable.h*//
int input_token_type(Token *t, pData *data);

int prec_check(pData *data);


void copy_token(Token *to, Token from);
