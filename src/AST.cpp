#include "AST.h"
using namespace std;

void CompUnitAST::Dump() const
{
    cout << "CompUnit { " << endl;
    for (const auto &def : *funcDefs)
    {

        def->Dump();
    }
    cout << " }";
}
string CompUnitAST::DumpIR() const
{
    cout << "decl @getint(): i32\ndecl @getch() : i32\ndecl @getarray(*i32) : i32\n";
    cout << "decl @putint(i32)\ndecl @putch(i32)\ndecl @putarray(i32, *i32)\ndecl @starttime()\ndecl @stoptime()" << endl;
    DeclData data;
    data.type = EDecl::Func;
    data.bType = EBType::Int;

    data.ident = "getint";
    symbolManager.AddSymbol("getint", data);
    data.ident = "getch";
    symbolManager.AddSymbol("getch", data);
    data.ident = "getarray";
    symbolManager.AddSymbol("getarray", data);

    data.bType = EBType::Void;
    data.ident = "putch";
    symbolManager.AddSymbol("putch", data);
    data.ident = "putint";
    symbolManager.AddSymbol("putint", data);
    data.ident = "putarray";
    symbolManager.AddSymbol("putarray", data);
    data.ident = "starttime";
    symbolManager.AddSymbol("starttime", data);
    data.ident = "stoptime";
    symbolManager.AddSymbol("stoptime", data);
    for (const auto &def : *funcDefs)
    {
        def->DumpIRGlobal();
        cout << endl;
    }
    return "";
}

void FuncDefAST::Dump() const
{
    cout << "FuncDefAST { " << endl;
    if (funcType == EBType::Int)
        cout << "int";
    else if (funcType == EBType::Void)
        cout << "void";
    cout << ", " << ident << ", ";
    block->Dump();
    cout << " }" << endl;
}
string FuncDefAST::DumpIR() const
{

    DeclData data;
    data.ident = ident;
    data.type = EDecl::Func;
    data.bType = funcType;

    symbolManager.AddSymbol(ident, data);
    symbolManager.EnterBlock();

    cout << "fun @" << ident << "(";
    if (fParams)
        for (int i = 0; i < fParams->size(); i++)
        {
            (*fParams)[i]->DumpIR();
            if (i != fParams->size() - 1)
                cout << ",";
        }
    cout << ") ";
    if (funcType == EBType::Int)
    {
        cout << ":i32";
    }
    cout << " {" << endl;
    cout << "\%entry:" << endl;
    if (fParams)
        for (const auto &param : *fParams)
        {
            DeclData tmp;
            tmp.bType = (EBType)(param->GetType());
            tmp.ident = param->GetIdent();
            tmp.type = EDecl::Variable;
            tmp = symbolManager.AddSymbol(tmp.ident, tmp);
            cout << "\t@" << tmp.globalIdent << " = alloc i32" << endl;
            cout << "\tstore @" << tmp.ident << ", @" << tmp.globalIdent << endl;
        }
    block->DumpIR();
    if (funcType == EBType::Void)
        cout << "\tret" << endl;
    cout << endl
         << "}";
    symbolManager.ExitBlock();
    return "";
}
int FuncDefAST::CalcExp()
{
    return block->CalcExp();
}
string FuncDefAST::DumpIRGlobal()
{
    return this->DumpIR();
}

void FuncFParamAST::Dump() const
{
    cout << "FuncFParamAST { ident: " << ident << " type: "
         << "int"
         << " } ";
}
string FuncFParamAST::DumpIR() const
{
    cout << "@" << ident << ": ";
    if (type == EBType::Int)
        cout << "i32";
    return "";
}
int FuncFParamAST::GetType() const
{
    return (int)(type);
}
string FuncFParamAST::GetIdent() const
{
    return ident;
}
void FuncRParamAST::Dump() const
{
}
string FuncRParamAST::DumpIR() const
{
    return exp->DumpIR();
}

void BlockAST::Dump() const
{
    symbolManager.EnterBlock();
    cout << "BlockAST { ";
    for (const auto &item : *itemList)
    {
        item->Dump();
    }
    cout << " }" << endl;
    symbolManager.ExitBlock();
}
string BlockAST::DumpIR() const
{
    string result;
    symbolManager.EnterBlock();
    for (const auto &item : *itemList)
    {
        string tmp = item->DumpIR();

        if (tmp == "ret")
        {
            result = tmp;
            // cout<<"DEBUGBLOCK:"<<result<<endl;
            break;
        }
        if (tmp == "break" && !whileStack.empty())
        {
            result = tmp;
            break;
        }
        if (tmp == "continue" && !whileStack.empty())
        {
            result = tmp;
            break;
        }
    }
    symbolManager.ExitBlock();
    return result;
}
int BlockAST::CalcExp()
{
    return stmt->CalcExp();
}

