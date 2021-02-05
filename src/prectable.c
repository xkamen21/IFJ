//#include "stack.h" // v stacku je scanner
//#include "error.h"
//#include "scanner.h"
#include "parser.h" //stack, scanner, error, parser tiez mi treba

#define TOK_TYPE_SYNTAX_ERR 98
#define TOK_TYPE_SEM_TYPE_ERR 99
#define KEY_WORD_LOADED 420

typedef enum prec_table_rule{
	E_PLUS_E,		// E -> E + E   0
	E_MINUS_E,		// E -> E - E   1
	E_MULTIPLY_E,   // E -> E * E   2
	E_DIVIDE_E,		// E -> E / E   3
    E_DIV_E,		// E -> E // E  4
	LBR_E_RBR,		// E -> (E)     5
	OPERAND,		// E -> i       6
	NOT_RULE,		// rule doesn't exist   7
	E_EQUAL_E,		// E -> E == E  8
	E_GROREQ_E,		// E -> E >= E  9
	E_LEOREQ_E,		// E -> E <= E	10
	E_LESS_E,		// E -> E < E	11
	E_GREATER_E,	// E -> E > E	12
	E_NOTEQUAL_E,	// E -> E != E  13
} prec_table_rule;

	bool strings_concat;
    bool e_na_stacku = false;
	int get;

	#define TOKEN()\
	get = get_token(&(data->token));\
	if (get == LEX_ERROR)\
	{\
		fprintf(stderr, "ERROR, ve vyrazu je lexikalni chyba\n" );\
		return LEX_ERROR;\
	}\
	else if (get == INTERN_ERROR)\
	{\
		fprintf(stderr, "ERROR, ve vyrazu je interni chyba\n" );\
		return INTERN_ERROR;\
	}\


int last_token_check(pData *data)
  {
    // IF,WHILE`
    if (data->prec_item == 0)
    {
        if (data->token.type != 24)
		{
            fprintf(stderr, "ERROR, IF nebo WHILE nekonci :\n" );
            return SYNTAX_ERROR;
        }
        else
          return SYNTAX_IS_FINE;

    }

    // Vyraz po =
    else if (data->prec_item == 1)
	{
        if (data->token.type != 25)
		{
            fprintf(stderr, "ERROR, Vyraz nekonci EOL :\n" );
            return SYNTAX_ERROR;
        }
        else
          return SYNTAX_IS_FINE;

    }
    //Parametr funkce
    else
	{
      if ((data->token.type != COMMA) && (data->token.type != RBRACKET))
	  {
          fprintf(stderr, "ERROR, Za vyrazem musi byt carka nebo prava zavorka\n" );
          return SYNTAX_ERROR;
      }
      else
        return SYNTAX_IS_FINE;

    } // IF ELSE ELSE
  }

