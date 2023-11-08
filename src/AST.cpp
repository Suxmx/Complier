#include "AST.h"
using namespace std;

void CompUnitAST::Dump() const
{
    cout << "CompUnit { " << endl;
    funcDef->Dump();
    cout << " }";
}
string CompUnitAST::DumpIR() const
{
    return funcDef->DumpIR();
}
int CompUnitAST::CalcExp()
{
    return funcDef->CalcExp();
}

void FuncDefAST::Dump() const
{
    cout << "FuncDefAST { " << endl;
    funcType->Dump();
    cout << ", " << ident << ", ";
    block->Dump();
    cout << " }" << endl;
}
string FuncDefAST::DumpIR() const
{
    cout << "fun @" << ident << "(): ";
    funcType->DumpIR();
    cout << " {" << endl;
    cout << "\%entry:" << endl;
    block->DumpIR();
    cout << endl
         << "}";
    return "";
}
int FuncDefAST::CalcExp()
{
    return block->CalcExp();
}

void FuncTypeAST::Dump() const
{
    cout << "FuncType{ " << (int)(type) << " }" << endl;
}
string FuncTypeAST::DumpIR() const
{
    cout << "i32";
    return "";
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
    else if (type == EStmt::Exp)
    {
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
    return "";
}
int UnaryExpAST::CalcExp()
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
    DeclData data{EDecl::ConstDecl, ident, (initVal->CalcExp())};
    symbolManager.AddSymbol(ident, data);
    return "";
}

void InitValAST::Dump() const
{
    cout << "InitValAST { " << exp->CalcExp();
    ;
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

void VarDefAST::Dump() const
{
    cout << "VarDefAST { ident: " << ident;
    cout << " } " << endl;
}
string VarDefAST::DumpIR() const
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