#pragma once
#include <memory>
#include <iostream>

using namespace std;

class BaseAST
{
public:
    virtual ~BaseAST() = default;
    virtual void Dump() const = 0;
};

class CompUnitAST : public BaseAST
{
public:
    unique_ptr<BaseAST> funcDef;
    void Dump() const override
    {
        cout << "CompUnit { ";
        funcDef->Dump();
        cout << " }";
    }
};

class FuncDefAST : public BaseAST
{
public:
    unique_ptr<BaseAST> funcType;
    string ident;
    unique_ptr<BaseAST> block;
    void Dump() const override
    {
        cout << "FuncDefAST { ";
        funcType->Dump();
        cout << ", " << ident << ", ";
        block->Dump();
        cout << " }";
    }
};
class FuncTypeAST : public BaseAST
{
public:
    string type;
    void Dump() const override
    {
        cout << "FuncType{ " << type << " }";
    }
};
class BlockAST : public BaseAST
{
public:
    unique_ptr<BaseAST> stmt;
    void Dump() const override
    {
        cout<<"BlockAST { ";
        stmt->Dump();
        cout<<" }";
    }
};
class StmtAST : public BaseAST
{
public:
    int num;
    void Dump() const override
    {
        cout<<"StmtAST { "<<num<<" }";
    }
};