#pragma once
#include <memory>
#include <iostream>

using namespace std;

class BaseAST
{
public:
    virtual ~BaseAST() = default;
    virtual void Dump() const = 0;
    virtual void DumpIR() const = 0;
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
    void DumpIR() const override
    {
        funcDef->DumpIR();
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
    void DumpIR() const override
    {
        cout << "fun @" << ident << "(): ";
        funcType->DumpIR();
        cout << " {" << endl;
        block->DumpIR();
        cout << endl
             << "}";
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
    void DumpIR() const override
    {
        cout << "i32";
    }
};
class BlockAST : public BaseAST
{
public:
    unique_ptr<BaseAST> stmt;
    void Dump() const override
    {
        cout << "BlockAST { ";
        stmt->Dump();
        cout << " }";
    }
    void DumpIR() const override
    {
        cout << "\%entry:" << endl;
        stmt->DumpIR();
    }
};
class StmtAST : public BaseAST
{
public:
    int num;
    void Dump() const override
    {
        cout << "StmtAST { " << num << " }";
    }
    void DumpIR() const override
    {
        cout << "\tret " << num;
    }
};