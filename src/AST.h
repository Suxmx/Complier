#pragma once
#include <memory>
#include <iostream>

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
    MulExp,
    AddExp
};
enum class EMulExp
{
    UnaryExp,
    MulExp
};
static int expNum = 0;
class BaseAST
{
public:
    virtual ~BaseAST() = default;
    virtual void Dump() const = 0;
    virtual string DumpIR() const = 0;
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
    void Dump() const override
    {
        cout << "BlockAST { ";
        stmt->Dump();
        cout << " }";
    }
    string DumpIR() const override
    {
        cout << "\%entry:" << endl;
        return stmt->DumpIR();
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
};
class PrimaryExpAST : public BaseAST
{
public:
    unique_ptr<BaseAST> exp;
    EPrimaryExp type;
    int num;
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
    }
};
class MulExpAST : public BaseAST
{
public:
    EMulExp type;
    unique_ptr<BaseAST> unaryExp;
    unique_ptr<BaseAST> lhs;
    unique_ptr<BaseAST> rhs;
    char op;
    void Dump() const override
    {
        cout << "MulExpAST { ";
        if (type == EMulExp::UnaryExp)
        {
            unaryExp->Dump();
        }
        else if (type == EMulExp::MulExp)
        {
            lhs->Dump();
            cout << op << " ";
            rhs->Dump();
        }
        cout << " } ";
    }
    string DumpIR() const override
    {

        if (type == EMulExp::UnaryExp)
        {
            return unaryExp->DumpIR();
        }
        else if (type == EMulExp::MulExp)
        {
            string lCalcReg = lhs->DumpIR();
            string rCalcReg = rhs->DumpIR();
            string resultReg = "%" + to_string(expNum);
            expNum++;
            if (op == '*')
            {
                cout << "\t" << resultReg << " = mul " << lCalcReg << ", " << rCalcReg << endl;
                return resultReg;
            }
            else if (op == '/')
            {
                cout << "\t" << resultReg << " = div " << lCalcReg << ", " << rCalcReg << endl;
                return resultReg;
            }
            else if (op == '%')
            {
                cout << "\t" << resultReg << " = mod " << lCalcReg << ", " << rCalcReg << endl;
                return resultReg;
            }
        }
    }
};
class AddExpAST : public BaseAST
{
public:
    EAddExp type;
    unique_ptr<BaseAST> mulExp;
    unique_ptr<BaseAST> lhs;
    unique_ptr<BaseAST> rhs;
    char op;
    void Dump() const override
    {
        cout << "AddExpAST { ";
        if (type == EAddExp::MulExp)
        {
            mulExp->Dump();
        }
        else if (type == EAddExp::AddExp)
        {
            lhs->Dump();
            cout << op << " ";
            rhs->Dump();
        }
        cout << " } ";
    }
    string DumpIR() const override
    {
        if (type == EAddExp::MulExp)
        {
            return mulExp->DumpIR();
        }
        else if (type == EAddExp::AddExp)
        {
            string lCalcReg = lhs->DumpIR();
            string rCalcReg = rhs->DumpIR();
            string resultReg = "%" + to_string(expNum);
            expNum++;
            if (op == '+')
            {
                cout << "\t" << resultReg << " = add " << lCalcReg << ", " << rCalcReg << endl;
                return resultReg;
            }
            else if (op == '-')
            {
                cout << "\t" << resultReg << " = sub " << lCalcReg << ", " << rCalcReg << endl;
                return resultReg;
            }
        }
    }
};