void StmtAST::Dump() const
{
    cout << "StmtAST { " << endl;
    exp->Dump();
    cout << " }" << endl;
}
string StmtAST::DumpIR() const
{
    if (type == EStmt::Return)
    {
        string resultReg = exp->DumpIR();
        cout << "\tret " << resultReg << endl;
        return "ret";
    }
    else if (type == EStmt::Var)
    {
        auto data = symbolManager.FindSymbol(lval);
        string res = exp->DumpIR();
        cout << "\tstore " << res << ", @" << data.globalIdent << endl;
    }
    else if (type == EStmt::Block)
    {
        return block->DumpIR();
    }
    else if (type == EStmt::Empty)
    {
        return "";
    }
    else if (type == EStmt::Exp)
    {
        return exp->DumpIR();
    }
    else if (type == EStmt::If)
    {
        ifNum++;
        int cache = ifNum;
        string condition = exp->DumpIR();
        cout << "\tbr " << condition << ", \%ifblock_" << cache << ", \%go_on_" << ifNum << endl;
        cout << endl;
        cout << "\%ifblock_" << cache << ":" << endl;
        string tmp = ifStmt->DumpIR();
        // cout<<"DEBUG:"<<tmp<<endl;
        if (tmp != "ret" && tmp != "break" && tmp != "continue")
            cout << "\tjump \%go_on_" << cache << endl;
        cout << endl;
        cout << "\%go_on_" << cache << ":" << endl;
    }
    else if (type == EStmt::IfElse)
    {
        ifNum++;
        int cache = ifNum;
        string condition = exp->DumpIR();
        cout << "\tbr " << condition << ", \%ifblock_" << cache << ", \%elseblock_" << ifNum << endl;
        cout << endl;
        cout << "\%ifblock_" << cache << ":" << endl;
        string ifRet = ifStmt->DumpIR();
        // cout<<"DEBUG:"<<tmp<<endl;
        if (ifRet != "ret" && ifRet != "break" && ifRet != "continue")
            cout << "\tjump \%go_on_" << cache << endl;
        cout << endl;
        cout << "\%elseblock_" << cache << ":" << endl;
        string elseRet = elseStmt->DumpIR();
        if (elseRet != "ret" && elseRet != "break" && elseRet != "continue")
            cout << "\tjump \%go_on_" << cache << endl;
        cout << endl;
        // if (ifRet != "ret" || elseRet != "ret")
        cout << "\%go_on_" << cache << ":" << endl;
    }
    else if (type == EStmt::While)
    {
        int cacheNum = whileNum++;
        whileStack.push(cacheNum);
        cout << "\tjump \%while_entry_" << cacheNum << endl;
        cout << endl;
        cout << "\%while_entry_" << cacheNum << ":" << endl;
        string condition = exp->DumpIR();
        cout << "\tbr " << condition << ", \%while_body_" << cacheNum << ", \%while_go_on_" << cacheNum << endl;
        cout << endl;
        cout << "\%while_body_" << cacheNum << ":" << endl;
        string ret = whileStmt->DumpIR();
        if (ret != "ret" && ret != "break" && ret != "continue")
            cout << "\tjump \%while_entry_" << cacheNum << endl;
        cout << endl;
        cout << "\%while_go_on_" << cacheNum << ":" << endl;
        whileStack.pop();
    }
    else if (type == EStmt::Break)
    {
        int num = whileStack.top();
        cout << "\tjump \%while_go_on_" << num << endl;
        return "break";
    }
    else if (type == EStmt::Continue)
    {
        int num = whileStack.top();
        cout << "\tjump \%while_entry_" << num << endl;
        return "continue";
    }
    else if (type == EStmt::Array)
    {
        auto data = symbolManager.FindSymbol(lval);
        string res = exp->DumpIR();
        string ptr = "%" + to_string(expNum++);
        cout << "\t" << ptr << " = getelemptr @" << data.globalIdent << ", " << (*arraySymbols)[0]->CalcExp() << endl;

        cout << "\tstore " << res << ", " << ptr << endl;
    }
    return "";
}
int StmtAST::CalcExp()
{
    return exp->CalcExp();
}

