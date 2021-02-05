//#include "scanner.h"
#include "parser.h"

int result;

dynamic_string out_;
dynamic_string main_;
int count_if = 49;
tStackTok *ParamsStack;

//globalny stack na identy a dedenty
extern tStack *stack;


/*funkcia pre kopirovanie tokenu*/
void copy_token(Token *to, Token from){
    to->type = from.type;
    to->s_top = from.s_top;
    to->e_type = from.e_type;

	if (to->string.size != 0){
		to->string.str[0] = '\0';
		free_dyn_string(&(to->string));
		to->string.size = 0;
	}

    int i = 0;
    //stringy musia mat rovnaku dlzku!!!
    while (from.string.str[i]!= '\0'){
        dyn_string(&(to->string), from.string.str[i]);
        i++;
    }

    to->string.size = from.string.size;

}




#define CHECK_COMMENT()\
    if(data->token.type == RBRACKET)\
        TOKEN()\
    if(data->token.type == COLON)\
        TOKEN()\
    if(data->token.type == DEDENT)\
        TOKEN()\
    if(data->token.type == EOL)\
        TOKEN()\
    while(data->token.type == COMMENT)\
        {\
            NEW_TOKEN_AND_CHECK(EOL)\
            TOKEN()\
        }

#define TOKEN()\
  result = get_token(&(data->token));\
  if(result == LEX_ERROR)\
  {fprintf(stderr, "LEX_ERROR\n");\
    return LEX_ERROR;}\
  if(result == INTERN_ERROR)\
  {fprintf(stderr, "INTERN_ERROR\n");\
    return INTERN_ERROR;}

#define CHECK(word)\
  if(data->token.type != word)\
  {\
    fprintf(stderr, "SYNTAX_ERROR\n");\
    return SYNTAX_ERROR;}

#define NEW_TOKEN_AND_CHECK(type)\
  TOKEN()\
  CHECK(type)

#define INSIDE_ANOTHER_FUNCTION_CHECK(name)\
  result = name(data);\
  if(result==LEX_ERROR)\
    {fprintf(stderr, "LEX_ERROR\n");\
    return LEX_ERROR;}\
  if(result==SYNTAX_ERROR)\
    {fprintf(stderr, "SYNTAX_ERROR\n");\
    return SYNTAX_ERROR;}\
  if(result==SEM_TYPE_ERROR)\
    {fprintf(stderr, "SEM_TYPE_ERROR\n");\
    return SEM_TYPE_ERROR;}\
  if(result==SEM_OTHER_ERROR)\
    {fprintf(stderr, "SEM_OTHER_ERROR\n");\
    return SEM_OTHER_ERROR;}\
  if(result==SEM_PARAM_ERROR)\
    {fprintf(stderr, "SEM_PARAM_ERROR\n");\
    return SEM_PARAM_ERROR;}\
  if(result==SEM_UNDEF_ERROR)\
    {fprintf(stderr, "SEM_UNDEF_ERROR\n");\
    return SEM_UNDEF_ERROR;}\
  if(result==DIV_0_ERROR)\
    {fprintf(stderr, "DIV_0_ERROR\n");\
    return DIV_0_ERROR;}\
  if(result==INTERN_ERROR)\
    {fprintf(stderr, "INTERN_ERROR\n");\
    return INTERN_ERROR;}

#define TOKEN_FREE()\
if(data->token.string.str != NULL)\
    free(data->token.string.str); //uvloneni naalokovane pameti dynamickeho string

#define CHECK_BUILT_IN_FUNC()\
    symtable_item *item = symtable_search(&data->global, data->remeber_function);\
    if(item != NULL)\
    {\
        do{\
            if(!strcmp(item->data.id, data->remeber_function))\
            {\
              break;\
          }\
            item = item->next;\
        }while(item != NULL);\
        if(item == NULL || item->data.def == T_UNDEF)\
        {return SEM_UNDEF_ERROR;}\
        if(data->params_count != item->data.params_count)\
        {fprintf(stderr, "SEM_PARAM_ERROR: spatne zadane parametry funkce %s\n", item->data.id);\
            return SEM_PARAM_ERROR;}\
    }\
    else{return SEM_UNDEF_ERROR;}



int realloc_and_copy_string(char *string, char *value);
int body(pData *data);
int body_after_indent(pData *data);
int after_id(pData *data);
int return_value(pData *data);
int params(pData *data);
int params_n(pData *data);
int def_params(pData *data);
int def_params_n(pData *data);



symtable_item *genc_glob_loc_search(pData *data, Token actual_tok)
{
    //Hledam v lokalni tabulce
    symtable_item *element = NULL;
    if(data->in_func)
    {
        element = symtable_search(&data->local, actual_tok.string.str);
        while(element != NULL){
            if(!strcmp(element->data.id, actual_tok.string.str))
            {
                break;
            }

            element = element->next;
        }
        if(element!=NULL)
        return element;
        else{
            return NULL;
        }
    }
    // Hledam v globalnni tabulce
    else
    {
        element = symtable_search(&data->global, actual_tok.string.str);
        while(element != NULL){
            if(!strcmp(element->data.id, actual_tok.string.str))
            {
                break;
            }

            element = element->next;
        }
        if(element!=NULL){
        return element;
       }
        else{
            return NULL;
        }
    }// end  if(data->in_func)
}

int genc_is_def(pData *data, Token actual_tok)
{
    if(actual_tok.type == ID)
    {
        symtable_item *element = genc_glob_loc_search(data, actual_tok);
        if(element != NULL)
        {
            if(element->data.def != true && element->data.function==false)
            {
                return SEM_UNDEF_ERROR;
            }
            else
            {
                return SYNTAX_IS_FINE;
            }
        }
        else
        {
            return SEM_UNDEF_ERROR;
        }
    }
    else
    {
        return SYNTAX_IS_FINE;
    }
}

int init_built_in_function(symtable *htab, char *name, int number_of_params)
{
    tData *data = malloc(sizeof(tData));
    data->id = malloc(sizeof(char));
    data->def = true;
    data->function = true;
    data->params_count = number_of_params;
    realloc_and_copy_string(data->id, name);
    if((symtable_insert(htab, name, *data))==1) //vlozeni prvku do hash table
    {
        free(data->id);
        free(data);
        return INTERN_ERROR;
    }
    free(data->id);
    free(data);
    return SYNTAX_IS_FINE;
}

int start_parser()
{
    pData *parser_data = malloc(sizeof(pData));
    if(parser_data == NULL)
    {
        return INTERN_ERROR;
    }
    symtable_init(&(parser_data->global));
    init_built_in_function(&(parser_data->global), "inputf", 0);
    init_built_in_function(&(parser_data->global), "inputi", 0);
    init_built_in_function(&(parser_data->global), "inputs", 0);
    init_built_in_function(&(parser_data->global), "substr", 3);
    init_built_in_function(&(parser_data->global), "ord", 2);
    init_built_in_function(&(parser_data->global), "chr", 1);
    init_built_in_function(&(parser_data->global), "len", 1);
    init_built_in_function(&(parser_data->global), "print", 0);
    symtable_init(&(parser_data->local));
    parser_data->remeber_function = malloc(sizeof(char));
    parser_data->remeber_variable = malloc(sizeof(char));
    if(parser_data->remeber_function == NULL || parser_data->remeber_variable == NULL)
    {
        return INTERN_ERROR;
    }
    result = body(parser_data);
    symtable_clear(&(parser_data->local));
    symtable_clear(&(parser_data->global));
    free(parser_data);
    return result;
}

