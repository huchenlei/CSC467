/* Definition section */
%{
  /**********************************************************************
   *
   * **YOUR GROUP INFO SHOULD GO HERE**
   *
   *  Interface to the scanner module for CSC467 course project
   **********************************************************************/


#include "common.h"
#include "parser.tab.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#define YY_USER_INIT { yyin = inputFile; }
#define	yyinput      input
#define yTRACE(x)    { if (traceScanner) fprintf(traceFile, "TOKEN %3d : %s\n", x, yytext); }
#define yERROR(x)    { fprintf(errorFile, "\nLEXICAL ERROR, LINE %d: %s\n", yyline, x); errorOccurred = TRUE; yyterminate(); }

  int CheckInt(void);
  int yyline = 1;

%}
%option noyywrap
/* Defines macros */
WS          [ \t\n]
DELIM       [ \t\n\(\)\[\]\{\},\;\+\-\*\/\^\=\!\<\>]|&&|"||"|==|!=|"<"=|">"=

%%
void/{DELIM} {yTRACE(T_VOID); return T_VOID;}
int/{DELIM} {yTRACE(T_INT); return T_INT;}
bool/{DELIM} {yTRACE(T_BOOL); return T_BOOL;}
float/{DELIM} {yTRACE(T_FLOAT); return T_FLOAT;}
vec2/{DELIM} {yTRACE(T_VEC2); return T_VEC2;}
vec3/{DELIM} {yTRACE(T_VEC3); return T_VEC3;}
vec4/{DELIM} {yTRACE(T_VEC4); return T_VEC4;}
bvec2/{DELIM} {yTRACE(T_BVEC2); return T_BVEC2;}
bvec3/{DELIM} {yTRACE(T_BVEC3); return T_BVEC3;}
bvec4/{DELIM} {yTRACE(T_BVEC4); return T_BVEC4;}
ivec2/{DELIM} {yTRACE(T_IVEC2); return T_IVEC2;}
ivec3/{DELIM} {yTRACE(T_IVEC3); return T_IVEC3;}
ivec4/{DELIM} {yTRACE(T_IVEC4); return T_IVEC4;}

"(" {yTRACE(LBRAC); return LBRAC;}
")" {yTRACE(RBRAC); return RBRAC;}
"[" {yTRACE(LSBRAC); return LSBRAC;}
"]" {yTRACE(RSBRAC); return RSBRAC;}
"{" {yTRACE(LSCOPE); return LSCOPE;}
"}" {yTRACE(RSCOPE); return RSCOPE;}

, {yTRACE(COMMA); return COMMA;}
; {yTRACE(SEMICOLON); return SEMICOLON;}

"+" {yTRACE(ADD); return ADD;}
"-" {yTRACE(SUB); return SUB;}
"*" {yTRACE(MUL); return MUL;}
"/" {yTRACE(DIV); return DIV;}
"^" {yTRACE(POW); return POW;}

"=" {yTRACE(ASSIGN); return ASSIGN;}

(([1-9][0-9]*)|0)/{DELIM} {
  /* INT */
  // numb ptr to store last address of non numerical value encountered in yytext
  // not used later
  char* numb_ptr;
  long val = strtol(yytext, &numb_ptr, 10);
  if (errno == ERANGE || (val > INT_MAX || val < INT_MIN)) {
    // number out of range of long
    // or number out of range of int
    yERROR("Int value out of bound!");
  } else {
    yylval.intVal = (int)val;
    yTRACE(V_INT);
    return V_INT;
  }
}

(([0-9]+"."[0-9]*)|([0-9]*"."[0-9]+))/{DELIM} {
  /* FLOAT */
  char* numb_ptr;
  float val = strtof(yytext, &numb_ptr);
  if (errno == ERANGE) {
    yERROR("Float value out of bound!");
  } else {
    yylval.floatVal = val;
    yTRACE(V_FLOAT);
    return V_FLOAT;
  }
}

(true|false)/{DELIM} {
  /* BOOLEAN */
  const char* trueVal = "true";
  yylval.boolVal = (strncmp(yytext, trueVal, (strlen(trueVal) + 1)*sizeof(char)) == 0);
  yTRACE(V_BOOL);
  return V_BOOL;
}

[a-zA-Z_][a-zA-Z0-9_]* {
  if (yyleng > 32) {
    yERROR("Identifier exceed maximun length(32)");
  } else {
    yylval.id = (char*) malloc((yyleng + 1) * sizeof(char));
    if (yylval.id != NULL) {
      strncpy(yylval.id, yytext, yyleng + 1);
      yTRACE(IDENTIFIER);
      return IDENTIFIER;
    } else {
      yERROR("Unable to allocate memory for identifier string");
    }
  }
}

   "/*"        {
   int c1      = 0;
   int c2      = yyinput();
   int curline = yyline;
   for(;;) {
     if (c2 == EOF) {
       fprintf(errorFile, "\nLEXICAL ERROR, LINE %d: Unmatched /*\n", curline);
       errorOccurred = TRUE;
       yyterminate();
     }
     if (c1 == '*' && c2 == '/')
       break;
     c1 = c2;
     c2 = yyinput();
     if (c1 == '\n' && c2 != EOF)
       yyline++;
   }
 }

{WS}+        {/*do nothing*/}

   .          { yERROR("Unknown token"); }

%%
