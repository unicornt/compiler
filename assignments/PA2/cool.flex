/*
 *  The scanner definition for COOL.
 */

/*
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Dont remove anything that was here initially
 */
%{
#include <cool-parse.h>
#include <stringtab.h>
#include <utilities.h>

/* The compiler assumes these identifiers. */
#define yylval cool_yylval
#define yylex  cool_yylex

/* Max size of string constants */
#define MAX_STR_CONST 1025
#define YY_NO_UNPUT   /* keep g++ happy */

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the Cool compiler.
 */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread( (char*)buf, sizeof(char), max_size, fin)) < 0) \
		YY_FATAL_ERROR( "read() in flex scanner failed");

char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr;

extern int curr_lineno;
extern int verbose_flag;

extern YYSTYPE cool_yylval;


/*
 *  Add Your own definitions here
 */

int comment_dep;
char char_tmp;

%}

/*
 * Define names for regular expressions here.
 */

%option noyywrap 
%x COMMENT
%x STRING
%x STRING_ESCAPE
DARROW          =>
OBJECT          [A-Za-z_][A-Za-z0-9_]*
TYPE            [A-Z_][A-Z_]*
WHITESPACE      [ \t\f\r\v]
CLASS           (?i:class)
INHERITS        (?i:INHERITS)
EOL             [\n]

%%

 /*
  *  Nested comments
  */
"(*" {
  comment_dep += 1;
  BEGIN(COMMENT);
}
<COMMENT>"*)" {
  comment_dep -= 1;
  if(comment_dep == 0)
    BEGIN(INITIAL);
}
<COMMENT><<EOF>> {
  cool_yylval.error_msg = "EOF in comment";
  BEGIN(INITIAL);
  return (ERROR);
}
<COMMENT>[^(\*\))] {
  if (yytext[0] == '\n')
    curr_lineno += 1;
}

 /*
  *  The multiple-character operators.
  */
{DARROW}		{ return (DARROW); }

 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */
{CLASS}     { return (CLASS); }
{INHERITS}  { return (INHERITS); }


 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */

[\"] {
  string_buf_ptr = string_buf;
  BEGIN(STRING);
}

<STRING>[\"] {
  cout << "END STRING" << endl;
  BEGIN(INITIAL);
}

<STRING>[\\] {
  BEGIN(STRING_ESCAPE);
}

<STRING_ESCAPE>[.] {
  char_tmp = yytext[0];
  switch(char_tmp){
    case 'n':
      char_tmp = '\n';
      break;
    case 'b':
      char_tmp = '\b';
      break;
    case 't':
      char_tmp = '\t';
      break;
    case '\\':
      char_tmp = '\\';
      break;
    default:
      break;
  }
  if(string_buf_ptr == string_buf + MAX_STR_CONST) {
    cool_yylval.error_msg = "Too long string length";
    return (ERROR);
  }
  *(string_buf_ptr++) = char_tmp;
  BEGIN(STRING);
}

{TYPE}    { 
  cool_yylval.symbol = idtable.add_string(yytext, yyleng);
  return TYPEID; 
}

{OBJECT}    { 
  cool_yylval.symbol = idtable.add_string(yytext, yyleng);
  return OBJECTID; 
}
{WHITESPACE} {}

{EOL} {
  curr_lineno += 1;
}

. {
  return yytext[0];
}

%%