int realloc_and_copy_string(char *string, char *value)
{
    string = realloc(string, sizeof(strlen(value) + 1));
    if(string == NULL)
    {
        return INTERN_ERROR;
    }
    else
    {
        strcpy(string, value);
    }
    return SYNTAX_IS_FINE;
}

int all_function_defined(pData *data)
{
    symtable_item *elem;
    for(int i = 0; i < SIZE; i++){
        if (data->global[i] == NULL){
            continue;
        }
        else{
            elem = data->global[i];
            while (elem!=NULL){
                if(elem->data.def != true)
                {
                    fprintf(stderr, "some function was not defined\n");
                    return SEM_UNDEF_ERROR;
                }
                elem = elem->next;
            }
        }
    }
    return SYNTAX_IS_FINE;
}


//############################################# MAIN BODY #########################################################

int body(pData *data)
{

    data->if_counter = 0;
    TOKEN()
    bool found = false; //pomocna promena pro nalezeni ve symtable
    data->in_func = false;
    //printf("token - body: %d %s\n", data->token.type, data->token.string.str);
    //<body_after_indent> -> def function_id ( <def_params> ) : EOL INDENT <body_in_function> DEDENT <body_after_indent>
    if(data->token.type == KW && !strcmp(data->token.string.str, "def")) //strcmp returned 0 when its true
    {
        NEW_TOKEN_AND_CHECK(ID)
        realloc_and_copy_string(data->remeber_function, data->token.string.str);
        //data->remeber_function = data->token; //uchovani tokenu pro zpetnou definici typu (string, int, atd..)
        symtable_item *item = symtable_search(&data->global, data->token.string.str);

        if(item != NULL)
        {
            do{
                if(!strcmp(item->data.id, data->token.string.str))
                {
                  found = true; //item byl nalezen v symtable
                  break;
                }

                item = item->next;
            }while(item != NULL);
        }

        if(found)
        {
            if(item->data.def == true)
            {
                fprintf(stderr, "SEM_UNDEF_ERROR: redefinice funkce\n");
                return SEM_UNDEF_ERROR;
            }
            NEW_TOKEN_AND_CHECK(LBRACKET)
            INSIDE_ANOTHER_FUNCTION_CHECK(def_params)
            if(data->params_count != item->data.params_count)
            {
                fprintf(stderr, "SEM_PARAM_ERROR: spatne zadane parametry funkce %s\n", item->data.id);
                return SEM_PARAM_ERROR;
            }
            CHECK(RBRACKET)
            NEW_TOKEN_AND_CHECK(COLON)
            CHECK_COMMENT()
            CHECK(INDENT)

            //nejspise CODE_GEN, mame dostatek informaci k tomu aby jsme mohli generovat, vse najdu v struct pData *data

            data->in_func = true;
            INSIDE_ANOTHER_FUNCTION_CHECK(body_after_indent)

            item->data.type = data->type_function; //vlozeni datoveho typu funkce do symtable

            CHECK(DEDENT)

            //nejspise CODE_GEN uzavreni definice funkce


            item->data.def = true; //nastvani definice na tru ve symtable
            data->in_func = false;

            symtable_clear(&(data->local));
            return body(data);
        }
        else
        {

            tData *tmp = malloc(sizeof(struct tData)); //pomocna pro ukladani dat do symtable
            if(tmp == NULL)
            {
                fprintf(stderr, "INTERN_ERROR: chyba allokace\n");
                return INTERN_ERROR;
            }
            tmp->id = malloc(strlen(data->remeber_function)+1); //need to alloc string
            if(tmp->id == NULL)
            {
                fprintf(stderr, "INTERN_ERROR: chyba allokace\n");
                return INTERN_ERROR;
            }

            /*******************************/
            /*******************************/
            /********** GENERATOR **********/
            /*******************************/
            /*******************************/
            genc_label(&out_, data->remeber_function );
            genc_begin(&out_);
            /*******************************/

            NEW_TOKEN_AND_CHECK(LBRACKET)
            INSIDE_ANOTHER_FUNCTION_CHECK(def_params)
            tmp->params_count = data->params_count;
            CHECK(RBRACKET)
            NEW_TOKEN_AND_CHECK(COLON)
            CHECK_COMMENT()
            CHECK(INDENT)

            //nejspise CODE_GEN, mame dostatek informaci k tomu aby jsme mohli generovat, vse najdu v struct pData *data

            data->in_func = true;
            INSIDE_ANOTHER_FUNCTION_CHECK(body_after_indent)
            CHECK(DEDENT)

            /*******************************/
            /*******************************/
            /********** GENERATOR **********/
            /*******************************/
            /*******************************/
            genc_end(&out_);
            genc_return(&out_);
            /*******************************/

            tmp->type = data->type_function;
            tmp->def = true;
            tmp->function = true;
            strcpy(tmp->id, data->remeber_function); //ulozeni nazvu funkce (identifier)
            symtable_insert(&(data->global), data->remeber_function, *tmp); //vlozeni prvku do hash table
            data->in_func = false;
            symtable_clear(&(data->local));
            free(tmp->id);
            free(tmp);
            return body(data);
        }
    }

    else if(data->token.type == STRING_BLOCK)
    {
        return body(data);
    }
    //<body>    -> id <after_id> EOL <body>
    else if(data->token.type == ID)
    {
        //data->remeber_variable = data->token;


        realloc_and_copy_string(data->remeber_variable, data->token.string.str);

        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/

        if (genc_is_def(data, data->token)==SEM_UNDEF_ERROR)
            genc_define(&main_,'G', (data->token.string.str));

        /*******************************/
        INSIDE_ANOTHER_FUNCTION_CHECK(after_id)
        if(data->token.type == RBRACKET)
            TOKEN()
        CHECK(EOL)
        return body(data);
    }

    //<body>    -> KW_function ( <params> )
    else if(data->token.type == KW && (!strcmp(data->token.string.str, "inputf") || !strcmp(data->token.string.str, "inputi") || !strcmp(data->token.string.str, "inputs")))
    {
        if(!strcmp(data->token.string.str, "inputf"))
        {
            realloc_and_copy_string(data->remeber_function, "inputf");
        }
        else if(!strcmp(data->token.string.str, "inputi"))
        {
            realloc_and_copy_string(data->remeber_function, "inputi");
        }
        else
        {
            realloc_and_copy_string(data->remeber_function, "inputs");
        }
        realloc_and_copy_string(data->remeber_function, data->token.string.str);
        NEW_TOKEN_AND_CHECK(LBRACKET)
        INSIDE_ANOTHER_FUNCTION_CHECK(params)
        CHECK_BUILT_IN_FUNC()
        CHECK(RBRACKET)
        return body(data);
    }

    else if(data->token.type == KW && !strcmp(data->token.string.str, "substr"))
    {
        realloc_and_copy_string(data->remeber_function, data->token.string.str);
        NEW_TOKEN_AND_CHECK(LBRACKET)
        INSIDE_ANOTHER_FUNCTION_CHECK(params)
        CHECK_BUILT_IN_FUNC()
        CHECK(RBRACKET)
        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/
        if (data->in_func) {
            genc_call(&out_, "substr");
        }
        else{
            genc_call(&main_, "substr");
        }
        /*******************************/
        return body(data);
    }

    else if(data->token.type == KW && !strcmp(data->token.string.str, "chr"))
    {
        realloc_and_copy_string(data->remeber_function, data->token.string.str);
        NEW_TOKEN_AND_CHECK(LBRACKET)
        INSIDE_ANOTHER_FUNCTION_CHECK(params)
        CHECK_BUILT_IN_FUNC()
        CHECK(RBRACKET)
        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/
        if (data->in_func) {
            genc_call(&out_, "chr");
        }
        else{
            genc_call(&main_, "chr");
        }
        /*******************************/

        return body(data);
    }

    else if(data->token.type == KW && !strcmp(data->token.string.str, "ord"))
    {
        realloc_and_copy_string(data->remeber_function, data->token.string.str);
        NEW_TOKEN_AND_CHECK(LBRACKET)
        INSIDE_ANOTHER_FUNCTION_CHECK(params)
        CHECK_BUILT_IN_FUNC()
        CHECK(RBRACKET)
        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/
        if (data->in_func) {
            genc_call(&out_, "ord");
        }
        else{
            genc_call(&main_, "ord");
        }
        /*******************************/
        return body(data);
    }

    else if(data->token.type == KW && !strcmp(data->token.string.str, "len"))
    {
        realloc_and_copy_string(data->remeber_function, data->token.string.str);
        NEW_TOKEN_AND_CHECK(LBRACKET)
        INSIDE_ANOTHER_FUNCTION_CHECK(params)
        CHECK_BUILT_IN_FUNC()
        CHECK(RBRACKET)
        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/
        if (data->in_func) {
            genc_call(&out_, "len");
        }
        else{
            genc_call(&main_, "len");
        }
        /*******************************/

        return body(data);
    }

    else if(data->token.type == KW && !strcmp(data->token.string.str, "print"))
    {
        realloc_and_copy_string(data->remeber_function, data->token.string.str);
        NEW_TOKEN_AND_CHECK(LBRACKET)
        INSIDE_ANOTHER_FUNCTION_CHECK(params)
        CHECK(RBRACKET)
        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/
        char par_cnt[5] = {'\0','\0','\0','\0','\0'};
        sprintf(par_cnt, "%d", data->params_count);


        if (data->in_func) {
            genc_pushs(&out_, 'I', par_cnt);
            genc_call(&out_, "print");
        }
        else{
            genc_pushs(&main_, 'I', par_cnt);
            genc_call(&main_, "print");
        }
        /*******************************/
        return body(data);
    }
    //<body>    -> <expression> <body>

    //TODO waiting for prectable


    //<body>    -> if <condition> : EOL INDENT <body_after_indent> DEDENT else : INDENT <body_after_indent> DEDENT <body>
    else if(data->token.type == KW && !strcmp(data->token.string.str, "if"))
    {
        data->if_counter = 0;
        data->prec_item = IFWHILE;
        TOKEN()
        INSIDE_ANOTHER_FUNCTION_CHECK(prec_check)
        CHECK(COLON)
        CHECK_COMMENT()

        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/


        char lbl[] = {'L','B','L', count_if, '\0'};
        char end[] = {'E','N','D', count_if, '\0'};
        count_if++;

        if (data->in_func) {
            genc_pops(&out_,'G', "res_express");
            genc_ifNeq(&out_, lbl, "GF@_res_express");

        }
        else{
            genc_pops(&main_,'G', "res_express");
            genc_ifNeq(&main_, lbl, "GF@_res_express");

        }
        /*******************************/


        CHECK(INDENT)
        INSIDE_ANOTHER_FUNCTION_CHECK(body_after_indent)
        if(!data->if_counter)
        {
            return SEM_OTHER_ERROR;
        }
        data->if_counter = 0;
        CHECK(DEDENT)
        CHECK_COMMENT()

        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/
        if (data->in_func) {
            genc_jump(&out_, end);
        }
        else{
            genc_jump(&main_, end);
        }
        /*******************************/

        if(data->token.type != KW)
        {
            TOKEN()
        }
        if(data->token.type != KW || strcmp(data->token.string.str, "else"))
        {
            fprintf(stderr, "SYNTAX_ERROR: neprisel else statement\n");
            return SYNTAX_ERROR;
        }
        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/
        if (data->in_func) {
            genc_label(&out_,lbl);
        }
        else{
            genc_label(&main_,lbl);
        }
        /*******************************/
        NEW_TOKEN_AND_CHECK(COLON)
        CHECK_COMMENT()
        CHECK(INDENT)
        INSIDE_ANOTHER_FUNCTION_CHECK(body_after_indent)
        if(!data->if_counter)
        {
            return SEM_OTHER_ERROR;
        }
        CHECK(DEDENT)
        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/
        genc_label(&main_,end);
        /*******************************/
        return body(data);
    }

    //<body>    -> while <condition> : EOL INDENT <body_after_indent> DEDENT <body>
    else if(data->token.type == KW && !strcmp(data->token.string.str, "while"))
    {
        data->prec_item = IFWHILE;
        TOKEN()
        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/

        char whil[] = {'W','H','I', count_if, '\0'};
        if (data->in_func) {
            genc_label(&out_, whil);
        }
        else{
            genc_label(&main_, whil);
        }
        /*******************************/
        INSIDE_ANOTHER_FUNCTION_CHECK(prec_check)
        CHECK(COLON)
        CHECK_COMMENT()
        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/


        char end0[] = {'E','N','D', count_if, '\0'};
        count_if++;

        if (data->in_func) {
            genc_pops(&out_,'G', "res_express");
            genc_ifNeq(&out_, end0, "GF@_res_express");
        }
        else{
            genc_pops(&main_,'G', "res_express");
            genc_ifNeq(&main_, end0, "GF@_res_express");
        }
        /*******************************/
        CHECK(INDENT)
        INSIDE_ANOTHER_FUNCTION_CHECK(body_after_indent)
        CHECK(DEDENT)
        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/
        if (data->in_func) {
            genc_jump(&out_, whil);
            genc_label(&out_, end0);
        }
        else{
            genc_jump(&main_, whil);
            genc_label(&main_, end0);
        }
        /*******************************/
        return body(data);
    }

    else if(data->token.type == INT || data->token.type == FLOAT || data->token.type == STRING || data->token.type == LBRACKET)
    {
        data->prec_item = ID_EXP;
        INSIDE_ANOTHER_FUNCTION_CHECK(prec_check)
        return body(data);
    }

    else if(data->token.type == COMMENT)
    {
        NEW_TOKEN_AND_CHECK(EOL)
        return body(data);
    }

    //<body>    -> pass EOL <body>
    else if(data->token.type == KW && !strcmp(data->token.string.str, "pass"))
    {
        return body(data);
    }

    //<body>    -> EOL <body>
    else if(data->token.type == EOL)
    {
        free(data->token.string.str);
        return body(data);
    }

    //<body>    -> EOF
    else if(data->token.type == EOF_T)
    {

        free(data->token.string.str);
        return all_function_defined(data);
    }

    else
    {
        //fprintf(stderr, "body function got wrong token");
        return SYNTAX_ERROR;
    }

    return SYNTAX_IS_FINE;
}



