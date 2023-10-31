#pragma once
#include<iostream>
#include<vector>
using namespace std;

enum class EUnaryExp
{
    PrimaryExp,
    UnaryExp
};
enum class EPrimaryExp
{
    Exp,
    Number
};
enum class EAddExp
{
    Single,
    Double
};
enum class EMulExp
{
    Single,
    Double
};
enum class ERelExp
{
    Single,
    Double
};
enum class ELAndExp{
    Single,
    Double
};
enum class EEqExp{
    Single,
    Double
};
enum class EOp
{
    Add=1,
    Sub=2,
    Mul=3,
    Div=4,
    Mod=5,
    Less=6,
    Larger=7,
    LessEq=8,
    LargerEq=9,
    Eq=10,
    Ne=11,
    And=12,
    Or=13,  

};
vector<string> ops={"+","-","*","/","%","<",">","<=",">=","==","!=","&&","||"};
string PrintOp(EOp op){
    return ops[(int)op-1];
}

