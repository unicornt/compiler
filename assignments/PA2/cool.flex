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
int skip_char;

%}

/*
 * Define names for regular expressions here.
 */

%option noyywrap 
%x COMMENT
%x STRING
%x STRING_ESCAPE
DARROW          =>
DIGIT           [0-9]+
OBJECT          [a-z_][A-Za-z0-9_]*
TYPE            [A-Z_][A-Za-z0-9_]*
WHITESPACE      [ \t\f\r\v]
CLASS           (?i:class)
ELSE            (?i:else)
FI              (?i:fi)
IF              (?i:if)
THEN            (?i:then)
IN              (?i:in)
INHERITS        (?i:inherits)
LET             (?i:let)
LOOP            (?i:loop)
POOL            (?i:pool)
WHILE           (?i:while)
CASE            (?i:case)
ESAC            (?i:esac)
OF              (?i:of)
NEW             (?i:new)
ISVOID          (?i:isvoid)
ASSIGN          <-
NOT             (?i:not)
LE              <=
EOL             [(\r)?\n]

%%

 /*
  *  Nested comments
  */
"(*" {
  comment_dep += 1;
  BEGIN(COMMENT);
}

[(|)] {
  return yytext[0];
}

<COMMENT>"*)" {
  comment_dep -= 1;
  if(comment_dep == 0)
    BEGIN(INITIAL);
}

"*)" {
  cool_yylval.error_msg = "Unmatched *)";
  return (ERROR);
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
{ASSIGN}    { return (ASSIGN); }
{LE}        { return (LE); }

 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */
{CLASS}     { return (CLASS); }
{ELSE}      { return (ELSE); }
{FI}        { return (FI); }
{IF}        { return (IF); }
{IN}        { return (IN); }
{INHERITS}  { return (INHERITS); }
{LET}       { return (LET); }
{LOOP}      { return (LOOP); }
{POOL}      { return (POOL); }
{THEN}      { return (THEN); }
{WHILE}     { return (WHILE); }
{CASE}      { return (CASE); }
{ESAC}      { return (ESAC); }
{OF}        { return (OF); }
{NEW}       { return (NEW); }
{ISVOID}    { return (ISVOID); }
{NOT}       { return (NOT); }
{ELSE}      { return (ELSE); }
{DIGIT}     {
  cool_yylval.symbol = inttable.add_string(yytext, yyleng);
  return (INT_CONST);
}

t[Rr][Uu][Ee] {
  cool_yylval.boolean = true;
  return (BOOL_CONST);
}

f[Aa][Ll][Ss][Ee] {
  cool_yylval.boolean = false;
  return (BOOL_CONST);
}

 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */

[\"] {
  string_buf_ptr = string_buf;
  skip_char = 0;
  BEGIN(STRING);
}

<STRING>[\"] {
  if(skip_char == 0){
    *string_buf_ptr = '\0';
    cool_yylval.symbol = inttable.add_string(string_buf);
    BEGIN(INITIAL);
    return (STR_CONST);
  }
  else {
    BEGIN(INITIAL);
  }
}

<STRING>[\\] {
  BEGIN(STRING_ESCAPE);
}

<STRING><<EOF>> {
  cool_yylval.error_msg = "EOF in string constant";
  BEGIN(INITIAL);
  return (ERROR);
}

<STRING>{EOL} {
  curr_lineno += 1;
  if(skip_char == 0){
    cool_yylval.error_msg = "Unterminated string constant";
    BEGIN(INITIAL);
    return (ERROR);
  }
  else
    BEGIN(INITIAL);
}

<STRING>'\0' {
  cool_yylval.error_msg = "String contains null character";
  skip_char = 1;
  return (ERROR);
}

<STRING>. {
  if(skip_char == 0) {
    if(string_buf_ptr == string_buf + MAX_STR_CONST) {
      cool_yylval.error_msg = "String constant too long";
      skip_char = 1;
      return (ERROR);
    }
    *(string_buf_ptr++) = yytext[0];
  }
}

<STRING_ESCAPE>{EOL} {
  curr_lineno += 1;
  if(string_buf_ptr == string_buf + MAX_STR_CONST) {
    cool_yylval.error_msg = "String constant too long";
    skip_char = 1;
    return (ERROR);
  }
  *(string_buf_ptr++) = '\n';
  BEGIN(STRING);
}

<STRING_ESCAPE>. {
  char_tmp = yytext[0];
  // cout << char_tmp << endl;
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
  if(skip_char == 0) {
    if(string_buf_ptr == string_buf + MAX_STR_CONST) {
      cool_yylval.error_msg = "String constant too long";
      skip_char = 1;
      return (ERROR);
    }
    *(string_buf_ptr++) = char_tmp;
  }
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

[\[\]\'>] {
  cool_yylval.error_msg = yytext;
  return (ERROR);
}

. {
  return yytext[0];
}

%%
 