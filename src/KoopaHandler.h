#pragma once

#include "koopa.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <map>
#include <cmath>
#include <string>

using namespace std;

vector<string> regNames = {"t1", "t2", "t3", "t4", "t5", "t6"};
vector<string> paramRegNames = {"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};

struct Reg
{
private:
    int regNum;
    string regName;

public:
    ERISVSave type;
    int stats;
    int offset;
    koopa_raw_value_t value;

    Reg()
    {
        offset = -1;
        SetReg(0);
    }

    Reg(int reg)
    {
        offset = -1;
        this->regNum = reg;
        if (reg >= 0 && reg < regNames.size())
            regName = regNames[reg];
        else if (reg >= regNames.size() && reg <= regNames.size() + paramRegNames.size() - 1)
            regName = paramRegNames[reg - regNames.size()];
        else
            regName = "Stack";
    }

    string GetRegName()
    {
        return regName;
    }

    int GetReg()
    {
        return regNum;
    }

    void SetReg(int reg)
    {
        assert(reg >= 0 && reg < regNames.size() + paramRegNames.size());
        this->regNum = reg;
        if (reg >= 0 && reg < regNames.size())
            regName = regNames[reg];
        else if (reg >= regNames.size() && reg <= regNames.size() + paramRegNames.size() - 1)
            regName = paramRegNames[reg - regNames.size()];
        else
            regName = "Stack";
    }
};

bool init = false;
bool infunc = false;
bool needSaveRa;
int stackTop = 0;
int stackSize = 0;
vector<Reg> regs;
vector<Reg> paramRegs;
vector<Reg> tmpRegs;
map<const koopa_raw_value_t, Reg *> valueMap;

void Visit(const koopa_raw_program_t &program);

void Visit(const koopa_raw_slice_t &slice);

void Visit(const koopa_raw_function_t &func);

void Visit(const koopa_raw_basic_block_t &block);

Reg *Visit(const koopa_raw_value_t &value);

void Visit(const koopa_raw_return_t &ret);

Reg *Visit(const koopa_raw_integer_t &integer, const koopa_raw_value_t &value);

Reg *Visit(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value);

Reg *Visit(const koopa_raw_store_t &store, const koopa_raw_value_t &value);

Reg *Visit(const koopa_raw_load_t &load, const koopa_raw_value_t &value);

void Visit(const koopa_raw_branch_t &branch, const koopa_raw_value_t &value);

void Visit(const koopa_raw_jump_t &jump, const koopa_raw_value_t &value);

void Visit(const koopa_raw_global_alloc_t &globalAlloc, const koopa_raw_value_t &value);

Reg *Visit(const koopa_raw_call_t &call, const koopa_raw_value_t &value);

Reg *Visit(const koopa_raw_get_elem_ptr_t &getPtr, const koopa_raw_value_t &value);

Reg *FindReg(const koopa_raw_value_t &value);

Reg *SaveToStack(const koopa_raw_value_t &value);

void InitRegs();

void ReleaseRegs(Reg *reg, bool erase = true);

int CalcSize(const koopa_raw_type_t &ptr);

void VisitAggregate(const koopa_raw_value_t &aggre);

void Visit(const koopa_raw_program_t &program)
{
    if (!init)
        InitRegs();
    Visit(program.values);
    infunc = true;
    Visit(program.funcs);
}

void Visit(const koopa_raw_slice_t &slice)
{
    for (int i = 0; i < slice.len; i++)
    {
        auto ptr = slice.buffer[i];
        switch (slice.kind)
        {
        case KOOPA_RSIK_FUNCTION:
            Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
            break;
        case KOOPA_RSIK_BASIC_BLOCK:
            Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
            break;
        case KOOPA_RSIK_VALUE:
            Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
            break;
        default:
            assert(false);
            break;
        }
    }
}

void Visit(const koopa_raw_function_t &function)
{
    if (function->bbs.len == 0)
        return;
    int maxArgNum = -1;

    stackSize = 0;
    stackTop = 0;
    needSaveRa = false;

    cout << "\t.text" << endl;
    cout << "\t.global " << (function->name + 1) << endl;
    cout << (function->name + 1) << ":" << endl;
    for (int i = 0; i < function->bbs.len; i++)
    {
        auto ptr = function->bbs.buffer[i];
        auto block = reinterpret_cast<koopa_raw_basic_block_t>(ptr);
        // cout<<block->insts.len<<endl;
        for (int j = 0; j < block->insts.len; j++)
        {

            ptr = block->insts.buffer[j];
            auto inst = reinterpret_cast<koopa_raw_value_t>(ptr);
            if (inst->ty->tag != KOOPA_RTT_UNIT)
            {
                if (inst->ty->tag == KOOPA_RVT_ALLOC)
                {
                    stackSize += CalcSize(inst->ty->data.pointer.base);
                }
                else
                    stackSize += 4;
            }
            if (inst->kind.tag == KOOPA_RVT_CALL)
            {
                needSaveRa = true;

                int argNum = inst->kind.data.call.args.len;

                if (argNum > maxArgNum)
                {
                    maxArgNum = argNum;
                }
            }
        }
    }

    if (maxArgNum > 8)
    {
        stackSize += (maxArgNum - 8) * 4;
        stackTop += (maxArgNum - 8) * 4;
    }

    stackSize = ceil(stackSize / 16.0) * 16;
    if (stackSize <= 2047 && stackSize > 0)
    {
        cout << "\taddi sp, sp, -" << stackSize << endl;
    }
    else if (stackSize > 2047)
    {
        cout << "\tli t0, " << -1 * stackSize << endl;
        cout << "\tadd sp, sp, t0" << endl;
    }
    if (needSaveRa)
    {
        if (stackSize - 4 <= 2047)
            cout << "\tsw ra, " << stackSize - 4 << "(sp)" << endl;
        else
        {
            cout << "\tli t0, " << stackSize - 4 << endl;
            cout << "\tadd s11, t0, sp" << endl;
            cout << "\tsw ra, (s11)" << endl;
        }
    }
    for (int i = 0; i < function->params.len; i++)
    {
        auto ptr = function->params.buffer[i];
        koopa_raw_value_t param = reinterpret_cast<koopa_raw_value_t>(ptr);
        if (i < 8)
        {
            auto reg = &paramRegs[i];
            valueMap[param] = reg;
            reg->value = param;
        }
        else
        {
            Reg stack;
            stack.offset = (i - 8) * 4 + stackSize;
            stack.value = param;
            tmpRegs.push_back(stack);
            valueMap[param] = &(tmpRegs[tmpRegs.size() - 1]);
        }
    }
    Visit(function->bbs);
}

void Visit(const koopa_raw_basic_block_t &block)
{
    string name = (block->name + 1);
    if (name.compare("entry"))
        cout << (block->name + 1) << ":" << endl;
    Visit(block->insts);
}

Reg *Visit(const koopa_raw_value_t &value)
{
    if (valueMap.count(value))
        return valueMap[value];
    switch (value->kind.tag)
    {
    case KOOPA_RVT_RETURN:
        Visit(value->kind.data.ret);
        return nullptr;
        break;
    case KOOPA_RVT_INTEGER:
        return Visit(value->kind.data.integer, value);
        break;
    case KOOPA_RVT_BINARY:
        return Visit(value->kind.data.binary, value);
        break;
    case KOOPA_RVT_ALLOC:

        if (valueMap.count(value))
        {
            return valueMap[value];
        }
        else
        {
            auto stackReg = SaveToStack(value);
            return stackReg;
        }
        break;
    case KOOPA_RVT_STORE:
        return Visit(value->kind.data.store, value);
        break;
    case KOOPA_RVT_LOAD:
        return Visit(value->kind.data.load, value);
        break;
    case KOOPA_RVT_JUMP:
        Visit(value->kind.data.jump, value);
        return nullptr;
        break;
    case KOOPA_RVT_BRANCH:
        Visit(value->kind.data.branch, value);
        return nullptr;
        break;
    case KOOPA_RVT_CALL:
        return Visit(value->kind.data.call, value);

        break;
    case KOOPA_RVT_GLOBAL_ALLOC:
        Visit(value->kind.data.global_alloc, value);
        break;
    case KOOPA_RVT_ZERO_INIT:
        return nullptr;
        break;
    case KOOPA_RVT_GET_ELEM_PTR:
        return Visit(value->kind.data.get_elem_ptr, value);
        break;
    default:
        cout << value->kind.tag;
        return nullptr;
        break;
    }
    return nullptr;
}

void Visit(const koopa_raw_return_t &ret)
{
    if (ret.value && ret.value->ty->tag != KOOPA_RTT_UNIT)
    {

        Reg *res = Visit(ret.value);
        if (res->offset >= 0)
        {
            if (res->offset <= 2047)
                cout << "\tlw a0, " << res->offset << "(sp)" << endl;
            else
            {
                cout << "\tli s11, " << res->offset << endl;
                cout << "\tadd s11, s11, sp" << endl;
                cout << "\tlw a0, 0(s11)" << endl;
            }
        }
        else
            cout << "\tmv a0," << res->GetRegName() << endl;
    }
    if (needSaveRa)
    {
        if (stackSize - 4 <= 2047)
            cout << "\tlw ra, " << stackSize - 4 << "(sp)" << endl;
        else
        {
            cout << "\tli s11, " << stackSize - 4 << endl;
            cout << "\tadd s11, s11, sp" << endl;
            cout << "\tlw ra, 0(s11)" << endl;
        }
    }
    if (stackSize > 0)
    {
        if (stackSize <= 2047)
            cout << "\taddi sp, sp, " << stackSize << endl;
        else
        {
            cout << "\tli s11, " << stackSize << endl;
            cout << "\tadd sp, sp, s11" << endl;
        }
    }
    cout << "\tret" << endl;
    cout << endl;
}

Reg *Visit(const koopa_raw_integer_t &integer, const koopa_raw_value_t &value)
{
    Reg *res;
    if (infunc)
    {
        res = FindReg(value);
        cout << "\tli " << res->GetRegName() << ", " << integer.value << endl;
    }
    else
    {
        Reg tmp;
        tmp.offset = integer.value;
        tmpRegs.push_back(tmp);
        res = &(tmpRegs[tmpRegs.size() - 1]);
    }
    return res;
}

Reg *Visit(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value)
{
    Reg *lhs = Visit(binary.lhs);
    Reg *rhs = Visit(binary.rhs);
    Reg *res = FindReg(value);
    if (lhs->offset >= 0)
    {
        auto tmp = FindReg(binary.lhs);
        if (lhs->offset <= 2047)
            cout << "\tlw " << tmp->GetRegName() << ", " << lhs->offset << "(sp)" << endl;
        else
        {
            cout << "\tli t0, " << stackSize - 4 << endl;
            cout << "\tadd s11, t0, sp" << endl;
            cout << "\tlw " << tmp->GetRegName() << ", 0(s11)" << endl;
        }
        lhs = tmp;
    }
    if (rhs->offset >= 0)
    {
        auto tmp = FindReg(binary.rhs);
        if (rhs->offset <= 2047)
            cout << "\tlw " << tmp->GetRegName() << ", " << rhs->offset << "(sp)" << endl;
        else
        {
            cout << "\tli t0, " << stackSize - 4 << endl;
            cout << "\tadd s11, t0, sp" << endl;
            cout << "\tlw " << tmp->GetRegName() << ", 0(s11)" << endl;
        }
        rhs = tmp;
    }
    switch (binary.op)
    {
    case KOOPA_RBO_ADD:
        cout << "\tadd " << res->GetRegName() << ", " << lhs->GetRegName() << ", " << rhs->GetRegName() << endl;
        break;
    case KOOPA_RBO_SUB:
        cout << "\tsub " << res->GetRegName() << ", " << lhs->GetRegName() << ", " << rhs->GetRegName() << endl;
        break;
    case KOOPA_RBO_EQ:
        cout << "\txor " << res->GetRegName() << ", " << lhs->GetRegName() << ", " << rhs->GetRegName() << endl;
        cout << "\tseqz " << res->GetRegName() << ", " << res->GetRegName() << endl;
        break;
    case KOOPA_RBO_NOT_EQ:
        cout << "\txor " << res->GetRegName() << ", " << lhs->GetRegName() << ", " << rhs->GetRegName() << endl;
        cout << "\tsnez " << res->GetRegName() << ", " << res->GetRegName() << endl;
        break;
    case KOOPA_RBO_MUL:
        cout << "\tmul " << res->GetRegName() << ", " << lhs->GetRegName() << ", " << rhs->GetRegName() << endl;
        break;
    case KOOPA_RBO_DIV:
        cout << "\tdiv " << res->GetRegName() << ", " << lhs->GetRegName() << ", " << rhs->GetRegName() << endl;
        break;
    case KOOPA_RBO_MOD:
        cout << "\trem " << res->GetRegName() << ", " << lhs->GetRegName() << ", " << rhs->GetRegName() << endl;
        break;
    case KOOPA_RBO_LE:
        cout << "\tsgt " << res->GetRegName() << ", " << lhs->GetRegName() << ", " << rhs->GetRegName() << endl;
        cout << "\tseqz " << res->GetRegName() << ", " << res->GetRegName() << endl;
        break;
    case KOOPA_RBO_GE:
        cout << "\tslt " << res->GetRegName() << ", " << lhs->GetRegName() << ", " << rhs->GetRegName() << endl;
        cout << "\tseqz " << res->GetRegName() << ", " << res->GetRegName() << endl;
        break;
    case KOOPA_RBO_LT:
        cout << "\tslt " << res->GetRegName() << ", " << lhs->GetRegName() << ", " << rhs->GetRegName() << endl;
        break;
    case KOOPA_RBO_GT:
        cout << "\tsgt " << res->GetRegName() << ", " << lhs->GetRegName() << ", " << rhs->GetRegName() << endl;
        break;
    case KOOPA_RBO_AND:
        cout << "\tand " << res->GetRegName() << ", " << lhs->GetRegName() << ", " << rhs->GetRegName() << endl;
        break;
    case KOOPA_RBO_OR:
        cout << "\tor " << res->GetRegName() << ", " << lhs->GetRegName() << ", " << rhs->GetRegName() << endl;
        break;

    default:
        cout << "Unknown op:" << binary.op << endl;
        assert(false);
    }
    if (stackTop <= 2047)
        cout << "\tsw " << res->GetRegName() << ", " << stackTop << "(sp)" << endl;
    else
    {
        cout << "\tli s11, " << stackTop << endl;
        cout << "\tadd s11, s11, sp" << endl;
        cout << "\tsw " << res->GetRegName() << ", 0(s11)" << endl;
    }
    ReleaseRegs(lhs);
    ReleaseRegs(rhs);
    ReleaseRegs(res);
    SaveToStack(value);
    return valueMap[value];
}

Reg *Visit(const koopa_raw_store_t &store, const koopa_raw_value_t &value)
{
    auto alloc = store.dest;
    auto valueReg = Visit(store.value);
    if (alloc->kind.tag == KOOPA_RVT_GLOBAL_ALLOC)
    {
        cout << "\tla t0, " << (alloc->name + 1) << endl;
        if (valueReg->offset >= 0)
        {
            auto tmp = FindReg(store.value);
            if (valueReg->offset <= 2047)
                cout << "\tlw " << tmp->GetRegName() << ", " << valueReg->offset << "(sp)" << endl;
            else
            {
                cout << "\tli s11, " << valueReg->offset << endl;
                cout << "\tadd s11, s11, sp" << endl;
                cout << "\tlw " << tmp->GetRegName() << ", 0(s11)" << endl;
            }
            valueReg = tmp;
        }
        cout << "\tsw " << valueReg->GetRegName() << ", 0(t0)" << endl;
        ReleaseRegs(valueReg);
        return nullptr;
    }
    assert(valueMap.count(alloc));
    auto allocReg = Visit(alloc);
    if (valueReg->offset >= 0)
    {
        auto tmp = FindReg(store.value);
        if (valueReg->offset <= 2047)
            cout << "\tlw " << tmp->GetRegName() << ", " << valueReg->offset << "(sp)" << endl;
        else
        {
            cout << "\tli s11, " << valueReg->offset << endl;
            cout << "\tadd s11, s11, sp" << endl;
            cout << "\tlw " << tmp->GetRegName() << ", "
                 << "0(s11)" << endl;
        }
        valueReg = tmp;
    }
    if (allocReg->offset <= 2047)
        cout << "\tsw " << valueReg->GetRegName() << ", " << allocReg->offset << "(sp)" << endl;
    else
    {
        cout << "\tli s11, " << allocReg->offset << endl;
        cout << "\tadd s11, s11, sp" << endl;
        cout << "\tsw " << valueReg->GetRegName() << ", "
             << "0(s11)" << endl;
    }
    ReleaseRegs(valueReg);
    return allocReg;
}

Reg *Visit(const koopa_raw_load_t &load, const koopa_raw_value_t &value)
{
    auto alloc = load.src;
    if (alloc->kind.tag == KOOPA_RVT_GLOBAL_ALLOC)
    {
        cout << "\tla s2, " << (alloc->name + 1) << endl;
        cout << "\tlw s2, 0(s2)" << endl;
        auto saveReg = SaveToStack(value);
        if (saveReg->offset <= 2047)
            cout << "\tsw s2, " << saveReg->offset << "(sp)" << endl;
        else
        {
            cout << "\tli s11, " << saveReg->offset << endl;
            cout << "\tadd s11, s11, sp" << endl;
            cout << "\tsw s2, "
                 << "0(s11)" << endl;
        }
        return saveReg;
    }
    auto allocReg = Visit(alloc);
    if (allocReg->offset >= 0)
    {
        if (allocReg->offset <= 2047)
            cout << "\tlw t0"
                 << ", " << allocReg->offset << "(sp)" << endl;
        else
        {
            cout << "\tli s11, " << allocReg->offset << endl;
            cout << "\tadd s11, s11, sp" << endl;
            cout << "\tlw t0"
                 << ", 0(s11)" << endl;
        }
    }
    else
        cout << "Error:Load from Reg whose offset is " << allocReg->offset << endl;
    auto saveReg = SaveToStack(value);
    if (saveReg->offset <= 2047)
        cout << "\tsw t0, " << saveReg->offset << "(sp)" << endl;
    else
    {
        cout << "\tli s11, " << saveReg->offset << endl;
        cout << "\tadd s11, s11, sp" << endl;
        cout << "\tsw t0, "
             << "0(s11)" << endl;
    }
    return saveReg;
}

Reg *SaveToStack(const koopa_raw_value_t &value)
{
    Reg stack(regs.size() + paramRegs.size());
    stack.offset = stackTop;
    stackTop += 4;
    assert(!valueMap.count(value));
    stack.value = value;
    regs.push_back(stack);
    valueMap[value] = &regs[regs.size() - 1];
    return &regs[regs.size() - 1];
}

void Visit(const koopa_raw_branch_t &branch, const koopa_raw_value_t &value)
{
    auto condition = Visit(branch.cond);
    if (condition->offset >= 0)
    {
        auto tmp = FindReg(branch.cond);
        if (condition->offset <= 2047)
            cout << "\tlw " << tmp->GetRegName() << ", " << condition->offset << "(sp)" << endl;
        else
        {
            cout << "\tli s11, " << condition->offset << endl;
            cout << "\tadd s11, s11, sp" << endl;
            cout << "\tlw " << tmp->GetRegName() << ", 0(s11)" << endl;
        }
        condition = tmp;
    }
    cout << "\tbnez " << condition->GetRegName() << ", " << (branch.true_bb->name + 1) << endl;
    cout << "\tj " << (branch.false_bb->name + 1) << endl;
    ReleaseRegs(condition);
}

void Visit(const koopa_raw_jump_t &jump, const koopa_raw_value_t &value)
{
    cout << "\tj " << (jump.target->name + 1) << endl;
}

Reg *Visit(const koopa_raw_call_t &call, const koopa_raw_value_t &value)
{
    for (int i = 0; i < call.args.len; i++)
    {
        auto ptr = call.args.buffer[i];
        koopa_raw_value_t ptrValue = reinterpret_cast<koopa_raw_value_t>(ptr);
        auto reg = Visit(ptrValue);
        if (i < 8)
        {
            if (reg->offset == -1)
                cout << "\tmv " << paramRegNames[i] << ", " << reg->GetRegName() << endl;
            else
            {
                if (reg->offset <= 2047)
                    cout << "\tlw t0, " << reg->offset << "(sp)" << endl;
                else
                {
                    cout << "\tli s11, " << reg->offset << endl;
                    cout << "\tadd s11, s11, sp" << endl;
                    cout << "\tlw t0, 0(s11)" << endl;
                }
                cout << "\tmv " << paramRegNames[i] << ", t0" << endl;
            }
        }
        else
        {
            if (reg->offset == -1)
            {
                if ((i - 8) * 4 <= 2047)
                    cout << "\tsw " << reg->GetRegName() << ", " << (i - 8) * 4 << "(sp)" << endl;
                else
                {
                    cout << "\tli s11, " << (i - 8) * 4 << endl;
                    cout << "\tadd s11, s11, sp" << endl;
                    cout << "\tsw " << reg->GetRegName() << ", 0(s11)" << endl;
                }
            }
            else
            {
                if (reg->offset <= 2047)
                    cout << "\tlw t0, " << reg->offset << "(sp)" << endl;
                else
                {
                    cout << "\tli s11, " << reg->offset << endl;
                    cout << "\tadd s11, s11, sp" << endl;
                    cout << "\tlw t0, 0(s11)" << endl;
                }
                if ((i - 8) * 4 <= 2047)
                    cout << "\tsw t0, " << (i - 8) * 4 << "(sp)" << endl;
                else
                {
                    cout << "\tli s11, " << (i - 8) * 4 << endl;
                    cout << "\tadd s11, s11, sp" << endl;
                    cout << "\tsw t0, 0(s11)" << endl;
                }
            }
        }
        ReleaseRegs(reg);
    }
    cout << "\tcall " << call.callee->name + 1 << endl;
    if (value->ty->tag != KOOPA_RTT_UNIT)
    {
        Reg ret;
        ret.offset = stackTop;
        tmpRegs.push_back(ret);
        if (stackTop <= 2047)
            cout << "\tsw a0, " << stackTop << "(sp)" << endl;
        else
        {
            cout << "\tli s11, " << stackTop << endl;
            cout << "\tadd s11, s11, sp" << endl;
            cout << "\tsw a0, 0(s11)" << endl;
        }
        stackTop += 4;
        valueMap[value] = &(tmpRegs[tmpRegs.size() - 1]);
        return &(tmpRegs[tmpRegs.size() - 1]);
    }
    else
        return nullptr;
}
void Visit(const koopa_raw_global_alloc_t &globalAlloc, const koopa_raw_value_t &value)
{
    cout << "\t.data" << endl;
    cout << "\t.global " << (value->name + 1) << endl;
    cout << (value->name + 1) << ":" << endl;
    switch (globalAlloc.init->kind.tag)
    {
    case KOOPA_RVT_ZERO_INIT:
        cout << "\t.zero " << CalcSize(globalAlloc.init->ty) << endl
             << endl;
        break;
    case KOOPA_RVT_INTEGER:
        cout << "\t.word " << globalAlloc.init->kind.data.integer.value << endl
             << endl;
        break;
    case KOOPA_RVT_AGGREGATE:
        VisitAggregate(globalAlloc.init);
        break;

    default:
        break;
    }
    // auto init = Visit(globalAlloc.init);
    // if (init == nullptr)
    // {
    //     cout << "\t.zero 4" << endl;
    // }
    // else
    // {
    //     cout << "\t.word " << init->offset << endl;
    // }
}

Reg *FindReg(const koopa_raw_value_t &value)
{
    if (!valueMap.count(value))
        for (int i = 0; i < regNames.size(); i++)
        {
            if (regs[i].stats == 0)
            {
                regs[i].stats = 1;
                valueMap[value] = &(regs[i]);
                regs[i].value = value;
                return &(regs[i]);
            }
        }
    else // 为处于栈中的数据找一个临时寄存器
        for (int i = 0; i < regNames.size(); i++)
        {
            if (regs[i].stats == 0)
            {
                regs[i].stats = 1;
                regs[i].value = value;
                return &(regs[i]);
            }
        }
    assert(false);
}
Reg *Visit(const koopa_raw_get_elem_ptr_t &getPtr, const koopa_raw_value_t &value)
{
    auto index = Visit(getPtr.index);
    auto source = Visit(getPtr.src);
    if (source->offset <= 2047)
    {
        cout << "\taddi s3, sp, " << source->offset << endl;
    }
    else
    {
        auto tmp = FindReg(getPtr.src);
        cout << "\tli " << tmp->GetRegName() << ", " << source->offset << endl;
        cout << "\tadd s3, sp" << tmp->GetRegName() << endl;
        ReleaseRegs(tmp, false);
    }
    if (index->offset >= 0)
    {
        auto tmp = FindReg(getPtr.index);
        if (index->offset <= 2047)
            cout << "\tlw " << tmp->GetRegName() << ", " << index->offset << "(sp)" << endl;
        else
        {
            cout << "\tli s11, " << index->offset << endl;
            cout << "\tadd s11, s11, sp" << endl;
            cout << "\tlw " << tmp->GetRegName() << ", 0(s11)" << endl;
        }
        index = tmp;
    }

    cout << "\tli s4, 4" << endl;
    cout << "\tmul s4, " << index->GetRegName() << ", s4" << endl;
    cout << "\tadd s3, s3, s4" << endl;
    ReleaseRegs(index);
    Reg saveReg;
    saveReg.offset = stackTop;
    stackTop += 4;
    tmpRegs.push_back(saveReg);
    if (saveReg.offset <= 2047)
    {
        cout << "\tlw s3,(s3)" << endl;
        cout << "\tsw s3, " << saveReg.offset << "(sp)" << endl;
    }
    else
    {
        auto tmp = FindReg(value);
        cout << "\tli " << tmp->GetRegName() << ", " << saveReg.offset << endl;
        cout << "\tadd s11, " << tmp->GetRegName() << ", sp" << endl;
        cout << "\tsw s3, 0"
             << "(s11)" << endl;
        ReleaseRegs(tmp, false);
    }
    valueMap[value] = &tmpRegs[tmpRegs.size() - 1];
    return &tmpRegs[tmpRegs.size() - 1];
}
void InitRegs()
{
    init = true;
    for (int i = 0; i < regNames.size(); i++)
    {
        Reg reg(i);
        reg.stats = 0;
        reg.type = ERISVSave::Reg;
        regs.push_back(reg);
    }
    for (int i = 0; i < paramRegNames.size(); i++)
    {
        Reg reg(i + regNames.size());
        reg.stats = 0;
        reg.type = ERISVSave::Reg;
        paramRegs.push_back(reg);
    }
}

void ReleaseRegs(Reg *reg, bool erase)
{
    if (reg->GetReg() < 0 || reg->GetReg() >= regs.size())
        return;
    regs[reg->GetReg()].stats = 0;
    if (erase && regs[reg->GetReg()].offset == -1)
        valueMap.erase(regs[reg->GetReg()].value);
}
int CalcSize(const koopa_raw_type_t &ptr)
{
    if (ptr->tag == KOOPA_RTT_ARRAY)
    {
        int child = CalcSize(ptr->data.array.base);
        int len = ptr->data.array.len;
        return len * child;
    }

    return 4;
}
void VisitAggregate(const koopa_raw_value_t &aggre)
{
    koopa_raw_slice_t elems = aggre->kind.data.aggregate.elems;
    for (int i = 0; i < elems.len; i++)
    {
        auto ptr = elems.buffer[i];
        auto value = reinterpret_cast<koopa_raw_value_t>(ptr);
        if (value->kind.tag == KOOPA_RVT_INTEGER)
            std::cout << "\t.word " << value->kind.data.integer.value << std::endl;
        else if (value->kind.tag == KOOPA_RVT_AGGREGATE)
            VisitAggregate(value);
    }
}