void ExpAST::Dump() const
{
    cout << "ExpAST { " << endl;
    unaryExp->Dump();
    cout << " } " << endl;
}
string ExpAST::DumpIR() const
{
    return unaryExp->DumpIR();
}
int ExpAST::CalcExp()
{
    return unaryExp->CalcExp();
}
int ExpAST::GetType() const
{
    return (int)EInitVal::Exp;
}

void PrimaryExpAST::Dump() const
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
    else if (type == EPrimaryExp::LVal)
    {
        cout << lval;
    }
    cout << " }" << endl;
}
string PrimaryExpAST::DumpIR() const
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
        auto data = symbolManager.FindSymbol(lval);
        if (data.type == EDecl::ConstDecl)
        {
            return to_string(data.constValue);
        }
        else if (data.type == EDecl::Variable)
        {
            string res = "%" + to_string(expNum++);
            cout << "\t" << res << " = load @" << data.globalIdent << endl;
            return res;
        }
    }
    else if (type == EPrimaryExp::Array)
    {
        auto data = symbolManager.FindSymbol(lval);
        string tmp = "%" + to_string(expNum++);
        cout << "\t" << tmp << " = getelemptr @" << data.globalIdent << ", " << (*arraySymbols)[0]->CalcExp() << endl;
        string res = "%" + to_string(expNum++);
        cout << "\t" << res << " = load " << tmp << endl;
        return res;
    }
    return "";
}
int PrimaryExpAST::CalcExp()
{
    if (type == EPrimaryExp::Exp)
        return exp->CalcExp();
    else if (type == EPrimaryExp::Number)
        return num;
    else if (type == EPrimaryExp::LVal)
    {
        auto data = symbolManager.FindSymbol(lval);
        if (data.type == EDecl::ConstDecl)
            return data.constValue;
        throw SymbolError("Symbol:" + lval + " is not a const value");
    }
    return 0;
}

