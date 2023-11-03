#pragma once
#include "koopa.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <map>
#include <cmath>
using namespace std;

vector<string> regNames = {"t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};
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
        assert(reg >= 0 && reg < regNames.size());
        this->regNum = reg;
        this->regName = regNames[reg];
    }
};
bool init = false;
int stackTop = 0;
int stackSize = 0;
vector<Reg> regs;
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

Reg *FindReg(const koopa_raw_value_t &value);
Reg *SaveToStack(const koopa_raw_value_t &value);
void InitRegs();
void ReleaseRegs(Reg *reg);

void Visit(const koopa_raw_program_t &program)
{
    if (!init)
        InitRegs();
    Visit(program.values);
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
    cout << "\t.text" << endl;
    cout << "\t.global " << (function->name + 1) << endl;
    cout << (function->name + 1) << ":" << endl;
    for (int i = 0; i < function->bbs.len; i++)
    {
        auto ptr = function->bbs.buffer[i];
        auto block = reinterpret_cast<koopa_raw_basic_block_t>(ptr);
        for (int j = 0; j < block->insts.len; j++)
        {
            ptr = block->insts.buffer[j];
            auto inst = reinterpret_cast<koopa_raw_value_t>(ptr);
            if (inst->ty->tag != KOOPA_RTT_UNIT)
            {
                stackSize += 4;
            }
        }
    }
    stackSize = ceil(stackSize / 16.0) * 16;
    if (stackSize <= 2047)
    {
        cout << "\taddi sp, sp, -" << stackSize << endl;
    }
    else
    {
        cout << "\tli t0, " << -1 * stackSize;
        cout << "\tadd sp, sp, t0" << endl;
    }
    Visit(function->bbs);
}
void Visit(const koopa_raw_basic_block_t &block)
{
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
            cout << "Read:" << endl;
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
        return nullptr;
        break;
    default:
        cout << value->kind.tag;
        return nullptr;
        break;
    }
}
void Visit(const koopa_raw_return_t &ret)
{
    Reg *res = Visit(ret.value);
    if (res->offset >= 0)
    {
        cout << "\tlw a0, "<<res->offset<<"(sp)"<<endl;
    }
    else
        cout << "\tmv a0," << res->GetRegName() << endl;
    cout<<"\taddi sp, sp, "<<stackSize<<endl;
    cout << "\tret" << endl;
}
Reg *Visit(const koopa_raw_integer_t &integer, const koopa_raw_value_t &value)
{
    Reg *res = FindReg(value);
    cout << "\tli " << res->GetRegName() << ", " << integer.value << endl;
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
        cout << "\tlw " << tmp->GetRegName() << ", " << lhs->offset << "(sp)" << endl;
        lhs = tmp;
    }
    if (rhs->offset >= 0)
    {
        auto tmp = FindReg(binary.rhs);
        cout << "\tlw " << tmp->GetRegName() << ", " << rhs->offset << "(sp)" << endl;
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
    cout << "\tsw " << res->GetRegName() << ", " << stackTop << "(sp)" << endl;
    ReleaseRegs(lhs);
    ReleaseRegs(rhs);
    ReleaseRegs(res);
    SaveToStack(value);
    return valueMap[value];
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
    else
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
Reg *Visit(const koopa_raw_store_t &store, const koopa_raw_value_t &value)
{
    auto alloc = store.dest;
    auto valueReg = Visit(store.value);
    assert(valueMap.count(alloc));
    auto allocReg = Visit(alloc);
    if (valueReg->offset >= 0)
    {
        auto tmp = FindReg(store.value);
        cout << "\tlw " << tmp->GetRegName() << ", " << valueReg->offset << "(sp)" << endl;
        valueReg = tmp;
    }
    cout << "\tsw " << valueReg->GetRegName() << ", " << allocReg->offset << "(sp)" << endl;
    ReleaseRegs(valueReg);
    return allocReg;
}
Reg *Visit(const koopa_raw_load_t &load, const koopa_raw_value_t &value)
{
    auto alloc = load.src;
    auto allocReg = Visit(alloc);
    if (allocReg->offset >= 0)
        cout << "\tlw t0"
             << ", " << allocReg->offset << "(sp)" << endl;
    else
        cout << "Error:Load from Reg whose offset is " << allocReg->offset << endl;
    auto saveReg = SaveToStack(value);
    cout << "\tsw t0, " << saveReg->offset << "(sp)" << endl;
    return saveReg;
}
Reg *SaveToStack(const koopa_raw_value_t &value)
{
    Reg stack(regs.size());
    stack.offset = stackTop;
    stackTop += 4;
    assert(!valueMap.count(value));
    stack.value = value;
    regs.push_back(stack);
    valueMap[value] = &regs[regs.size() - 1];
    return &regs[regs.size() - 1];
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
}

void ReleaseRegs(Reg *reg)
{
    regs[reg->GetReg()].stats = 0;
    if (regs[reg->GetReg()].offset == -1)
        valueMap.erase(regs[reg->GetReg()].value);
}