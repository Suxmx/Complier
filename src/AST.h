#pragma once
#include <memory>
#include <iostream>
#include <vector>
#include <map>
#include <cassert>
#include "Utilities.h"
#include "SymbolManager.h"
#include "Error.h"

using namespace std;
typedef map<string, DeclData> symbol_table;
static int expNum = 0;
static int ifNum = 0;
static int cuttingOut=0;
static int whileNum=0;
static SymbolTableManager symbolManager;
class StmtAST;
class BaseAST
{
public:
    virtual ~BaseAST() = default;
    virtual void Dump() const = 0;
    virtual string DumpIR() const = 0;
    virtual int CalcExp() { return 0; }
};

class CompUnitAST : public BaseAST
{
public:
    unique_ptr<BaseAST> funcDef;
    void Dump() const override;
    string DumpIR() const override;
    int CalcExp() override;
};

class FuncDefAST : public BaseAST
{
public:
    unique_ptr<BaseAST> funcType;
    string ident;
    unique_ptr<BaseAST> block;
    void Dump() const override;
    string DumpIR() const override;
    int CalcExp() override;
};
class FuncTypeAST : public BaseAST
{
public:
    string type;
    void Dump() const override;
    string DumpIR() const override;
};
class BlockAST : public BaseAST
{
public:
    unique_ptr<BaseAST> stmt;
    unique_ptr<BaseAST> decl;
    unique_ptr<vector<unique_ptr<BaseAST>>> itemList;
    void Dump() const override;
    string DumpIR() const override;
    int CalcExp() override;
};
class StmtAST : public BaseAST
{
public:
    EStmt type;
    int num;
    unique_ptr<BaseAST> exp;
    unique_ptr<BaseAST> block;
    unique_ptr<BaseAST> ifStmt;
    unique_ptr<BaseAST> elseStmt;
    unique_ptr<BaseAST> whileStmt;
    string lval;
    void Dump() const override;
    string DumpIR() const override;
    int CalcExp() override;
};

class ExpAST : public BaseAST
{
public:
    unique_ptr<BaseAST> unaryExp;
    void Dump() const override;
    string DumpIR() const override;
    int CalcExp() override;
};
class PrimaryExpAST : public BaseAST
{
public:
    unique_ptr<BaseAST> exp;
    EPrimaryExp type;
    int num;
    string lval;
    void Dump() const override;
    string DumpIR() const override;
    int CalcExp() override;
};
class UnaryExpAST : public BaseAST
{
public:
    EUnaryExp type;
    unique_ptr<BaseAST> primaryExp;
    unique_ptr<BaseAST> unaryExp;
    char op;
    void Dump() const override;
    string DumpIR() const override;
    int CalcExp() override;
};
class MulExpAST : public BaseAST
{
public:
    EMulExp type;
    unique_ptr<BaseAST> unaryExp;
    unique_ptr<BaseAST> lhs;
    unique_ptr<BaseAST> rhs;
    // char op;
    EOp op;
    void Dump() const override;
    string DumpIR() const override;
    int CalcExp() override;
};
class AddExpAST : public BaseAST
{
public:
    EAddExp type;
    unique_ptr<BaseAST> mulExp;
    unique_ptr<BaseAST> lhs;
    unique_ptr<BaseAST> rhs;
    // char op;
    EOp op;
    void Dump() const override;
    string DumpIR() const override;
    int CalcExp() override;
};
class RelExpAST : public BaseAST
{
public:
    ERelExp type;
    EOp op;
    unique_ptr<BaseAST> single;
    unique_ptr<BaseAST> lhs;
    unique_ptr<BaseAST> rhs;
    void Dump() const override;
    string DumpIR() const override;
    int CalcExp() override;
};
class EqExpAST : public BaseAST
{
public:
    EEqExp type;
    EOp op;
    unique_ptr<BaseAST> single;
    unique_ptr<BaseAST> lhs;
    unique_ptr<BaseAST> rhs;
    void Dump() const override;
    string DumpIR() const override;
    int CalcExp() override;
};
class LAndExpAST : public BaseAST
{
public:
    ELAndExp type;
    EOp op;
    unique_ptr<BaseAST> single;
    unique_ptr<BaseAST> lhs;
    unique_ptr<BaseAST> rhs;
    void Dump() const override;
    string DumpIR() const override;
    int CalcExp() override;
};
class LOrExpAST : public BaseAST
{
public:
    ELOrExp type;
    EOp op;
    unique_ptr<BaseAST> single;
    unique_ptr<BaseAST> lhs;
    unique_ptr<BaseAST> rhs;
    void Dump() const override;
    string DumpIR() const override;
    int CalcExp() override;
};
class DeclAST : public BaseAST
{
public:
    EDecl type;
    unique_ptr<BaseAST> decl;
    void Dump() const override;
    string DumpIR() const override;
};
class ConstDeclAST : public BaseAST
{
public:
    EBType type;
    unique_ptr<vector<unique_ptr<BaseAST>>> defs;
    void Dump() const override;
    string DumpIR() const override;
};
class ConstDefAST : public BaseAST
{
public:
    string ident;
    unique_ptr<BaseAST> initVal;
    void Dump() const override;
    string DumpIR() const override;
};
class InitValAST : public BaseAST
{
public:
    unique_ptr<BaseAST> exp;
    void Dump() const override;
    string DumpIR() const override;
    int CalcExp() override;
};
class ConstExpAST : public BaseAST
{
public:
    unique_ptr<BaseAST> exp;
    void Dump() const override;
    string DumpIR() const override;
    int CalcExp() override;
};
class BlockItemAST : public BaseAST
{
public:
    EBlockItem type;
    unique_ptr<BaseAST> item;
    void Dump() const override;
    string DumpIR() const override;
};
class VarDeclAST : public BaseAST
{
public:
    EBType type;
    unique_ptr<vector<unique_ptr<BaseAST>>> defs;
    void Dump() const override;
    string DumpIR() const override;
};
class VarDefAST : public BaseAST
{
public:
    string ident;
    unique_ptr<BaseAST> initVal;
    void Dump() const override;
    string DumpIR() const override;
};