int body_after_indent(pData *data)
{
    TOKEN()
    //printf("token - body_after_indent: %d\n", data->token.type);
    data->type_function = T_NONE;

    if(data->token.type == ID)
    {
        data->if_counter++;

        //data->remeber_variable = data->token;
        realloc_and_copy_string(data->remeber_variable, data->token.string.str);

        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/
        if (data->in_func){
            if (genc_is_def(data, data->token)==SEM_UNDEF_ERROR)
                genc_define(&out_,'L', (data->token.string.str));
        }
        else{
            if (genc_is_def(data, data->token)==SEM_UNDEF_ERROR)
                genc_define(&main_,'G', (data->token.string.str));
        }
        /*******************************/


        INSIDE_ANOTHER_FUNCTION_CHECK(after_id)

        CHECK(EOL)
        return body_after_indent(data);
    }
    //<body>    -> KW_function ( <params> )
    else if(data->token.type == KW && (!strcmp(data->token.string.str, "inputf") || !strcmp(data->token.string.str, "inputi") || !strcmp(data->token.string.str, "inputs")))
    {
        data->if_counter++;
        if(!strcmp(data->token.string.str, "inputf"))
        {
            realloc_and_copy_string(data->remeber_function, "inputf");
        }
        else if(!strcmp(data->token.string.str, "inputi"))
        {
            realloc_and_copy_string(data->remeber_function, "inputi");
        }
        else
        {
            realloc_and_copy_string(data->remeber_function, "inputs");
        }
        realloc_and_copy_string(data->remeber_function, data->token.string.str);
        NEW_TOKEN_AND_CHECK(LBRACKET)
        INSIDE_ANOTHER_FUNCTION_CHECK(params)
        CHECK_BUILT_IN_FUNC()
        CHECK(RBRACKET)
        return body_after_indent(data);
    }

    else if(data->token.type == KW && !strcmp(data->token.string.str, "substr"))
    {
        realloc_and_copy_string(data->remeber_function, data->token.string.str);
        data->if_counter++;
        NEW_TOKEN_AND_CHECK(LBRACKET)
        INSIDE_ANOTHER_FUNCTION_CHECK(params)
        CHECK_BUILT_IN_FUNC()
        CHECK(RBRACKET)
        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/
        if (data->in_func) {
            genc_call(&out_, "substr");
        }
        else{
            genc_call(&main_, "substr");
        }
        /*******************************/
        return body_after_indent(data);
    }

    else if(data->token.type == KW && !strcmp(data->token.string.str, "chr"))
    {
        data->if_counter++;
        realloc_and_copy_string(data->remeber_function, data->token.string.str);
        NEW_TOKEN_AND_CHECK(LBRACKET)
        INSIDE_ANOTHER_FUNCTION_CHECK(params)
        CHECK_BUILT_IN_FUNC()
        CHECK(RBRACKET)
        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/
        if (data->in_func) {
            genc_call(&out_, "chr");
        }
        else{
            genc_call(&main_, "chr");
        }
        /*******************************/
        return body_after_indent(data);
    }

    else if(data->token.type == KW && !strcmp(data->token.string.str, "ord"))
    {
        data->if_counter++;
        realloc_and_copy_string(data->remeber_function, data->token.string.str);
        NEW_TOKEN_AND_CHECK(LBRACKET)
        INSIDE_ANOTHER_FUNCTION_CHECK(params)
        CHECK_BUILT_IN_FUNC()
        CHECK(RBRACKET)
        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/
        if (data->in_func) {
            genc_call(&out_, "ord");
        }
        else{
            genc_call(&main_, "ord");
        }
        /*******************************/
        return body_after_indent(data);
    }

    else if(data->token.type == KW && !strcmp(data->token.string.str, "len"))
    {
        data->if_counter++;
        realloc_and_copy_string(data->remeber_function, data->token.string.str);
        NEW_TOKEN_AND_CHECK(LBRACKET)
        INSIDE_ANOTHER_FUNCTION_CHECK(params)
        CHECK_BUILT_IN_FUNC()
        CHECK(RBRACKET)
        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/
        if (data->in_func) {
            genc_call(&out_, "len");
        }
        else{
            genc_call(&main_, "len");
        }
        /*******************************/

        return body_after_indent(data);
    }

    else if(data->token.type == KW && !strcmp(data->token.string.str, "print"))
    {
        realloc_and_copy_string(data->remeber_function, data->token.string.str);
        data->if_counter++;
        NEW_TOKEN_AND_CHECK(LBRACKET)
        INSIDE_ANOTHER_FUNCTION_CHECK(params)
        CHECK(RBRACKET)
        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/
        char par_cnt[5] = {'\0','\0','\0','\0','\0'};
        sprintf(par_cnt, "%d", data->params_count);


        if (data->in_func) {
            genc_pushs(&out_, 'I', par_cnt);
            genc_call(&out_, "print");
        }
        else{
            genc_pushs(&main_, 'I', par_cnt);
            genc_call(&main_, "print");
        }
        /*******************************/
        return body_after_indent(data);
    }
    //<body>    -> <expression> <body>

    //TODO waiting for prectable

    //<body>    -> if <condition> : EOL INDENT <body_after_indent> DEDENT <body>
    else if(data->token.type == KW && !strcmp(data->token.string.str, "if"))
    {
        data->if_counter = 0;
        //INSIDE_ANOTHER_FUNCTION_CHECK(expression) //condition
        TOKEN()
        data->prec_item = IFWHILE;
        INSIDE_ANOTHER_FUNCTION_CHECK(prec_check)
        CHECK(COLON)
        CHECK_COMMENT()
        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/


        char lbl_[] = {'L','B','L', count_if, '\0'};
        char end_[] = {'E','N','D', count_if, '\0'};
        count_if++;

        if (data->in_func) {
            genc_pops(&out_,'G', "res_express");
            genc_ifNeq(&out_, lbl_, "GF@_res_express");

        }
        else{
            genc_pops(&main_,'G', "res_express");
            genc_ifNeq(&main_, lbl_, "GF@_res_express");

        }
        /*******************************/

        CHECK(INDENT)
        INSIDE_ANOTHER_FUNCTION_CHECK(body_after_indent)
        if(!data->if_counter)
        {
            return SEM_OTHER_ERROR;
        }
        data->if_counter = 0;
        CHECK(DEDENT)
        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/
        if (data->in_func) {
            genc_jump(&out_, end_);
        }
        else{
            genc_jump(&main_, end_);
        }
        /*******************************/
        TOKEN()
        if(data->token.type != KW || strcmp(data->token.string.str, "else"))
        {
            fprintf(stderr, "SYNTAX_ERROR: neprisel else statement\n");
            return SYNTAX_ERROR;
        }
        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/
        if (data->in_func) {
            genc_label(&out_,lbl_);
        }
        else{
            genc_label(&main_,lbl_);
        }
        /*******************************/
        NEW_TOKEN_AND_CHECK(COLON)
        CHECK_COMMENT()
        CHECK(INDENT) //nesjspi predelat vsechny NEW_TOKEN_AND_CHECK() na jen CHECK() abych v makru CHECK_COMMENT() mohl nacitay i dalsi token a zjistil zda neprichazi dalsi COMMENT
        INSIDE_ANOTHER_FUNCTION_CHECK(body_after_indent)
        if(!data->if_counter)
        {
            return SEM_OTHER_ERROR;
        }
        CHECK(DEDENT)
        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/
        if (data->in_func) {
            genc_label(&out_,end_);
        }
        else{
            genc_label(&main_,end_);
        }

        /*******************************/
        return body_after_indent(data);
    }
    //<body>    -> while <condition> : EOL INDENT <body_after_indent> DEDENT <body>
    else if(data->token.type == KW && !strcmp(data->token.string.str, "while"))
    {
        //INSIDE_ANOTHER_FUNCTION_CHECK(expression) //condition
        data->if_counter++;
        data->prec_item = IFWHILE;
        TOKEN()
        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/

        char whil[] = {'W','H','I', count_if, '\0'};
        if (data->in_func) {
            genc_label(&out_, whil);
        }
        else{
            genc_label(&main_, whil);
        }
        /*******************************/
        INSIDE_ANOTHER_FUNCTION_CHECK(prec_check)
        CHECK(COLON)
        CHECK_COMMENT()
        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/


        char end0[] = {'E','N','D', count_if, '\0'};
        count_if++;

        if (data->in_func) {
            genc_pops(&out_,'G', "res_express");
            genc_ifNeq(&out_, end0, "GF@_res_express");
        }
        else{
            genc_pops(&main_,'G', "res_express");
            genc_ifNeq(&main_, end0, "GF@_res_express");
        }
        /*******************************/
        CHECK(INDENT)
        INSIDE_ANOTHER_FUNCTION_CHECK(body_after_indent)
        CHECK(DEDENT)
        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/
        if (data->in_func) {
            genc_jump(&out_, whil);
            genc_label(&out_, end0);
        }
        else{
            genc_jump(&main_, whil);
            genc_label(&main_, end0);
        }
        /*******************************/
        return body_after_indent(data);
    }

    //<body>    -> pass EOL <body>
    else if(data->token.type == KW && !strcmp(data->token.string.str, "pass"))
    {
        data->if_counter++;
        return body_after_indent(data);
    }

    //<body_after_indent> -> return <return_value>
    else if(data->token.type == KW && !strcmp(data->token.string.str, "return"))
    {
        //bool fine = false;
        if(!(data->in_func))
        {
            return SYNTAX_ERROR;
        }
        INSIDE_ANOTHER_FUNCTION_CHECK(return_value)
        CHECK(EOL)

        return body_after_indent(data);
    }

    else if(data->token.type == INT || data->token.type == FLOAT || data->token.type == STRING || data->token.type == LBRACKET)
    {
        data->if_counter = 0;
        data->prec_item = ID_EXP;
        INSIDE_ANOTHER_FUNCTION_CHECK(prec_check)
        return body_after_indent(data);
    }

    else if(data->token.type == COMMENT)
    {
        NEW_TOKEN_AND_CHECK(EOL)
        return body_after_indent(data);
    }

    //<body>    -> EOL <body>
      else if(data->token.type == EOL)
      {
          return body_after_indent(data);
      }

      else if(data->token.type == DEDENT)
      {

          return SYNTAX_IS_FINE;
      }
      else
      {
          return SYNTAX_ERROR;
      }

      return SYNTAX_IS_FINE;
}



