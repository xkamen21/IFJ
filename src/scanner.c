//#include "scanner.h"
#include "error.h"
#include "dynamicstring.h"
#include "stack.h" // v stacku je scanner.h

//globalny stack na identy a dedenty
tStack *stack;

//globalna premenna pre zistenie ci som na zaciatku riadku pre indent
bool newline = true;

//globalna premenna na token pre dedent
bool dedent_token = false;
int dedent_spaces;

bool com_ded = false;

int control; //premenna na kontrolu returnu dyn_string funkcie
bool n_before_eof = false; // pred eof musi byt vzdy eol

/*** Pole na klucove slova a vstavane funckie ***/
static char keywords[15][10] = {"def", "else", "if", "None", "pass", "return",
                        "while", "inputs", "inputi", "inputf", "print",
                        "len", "substr", "ord", "chr"};


/*** Stavy scanneru ***/
#define TOKEN_START 100
#define START_OF_ID 101
#define START_OF_COMMENT 102
#define START_OF_STRING_BLOCK 103
#define OPERATOR 104
#define EQUAL_STATE 105
#define GREATER_STATE 106
#define LESS_STATE 107
#define NEGATION_STATE 108
#define START_OF_NUMBER 109
#define START_OF_STRING 110
#define DIVISION_STATE 111

/**************************************************/

void set_token(Token *t, type_token type, char *str, int s_top){

    t->string.size = 0;
    //printf("XXXXX  %s  XXXXX\n", str);

    int i = 0;
    while (str[i]!= '\0'){
        dyn_string(&(t->string), str[i]);
        i++;
    }

    t->type = type;
    t->s_top = s_top;
    return;
}

/**************************************************/

int is_id_kw(char *s){

    int kw = -1;
    for (int i = 0; i < 15; i++) {
        int res = strcmp(s, keywords[i]);
        if (res == 0){
            kw = i;
            return kw;
        }
    }
    return kw;
}

/**************************************************/

int float_or_int(char *str){

    int len = strlen(str);
    int fl = 0;
    for (int i = 0; i<len-1; i++){
        if (str[i] == '.')
            fl++;
    }
    if (fl == 0){
        return 0; // je to int
    }
    else if (fl == 1){
        return 1; // je to float
    }
    else{
        return -1; // nekorektny zapis
    }
}

/**************************************************/

/*** CHOOSING OPERATOR ***/
void choose_op(Token *t, char c){

    switch (c){
        case '+':
            set_token(t, PLUS, "$", stackTop(stack));
            break;
        case '-':
            set_token(t, MINUS, "$", stackTop(stack));
            break;
        case '*':
            set_token(t, MULTIPLY, "$", stackTop(stack));
            break;
        case ',':
            set_token(t, COMMA, "$", stackTop(stack));
            break;
        } // switch
    return;
    }

/**************************************************/