void UnaryExpAST::Dump() const
{
    cout << "UnaryExpAST { " << endl;
    if (type == EUnaryExp::PrimaryExp)
    {
        primaryExp->Dump();
    }
    else if (type == EUnaryExp::UnaryExp)
    {
        cout << op << " ";
        unaryExp->Dump();
    }
    cout << " } " << endl;
}
string UnaryExpAST::DumpIR() const
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
    else if (type == EUnaryExp::FuncCall)
    {
        vector<string> params;
        for (const auto &param : *rParams)
        {
            params.push_back(param->DumpIR());
        }
        auto data = symbolManager.FindSymbol(funcIdent);

        if (data.bType == EBType::Int)
        {

            string res = "%" + to_string(expNum++);
            cout << "\t" << res << " = call @" << data.ident << "(";
            for (int i = 0; i < params.size(); i++)
            {
                cout << params[i];
                if (i != params.size() - 1)
                    cout << ",";
            }
            cout << ")" << endl;
            return res;
        }
        else if (data.bType == EBType::Void)
        {
            cout << "\tcall @" << data.ident << "(";
            // cout<<params.size();
            for (int i = 0; !params.empty() && i < params.size(); i++)
            {
                cout << params[i];
                if (i != params.size() - 1)
                    cout << ",";
            }
            cout << ")" << endl;
        }
    }
    return "";
}
int UnaryExpAST::CalcExp()
{
    if (type == EUnaryExp::PrimaryExp)
        return primaryExp->CalcExp();
    else if (type == EUnaryExp::PrimaryExp)
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

void MulExpAST::Dump() const
{
    cout << "MulExpAST { " << endl;
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
    cout << " } " << endl;
}
string MulExpAST::DumpIR() const
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
int MulExpAST::CalcExp()
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

void AddExpAST::Dump() const
{
    cout << "AddExpAST { " << endl;
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
    cout << " } " << endl;
}
string AddExpAST::DumpIR() const
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
int AddExpAST::CalcExp()
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

void RelExpAST::Dump() const
{
    cout << "RelExpAST { " << endl;
    if (type == ERelExp::Single)
    {
        single->Dump();
        cout << " } " << endl;
    }
    else if (type == ERelExp::Double)
    {
        lhs->Dump();
        cout << " " << PrintOp(op);
        rhs->Dump();
        cout << " }" << endl;
    }
}
string RelExpAST::DumpIR() const
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
int RelExpAST::CalcExp()
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

void EqExpAST::Dump() const
{
    cout << "EqExpAST { " << endl;
    if (type == EEqExp::Single)
    {
        single->Dump();
        cout << " } " << endl;
    }
    else if (type == EEqExp::Double)
    {
        lhs->Dump();
        cout << " " << PrintOp(op);
        rhs->Dump();
        cout << " }" << endl;
    }
}
string EqExpAST::DumpIR() const
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
int EqExpAST::CalcExp()
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

void LAndExpAST::Dump() const
{
    cout << "LAndExpAST { " << endl;
    if (type == ELAndExp::Single)
    {
        single->Dump();
        cout << " } " << endl;
    }
    else if (type == ELAndExp::Double)
    {
        lhs->Dump();
        cout << " " << PrintOp(op);
        rhs->Dump();
        cout << " }" << endl;
    }
}
string LAndExpAST::DumpIR() const
{
    if (type == ELAndExp::Single)
    {
        return single->DumpIR();
    }
    else if (type == ELAndExp::Double)
    {
        int cacheCuttingNum = cuttingOut++;
        string lCalcReg = lhs->DumpIR();
        string leftRes = "%" + to_string(expNum++);
        string resVar = "\%cutting_res_" + to_string(cacheCuttingNum);
        cout << "\t" << resVar << " = alloc i32" << endl;
        cout << "\t" << leftRes << " = eq " << lCalcReg << ", 0" << endl;
        cout << "\tbr " << leftRes << ", "
             << "\%cutting_" << cacheCuttingNum << ", \%uncutting_" << cacheCuttingNum << endl;
        cout << endl;
        cout << "\%cutting_" << cacheCuttingNum << ":" << endl;
        cout << "\tstore 0, " << resVar << endl;
        cout << "\tjump \%aftercutting_" << cacheCuttingNum << endl;
        cout << endl;
        cout << "\%uncutting_" << cacheCuttingNum << ":" << endl;
        string rCalcReg = rhs->DumpIR();
        string rightRes = "%" + to_string(expNum++);
        cout << "\t" << rightRes << " = ne " << rCalcReg << ", 0" << endl;
        cout << "\tstore " << rightRes << ", " << resVar << endl;
        cout << "\tjump \%aftercutting_" << cacheCuttingNum << endl;
        cout << endl;
        string resultReg = "%" + to_string(expNum++);
        cout << "\%aftercutting_" << cacheCuttingNum << ":" << endl;
        cout << "\t" << resultReg << " = load " << resVar << endl;
        return resultReg;
    }

    return "";
}
int LAndExpAST::CalcExp()
{
    if (type == ELAndExp::Single)
        return single->CalcExp();
    else
    {
        return lhs->CalcExp() && rhs->CalcExp();
    }
}

void LOrExpAST::Dump() const
{
    cout << "LOrExpAST { " << endl;
    if (type == ELOrExp::Single)
    {
        single->Dump();
        cout << " } " << endl;
    }
    else if (type == ELOrExp::Double)
    {
        lhs->Dump();
        cout << " " << PrintOp(op);
        rhs->Dump();
        cout << " }" << endl;
    }
}
string LOrExpAST::DumpIR() const
{
    if (type == ELOrExp::Single)
    {
        return single->DumpIR();
    }
    else if (type == ELOrExp::Double)
    {
        int cacheCuttingNum = cuttingOut++;
        string lCalcReg = lhs->DumpIR();
        string leftRes = "%" + to_string(expNum++);
        string resVar = "\%cutting_res_" + to_string(cacheCuttingNum);
        cout << "\t" << resVar << " = alloc i32" << endl;
        cout << "\t" << leftRes << " = ne " << lCalcReg << ", 0" << endl;
        cout << "\tbr " << leftRes << ", "
             << "\%cutting_" << cacheCuttingNum << ", \%uncutting_" << cacheCuttingNum << endl;
        cout << endl;
        cout << "\%cutting_" << cacheCuttingNum << ":" << endl;
        cout << "\tstore 1, " << resVar << endl;
        cout << "\tjump \%aftercutting_" << cacheCuttingNum << endl;
        cout << endl;
        cout << "\%uncutting_" << cacheCuttingNum << ":" << endl;
        string rCalcReg = rhs->DumpIR();
        string rightRes = "%" + to_string(expNum++);
        cout << "\t" << rightRes << " = ne " << rCalcReg << ", 0" << endl;
        cout << "\tstore " << rightRes << ", " << resVar << endl;
        cout << "\tjump \%aftercutting_" << cacheCuttingNum << endl;
        cout << endl;
        string resultReg = "%" + to_string(expNum++);
        cout << "\%aftercutting_" << cacheCuttingNum << ":" << endl;
        cout << "\t" << resultReg << " = load " << resVar << endl;

        return resultReg;
    }
    return "";
}
int LOrExpAST::CalcExp()
{
    if (type == ELOrExp::Single)
        return single->CalcExp();
    else
        return lhs->CalcExp() || rhs->CalcExp();
}

void DeclAST::Dump() const
{
    cout << "DeclAST { " << endl;
    decl->Dump();
    cout << " } " << endl;
}
string DeclAST::DumpIR() const
{
    decl->DumpIR();
    return "";
}
string DeclAST::DumpIRGlobal()
{
    decl->DumpIRGlobal();
    return "";
}

void ConstDeclAST::Dump() const
{
    cout << "ConstDeclAST { " << endl;
    for (const auto &def : *defs)
    {
        def->Dump();
        cout << endl;
    }
    cout << " } " << endl;
}
string ConstDeclAST::DumpIR() const
{
    for (const auto &def : *defs)
    {
        def->DumpIR();
    }
    return "";
}
string ConstDeclAST::DumpIRGlobal()
{
    for (const auto &def : *defs)
    {
        def->DumpIRGlobal();
    }
    return "";
}

void ConstDefAST::Dump() const
{
    DeclData data{EDecl::ConstDecl, ident, (initVal->CalcExp())};
    symbolManager.AddSymbol(ident, data);
    cout << "ConstDefAST { " << endl;
    cout << "Ident: " << ident << " Initval: ";
    initVal->Dump();
    cout << " } " << endl;
}
string ConstDefAST::DumpIR() const
{
    if (!arraySymbols)
    {
        DeclData data{EDecl::ConstDecl, ident, (initVal->CalcExp())};
        symbolManager.AddSymbol(ident, data);
        return "";
    }
    DeclData data{EDecl::ConstArray, ident};
    data = symbolManager.AddSymbol(ident, data);
    cout << "\t@" << data.globalIdent << " = alloc ";
    for (int i = arraySymbols->size() - 1; i >= 0; i--)
    {
        cout << "[";
    }
    cout << "i32, ";
    for (int i = arraySymbols->size() - 1; i >= 0; i--)
    {
        if (i != 0)
            cout << (*arraySymbols)[i]->CalcExp() << "], ";
        else
            cout << (*arraySymbols)[i]->CalcExp() << "]";
    }
    cout << endl;
    // 以下暂时只能处理一维
    int size = (*arraySymbols)[0]->CalcExp(); // 获取数组大小
    auto initList = initVal->GetList();
    for (int i = 0; i < size; i++)
    {
        cout << "\t%" << expNum << " = getelemptr @" << data.globalIdent << ", " << i << endl;
        cout << "\tstore ";
        if (i <= initList.size() - 1)
            cout << initList[i];
        else
            cout << "0";
        cout << ", %" << expNum++;
        cout << endl;
    }
    return "";
}
string ConstDefAST::DumpIRGlobal()
{
    if (!arraySymbols)
    {
        DeclData data{EDecl::ConstDecl, ident, (initVal->CalcExp())};
        symbolManager.AddSymbol(ident, data);
        return "";
    }
    DeclData data{EDecl::ConstArray, ident};
    data = symbolManager.AddSymbol(ident, data);
    cout << "global @" << data.globalIdent << " = alloc ";
    for (int i = arraySymbols->size() - 1; i >= 0; i--)
    {
        cout << "[";
    }
    cout << "i32, ";
    for (int i = arraySymbols->size() - 1; i >= 0; i--)
    {
        if (i != 0)
            cout << (*arraySymbols)[i]->CalcExp() << "], ";
        else
            cout << (*arraySymbols)[i]->CalcExp() << "]";
    }
    cout << ", {";
    int size = (*arraySymbols)[0]->CalcExp(); // 获取数组大小
    auto initList = initVal->GetList();
    for (int i = 0; i < size; i++)
    {
        if (i <= initList.size() - 1)
            cout << initList[i];
        else
            cout << "0";
        if (i < size - 1)
            cout << ", ";
    }
    cout << "}" << endl;
    return "";
}

void InitValAST::Dump() const
{
    cout << "InitValAST { " << exp->CalcExp();
    cout << " } " << endl;
}
string InitValAST::DumpIR() const
{
    return exp->DumpIR();
    return "";
}
int InitValAST::CalcExp()
{
    return exp->CalcExp();
}
int InitValAST::GetType() const
{
    return (int)(type);
}
vector<int> InitValAST::GetList()
{
    vector<int> tmp;
    for (const auto &init : *list)
    {
        tmp.push_back(init->CalcExp());
    }

    return tmp;
}

void ConstExpAST::Dump() const
{
    cout << "ConstExpAST { " << endl;
    exp->Dump();
    cout << " } " << endl;
}
string ConstExpAST::DumpIR() const
{

    return "";
}
int ConstExpAST::CalcExp()
{
    return exp->CalcExp();
}

void BlockItemAST::Dump() const
{
    cout << "BlockItemAST { " << endl;
    item->Dump();
    cout << " } " << endl;
}
string BlockItemAST::DumpIR() const
{
    return item->DumpIR();
    return "";
}

void VarDeclAST::Dump() const
{
    cout << "VarDeclAST { " << endl;
    for (const auto &def : *defs)
    {
        def->Dump();
        cout << endl;
    }
    cout << " } " << endl;
}
string VarDeclAST::DumpIR() const
{
    for (const auto &def : *defs)
    {
        def->DumpIR();
    }
    return "";
}
string VarDeclAST::DumpIRGlobal()
{
    for (const auto &def : *defs)
    {
        def->DumpIRGlobal();
    }
    return "";
}

void VarDefAST::Dump() const
{
    cout << "VarDefAST { ident: " << ident;
    cout << " } " << endl;
}
string VarDefAST::DumpIR() const
{
    if (!arraySymbols)
    {
        string value = "0";
        if (initVal)
            value = initVal->DumpIR();
        auto data = DeclData{EDecl::Variable, ident, 0, value};
        data = symbolManager.AddSymbol(ident, data);
        cout << "\t@" << data.globalIdent << " = alloc i32" << endl;
        cout << "\tstore " << value << ", @" << data.globalIdent << endl;
        return "";
    }
    DeclData data{EDecl::VarArray, ident};
    data = symbolManager.AddSymbol(ident, data);
    cout << "\t@" << data.globalIdent << " = alloc ";
    for (int i = arraySymbols->size() - 1; i >= 0; i--)
    {
        cout << "[";
    }
    cout << "i32, ";
    for (int i = arraySymbols->size() - 1; i >= 0; i--)
    {
        if (i != 0)
            cout << (*arraySymbols)[i]->CalcExp() << "], ";
        else
            cout << (*arraySymbols)[i]->CalcExp() << "]";
    }
    cout << endl;
    if (!initVal)
        return "";
    // 以下暂时只能处理一维
    int size = (*arraySymbols)[0]->CalcExp(); // 获取数组大小
    auto initList = initVal->GetList();
    for (int i = 0; i < size; i++)
    {
        cout << "\t%" << expNum << " = getelemptr @" << data.globalIdent << ", " << i << endl;
        cout << "\tstore ";
        if (i <= initList.size() - 1)
            cout << initList[i];
        else
            cout << "0";
        cout << ", %" << expNum++;
        cout << endl;
    }
    return "";
}
string VarDefAST::DumpIRGlobal()
{
    if (!arraySymbols)
    {
        DeclData data{EDecl::Variable, ident};
        data = symbolManager.AddSymbol(ident, data);
        cout << "global @" << data.globalIdent << " = alloc i32, ";
        if (initVal)
            cout << initVal->CalcExp() << endl;
        else
            cout << "zeroinit" << endl;
        return "";
    }
    DeclData data{EDecl::VarArray, ident};
    data = symbolManager.AddSymbol(ident, data);
    cout << "global @" << data.globalIdent << " = alloc ";
    for (int i = arraySymbols->size() - 1; i >= 0; i--)
    {
        cout << "[";
    }
    cout << "i32, ";
    for (int i = arraySymbols->size() - 1; i >= 0; i--)
    {
        if (i != 0)
            cout << (*arraySymbols)[i]->CalcExp() << "], ";
        else
            cout << (*arraySymbols)[i]->CalcExp() << "]";
    }
    if (!initVal)
    {
        cout << ", zeroinit" << endl;
        return "";
    }
    cout << ", {";
    int size = (*arraySymbols)[0]->CalcExp(); // 获取数组大小
    auto initList = initVal->GetList();
    for (int i = 0; i < size; i++)
    {
        if (i <= initList.size() - 1)
            cout << initList[i];
        else
            cout << "0";
        if (i < size - 1)
            cout << ", ";
    }
    cout << "}" << endl;
    return "";
}
