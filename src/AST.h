#pragma once
#include <memory>
#include <iostream>
#include <vector>
#include <map>
#include <cassert>
#include "Utilities.h"

using namespace std;

static int expNum = 0;
static map<string, int> LVals;
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
    void Dump() const override
    {
        cout << "CompUnit { ";
        funcDef->Dump();
        cout << " }";
    }
    string DumpIR() const override
    {
        return funcDef->DumpIR();
    }
    int CalcExp() override
    {
        return funcDef->CalcExp();
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
    string DumpIR() const override
    {
        cout << "fun @" << ident << "(): ";
        funcType->DumpIR();
        cout << " {" << endl;
        block->DumpIR();
        cout << endl
             << "}";
        return "";
    }
    int CalcExp() override
    {
        return block->CalcExp();
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
    string DumpIR() const override
    {
        cout << "i32";
        return "";
    }
};
class BlockAST : public BaseAST
{
public:
    unique_ptr<BaseAST> stmt;
    unique_ptr<BaseAST> decl;
    unique_ptr<vector<unique_ptr<BaseAST>>> itemList;
    void Dump() const override
    {
        cout << "BlockAST { ";
        for (const auto &item : *itemList)
        {
            item->Dump();
        }
        cout << " }";
    }
    string DumpIR() const override
    {
        cout << "\%entry:" << endl;
        for (const auto &item : *itemList)
        {
            item->DumpIR();
        }
        return "";
    }
    int CalcExp() override
    {
        return stmt->CalcExp();
    }
};
class StmtAST : public BaseAST
{
public:
    int num;
    unique_ptr<BaseAST> exp;
    void Dump() const override
    {
        cout << "StmtAST { ";
        exp->Dump();
        cout << " }";
    }
    string DumpIR() const override
    {
        string resultReg = exp->DumpIR();
        cout << "\tret " << resultReg << endl;
        return "";
    }
    int CalcExp() override
    {
        return exp->CalcExp();
    }
};

class ExpAST : public BaseAST
{
public:
    unique_ptr<BaseAST> unaryExp;
    void Dump() const override
    {
        cout << "ExpAST { ";
        unaryExp->Dump();
        cout << " } ";
    }
    string DumpIR() const override
    {
        return unaryExp->DumpIR();
    }
    int CalcExp() override
    {
        return unaryExp->CalcExp();
    }
};
class PrimaryExpAST : public BaseAST
{
public:
    unique_ptr<BaseAST> exp;
    EPrimaryExp type;
    int num;
    string lval;
    void Dump() const override
    {
        cout << "PrimaryExpAST { ";
        if (type == EPrimaryExp::Number)
        {
            cout << num;
        }
        else if (type == EPrimaryExp::Exp)
        {
            cout << "(";
            exp->Dump();
            cout << ")";
        }
        cout << " }";
    }
    string DumpIR() const override
    {
        if (type == EPrimaryExp::Number)
        {
            return to_string(num);
        }
        else if (type == EPrimaryExp::Exp)
        {
            return exp->DumpIR();
        }
        else if (type == EPrimaryExp::LVal)
        {
            assert(LVals.count(lval));
            return to_string(LVals[lval]);
        }
        return "";
    }
    int CalcExp() override
    {
        if (type == EPrimaryExp::Exp)
            return exp->CalcExp();
        else if (type == EPrimaryExp::Number)
            return num;
        else if (type == EPrimaryExp::LVal)
        {
            assert(LVals.count(lval));
            return LVals[lval];
        }
        return 0;
    }
    
};
class UnaryExpAST : public BaseAST
{
public:
    EUnaryExp type;
    unique_ptr<BaseAST> primaryExp;
    unique_ptr<BaseAST> unaryExp;
    char op;
    void Dump() const override
    {
        cout << "UnaryExpAST { ";
        if (type == EUnaryExp::PrimaryExp)
        {
            primaryExp->Dump();
        }
        else if (type == EUnaryExp::UnaryExp)
        {
            cout << op << " ";
            unaryExp->Dump();
        }
        cout << " } ";
    }
    string DumpIR() const override
    {
        if (type == EUnaryExp::PrimaryExp)
        {
            return primaryExp->DumpIR();
        }
        else if (type == EUnaryExp::UnaryExp)
        {
            string calcReg = unaryExp->DumpIR();
            string resultReg = "%" + to_string(expNum);
            if (op == '+')
            {
                return calcReg;
            }
            else if (op == '!')
            {
                cout << "\t" << resultReg << " = eq " << calcReg << ", 0" << endl;
                expNum++;
                return resultReg;
            }
            else if (op == '-')
            {
                cout << "\t" << resultReg << " = sub 0, " << calcReg << endl;
                expNum++;
                return resultReg;
            }
        }
        return "";
    }
    int CalcExp() override
    {
        if (type == EUnaryExp::PrimaryExp)
            return primaryExp->CalcExp();
        else
        {
            if (op == '+')
                return unaryExp->CalcExp();
            else if (op == '-')
                return -1 * unaryExp->CalcExp();
            else if (op == '!')
                return !unaryExp->CalcExp();
        }
        return 0;
    }
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
    void Dump() const override
    {
        cout << "MulExpAST { ";
        if (type == EMulExp::Single)
        {
            unaryExp->Dump();
        }
        else if (type == EMulExp::Double)
        {
            lhs->Dump();
            cout << PrintOp(op) << " ";
            rhs->Dump();
        }
        cout << " } ";
    }
    string DumpIR() const override
    {

        if (type == EMulExp::Single)
        {
            return unaryExp->DumpIR();
        }
        else if (type == EMulExp::Double)
        {
            string lCalcReg = lhs->DumpIR();
            string rCalcReg = rhs->DumpIR();
            string resultReg = "%" + to_string(expNum);
            expNum++;
            if (op == EOp::Mul)
            {
                cout << "\t" << resultReg << " = mul " << lCalcReg << ", " << rCalcReg << endl;
                return resultReg;
            }
            else if (op == EOp::Div)
            {
                cout << "\t" << resultReg << " = div " << lCalcReg << ", " << rCalcReg << endl;
                return resultReg;
            }
            else if (op == EOp::Mod)
            {
                cout << "\t" << resultReg << " = mod " << lCalcReg << ", " << rCalcReg << endl;
                return resultReg;
            }
        }
        return "";
    }
    int CalcExp() override
    {
        if (type == EMulExp::Single)
            return unaryExp->CalcExp();
        else
        {
            if (op == EOp::Mul)
                return lhs->CalcExp() * rhs->CalcExp();
            else if (op == EOp::Div)
                return lhs->CalcExp() / rhs->CalcExp();
            else if (op == EOp::Mod)
                return lhs->CalcExp() % rhs->CalcExp();
        }
        return 0;
    }
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
    void Dump() const override
    {
        cout << "AddExpAST { ";
        if (type == EAddExp::Single)
        {
            mulExp->Dump();
        }
        else if (type == EAddExp::Double)
        {
            lhs->Dump();
            cout << PrintOp(op) << " ";
            rhs->Dump();
        }
        cout << " } ";
    }
    string DumpIR() const override
    {
        if (type == EAddExp::Single)
        {
            return mulExp->DumpIR();
        }
        else if (type == EAddExp::Double)
        {
            string lCalcReg = lhs->DumpIR();
            string rCalcReg = rhs->DumpIR();
            string resultReg = "%" + to_string(expNum);
            expNum++;
            if (op == EOp::Add)
            {
                cout << "\t" << resultReg << " = add " << lCalcReg << ", " << rCalcReg << endl;
                return resultReg;
            }
            else if (op == EOp::Sub)
            {
                cout << "\t" << resultReg << " = sub " << lCalcReg << ", " << rCalcReg << endl;
                return resultReg;
            }
        }
        return "";
    }
    int CalcExp() override
    {
        if (type == EAddExp::Single)
            return mulExp->CalcExp();
        else
        {
            if (op == EOp::Add)
                return lhs->CalcExp() + rhs->CalcExp();
            else if (op == EOp::Sub)
                return lhs->CalcExp() - rhs->CalcExp();
        }
        return 0;
    }
};
class RelExpAST : public BaseAST
{
public:
    ERelExp type;
    EOp op;
    unique_ptr<BaseAST> single;
    unique_ptr<BaseAST> lhs;
    unique_ptr<BaseAST> rhs;
    void Dump() const override
    {
        cout << "RelExpAST { ";
        if (type == ERelExp::Single)
        {
            single->Dump();
            cout << " } ";
        }
        else if (type == ERelExp::Double)
        {
            lhs->Dump();
            cout << " " << PrintOp(op);
            rhs->Dump();
            cout << " }";
        }
    }
    string DumpIR() const override
    {
        if (type == ERelExp::Single)
        {
            return single->DumpIR();
        }
        else if (type == ERelExp::Double)
        {
            string lCalcReg = lhs->DumpIR();
            string rCalcReg = rhs->DumpIR();
            string resultReg = "%" + to_string(expNum);
            expNum++;
            if (op == EOp::Less)
            {
                cout << "\t" << resultReg << " = lt " << lCalcReg << ", " << rCalcReg << endl;
                return resultReg;
            }
            else if (op == EOp::Larger)
            {
                cout << "\t" << resultReg << " = gt " << lCalcReg << ", " << rCalcReg << endl;
                return resultReg;
            }
            else if (op == EOp::LessEq)
            {
                cout << "\t" << resultReg << " = le " << lCalcReg << ", " << rCalcReg << endl;
                return resultReg;
            }
            else if (op == EOp::LargerEq)
            {
                cout << "\t" << resultReg << " = ge " << lCalcReg << ", " << rCalcReg << endl;
                return resultReg;
            }
        }
        return "";
    }
    int CalcExp() override
    {
        if (type == ERelExp::Single)
            return single->CalcExp();
        else
        {
            if (op == EOp::Less)
                return lhs->CalcExp() < rhs->CalcExp();
            else if (op == EOp::Larger)
                return lhs->CalcExp() > rhs->CalcExp();
            else if (op == EOp::LargerEq)
                return lhs->CalcExp() >= rhs->CalcExp();
            else if (op == EOp::LessEq)
                return lhs->CalcExp() <= rhs->CalcExp();
        }
        return 0;
    }
};
class EqExpAST : public BaseAST
{
public:
    EEqExp type;
    EOp op;
    unique_ptr<BaseAST> single;
    unique_ptr<BaseAST> lhs;
    unique_ptr<BaseAST> rhs;
    void Dump() const override
    {
        cout << "EqExpAST { ";
        if (type == EEqExp::Single)
        {
            single->Dump();
            cout << " } ";
        }
        else if (type == EEqExp::Double)
        {
            lhs->Dump();
            cout << " " << PrintOp(op);
            rhs->Dump();
            cout << " }";
        }
    }
    string DumpIR() const override
    {
        if (type == EEqExp::Single)
        {
            return single->DumpIR();
        }
        else if (type == EEqExp::Double)
        {
            string lCalcReg = lhs->DumpIR();
            string rCalcReg = rhs->DumpIR();
            string resultReg = "%" + to_string(expNum);
            expNum++;
            if (op == EOp::Eq)
            {
                cout << "\t" << resultReg << " = eq " << lCalcReg << ", " << rCalcReg << endl;
                return resultReg;
            }
            else if (op == EOp::Ne)
            {
                cout << "\t" << resultReg << " = ne " << lCalcReg << ", " << rCalcReg << endl;
                return resultReg;
            }
        }
        return "";
    }
    int CalcExp() override
    {
        if (type == EEqExp::Single)
            return single->CalcExp();
        else
        {
            if (op == EOp::Eq)
                return lhs->CalcExp() == rhs->CalcExp();
            else if (op == EOp::Ne)
                return lhs->CalcExp() != rhs->CalcExp();
        }
        return 0;
    }
};
class LAndExpAST : public BaseAST
{
public:
    ELAndExp type;
    EOp op;
    unique_ptr<BaseAST> single;
    unique_ptr<BaseAST> lhs;
    unique_ptr<BaseAST> rhs;
    void Dump() const override
    {
        cout << "LAndExpAST { ";
        if (type == ELAndExp::Single)
        {
            single->Dump();
            cout << " } ";
        }
        else if (type == ELAndExp::Double)
        {
            lhs->Dump();
            cout << " " << PrintOp(op);
            rhs->Dump();
            cout << " }";
        }
    }
    string DumpIR() const override
    {
        if (type == ELAndExp::Single)
        {
            return single->DumpIR();
        }
        else if (type == ELAndExp::Double)
        {
            string lCalcReg = lhs->DumpIR();
            string rCalcReg = rhs->DumpIR();
            string resultReg = "%" + to_string(expNum);
            expNum++;
            string tmpLReg = "%" + to_string(expNum);
            cout << "\t" << tmpLReg << " = ne " << lCalcReg << ", "
                 << "0" << endl;
            expNum++;
            string tmpRReg = "%" + to_string(expNum);
            cout << "\t" << tmpRReg << " = ne " << rCalcReg << ", "
                 << "0" << endl;
            cout << "\t" << resultReg << " = and" << tmpLReg << ", " << tmpRReg << endl;
            expNum++;
            return resultReg;
        }

        return "";
    }
    int CalcExp() override
    {
        if (type == ELAndExp::Single)
            return single->CalcExp();
        else
        {
            return lhs->CalcExp() && rhs->CalcExp();
        }
    }
};
class LOrExpAST : public BaseAST
{
public:
    ELOrExp type;
    EOp op;
    unique_ptr<BaseAST> single;
    unique_ptr<BaseAST> lhs;
    unique_ptr<BaseAST> rhs;
    void Dump() const override
    {
        cout << "LOrExpAST { ";
        if (type == ELOrExp::Single)
        {
            single->Dump();
            cout << " } ";
        }
        else if (type == ELOrExp::Double)
        {
            lhs->Dump();
            cout << " " << PrintOp(op);
            rhs->Dump();
            cout << " }";
        }
    }
    string DumpIR() const override
    {
        if (type == ELOrExp::Single)
        {
            return single->DumpIR();
        }
        else if (type == ELOrExp::Double)
        {
            string lCalcReg = lhs->DumpIR();
            string rCalcReg = rhs->DumpIR();
            string resultReg = "%" + to_string(expNum);
            expNum++;
            string tmpLReg = "%" + to_string(expNum);
            cout << "\t" << tmpLReg << " = ne " << lCalcReg << ", "
                 << "0" << endl;
            expNum++;
            string tmpRReg = "%" + to_string(expNum);
            cout << "\t" << tmpRReg << " = ne " << rCalcReg << ", "
                 << "0" << endl;
            cout << "\t" << resultReg << " = or" << tmpLReg << ", " << tmpRReg << endl;
            expNum++;
            return resultReg;
        }
        return "";
    }
    int CalcExp() override
    {
        if (type == ELOrExp::Single)
            return single->CalcExp();
        else
            return lhs->CalcExp() || rhs->CalcExp();
    }
};
class DeclAST : public BaseAST
{
public:
    EDecl type;
    unique_ptr<BaseAST> decl;
    void Dump() const override
    {
        cout << "DeclAST { ";
        decl->Dump();
        cout << " } ";
    }
    string DumpIR() const override
    {
        decl->DumpIR();
        return "";
    }
};
class ConstDeclAST : public BaseAST
{
public:
    EBType type;
    unique_ptr<vector<unique_ptr<BaseAST>>> defs;
    void Dump() const override
    {
        cout << "ConstDeclAST { ";
        for (const auto &def : *defs)
        {
            def->Dump();
        }
        cout << " } ";
    }
    string DumpIR() const override
    {
        for (const auto &def : *defs)
        {
            def->DumpIR();
        }
        return "";
    }
};
class ConstDefAST : public BaseAST
{
public:
    string ident;
    unique_ptr<BaseAST> initVal;
    void Dump() const override
    {
        cout << "ConstDefAST { ";
        cout << "Ident: " << ident << " Initval: ";
        initVal->Dump();
        cout << " } ";
    }
    string DumpIR() const override
    {
        assert(!LVals.count(ident));
        LVals[ident] = initVal->CalcExp();
        return "";
    }
};
class ConstInitValAST : public BaseAST
{
public:
    unique_ptr<BaseAST> constExp;
    void Dump() const override
    {
        cout << "ConstInitValAST { ";
        constExp->Dump();
        cout << " } ";
    }
    string DumpIR() const override
    {

        return "";
    }
    int CalcExp() override
    {
        return constExp->CalcExp();
    }
};
class ConstExpAST : public BaseAST
{
public:
    unique_ptr<BaseAST> exp;
    void Dump() const override
    {
        cout << "ConstExpAST { ";
        exp->Dump();
        cout << " } ";
    }
    string DumpIR() const override
    {

        return "";
    }
    int CalcExp() override
    {
        return exp->CalcExp();
    }
};
class BlockItemAST : public BaseAST
{
public:
    EBlockItem type;
    unique_ptr<BaseAST> item;
    void Dump() const override
    {
        cout << "BlockItemAST { ";
        item->Dump();
        cout << " } ";
    }
    string DumpIR() const override
    {
        item->DumpIR();
        return "";
    }
};