int return_value(pData *data)
{
    //printf("token - return_value %d\n", data->token.type);
    TOKEN()

    //<return_value> -> ε
    if(data->token.type == EOL)
    {
        data->type_function = T_NONE; //zde ma byt None
        return SYNTAX_IS_FINE;
    }

    else if(data->token.type == COMMENT)
    {
        data->type_function = T_NONE; //zde ma byt None
        NEW_TOKEN_AND_CHECK(EOL)
        return SYNTAX_IS_FINE;
    }

    //<return_value> -> expression
    else if(data->token.type == ID || data->token.type == INT || data->token.type == FLOAT || data->token.type == STRING || data->token.type == STRING_BLOCK)
    {
        data->prec_item = ID_EXP;
        INSIDE_ANOTHER_FUNCTION_CHECK(prec_check)
        CHECK(EOL)
        data->type_function = data->type_id;
        return SYNTAX_IS_FINE;
    }
    else
    {
        return SYNTAX_ERROR;
    }

    /*******************************/
    /*******************************/
    /********** GENERATOR **********/
    /*******************************/
    /*******************************/
    genc_end(&out_);
    genc_return(&out_);
    /*******************************/
    return SYNTAX_IS_FINE;
}


int params(pData *data)
{
    TOKEN()
    //printf("token - params %d\n", data->token.type);
    data->params_count = 0;
    //<params>  -> ε
    if(data->token.type == RBRACKET)
    {
        data->params_count = 0;
        return SYNTAX_IS_FINE;
    }

    //<params>  -> id <params_n>
    else if(data->token.type == ID)
    {
        data->params_count = 1;
        symtable_item *ID_found = genc_glob_loc_search(data, data->token);
        if(ID_found==NULL || ID_found->data.def != true)
        {
            fprintf(stderr, "SEM_UNDEF_ERROR\n");
            return SEM_UNDEF_ERROR;
        }
        ID_found = NULL;

       stackTokPush(ParamsStack, data->token);


        INSIDE_ANOTHER_FUNCTION_CHECK(params_n)

        return SYNTAX_IS_FINE;
    }

    else if(data->token.type == INT || data->token.type == FLOAT || data->token.type == STRING || data->token.type == LBRACKET || data->token.type == STRING_BLOCK || !strcmp(data->token.string.str,"None") )
    {
        data->prec_item = FUNC_EXP;
        INSIDE_ANOTHER_FUNCTION_CHECK(prec_check)
        data->params_count = 1;


        INSIDE_ANOTHER_FUNCTION_CHECK(params_n)
    }

    else
    {
        return SYNTAX_ERROR;
    }
    return SYNTAX_IS_FINE;
}



