%{
  #include "../inc/helpers.hpp"
  #include <cstdlib>  //#include <stdlib.h>
  #include <iostream>
  #include <stdio.h>
  
  //using namespace std;
  
  
  extern int yylex(void);
    
  extern int line_num;

  void yyerror(const char* s);

%}

//%output "parser.cpp"
//%defines "parser.hpp"



%union {
  int num;
  char *ident;
  struct arg *arg;
  struct instr *instr;

  char *name;
}


%token ENDL
%token END
%token TOKEN_COLON
%token TOKEN_COMMA

%token <ident> TOKEN_GPR_PERC
%token <ident> TOKEN_CSR

%token <ident> TOKEN_NUM_DOLLAR
%token <ident> TOKEN_IDENT_DOLLAR

%token <ident> TOKEN_NUM
%token <ident> TOKEN_IDENT


%token <name> TOKEN_DIR_GLOBEXT
%token <name> TOKEN_DIR_SECT
%token <name> TOKEN_DIR_WORD
%token <name> TOKEN_DIR_SKIP

%token TOKEN_LBRACKET
%token TOKEN_RBRACKET
%token TOKEN_PLUS


%token <name> TOKEN_INST_NOPAR
%token <name> TOKEN_INST_CALJMP
%token <name> TOKEN_INST_BRCOND
%token <name> TOKEN_INST_PUSHPOPNOT
%token <name> TOKEN_INST_ARITHLOG
%token <name> TOKEN_LD
%token <name> TOKEN_ST
%token <name> TOKEN_CSRRD
%token <name> TOKEN_CSRWR


%type <arg> operand_st
%type <arg> operand_ld
%type <instr> asemblerska_naredba

%type <ident> literal
%type <ident> simbol
%type <ident> literal_dollar
%type <ident> simbol_dollar
%type <ident> simbol_ili_literal

%type <arg> lista_simbola_i_literala
%type <arg> lista_simbola

%type <instr> linija_bez_labele


%%

prog : endlss linije opcija1
     ;

opcija1 : endlss ENDL END { YYACCEPT; }
        |
        endlss
        ;     

linije : linije endlss ENDL linija
        |
        linija
        ;

linija : TOKEN_IDENT TOKEN_COLON endlss linija_bez_labele
         { struct instr *a = $4; a->label=$1; }
        |
        linija_bez_labele
        ;       

linija_bez_labele : TOKEN_DIR_GLOBEXT lista_simbola
                    { $$ = mk_instruction(nullptr,$1,$2,nullptr); }
                    |
                    TOKEN_DIR_SECT TOKEN_IDENT
                    { $$ = mk_instruction(nullptr,$1,mk_argument($2,0,nullptr),nullptr); }
                    |
                    TOKEN_DIR_WORD lista_simbola_i_literala
                    { $$ = mk_instruction(nullptr,$1,$2,nullptr); }
                    |
                    TOKEN_DIR_SKIP literal
                    { $$ = mk_instruction(nullptr,$1,mk_argument($2,0,nullptr),nullptr); }
                    |
                    asemblerska_naredba{ $$ = $1; }
                    ;

endlss : endlss ENDL
       |
       /* epsilon */
       ;

lista_simbola_i_literala : lista_simbola_i_literala TOKEN_COMMA simbol_ili_literal
                           { struct arg *a = $1; while(a->next!=nullptr) a=a->next; 
                           a->next = mk_argument($3,0,nullptr); $$ = $1; } 
                         |
                         simbol_ili_literal{ $$ = mk_argument($1,0,nullptr); }                                               
                         ;

simbol_ili_literal : simbol{ $$ = $1; } | literal{ $$ = $1; };

lista_simbola : lista_simbola TOKEN_COMMA simbol
                { struct arg *a = $1; while(a->next!=nullptr) a=a->next; 
                a->next = mk_argument($3,0,nullptr); $$ = $1; } 
              |
              simbol{ $$ = mk_argument($1,0,nullptr); }         
              ;

simbol : TOKEN_IDENT{ $$ = $1; };

literal : TOKEN_NUM{ $$ = $1; };

simbol_dollar : TOKEN_IDENT_DOLLAR{ $$ = $1; }; // ako ne bude htelo da se napravi argument onda pravim $simbol sa razmakom

