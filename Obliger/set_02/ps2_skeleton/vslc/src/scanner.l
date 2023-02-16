%{
#include <vslc.h>
// The tokens defined in parser.y
#include "y.tab.h"
%}
%option noyywrap
%option array
%option yylineno

WHITESPACE [\ \t\v\r\n]
COMMENT \/\/[^\n]+
QUOTED \"([^\"\n]|\\\")*\"

STRINGS \"^\n*\"
NUMBERS [0-9]+
IDENTIFIERS [a-zA-Z][a-zA-Z0-9_]*

%%
func                    {return FUNC;}
print                   {return PRINT;}
return                  {return RETURN;}
break                   {return BREAK;}
if                      {return IF;}
then                    {return THEN;}
else                    {return ELSE;}
while                   {return WHILE;}
for                     {return FOR;}
in                      {return IN;}
do                      {return DO;}
begin                   {return OPENBLOCK;}
end                     {return CLOSEBLOCK;}
var                     {return VAR;}

{WHITESPACE}+           { /* Eliminate whitespace */ }
{COMMENT}               { /* Eliminate comments */ }
{QUOTED}                { return STRING; }
  /* 
   TODO:

   Add the rest of the translation rules here.
   See the lexical structure definition of the modified VSL in PS2.
   Also see the `%token` directives in parser.y for all symbolic names that can be returned - e.g. STRING, NUMBER, IDENTIFIER.

   Hint to get you started:
   The scanner returns STRING when matching the QUOTED regex above.
   When should the scanner return a NUMBER, IDENTIFIER, etc?
   In which specific scenarios should the scanner return a FUNC or PRINT?
  */

  /* Unknown chars get returned as single char tokens */

{STRINGS}               { return STRING; }
{NUMBERS}               { return NUMBER; }
{IDENTIFIERS}           { return IDENTIFIER; }

.                       { return yytext[0]; }
%%