int params_n(pData *data)
{
    Token control_dno;
    Token control;


    if(data->token.type == RBRACKET)
    {
        control_dno.string.size = 0;

        control_dno.type = COLON;
        control_dno.s_top = 0;

        control.string.size = 0;

        stackTokTop(ParamsStack, &control);

        while (control.type != control_dno.type){
            if (data->in_func){
                if (control.e_type == INT)
                    genc_pushs(&out_, 'I', control.string.str);
                else if (control.e_type == FLOAT)
                    genc_pushs(&out_, 'F', control.string.str);
                else if (control.e_type == ID)
                    genc_pushs(&out_, 'L', control.string.str);
                else if ((control.e_type == STRING) || (control.e_type == STRING_BLOCK))
                    genc_pushs(&out_, 'S', control.string.str);
            }
            else{

                if (control.e_type == INT)
                    genc_pushs(&main_, 'I', control.string.str);
                else if (control.e_type == FLOAT)
                    genc_pushs(&main_, 'F', control.string.str);
                else if (control.e_type == ID)
                    genc_pushs(&main_, 'G', control.string.str);
                else if ((control.e_type == STRING) || (control.e_type == STRING_BLOCK))
                    genc_pushs(&main_, 'S', control.string.str);
            }
            stackTokPop(ParamsStack);
            stackTokTop(ParamsStack, &control);
        }

        return SYNTAX_IS_FINE;
    }
    if(data->token.type != COMMA)
        TOKEN()
    fprintf(stderr,"token - params_n %d\n", data->token.type);

    if(data->token.type == RBRACKET)
    {
        control_dno.string.size = 0;

        control_dno.type = COLON;
        control_dno.s_top = 0;

        control.string.size = 0;

        stackTokTop(ParamsStack, &control);

        while (control.type != control_dno.type){
            if (data->in_func){
                if (control.e_type == INT)
                    genc_pushs(&out_, 'I', control.string.str);
                else if (control.e_type == FLOAT)
                    genc_pushs(&out_, 'F', control.string.str);
                else if (control.e_type == ID)
                    genc_pushs(&out_, 'L', control.string.str);
                else if ((control.e_type == STRING) || (control.e_type == STRING_BLOCK))
                    genc_pushs(&out_, 'S', control.string.str);
            }
            else{

                if (control.e_type == INT)
                    genc_pushs(&main_, 'I', control.string.str);
                else if (control.e_type == FLOAT)
                    genc_pushs(&main_, 'F', control.string.str);
                else if (control.e_type == ID)
                    genc_pushs(&main_, 'G', control.string.str);
                else if ((control.e_type == STRING) || (control.e_type == STRING_BLOCK))
                    genc_pushs(&main_, 'S', control.string.str);
            }
            stackTokPop(ParamsStack);
            stackTokTop(ParamsStack, &control);
        }
        return SYNTAX_IS_FINE;
    }

    //<params_n> -> , id <params_n>
    else if(data->token.type == COMMA)
    {
        //printf("debug %d\n", data->token.type);
        TOKEN()
        if(data->token.type == ID)
        {
            data->params_count += 1;

            /*****   GENERATOR   *****/
            stackTokPush(ParamsStack, data->token);

            INSIDE_ANOTHER_FUNCTION_CHECK(params_n)
        }

        //<params_n> -> , expression <params_n>
        else if(data->token.type == INT || data->token.type == FLOAT || data->token.type == STRING || data->token.type == LBRACKET || data->token.type == STRING_BLOCK || !strcmp(data->token.string.str,"None") )
        {
            data->prec_item = FUNC_EXP;
            INSIDE_ANOTHER_FUNCTION_CHECK(prec_check)
            data->params_count += 1;
            INSIDE_ANOTHER_FUNCTION_CHECK(params_n)
        }
        else
        {
            return SYNTAX_ERROR;
        }
    }

    else
    {
        return SYNTAX_ERROR;
    }

    return SYNTAX_IS_FINE;
}



