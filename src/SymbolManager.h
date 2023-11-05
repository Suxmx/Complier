#pragma once
#include <exception>
#include <map>
#include <stack>
#include <iostream>
#include <memory>
#include "Utilities.h"
#include "Error.h"

using namespace std;
struct SymbolTable
{
    map<string, DeclData> symbols;
    shared_ptr<SymbolTable> parent; // 指向上一级符号表的指针
};

class SymbolTableManager
{
private:
    stack<shared_ptr<SymbolTable>> symbolStack;

public:
    SymbolTableManager()
    {
        shared_ptr<SymbolTable> globalSymbolTable = make_shared<SymbolTable>();
        symbolStack.push(globalSymbolTable);
    }
    void EnterBlock()
    {
        auto symbolTable = make_shared<SymbolTable>();
        symbolTable->parent = symbolStack.top();
        symbolStack.push(symbolTable);
    }
    void ExitBlock()
    {
        symbolStack.pop();
    }
    void AddSymbol(string ident, DeclData data)
    {
        auto cur = symbolStack.top();
        if (cur->symbols.count(ident))
            throw SymbolError("Symbol:" + ident + " already exist.");
        cur->symbols[ident] = data;
    }
    DeclData FindSymbol(const string &ident)
    {
        auto cur = symbolStack.top();
        while (cur)
        {
            if (cur->symbols.count(ident))
            {
                return cur->symbols[ident];
            }
            cur = cur->parent;
        }
        throw SymbolError("Symbol:"+ident+" not exist!");
    }
};