int operand_check(Token actual, Token previous, pData *data)
{
	// Kontorla prichazejici funkce ktera se ma priradit do vyrazu
	if (previous.type == ID && actual.type == LBRACKET)
	{
		if(params(data) == SYNTAX_IS_FINE)
		 {
			 tData *tmp = malloc(sizeof(struct tData));
			 tmp->id = malloc(strlen(previous.string.str)+1); //need to alloc string
			 strcpy(tmp->id, previous.string.str);
			 tmp->params_count=data->params_count;
			 tmp->def = false;
			 tmp->function = true;
			 tmp->type = T_NONE;
			 data->type_id = T_NONE;
			 if((symtable_insert(&(data->global), previous.string.str, *tmp))==1) //vlozeni prvku do hash table
             {
                 fprintf(stderr, "INTERN_ERROR: (prectable) spatne vlozeni do symtable\n");
                 free(tmp->id);
                 free(tmp);
                 return INTERN_ERROR;
             }
			 free(tmp->id);
			 free(tmp);
			 return 69;
		 }
		 else
		 {
			 return SYNTAX_ERROR;
		 }
	}
    // KOntorla dvou operandu vedle sebe
    if ( (actual.type >= PLUS && actual.type <= NOT_EQUAL) && (previous.type >= PLUS && previous.type <= NOT_EQUAL) )
	{
        fprintf(stderr, "ERROR, 2 operandy vedle sebe \n" );
        return SYNTAX_ERROR;
    }

    // Kontorla deleni nulou
    if ( ((previous.type == 10) || (previous.type == 13)) && ((!strcmp(actual.string.str,"0"))||(!strcmp(actual.string.str,"0.0"))||
    (!strcmp(actual.string.str,"0.00"))||(!strcmp(actual.string.str,"0.000"))||(!strcmp(actual.string.str,"0.0000"))||
    (!strcmp(actual.string.str,"0.00000"))||(!strcmp(actual.string.str,"0.000000"))||(!strcmp(actual.string.str,"0.000000"))))
    {
     fprintf(stderr, "ERROR, deleni 0\n" );
      return DIV_0_ERROR;
    }
    return SYNTAX_IS_FINE;
  }

 symtable_item *global_and_local_search(pData *data, Token actual_tok)
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
              fprintf(stderr, "ERROR, promenna %s nebyla definovana (LOKAL SEARCH)\n",actual_tok.string.str );
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
              fprintf(stderr, "ERROR, promenna %s nebyla definovana (GLOBAL SEARCH)\n",actual_tok.string.str );
              return NULL;
          }
      }// end  if(data->in_func)
  }

  int is_defined(pData *data, Token actual_tok	)
  {
      if(actual_tok.type == ID)
      {
          symtable_item *element = global_and_local_search(data, actual_tok);
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
          // else if(element != NULL)
          else
          {
              return SEM_UNDEF_ERROR;
          }
      } // else if(data->token.type == ID)
      else
      {
          return SYNTAX_IS_FINE;
      }
  }

  prec_table_rule wchich_rule(int count, Token t1, Token t2, Token t3)
  {
      if (count == 1)
	  {
          if (t1.e_type == ID || t1.e_type == KW)
          {
              return OPERAND;
          }
          else if (t1.e_type >= STRING && t1.e_type <= FLOAT)
          {
              return OPERAND;
          }
          else{
              return NOT_RULE;
          }
      }
      else if (count == 3)
	  {
          if (t1.type == RBRACKET && t3.type == LBRACKET && t2.type == E_TOK)
          {
              return LBR_E_RBR;
          }
          else if (t1.type == E_TOK && t3.type == E_TOK)
          {
              switch (t2.type)
			  {
                  case PLUS:
                     return E_PLUS_E;
                  case MINUS:
                     return E_MINUS_E;
                 case MULTIPLY:
                    return E_MULTIPLY_E;
                 case DIVIDE:
                    return E_DIVIDE_E;
                 case DIV:
                    return E_DIV_E;
				case EQUAL:
					return E_EQUAL_E;
				case GR_OR_EQ:
					return E_GROREQ_E;
				case LE_OR_EQ:
					return E_LEOREQ_E;
				case LESS:
					return E_LESS_E;
				case GREATER:
					return E_GREATER_E;
				case NOT_EQUAL:
					return E_NOTEQUAL_E;

                 default:
                     return NOT_RULE;
              }
          }
          else
          {
              return NOT_RULE;
          }
      } // END ELSE IF (count == 3)
      else
      {
          return NOT_RULE;
      }
   }