int def_params(pData *data)
{
    TOKEN()
    data->params_count = 0;
    //<def_params>  -> ε
    if(data->token.type == RBRACKET)
    {
        data->params_count = 0;
        return SYNTAX_IS_FINE;
    }

    //<def_params>  -> id <def_params_n>
    else if(data->token.type == ID)
    {
        data->params_count = 1;
        tData *param_func = malloc(sizeof(tData));
        param_func->id = malloc(strlen(data->token.string.str)+1);
        strcpy(param_func->id, data->token.string.str);
        param_func->def = true;
        param_func->type = T_NONE;
        param_func->function = false;
        param_func->params_count = 0;
        symtable_insert(&(data->local), data->token.string.str, *param_func);
        free(param_func->id);
        free(param_func);

        //GENERATOR
        genc_define(&out_, 'L', data->token.string.str);
        genc_pops(&out_, 'L', data->token.string.str);


        INSIDE_ANOTHER_FUNCTION_CHECK(def_params_n)

        return SYNTAX_IS_FINE;
    }
    else
    {
        return SYNTAX_ERROR;
    }
}



int def_params_n(pData *data)
{
    TOKEN()
    //<def_params_n> -> ε
    if(data->token.type == RBRACKET)
    {
        /*******************************/
        /*******************************/
        /********** GENERATOR **********/
        /*******************************/
        /*******************************/
        /*int counter_param = data->params_count;
        int i = 49;
        while (counter_param!=0){
            char arg[] = {'a','r','g', i, '\0'};
            if (genc_is_def(data, data->token)==SEM_UNDEF_ERROR)
                genc_define (&out_, 'L', arg);
            genc_pops(&out_, 'L', arg);
            counter_param--;
            i++;
        }*/

        /*******************************/
        return SYNTAX_IS_FINE;
    }

    //<def_params_n> -> , id <def_params_n>
    else if(data->token.type == COMMA)
    {
        TOKEN()
        if(data->token.type == ID)
        {
            data->params_count += 1;
            tData *param_func = malloc(sizeof(tData));
            param_func->id = malloc(strlen(data->token.string.str)+1);
            strcpy(param_func->id, data->token.string.str);
            param_func->def = true;
            param_func->type = T_NONE;
            param_func->function = false;
            param_func->params_count = 0;
            if(symtable_insert(&(data->local), data->token.string.str, *param_func)==1)
            {
                free(param_func->id);
                free(param_func);
                return SEM_UNDEF_ERROR;
            }
            free(param_func->id);
            free(param_func);

            /********** GENERATOR **********/
            genc_define(&out_, 'L', data->token.string.str);
            genc_pops(&out_, 'L', data->token.string.str);



            INSIDE_ANOTHER_FUNCTION_CHECK(def_params_n)
        }
        else
        {
            return SYNTAX_ERROR;
        }
    }
    else
    {
        return SYNTAX_ERROR;
    }

    return SYNTAX_IS_FINE;
}



