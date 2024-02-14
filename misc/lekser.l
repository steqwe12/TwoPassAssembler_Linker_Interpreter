%{
  #include <iostream>
  #include <cstring>
  #include "../inc/parser.hpp"
  #include "../inc/helpers.hpp"
  int line_num = 1;
%}





%option noyywrap
%xstate COMMENT


GPR (r[0123456789])|r10|r11|r12|r13|r14|r15|sp|pc 
CSR status|handler|cause
HEX 0[Xx][0-9A-Fa-f]+
DEC [0-9]+
IDENT [A-Za-z_\.][A-Za-z0-9_\.]*


%%

" "     { }
"\r"    { }
"\b"    { }
"\t"    { }
"\n"    { ++line_num; return ENDL; }
"\r\n"  { ++line_num; return ENDL; }
"\f"    { }

".global"   { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_DIR_GLOBEXT; }
".extern"   { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_DIR_GLOBEXT; }
".end"      { return END; }
".section"  { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_DIR_SECT; }
".word"     { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_DIR_WORD; }
".skip"     { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_DIR_SKIP; }
":"         { return TOKEN_COLON; }
","         { return TOKEN_COMMA; }
"["         { return TOKEN_LBRACKET; }
"]"         { return TOKEN_RBRACKET; }
"+"         { return TOKEN_PLUS; }



"halt"      { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_INST_NOPAR; }
"int"       { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_INST_NOPAR; }
"iret"      { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_INST_NOPAR; }
"call"      { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_INST_CALJMP; }
"ret"       { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_INST_NOPAR; }
"jmp"       { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_INST_CALJMP; }
"beq"       { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_INST_BRCOND; }
"bne"       { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_INST_BRCOND; }
"bgt"       { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_INST_BRCOND; }
"push"      { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_INST_PUSHPOPNOT; }
"pop"       { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_INST_PUSHPOPNOT; }
"xchg"      { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_INST_ARITHLOG; }
"add"       { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_INST_ARITHLOG; }
"sub"       { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_INST_ARITHLOG; }
"mul"       { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_INST_ARITHLOG; }
"div"       { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_INST_ARITHLOG; }
"not"       { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_INST_PUSHPOPNOT; }
"and"       { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_INST_ARITHLOG; }
"or"        { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_INST_ARITHLOG; }
"xor"       { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_INST_ARITHLOG; }
"shl"       { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_INST_ARITHLOG; }
"shr"       { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_INST_ARITHLOG; }
"ld"        { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_LD; }
"st"        { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_ST; }
"csrrd"     { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_CSRRD; }
"csrwr"     { yylval.name=new char[strlen(yytext) + 1]; std::strcpy(yylval.name,yytext); return TOKEN_CSRWR; }

"#"             { BEGIN(COMMENT); }
<COMMENT>"\n"   { BEGIN(INITIAL); ++line_num; return ENDL; }
<COMMENT>"\r\n" { BEGIN(INITIAL); ++line_num; return ENDL; }
<COMMENT>.      { BEGIN(COMMENT); }


"%"{GPR}  {  yylval.ident=new char[strlen(yytext) + 1]; 
  std::strcpy(yylval.ident,yytext); // copy_str ako ne radi strdup
return TOKEN_GPR_PERC; }
"%"{CSR}  { yylval.ident=new char[strlen(yytext) + 1];
  std::strcpy(yylval.ident,yytext); 
return TOKEN_CSR; }


"$"{HEX}     { yylval.ident=new char[strlen(yytext) + 1];
  std::strcpy(yylval.ident,yytext); return TOKEN_NUM_DOLLAR; }
"$"{DEC}     { yylval.ident=new char[strlen(yytext) + 1];
  std::strcpy(yylval.ident,yytext); return TOKEN_NUM_DOLLAR; }

{HEX}     { yylval.ident=new char[strlen(yytext) + 1];
  std::strcpy(yylval.ident,yytext); return TOKEN_NUM; }
{DEC}     { yylval.ident=new char[strlen(yytext) + 1];
  std::strcpy(yylval.ident,yytext); return TOKEN_NUM; }


"$"{IDENT}   { yylval.ident=new char[strlen(yytext) + 1];
  std::strcpy(yylval.ident,yytext); 
return TOKEN_IDENT_DOLLAR; }

{IDENT}   { yylval.ident=new char[strlen(yytext) + 1];
  std::strcpy(yylval.ident,yytext); 
return TOKEN_IDENT; }



%%