int semantics_chceck(prec_table_rule rule,Token t1, Token t3,type_token *final_type, pData *data)
{
      switch (rule)
	  {
          case OPERAND:
            if(t1.e_type == ID)
			{
					symtable_item *element = global_and_local_search(data, t1);
		            if(element != NULL)
		            {
						switch (element->data.type)
						{
							case T_INT:
								*final_type = INT;
								break;
							case T_FLOAT:
								*final_type = FLOAT;
								break;
							case T_STRING:
								*final_type = STRING;
								break;
							case T_UNDEF:
								fprintf(stderr, "ERROR, promenna %s je typu UNDEF, nelze s ni pracovat,\n", t1.string.str );
								return SYNTAX_ERROR;
							case T_NONE:
								//fprintf(stderr, "ERROR, promenna %s je typu NONE, nelze s ni pracovat,\n", t1.string.str );
								//return SYNTAX_ERROR;
								*final_type = STOP;
								break;
							case T_BOOL:
								fprintf(stderr, "ERROR, promenna %s je typu BOOL, nelze s ni pracovat,\n", t1.string.str );
								return SYNTAX_ERROR;
						}// switch
		            }
		            // else if(element != NULL)
		            else
		            {
						fprintf(stderr, "Nenasel jsem %s v semantics_chceck\n", t1.string.str);
		                return SYNTAX_ERROR;
		            }
			}
			else if (t1.e_type == STRING)
			{
                    *final_type = STRING;
            }
			else if (t1.e_type == FLOAT)
			{
                    *final_type = FLOAT;
			}
            else if (t1.e_type == INT)
			{
                    *final_type = INT;
			}
			else if (t1.e_type == KW)
			{
				fprintf(stderr, "NIL !!!!!!!!!!\n" );

				*final_type = STOP;
			}

                break;
         case E_PLUS_E:
         case E_MINUS_E:
         case E_MULTIPLY_E:
            if (t1.e_type == STRING && t3.e_type == STRING && rule == E_PLUS_E)
			{
                *final_type = STRING;
                break;
            }
            if (t1.e_type == STRING || t3.e_type == STRING)
			{
                fprintf(stderr, "Snazis se spojit retezec a cislo\n" );
                return SEM_TYPE_ERROR;
            }
            if (t1.e_type == INT && t3.e_type == INT)
			{
                *final_type = INT;
                break;
            }
            *final_type = FLOAT;
                break;
         case E_DIVIDE_E:
		 case E_DIV_E:
             if (t1.e_type == STRING || t3.e_type == STRING)
			 {
                 fprintf(stderr, "Snazis se podelit retezec a cislo\n" );
                 return SEM_TYPE_ERROR;
             }
			 if (rule == E_DIV_E && t3.e_type == FLOAT)
			 {
			 	return SEM_TYPE_ERROR;
			 }
			 if (t1.e_type == INT && t3.e_type == STRING)
			 {
 				return SEM_TYPE_ERROR;
			 }
			 if (t1.e_type == FLOAT && t3.e_type == STRING)
			 {
 				return SEM_TYPE_ERROR;
			 }
			 if (t1.e_type == STRING && t3.e_type == INT)
			 {
 				return SEM_TYPE_ERROR;
			 }
			 if (t1.e_type == STRING && t3.e_type == FLOAT)
			 {
 				return SEM_TYPE_ERROR;
			 }
			 if (rule == E_DIVIDE_E)
			 {
				*final_type = FLOAT;
			 }
			 else
			 {
				 if (t1.e_type == FLOAT && t3.e_type == INT)
				 {
					return SEM_TYPE_ERROR;
				 }
			 	 if (t1.e_type == INT && t3.e_type == FLOAT)
				 {
					return SEM_TYPE_ERROR;
				 }
			*final_type = INT;
			}
			 break;
		case E_GROREQ_E:
		case E_LEOREQ_E:
		case E_LESS_E:
		case E_GREATER_E:

		if (t1.e_type == INT && t3.e_type == STRING)
		{
			return SEM_TYPE_ERROR;
		}
		if (t1.e_type == FLOAT && t3.e_type == STRING)
		{
			return SEM_TYPE_ERROR;
		}
		if (t1.e_type == STRING && t3.e_type == INT)
		{
			return SEM_TYPE_ERROR;
		}
		if (t1.e_type == STRING && t3.e_type == FLOAT)
		{
			return SEM_TYPE_ERROR;
		}
			if (t1.e_type == INT && t3.e_type == FLOAT)
			{
				// PRETYPOVANI t1 na float
			}
			if (t1.e_type == FLOAT && t3.e_type == INT)
			{
				// PRETYPOVANI t3 na float
			}

			*final_type = INDENT;
			break;

		case E_EQUAL_E:
		case E_NOTEQUAL_E:
			*final_type = INDENT;
			break;

        default: break;
      }
      return SYNTAX_IS_FINE;
}
int reduce(tStackTok *s, type_token *final_type, pData *data)
{
      tStackTok *stack_help;
	  stack_help = malloc(sizeof(tStackTok));
	  if (stack_help==NULL)
	  {
		  return INTERN_ERROR;
	  }
      stackTokInit(stack_help);

      Token token_loaded,e_token,reducing_token, dno;
	  token_loaded.string.size = 0;
	  e_token.string.size = 0;
	  reducing_token.string.size = 0;
	  dno.string.size = 0;
      e_token.type = E_TOK;
      e_token.s_top = 0;
	  dno.type = COLON;
	  dno.s_top = 0;

	  char str2[2] = "$";

	  int i = 0;
	  //stringy musia mat rovnaku dlzku!!!
	  while (str2[i]!= '\0'){
		  dyn_string(&(dno.string), str2[i]);
		  dyn_string(&(e_token.string), str2[i]);
		  i++;
	  }
	  stackTokPush(stack_help, dno);

      prec_table_rule rule = NOT_RULE;

	  int sem_check_return;
      int count = 0; // Counter popnutych znaku ze stacku
	  bool budeID = false;
	  bool budeKW = false;
      stackTokTop(s, &token_loaded); // nactu vrchni znak

      // Dokud na zasobniku nenarazim na STOP nebo KONEC STACKU nacitam
      while ((token_loaded.type != STOP) && (token_loaded.type != COLON))
      {
          stackTokPop(s); // Popnu vrchni znak
          stackTokPush(stack_help,token_loaded); // Vrchni znak ulozim na pomocny zasobnik
          count++;
          stackTokTop(s, &token_loaded);// Nactu dalsi
      }// While


      // Rule E -> i
      if (count == 1)
      {
          stackTokPop(s); // Odstanim STOP zaráažku.
          stackTokTop(stack_help, &reducing_token); //Nactu operand

          reducing_token.e_type = reducing_token.type; //jeto typ ulozim do e_type
          reducing_token.type = E_TOK; //Udelam z nej E_token
          stackTokPop(stack_help);

          rule = wchich_rule(count,reducing_token,reducing_token,reducing_token);
          if (rule == NOT_RULE)
          {
              fprintf(stderr, "ERROR, neexistuje pravidlo pro redukci vyrazu\n" );
              return SYNTAX_ERROR;
          }
		  sem_check_return = semantics_chceck(rule,reducing_token,reducing_token,final_type,data);
          if(sem_check_return != SYNTAX_IS_FINE)
		  {
			  return sem_check_return;
		  }

		  if (reducing_token.e_type == ID)
		  {
			budeID = true;
	 	  }
		  if (reducing_token.e_type == KW){
			  budeKW = true;
		  }

		  reducing_token.e_type = *final_type;

          stackTokPush(s,reducing_token); // Zpracovany reducing_token vymenim za E a jeho typ vlozim do e_type
		  if (budeID == true)
		  {
		  	reducing_token.e_type = ID;
		  }
		  if (budeKW == true)
		  {
			reducing_token.e_type = KW;
		  }

      }
      // Rule E-> E+E/E*E/E-E/..
      else if (count == 3)
      {
          stackTokPop(s); // Odstanim STOP zaráažku.
          Token t1,t3;
		  t1.string.size = 0;
		  t3.string.size = 0;

          stackTokTop(stack_help, &t3); //Nactu treti token z pomocneho STACKU
          stackTokPop(stack_help);
          stackTokTop(stack_help, &reducing_token);; //Nactu druhy token z pomocneho STACKU
          stackTokPop(stack_help);
          stackTokTop(stack_help, &t1); //Nactu prvni token z pomocneho STACKU
          stackTokPop(stack_help);

          rule = wchich_rule(count,t1,reducing_token,t3);
          if (rule == NOT_RULE)
          {
              fprintf(stderr, "ERROR, neexistuje pravidlo pro redukci vyrazu\n" );
              return SYNTAX_ERROR;
          }
		  sem_check_return = semantics_chceck(rule,t1,t3,final_type,data);
          if(sem_check_return != SYNTAX_IS_FINE)
		  {
			  return sem_check_return;
		  }

          e_token.e_type = *final_type;

          stackTokPush(s,e_token); // Zpracovane pravidlo reducing_token vymenim za E
      }
      // Pokud jsem nacetl jiny pocet, vstup neodpovida pravidlum
      else
      {
          fprintf(stderr, "ERROR,Na zasobniku jsou spatne usporadane neterminaly\n" );
          return SYNTAX_ERROR;
      }

      if (rule == NOT_RULE)
      {
          fprintf(stderr, "ERROR, Nelze pouzit zadne pravidlo -> spatny vstup\n" );
          return SYNTAX_ERROR;
      }
      else
      {




















//============== GENEROVANI KODU =====================================
// Vse co potrebujes mas v reducing_token
//      reducing_token
//           type - nepouzivas, tam je E_TOK
//           string - dane cislo
//           e_type - jeho typ

            switch (rule) {
                case E_PLUS_E:
                //genc_plus();

				if (data->in_func){
					//if (strings_concat)
					//	genc_concats(&out_);
					//else
						genc_adds(&out_);
				}
				else{
					//if (strings_concat)
					//	genc_concats(&main_);
					//else
						genc_adds(&main_);
				}

                break;
                case E_MINUS_E:
				if (data->in_func){
					genc_subs(&out_);
				}
				else{
					genc_subs(&main_);
				}

                break;
                case E_MULTIPLY_E:

				if (data->in_func){
					genc_muls(&out_);
				}
				else{
					genc_muls(&main_);
				}

                break;
                case E_DIVIDE_E:
                //    genc_deleno(..to si tam ja pridam argumenty...)
				if (data->in_func){
					genc_divs(&out_);
				}
				else{
					genc_divs(&main_);
				}

                break;
                case E_DIV_E:

				if (data->in_func){
					genc_idivs(&out_);
				}
				else{
					genc_idivs(&main_);
				}

                break;
                case LBR_E_RBR:

                break;
                case OPERAND:

				//ak to nie je v parametroch funckie
				if (data->prec_item != FUNC_EXP){
					if (data->in_func){
						if (reducing_token.e_type == INT)
							genc_pushs(&out_, 'I', reducing_token.string.str);
						if (reducing_token.e_type == FLOAT)
							genc_pushs(&out_, 'F', reducing_token.string.str);
						if (reducing_token.e_type == ID)
							genc_pushs(&out_, 'L', reducing_token.string.str);
						if (reducing_token.e_type == KW){
							genc_pushs(&out_, 'N', "nil");
						}
						if ((reducing_token.e_type == STRING) || (reducing_token.e_type == STRING_BLOCK))
							genc_pushs(&out_, 'S', reducing_token.string.str);
					}
					else{
						if (reducing_token.e_type == ID){
							genc_pushs(&main_, 'G', reducing_token.string.str);
						}
						if (reducing_token.e_type == KW){
							//fprintf(stderr, "NIL !!!!!!!!!!\n" );
							genc_pushs(&main_, 'N', "nil");
						}
						if (reducing_token.e_type == INT){
							genc_pushs(&main_, 'I', reducing_token.string.str);
						}
						if (reducing_token.e_type == FLOAT)
							genc_pushs(&main_, 'F', reducing_token.string.str);
						if ((reducing_token.e_type == STRING) || (reducing_token.e_type == STRING_BLOCK)){
							genc_pushs(&main_, 'S', reducing_token.string.str);
							//strings_concat = true;
						}
					}
				}
				else{
					if (reducing_token.e_type == INT){
						stackTokPush(ParamsStack, reducing_token);
					}
					if (reducing_token.e_type == FLOAT)
						stackTokPush(ParamsStack, reducing_token);
					if ((reducing_token.e_type == STRING) || (reducing_token.e_type == STRING_BLOCK)){
						stackTokPush(ParamsStack, reducing_token);
					}

				}

				break;
				case E_GROREQ_E:
					if (data->in_func){
						genc_ges(&out_);
					}
					else{
						genc_ges(&main_);
					}
				break;
				case E_LEOREQ_E:
					if (data->in_func){
						genc_les(&out_);
					}
					else{
						genc_les(&main_);
					}
				break;
				case E_LESS_E:
					//codegen_send(&main_, "LTS\n");
					if (data->in_func){
						genc_lts(&out_);
					}
					else{
						genc_lts(&main_);
					}
				break;
				case E_GREATER_E:
					if (data->in_func){
						genc_gts(&out_);
					}
					else{
						genc_gts(&main_);
					}
				break;
				case E_EQUAL_E:
					if (data->in_func){
						genc_eqs(&out_);
					}
					else{
						genc_eqs(&main_);
					}
				break;
				case E_NOTEQUAL_E:
					if (data->in_func){
						genc_neqs(&out_);
					}
					else{
						genc_neqs(&main_);
					}
				break;

                default: break;
            } // end SWITCH
      } // end else if (rule == NOT_RULE)

      return SYNTAX_IS_FINE;
  }

  int input_token_type(Token *t, pData *data)
  {
	 // Kontrola vyrazu pouze v parametrech funkce
	if (data->prec_item == FUNC_EXP)
	{
		switch(t->type)
		{
		  case MINUS:
		  case PLUS: return 0;
		  case MULTIPLY:
		  case DIVIDE:
		  case DIV: return 1;
		  case ID:
		  case STRING:
		  case INT:
		  case FLOAT: return 5;
		  case RBRACKET:
		  case COLON:
		  case COMMA: return 6;
			// chyba SA, nic jineho nemuze byt ve vyrazu
			default: return TOK_TYPE_SEM_TYPE_ERR;
		}
	}
	//Kontrola vyrazu v pondince pripadne za =
	else
	{
	  	switch(t->type)
		{
	      case MINUS:
	      case PLUS: return 0;
	      case MULTIPLY:
	      case DIVIDE:
	      case DIV: return 1;
	      case EQUAL:
	      case GR_OR_EQ:
	      case LE_OR_EQ:
	      case LESS:
	      case GREATER:
	      case NOT_EQUAL: return 2;
	      case LBRACKET: return 3;
	      case RBRACKET: return 4;
	      case ID:
	      case STRING:
	      case INT:
	      case FLOAT: return 5;
		  case KW:
		  	if (!strcmp(t->string.str,"None"))
			{
				return 5;
		  	}
			fprintf(stderr, "KLicove slovo bylo jine nez None\n" );
			return TOK_TYPE_SYNTAX_ERR;
	      case EOL:
	      case COLON: return 6;
	  		// chyba SA, nic jineho nemuze byt ve vyrazu
	  		default: return TOK_TYPE_SYNTAX_ERR;
  		} //switch
	} //else
  }

  // Szntakticka precedencni tabulka
  char prec_table [7][7]={
      //          | +- |*,/,// | rel |   (   |  )   |  i   |  $
      /*   +-  */ {'>' , '<'  , '>'  ,  '<'  , '>'  , '<'  , '>' },
      /* *,/,//*/ {'>' , '>'  , '>'  ,  '<'  , '>'  , '<'  , '>' },
      /*  rel  */ {'<' , '<'  , '-'  ,  '<'  , '>'  , '<'  , '>' },  // rel = relacni operator: <, <=, >, >=, ==, !=
      /*   (   */ {'<' , '<'  , '<'  ,  '<'  , '='  , '<'  , '-' },
      /*   )   */ {'>' , '>'  , '>'  ,  '-'  , '>'  , '-'  , '>' },
      /*   i   */ {'>' , '>'  , '>'  ,  '-'  , '>'  , '-'  , '>' }, // i = id, int, double, string
      /*   $   */ {'<' , '<'  , '<'  ,  '<'  , '-'  , '<'  , '_' },
  };