literal_dollar : TOKEN_NUM_DOLLAR{ $$ = $1; }; // isto ovde

asemblerska_naredba : TOKEN_INST_NOPAR{ $$ = mk_instruction(nullptr,$1,nullptr,nullptr); }
                    |
                    TOKEN_INST_CALJMP simbol_ili_literal
                    { $$ = mk_instruction(nullptr,$1,mk_argument($2,0,nullptr),nullptr); }                   
                    |
                    TOKEN_INST_BRCOND TOKEN_GPR_PERC TOKEN_COMMA TOKEN_GPR_PERC TOKEN_COMMA simbol_ili_literal
                    { $$ = mk_instruction(nullptr,$1,mk_argument($2,0,mk_argument($4,0,mk_argument($6,0,nullptr))),nullptr); }
                    |
                    TOKEN_INST_PUSHPOPNOT TOKEN_GPR_PERC
                    { $$ = mk_instruction(nullptr,$1,mk_argument($2,0,nullptr),nullptr); }
                    |
                    TOKEN_INST_ARITHLOG TOKEN_GPR_PERC TOKEN_COMMA TOKEN_GPR_PERC
                    { $$ = mk_instruction(nullptr,$1,mk_argument($2, 0, mk_argument($4, 0, nullptr)),nullptr); }
                    |
                    TOKEN_LD operand_ld TOKEN_COMMA TOKEN_GPR_PERC
                    { struct arg *a = $2; while(a->next!=nullptr) a=a->next; 
                      a->next = mk_argument($4,0,nullptr);
                      $$ = mk_instruction(nullptr,$1,$2,nullptr);
                      }
                    |
                    TOKEN_ST TOKEN_GPR_PERC TOKEN_COMMA operand_st
                    { 
                      $$ = mk_instruction(nullptr,$1,mk_argument($2,0,$4),nullptr);
                     }
                    |
                    TOKEN_CSRRD TOKEN_CSR TOKEN_COMMA TOKEN_GPR_PERC
                    { $$ = mk_instruction(nullptr,$1,mk_argument($2, 0, mk_argument($4, 0, nullptr)),nullptr); }
                    |
                    TOKEN_CSRWR TOKEN_GPR_PERC TOKEN_COMMA TOKEN_CSR
                    { $$ = mk_instruction(nullptr,$1,mk_argument($2, 0, mk_argument($4, 0, nullptr)),nullptr); }
                    ;


operand_ld : simbol{ $$ = mk_argument($1, 4, nullptr); }
        |
        literal{ $$ = mk_argument($1, 3, nullptr); }
        |
        simbol_dollar{ $$ = mk_argument($1, 2, nullptr); }
        |
        literal_dollar{ $$ = mk_argument($1, 1, nullptr); }
        |
        TOKEN_GPR_PERC{ $$ = mk_argument($1, 5, nullptr); }
        |
        TOKEN_LBRACKET TOKEN_GPR_PERC TOKEN_RBRACKET
        { $$ = mk_argument($2, 6, nullptr); }
        |
        TOKEN_LBRACKET TOKEN_GPR_PERC TOKEN_PLUS literal TOKEN_RBRACKET
        { $$ = mk_argument($2, 7, mk_argument($4, 7, nullptr)); }
        ;

operand_st : simbol{ $$ = mk_argument($1, 4, nullptr); }
        |
        literal{ $$ = mk_argument($1, 3, nullptr); }
        |
        TOKEN_LBRACKET TOKEN_GPR_PERC TOKEN_RBRACKET{ $$ = mk_argument($2, 6, nullptr); }
        |
        TOKEN_LBRACKET TOKEN_GPR_PERC TOKEN_PLUS simbol TOKEN_RBRACKET
        { $$ = mk_argument($2, 8, mk_argument($4, 8, nullptr)); }
        |
        TOKEN_LBRACKET TOKEN_GPR_PERC TOKEN_PLUS literal TOKEN_RBRACKET
        { $$ = mk_argument($2, 7, mk_argument($4, 7, nullptr)); }
        ;       

%%

void yyerror(const char* s){
    std::cout << "lexxxer error: " << s << " : linija: " << line_num << std::endl;
    //cout << " parse error on line " << line_num << "!  Message: " << s << endl;
    // ovde bi trebala dealokacija da ne bude mem leak mozda
    std::exit(-1); 
}