int get_token(Token *t){

    //aktualny stav v ktorom sa nachadza token
    int state = TOKEN_START;


    //pomocna premenna na ulozenie znakov a na cyklus
    int c;

    int i = 0;

    dynamic_string str;
    str.size = 0;
    str.str = NULL;

    // premenna na pocitanie indentu
    int space = 0;

    // INDENT a DEDENT
    if (dedent_token == true){
        if (stackTop(stack)!=0)
            stackPop(stack);
        if ((stackTop(stack) == 0) && (stackTop(stack) != dedent_spaces)){
            fprintf( stderr, "CHYBA lexikalna - zle odsadenie\n");
            free_dyn_string(&str);
            return LEX_ERROR;
        }
        if (stackTop(stack) != dedent_spaces){
            set_token(t, DEDENT, "$", stackTop(stack));
            return 0;
        }
        else{
            dedent_token = false;
            set_token(t, DEDENT, "$", stackTop(stack));
            return 0;
        }
    }

    bool exp; // exponent je len jeden
    bool point; //desatinna ciarka je len jedna
    bool end_of_file = false;
    int h1; // premenna pre hexadec v stringu
    bool zero;

    while (1){

        c = fgetc(stdin);

        /*** TOKEN START ***/
        if (state == TOKEN_START){

            if (c == EOF){
                if (n_before_eof == false){
                    ungetc('\n',stdin);
                    // eof nemusim vraciat naspat - da sa tam automaticky
                    continue;
                }
                end_of_file = true;
                break;
            }

            else if (c == '#'){
                state = START_OF_COMMENT;
                continue;
            }

            n_before_eof = false; // pred eof musi byt vzdy eol


            if ((c == ' ')&&(newline == true)){
                space++;
                continue;
            }
            else if ((c != ' ') && (newline == true)){
                newline = false;
                if (c == '#'){
                    com_ded = true;
                }
                else{
                    com_ded = false;
                }
                ungetc(c, stdin);
                if (space > stackTop(stack)){
                    //printf(">>> INDENT <<<\n" );
                    stackPush(stack, space);
                    set_token(t, INDENT, "$", stackTop(stack));
                    break;
                }
                else if ((space < stackTop(stack))&&(com_ded == false)){
                    //printf(">>> DEDENT <<<\n" );
                    stackPop(stack);
                    set_token(t, DEDENT, "$", stackTop(stack));
                    if (stackTop(stack) != space){
                        dedent_token = true;
                        dedent_spaces = space;
                    }
                    break;
                }
                else{
                    continue;
                }

            }


            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_')){

                control = dyn_string(&str, c);

                if (control == INTERN_ERROR){
                    return INTERN_ERROR;
                }
                state = START_OF_ID;
                continue;
            }

            else if (c == '\n'){
                newline = true;
                n_before_eof = true;
                set_token(t, EOL, "$", stackTop(stack));
                break;
            }

            else if ((c == '+') || (c == '-') || (c == '*') || (c == ',')){
                choose_op(t, c); // vyberie a nastavi operator do tokena
                break;
            }

            else if (c == ':'){
                set_token(t, COLON, "$", stackTop(stack));
                break;
            }

            else if (c == '('){
                set_token(t, LBRACKET, "$", stackTop(stack));
                break;
            }

            else if (c == ')'){
                set_token(t, RBRACKET, "$", stackTop(stack));
                break;
            }

            else if (c == '='){
                state = EQUAL_STATE;
                continue;
            }

            else if (c == '/'){
                state = DIVISION_STATE;
                continue;
            }

            else if (c == '>'){
                state = GREATER_STATE;
                continue;
            }

            else if (c == '<'){
                state = LESS_STATE;
                continue;
            }

            else if (c == '!'){
                state = NEGATION_STATE;
                continue;
            }

            else if (c >= '0' && c <= '9'){
                control = dyn_string(&str, c);
                if (control == INTERN_ERROR){
                    return INTERN_ERROR;
                }
                state = START_OF_NUMBER;
                exp = false; // exponent sa tam zatial nenachadza
                point = false; // desatinna ciarka
                if (c == '0')
                    zero = true;
                continue;
            }

            else if (c == 39){  // 39 == ' v ASCII
                i = 0;
                state = START_OF_STRING;
                continue;
            }

            else if (c == '"'){
                c = fgetc(stdin);
                // " 2.krat
                if (c == '"'){
                    char c_back_2 = c;
                    c = fgetc(stdin);
                    // " 3. krat
                    if (c == '"'){
                        state = START_OF_STRING_BLOCK;
                        continue;
                    } // " 3.krat
                    else{
                        ungetc(c,stdin);
                        ungetc(c_back_2,stdin);
                        // nastavi token - token = "
                        //set_token(t, QUOTE, "$", stackTop(stack));
                        free_dyn_string(&str);
                        return LEX_ERROR;
                        break;
                    }
                } // " 2.krat
                else{
                    ungetc(c,stdin);
                    // nastavi token - token = "
                    //set_token(t, QUOTE, "$", stackTop(stack));
                    free_dyn_string(&str);
                    return LEX_ERROR;
                    break;
                }
            }

            else if (c == ' '){
                continue;
            }

            else{
                free_dyn_string(&str);
                return LEX_ERROR;
            }
        } // state == TOKEN_START


        /*** START OF ID ***/
        if (state == START_OF_ID){

            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_')){
                control = dyn_string(&str, c);

                if (control == INTERN_ERROR){
                    return INTERN_ERROR;
                }
            }

            else{
                // treba vratit posledny nacitany znak aby sme on neprisli
                ungetc(c,stdin);

                int kw = is_id_kw(str.str);

                if (kw == -1){
                    set_token(t, ID, str.str, stackTop(stack));
                    break;
                } // if kw == -1 -> str nie je kw ale id

                else{
                    set_token(t, KW, str.str, stackTop(stack));
                    break;

                } // if kw != -1
            } // if c == ' '
        } // if state == START_OF_ID


        /*** EQUAL_STATE ***/
        if (state == EQUAL_STATE){

            if (c == '='){
                set_token(t, EQUAL, "$", stackTop(stack));
                break;
            }
            else{
                ungetc(c, stdin);
                set_token(t, ASSIGN, "$", stackTop(stack));
                break;
            }
        }


        /*** GREATER_STATE ***/
        if (state == GREATER_STATE){

            if (c == '='){
                set_token(t, GR_OR_EQ, "$", stackTop(stack));
                break;
            }
            else{
                ungetc(c, stdin);
                set_token(t, GREATER, "$", stackTop(stack));
                break;
            }
        }


        /*** LESS_STATE ***/
        if (state == LESS_STATE){

            if (c == '='){
                set_token(t, LE_OR_EQ, "$", stackTop(stack));
                break;
            }
            else{
                ungetc(c, stdin);
                set_token(t, LESS, "$", stackTop(stack));
                break;
            }
        }


        /*** NEGATION_STATE ***/
        if (state == NEGATION_STATE){

            if (c == '='){
                set_token(t, NOT_EQUAL, "$", stackTop(stack));
                break;
            }
            else{
                ungetc(c, stdin);
                set_token(t, NEGATION, "$", stackTop(stack));
                break;
            }
        }


        /*** DIVISION_STATE ***/
        if (state == DIVISION_STATE){

            if (c == '/'){
                set_token(t, DIV, "$", stackTop(stack));
                break;
            }
            else{
                ungetc(c, stdin);
                set_token(t, DIVIDE, "$", stackTop(stack));
                break;
            }
        }


        /*** START_OF_NUMBER ***/
        if (state == START_OF_NUMBER) {
            if ((c >= '0' && c <= '9') || (((c == 'e') || ((c == '.') && (point == false)) || (c == 'E')) && (exp == false))){
                control = dyn_string(&str, c);
                if (control == INTERN_ERROR){
                    return INTERN_ERROR;
                }

                if ((c == '0')&&(zero == true)){
                    free_dyn_string(&str);
                    return LEX_ERROR;
                }
                else if((c != '.')&&(zero == true)){
                    free_dyn_string(&str);
                    return LEX_ERROR;
                }
                else{
                    zero = false;
                }

                if (c == '.'){
                    if (point==true){
                        free_dyn_string(&str);
                        return LEX_ERROR;
                    }
                    point = true;
                }

                if ((c == 'e') || (c == 'E')){
                    if (exp==true){
                        free_dyn_string(&str);
                        return LEX_ERROR;
                    }
                    exp = true; // exponent sa tam uz nachadza
                    // kontrola ci tam nahodou nie je plus alebo MINUS
                    c = fgetc(stdin);
                    if ((c == '+') || (c == '-')){
                        control = dyn_string(&str, c);
                        if (control == INTERN_ERROR){
                            return INTERN_ERROR;
                        }
                    }
                    else{
                        ungetc(c, stdin);
                    }
                }
            }

            else if((c >= 'a' && c <= 'd') || (c >= 'f' && c <= 'z') || (c >= 'A' && c <= 'D') || (c >= 'F' && c <= 'Z')){
                free_dyn_string(&str);
                return LEX_ERROR;
            }
            else{
                // treba vratit posledny nacitany znak aby sme on neprisli
                ungetc(c,stdin);

                int number = float_or_int(str.str);

                if(str.str[strlen(str.str)-1] == '.'){
                    free_dyn_string(&str);
                    return LEX_ERROR;
                }

                if (number == 0){
                    set_token(t, INT, str.str, stackTop(stack));
                    break;
                }
                else if (number == 1){
                    set_token(t, FLOAT, str.str, stackTop(stack));
                    break;
                }
                else{
                    free_dyn_string(&str);
                    return LEX_ERROR; //chyba!!!
                }
            } // else
        } // START_OF_NUMBER


        /*** START_OF_STRING ***/
        if (state == START_OF_STRING) {
            if ((c == EOF) || (c == '\n'))
                return LEX_ERROR;
            if ((c == 39)&&(i == 0)){

                set_token(t, STRING, str.str, stackTop(stack));
                break;
            }

            else if ((c == 39) && (i != 0)){

                set_token(t, STRING, str.str, stackTop(stack));
                break;
            }

            else{
                //escape sekvencie

                if (c == 92){
                    int c2 = fgetc(stdin);
                    switch (c2){
                        case EOF:
                            free_dyn_string(&str);
                            return LEX_ERROR;
                        case 'n':
                            control = dyn_string(&str, '\n');
                            if (control == INTERN_ERROR){
                                return INTERN_ERROR;
                            }
                            break;
                        case 't':
                            control = dyn_string(&str, '\t');
                            if (control == INTERN_ERROR){
                                return INTERN_ERROR;
                            }
                            break;
                        case 92:
                            control = dyn_string(&str, 92);
                            if (control == INTERN_ERROR){
                                return INTERN_ERROR;
                            }
                            break;
                        case  39:
                            control = dyn_string(&str, 39);
                            if (control == INTERN_ERROR){
                                return INTERN_ERROR;
                            }
                            break;
                        case '"':
                            control = dyn_string(&str, '"');
                            if (control == INTERN_ERROR){
                                return INTERN_ERROR;
                            }
                            break;
                        case 'x':
                            h1 = fgetc(stdin);
                            if (!((h1 >= 'a' && h1 <= 'f') || (h1 >= 'A' && h1 <= 'F') || (h1 >= '0' && h1 <= '9'))){
                                free_dyn_string(&str);
                                return LEX_ERROR;
                            }
                            int h2 = fgetc(stdin);
                            if (!((h2 >= 'a' && h2 <= 'f') || (h2 >= 'A' && h2 <= 'F') || (h2 >= '0' && h2 <= '9'))){
                                free_dyn_string(&str);
                                return LEX_ERROR;
                            }
                            char hexa_str[3] = {'\0', '\0', '\0'};
                            hexa_str[0] = h1;
                            hexa_str[1] = h2;
                            int hex_num = strtol(hexa_str, NULL, 16);

                            control = dyn_string(&str, hex_num);
                            if (control == INTERN_ERROR){
                                return INTERN_ERROR;
                            }

                            break;
                        default :
                            ungetc(c2,stdin);
                            control = dyn_string(&str, c);
                            if (control == INTERN_ERROR){
                                return INTERN_ERROR;
                            }
                            break;
                    }
                }
                else
                control = dyn_string(&str, c);
                if (control == INTERN_ERROR){
                    return INTERN_ERROR;
                }

                continue;
            }
        }


        /*** START OF COMMENT ***/
        if (state == START_OF_COMMENT){
            if ((c == '\n') || (c == EOF)){
                ungetc(c, stdin);
                set_token(t, COMMENT, "$", stackTop(stack));
                //state = TOKEN_START;
                //continue;
                break;
            }
            else{
                continue;
            }
        } // if state == START_OF_COMMENT


        /*** START OF BLOCK COMMENT ***/
        if (state == START_OF_STRING_BLOCK){

            if (c == EOF){
                free_dyn_string(&str);
                return LEX_ERROR;
            }
            if (c == 92){
                int c2 = fgetc(stdin);
                if (c2 == '"'){
                    control = dyn_string(&str, c2);
                    if (control == INTERN_ERROR){
                        return INTERN_ERROR;
                    }

                    //i++;
                    c = fgetc(stdin);
                }
                else{
                    ungetc(c2,stdin);
                }
            }

            if (c == '"'){
                int c_1 = fgetc(stdin);
                if (c_1 == EOF){
                    free_dyn_string(&str);
                    return LEX_ERROR;
                }
                // " 2.krat
                if (c_1 == '"'){
                    int c_2 = fgetc(stdin);
                    if (c_2 == EOF){
                        free_dyn_string(&str);
                        return LEX_ERROR;
                    }
                    // " 3. krat
                    if (c_2 == '"'){

                        set_token(t, STRING_BLOCK, str.str, stackTop(stack));
                        break;
                    } // " 3.krat
                    else{
                        ungetc(c_2,stdin);
                        ungetc(c_1,stdin);
                        control = dyn_string(&str, c);
                        if (control == INTERN_ERROR){
                            return INTERN_ERROR;
                        }
                        continue;
                    }
                } // " 2.krat
                else{
                    ungetc(c_1,stdin);
                    control = dyn_string(&str, c);
                    if (control == INTERN_ERROR){
                        return INTERN_ERROR;
                    }
                    //i++;
                    continue;
                }
            } // " 1.krat
            else{
                control = dyn_string(&str, c);
                if (control == INTERN_ERROR){
                    return INTERN_ERROR;
                }
                continue;
            }
        } // if state == START_OF_STRING_BLOCK

    } // while

    free_dyn_string(&str);

    // koniec kodu - return EOF - dalsi token uz nemoze byt nacitany
    if ((c == EOF) && (end_of_file == true)){
        if (stackTop(stack) != 0){
            stackPop(stack);
            set_token(t, DEDENT, "$", stackTop(stack));
            return 0;
        }
        else{
            set_token(t, EOF_T, "$", stackTop(stack));
            return 0;
        }
    }

//    fprintf(stderr, "TOKEN ---> %d\n",t->type );
    return 0;
}
