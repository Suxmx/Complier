%option noyywrap
%option nounput
%option noinput

%{

#include <cstdlib>
#include <string>

// 因为 Flex 会用到 Bison 中关于 token 的定义
// 所以需要 include Bison 生成的头文件
#include "sys.tab.hpp"
//test
using namespace std;

%}

/* 空白符和注释 */
WhiteSpace    [ \t\n\r]*
BlockComment  "/*"([^\*/]*)"*/"
LineComment   "//".*

/* 标识符 */
Identifier    [a-zA-Z_][a-zA-Z0-9_]*

/*  */
LessEq        "<="
LargerEq      ">="
Less          "<"
Larger        ">"
Or            "||"
And           "&&"
Eq            "=="
Ne            "!="
/* 整数字面量 */
Decimal       [1-9][0-9]*
Octal         0[0-7]*
Hexadecimal   0[xX][0-9a-fA-F]+
Const         "const"
If            "if"
Else          "else"
While         "while"
Break         "break"
Continue      "continue"

%%

{WhiteSpace}    { /* 忽略, 不做任何操作 */ }
{BlockComment}  {}
{LineComment}   { /* 忽略, 不做任何操作 */ }
{If}            { return IF; }
{Else}          { return ELSE; }
"int"           { return INT; }
"return"        { return RETURN; }
{Const}         { return CONST; }
{While}         { return WHILE; }
{Break}         { return BREAK; }
{Continue}      {return CONTINUE; }


{Identifier}    { yylval.str_val = new string(yytext); return IDENT; }


{LessEq}        { return LESSEQ; }
{LargerEq}      { return LARGEREQ; }
{Less}          { return LESS; }
{Larger}        { return LARGER; }
{And}           { return AND; }
{Or}            { return OR; }
{Eq}            { return EQ; }
{Ne}            { return NE; }

{Decimal}       { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Octal}         { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Hexadecimal}   { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }

.               { return yytext[0]; }

%%