int after_id(pData *data)
{
    //bool fine = false;
    tData *tmp = malloc(sizeof(struct tData));
    if (tmp == NULL){
        fprintf(stderr, "INTERN_ERROR: chyba allokace\n");
        return INTERN_ERROR;
    }
    tmp->id = malloc(strlen(data->remeber_variable)+1); //need to alloc string
    if (tmp->id == NULL){
        fprintf(stderr, "INTERN_ERROR: chyba allokace\n");
        return INTERN_ERROR;
    }
    tmp->id[0] = '\0'; // ked nebolo vynulovane tak hadzalo pri to dlhom identifikatore segfaulty a padal tam malloc - tu som nasiel chybu

    realloc_and_copy_string(data->remeber_variable, data->token.string.str);
    TOKEN()
    //printf("token - after_id: %d\n", data->token.type);
    // /*eps*/
    if(data->token.type == EOL)
    {
        symtable_item *item = symtable_search(&data->global, data->remeber_variable);
        if(item == NULL)
        {
            if(data->in_func)
            {
                item = symtable_search(&data->local, data->remeber_variable);
                if(item == NULL)
                {
                    free(tmp->id);
                    free(tmp);
                    return SEM_UNDEF_ERROR;
                }
            }
            else
            {
                free(tmp->id);
                free(tmp);
                return SEM_UNDEF_ERROR;
            }
        }
        free(tmp->id);
        free(tmp);
        return SYNTAX_IS_FINE;
    }

    else if(data->token.type == LBRACKET)
    {
        bool found = false;
        char *token_saved = NULL;
        if(data->remeber_function != NULL)
        {
            token_saved = malloc(strlen(data->remeber_function) + 1);
            if(token_saved == NULL)
            {
                fprintf(stderr, "INTERN_ERROR: chyba allokace\n");
                free(tmp->id);
                free(tmp);
                return INTERN_ERROR;
            }
            strcpy(token_saved, data->remeber_function);
        }
        data->remeber_function = data->remeber_variable;
        realloc_and_copy_string(data->remeber_function, data->remeber_variable);
        symtable_item *item = symtable_search(&(data->global), data->remeber_function);
        if(item != NULL)
        {
            do{
                if(!strcmp(item->data.id, data->remeber_function))
                {
                  found = true; //item byl nalezen v symtable
                  break;
                }

                item = item->next;
            }while(item != NULL);
        }
        if(found)
        {
            INSIDE_ANOTHER_FUNCTION_CHECK(params)

            if(data->params_count != item->data.params_count)
            {
                fprintf(stderr, "SEM_PARAM_ERROR: spatne zadane parametry funkce %s\n", item->data.id);
                if(token_saved != NULL)
                {
                    data->remeber_function = token_saved;
                    free(token_saved);
                }
                free(tmp->id);
                free(tmp);
                return SEM_PARAM_ERROR;
            }
            CHECK(RBRACKET)
            TOKEN()
            if(token_saved != NULL)
            {
                data->remeber_function = token_saved;
                free(token_saved);
            }
            free(tmp->id);
            free(tmp);
            return SYNTAX_IS_FINE;
        }
        else
        {
            INSIDE_ANOTHER_FUNCTION_CHECK(params)
            tmp->params_count = data->params_count;
            CHECK(RBRACKET)
            strcpy(tmp->id, data->remeber_function);
            tmp->def = false;
            tmp->function = true;
            tmp->type = T_UNDEF;
            if((symtable_insert(&(data->global), data->remeber_function, *tmp))==1) //vlozeni prvku do hash table
            {
                fprintf(stderr, "INTERN_ERROR: spatne vlozeni do symtable\n");
                if(token_saved != NULL)
                {
                    //data->remeber_function = token_saved;
                    realloc_and_copy_string(data->remeber_function, token_saved);
                    free(token_saved);
                }
                free(tmp->id);
                free(tmp);
                return INTERN_ERROR;
            }
        }

        TOKEN()
        if(token_saved != NULL)
        {
            //data->remeber_function.string.str = token_saved;
            realloc_and_copy_string(data->remeber_function, token_saved);
            free(token_saved);
        }
        free(tmp->id);
        free(tmp);
        return SYNTAX_IS_FINE;
    }

    else if(data->token.type == ASSIGN)
    {
        //free(data->token.string.str); //need to be solved better
        TOKEN()
        if(data->token.type == ID || data->token.type == KW)
        {
            symtable_item *func = symtable_search(&data->global, data->token.string.str);
            if(func != NULL)
            {
                while(func!=NULL)
                {
                    if(!strcmp(func->data.id, data->token.string.str))
                    {
                        break;
                    }
                    func = func->next;
                }
            }
            else if(data->in_func)
            {
                func = symtable_search(&data->local, data->token.string.str);
                while(func!=NULL)
                {
                    if(!strcmp(func->data.id, data->token.string.str))
                    {
                        break;
                    }
                    func = func->next;
                }
            }
            if(func != NULL && func->data.function)
            {
                realloc_and_copy_string(data->remeber_function, func->data.id);
                NEW_TOKEN_AND_CHECK(LBRACKET)
                INSIDE_ANOTHER_FUNCTION_CHECK(params)
                if(data->params_count != func->data.params_count)
                {
                    fprintf(stderr, "SEM_PARAM_ERROR: spatne zadane parametry funkce %s\n", func->data.id);
                    return SEM_PARAM_ERROR;
                }
                /*******************************/
                /*******************************/
                /********** GENERATOR **********/
                /*******************************/
                /*******************************/
                if (data->in_func) {
                    genc_call(&out_, data->remeber_function);
                    genc_pops(&out_, 'G', "res_express");
                    genc_move(&out_,'L',data->remeber_variable,'G', "res_express");
                }
                else{
                    genc_call(&main_, data->remeber_function);
                    genc_pops(&main_, 'G', "res_express");
                    genc_move(&main_,'G',data->remeber_variable,'G', "res_express");
                }
                /*******************************/


                CHECK(RBRACKET)
                NEW_TOKEN_AND_CHECK(EOL)

                tmp->type = func->data.type;
                tmp->def = true;
                tmp->params_count = 0;
                tmp->function = false;

                realloc_and_copy_string(tmp->id, data->remeber_variable);

                symtable_item *prom = symtable_search(&data->global, data->remeber_variable);
                if(prom != NULL)
                {
                    while(prom!=NULL)
                    {
                        if(!strcmp(prom->data.id, data->remeber_variable))
                        {
                            break;
                        }
                        prom = prom->next;
                    }
                }
                else if(data->in_func)
                {
                    prom = symtable_search(&data->local, data->remeber_variable);
                    while(prom!=NULL)
                    {
                        if(!strcmp(prom->data.id, data->remeber_variable))
                        {
                            break;
                        }
                        prom = prom->next;
                    }
                }
                if(prom!=NULL && prom->data.def)
                {
                    free(tmp->id);
                    free(tmp);
                    return SYNTAX_IS_FINE;
                }

                //printf("tmp->id: %s, data->remeber_variable.string.str: %s\n",tmp->id,  data->remeber_variable.string.str);
                if(data->in_func)
                {
                    if((symtable_insert(&data->local, data->remeber_variable, *tmp))==1) //vlozeni prvku do hash table
                    {
                        free(tmp->id);
                        free(tmp);
                        return SYNTAX_ERROR;
                    }
                }
                else
                {
                    if((symtable_insert(&data->global, data->remeber_variable, *tmp))==1) //vlozeni prvku do hash table
                    {
                        free(tmp->id);
                        free(tmp);
                        return SYNTAX_ERROR;
                    }

                }
                free(tmp->id);
                free(tmp);

                return SYNTAX_IS_FINE;
            }
            else
            {
                data->type_id = T_UNDEF;
                data->prec_item = ID_EXP;
                INSIDE_ANOTHER_FUNCTION_CHECK(prec_check)

                //TODO waiting for prectable
                /*******************************/
                /*******************************/
                /********** GENERATOR **********/
                /*******************************/
                /*******************************/
                //ak som v maine tak
                if(data->in_func){
                    genc_pops(&out_, 'G', "res_express");
                    genc_move(&out_,'L',data->remeber_variable,'G', "res_express");

                }
                else{
                    genc_pops(&main_, 'G', "res_express");
                    genc_move(&main_,'G',data->remeber_variable,'G', "res_express");

                }
                /*******************************/

                tmp->type = data->type_id;
                tmp->def = true;
                tmp->params_count = 0;
                tmp->function = false;

                strcpy(tmp->id, data->remeber_variable);

                symtable_item *prom = symtable_search(&data->global, data->remeber_variable);
                if(prom != NULL)
                {
                    while(prom!=NULL)
                    {
                        if(!strcmp(prom->data.id, data->remeber_variable))
                        {
                            break;
                        }
                        prom = prom->next;
                    }
                }
                else if(data->in_func)
                {
                    prom = symtable_search(&data->local, data->remeber_variable);
                    while(prom!=NULL)
                    {
                        if(!strcmp(prom->data.id, data->remeber_variable))
                        {
                            break;
                        }
                        prom = prom->next;
                    }
                }
                if(prom!=NULL && prom->data.def)
                {
                    free(tmp->id);
                    free(tmp);
                    return SYNTAX_IS_FINE;
                }

                //printf("tmp->id: %s, data->remeber_variable.string.str: %s\n",tmp->id,  data->remeber_variable.string.str);
                if(data->in_func)
                {
                    if((symtable_insert(&data->local, data->remeber_variable, *tmp))==1) //vlozeni prvku do hash table
                    {
                        free(tmp->id);
                        free(tmp);
                        return SYNTAX_ERROR;
                    }
                }
                else
                {
                    if((symtable_insert(&data->global, data->remeber_variable, *tmp))==1) //vlozeni prvku do hash table
                    {
                        free(tmp->id);
                        free(tmp);
                        return SYNTAX_ERROR;
                    }

                }
                free(tmp->id);
                free(tmp);

                return SYNTAX_IS_FINE;
            }
        }
        else if(data->token.type == INT || data->token.type == FLOAT || data->token.type == STRING || data->token.type == LBRACKET || data->token.type == STRING_BLOCK )
        {
            data->type_id = T_UNDEF;
            data->prec_item = ID_EXP;

            INSIDE_ANOTHER_FUNCTION_CHECK(prec_check)

            //TODO waiting for prectable

            /*******************************/
            /*******************************/
            /********** GENERATOR **********/
            /*******************************/
            /*******************************/
            //ak som v maine tak
            if(data->in_func){
                genc_pops(&out_, 'G', "res_express");
                genc_move(&out_,'L',data->remeber_variable,'G', "res_express");

            }
            else{
                genc_pops(&main_, 'G', "res_express");
                genc_move(&main_,'G',data->remeber_variable,'G', "res_express");

            }
            strcpy(tmp->id, data->remeber_variable);
            tmp->type = data->type_id;
            tmp->def = true;
            tmp->params_count = 0;
            tmp->function = false;

            symtable_item *prom = symtable_search(&data->global, data->remeber_variable);
            if(prom != NULL)
            {
                while(prom!=NULL)
                {
                    if(!strcmp(prom->data.id, data->remeber_variable))
                    {
                        break;
                    }
                    prom = prom->next;
                }
            }
            else if(data->in_func)
            {
                prom = symtable_search(&data->local, data->remeber_variable);
                while(prom!=NULL)
                {
                    if(!strcmp(prom->data.id, data->remeber_variable))
                    {
                        break;
                    }
                    prom = prom->next;
                }
            }
            if(prom!=NULL && prom->data.def)
            {
                free(tmp->id);
                free(tmp);
                return SYNTAX_IS_FINE;
            }
            if(data->in_func)
            {

                if((symtable_insert(&data->local, data->remeber_variable, *tmp))==1) //vlozeni prvku do hash table
                {
                    free(tmp->id);
                    free(tmp);
                    return SYNTAX_ERROR;
                }
            }
            else
            {
                if((symtable_insert(&data->global, data->remeber_variable, *tmp))==1) //vlozeni prvku do hash table
                {
                    free(tmp->id);
                    free(tmp);
                    return SYNTAX_ERROR;
                }

            }
            free(tmp->id);
            free(tmp);

            return SYNTAX_IS_FINE;
        }
        else
        {
            free(tmp->id);
            free(tmp);
            return SYNTAX_ERROR;
        }
    }
    else
    {
        free(tmp->id);
        free(tmp);
        return SYNTAX_ERROR;
    }

    TOKEN()
    return SYNTAX_IS_FINE;
}