int prec_check(pData *data)
{
  	 // Vytvoreni a inicializace stacku na tokeny
  	 tStackTok *stack;
	 stack = malloc(sizeof(tStackTok));
	 if (stack==NULL)
	 {
		 return INTERN_ERROR;
	 }
  	 stackTokInit(stack);

     int column,line,kontorla_vyrazu,get,reduce_return;
     char prec_symbol = '\0';
     bool no_scan = false;

     type_token final_type;

     Token on_stack,previous_tok,actual_tok;
	 on_stack.string.size = 0;
	 previous_tok.string.size = 0;
	 actual_tok.string.size = 0;
     Token dno,e_token;
     dno.type = COLON;
     dno.s_top = 0;
	 dno.string.size = 0;

	 e_token.type = E_TOK;
	 e_token.s_top = 0;
	 e_token.string.size = 0;

	 char str1[2] = {'$', '\0'};

	 int i = 0;

	 //stringy musia mat rovnaku dlzku!!!
	 while (str1[i]!= '\0')
	 {
		 dyn_string(&(dno.string), str1[i]);
		 dyn_string(&(e_token.string), str1[i]);
		 i++;
	 }

     stackTokPush(stack,dno);

     copy_token(&previous_tok,dno);
     // Jelikoz prvni token mi nacte scanner, preskakuju prvni nacteni
     goto FIRST_TOKEN_SCANNED;

     // Dokud se nenactu cely vyraz
	 while (1) {

	    if (no_scan == false)
		{
	               copy_token(&previous_tok, actual_tok);
	               TOKEN()
	         FIRST_TOKEN_SCANNED:
	               copy_token(&actual_tok, data->token);

				   if (data->token.type == STRING_BLOCK) {
				   	data->token.type = STRING;
				   }
				   if (data->token.type == COMMENT) {
				   	continue;
				   }


	               kontorla_vyrazu = operand_check(actual_tok,previous_tok,data);
	               if (kontorla_vyrazu == DIV_0_ERROR)
	               {
	                    return DIV_0_ERROR;
	               }
	               else if ( kontorla_vyrazu == SYNTAX_ERROR)
				   {
	                    return SYNTAX_ERROR;
	               }
				   else if (kontorla_vyrazu == 69)
				   {
				   	return SYNTAX_IS_FINE;
				   }
	   } //If no scan

       no_scan = false;

	   column = input_token_type(&(data->token),data);
       if (column == TOK_TYPE_SEM_TYPE_ERR || column == TOK_TYPE_SYNTAX_ERR )
	   {
           fprintf(stderr, "CHYBA, nacteny token obsahuje nepovoleny znak pro expression %d \n", data->token.type );
		   if (column == TOK_TYPE_SYNTAX_ERR)
		   {
		   		return SYNTAX_ERROR;
		   }
		   //column = TOK_TYPE_SEM_TYPE_ERR
		   else
		   {
  			return SEM_TYPE_ERROR;
	       }
       }

       stackTokTop(stack, &on_stack);

       // Na zjisteni z precendecni tabuly potrebuju terminal, pokud je na vrcholu nererminal, preskocim jej
       if (on_stack.type == 29)
	   {
           e_na_stacku = true;
		   copy_token(&e_token,on_stack);
           stackTokPop(stack);
           stackTokTop(stack, &on_stack);
       }

       line = input_token_type(&on_stack,data);

       if(e_na_stacku == true)
       {
           stackTokPush(stack,e_token);
           e_na_stacku = false;
       }

       prec_symbol = prec_table[line][column];

       if (prec_symbol == '<')
	   {
            insert_stop(stack);
    		stackTokPush (stack,data->token);
    		continue;	//Pokud pushnu, rovnou nacitam dalsi znak.
       }

       if (prec_symbol == '>')
       {
		     reduce_return = reduce(stack,&final_type,data);
             if(reduce_return != SYNTAX_IS_FINE)
             {
				 if (is_defined(data,previous_tok)!=SYNTAX_IS_FINE)
				 {
				  	return SEM_TYPE_ERROR;
			  	 }
                 return reduce_return;
             }

             no_scan = true; //Nechci nacitat dalsi znak, jelikoz jsem zadny nepushnul
             continue;
       }

       if (prec_symbol == '=')
	   {
           stackTokPush(stack,data->token);
           continue;
       }

       if (prec_symbol == '-')
	   {
           fprintf(stderr, "\nCHYBA ve vyrazu\n" );
           return SYNTAX_ERROR;
       }

       if (prec_symbol == '_')
	   {
		   switch (final_type)
		   {
			   	case INT:
					data->type_id = T_INT;
					break;
				case FLOAT:
					data->type_id = T_FLOAT;
					break;
				case STRING:
					data->type_id = T_STRING;
					break;
				case INDENT:
					data->type_id = T_BOOL;
					break;
				case STOP:
				 	data->type_id = T_NONE;
					break;
				default: fprintf(stderr, "ERROR, Chybna navratova hodnota\n" );
				 return SYNTAX_ERROR;
		   }
           fprintf( stderr, "\nVyraz byl vyhodnocen spravne a final_type je %d \n", data->type_id );
           break;
       }

	}  // DO (1)
   return last_token_check(data);
}
