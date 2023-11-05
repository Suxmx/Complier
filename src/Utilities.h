#pragma once
#ifndef Utilites_h
#define Utilites_h
#include <iostream>
#include <vector>
using namespace std;

enum class EUnaryExp
{
    PrimaryExp,
    UnaryExp
};
enum class EPrimaryExp
{
    Exp,
    Number,
    LVal
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
enum class ELAndExp
{
    Single,
    Double
};
enum class EEqExp
{
    Single,
    Double
};
enum class EOp
{
    Add = 1,
    Sub = 2,
    Mul = 3,
    Div = 4,
    Mod = 5,
    Less = 6,
    Larger = 7,
    LessEq = 8,
    LargerEq = 9,
    Eq = 10,
    Ne = 11,
    And = 12,
    Or = 13,

};
enum class ELOrExp
{
    Single,
    Double
};
enum class EBlockType
{
    Stmt,
    Decl
};
enum class EDecl
{
    ConstDecl,
    Variable
};
enum class EBType
{
    Int = 1
};
enum class EBlockItem
{
    Decl,
    Stmt
};
enum class EStmt
{
    Return,
    Var,
    Exp,
    Block
};
enum class ERISVSave
{
    Reg,
    Stack
};
inline string PrintOp(EOp op)
{
    vector<string> opes = {"+", "-", "*", "/", "%", "<", ">", "<=", ">=", "==", "!=", "&&", "||"};
    return opes[(int)op - 1];
}
struct DeclData
{
    EDecl type;
    string ident;
    int constValue;
    string varReg;
    string globalIdent;//为了避免IR不能重名的问题
};
#endif