int main(){

//*** zasobnik na riesenie indentu a dedentu ***/
//***               NEMAZAT!!!               ***/
    stack = malloc(sizeof(tStack));
    if (stack == NULL){
        fprintf(stderr, "Chyba pri alokovani zasobniku\n");
        return INTERN_ERROR;
    }
    stackInit(stack);
    stackPush(stack, 0);

    Token dno;
    dno.string.size = 0;

    char str[2] = "$";

	int i = 0;
	while (str[i]!= '\0'){
		dyn_string(&(dno.string), str[i]);
		i++;
	}

    dno.type = COLON;
    dno.s_top = 0;

    ParamsStack = malloc(sizeof(tStackTok));
    if (ParamsStack==NULL){
        return INTERN_ERROR;
    }

    stackTokInit(ParamsStack);
    stackTokPush(ParamsStack, dno);

    out_.size = 0;
    main_.size = 0;

    genc_init(&out_);
    //***********built-in funckie***********//
    genc_len(&out_);
    genc_inputf(&out_);
    genc_inputs(&out_);
    genc_inputi(&out_);
    genc_substr(&out_);
    genc_ord(&out_);
    genc_chr(&out_);
    genc_print(&out_);

    /***************************************/
    genc_init_main(&main_);
    result = start_parser();


    if (result==0){
        codegen_flush(&out_);
        codegen_flush(&main_);
    }

    return result;
/***********************************************/
}
