#pragma once
#include "koopa.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <map>
using namespace std;

vector<string> regNames = {"t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};
struct Reg
{
private:
    int regNum;
    string regName;

public:
    int stats;
    Reg()
    {
        SetReg(0);
    }
    Reg(int reg)
    {
        this->regNum = reg;
        if (reg >= 0 && reg < regNames.size())
            regName = regNames[reg];
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
vector<Reg> regs;
map<const koopa_raw_value_t, Reg> valueMap;

void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &block);
Reg Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_return_t &ret);
Reg Visit(const koopa_raw_integer_t &integer, const koopa_raw_value_t &value);
Reg Visit(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value);

Reg FindReg(const koopa_raw_value_t &value);
void InitRegs();
void ReleaseRegs(Reg &reg);

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
    Visit(function->bbs);
}
void Visit(const koopa_raw_basic_block_t &block)
{
    Visit(block->insts);
}
Reg Visit(const koopa_raw_value_t &value)
{
    if (valueMap.count(value))
        return valueMap[value];
    switch (value->kind.tag)
    {
    case KOOPA_RVT_RETURN:
        Visit(value->kind.data.ret);
        return Reg(0);
        break;
    case KOOPA_RVT_INTEGER:
        return Visit(value->kind.data.integer, value);
        break;
    case KOOPA_RVT_BINARY:
        return Visit(value->kind.data.binary, value);
        break;
    default:
        cout << value->kind.tag;
        return Reg(0);
        break;
    }

}
void Visit(const koopa_raw_return_t &ret)
{
    Reg res = Visit(ret.value);
    cout << "\tmv a0," << res.GetRegName() << endl;
    cout << "\tret" << endl;
}
Reg Visit(const koopa_raw_integer_t &integer, const koopa_raw_value_t &value)
{
    Reg res = FindReg(value);
    cout << "\tli " << res.GetRegName() << ", " << integer.value << endl;
    return res;
}
Reg Visit(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value)
{
    Reg lhs = Visit(binary.lhs);
    Reg rhs = Visit(binary.rhs);
    Reg res = FindReg(value);
    switch (binary.op)
    {
    case KOOPA_RBO_ADD:
        cout << "\tadd " << res.GetRegName() << ", " << lhs.GetRegName() << ", " << rhs.GetRegName() << endl;
        break;
    case KOOPA_RBO_SUB:
        cout << "\tsub " << res.GetRegName() << ", " << lhs.GetRegName() << ", " << rhs.GetRegName() << endl;
        break;
    case KOOPA_RBO_EQ:
        cout << "\txor " << res.GetRegName() << ", " << lhs.GetRegName() << ", " << rhs.GetRegName() << endl;
        cout << "\tseqz " << res.GetRegName() << ", " << res.GetRegName() << endl;
        break;
    case KOOPA_RBO_MUL:
        cout << "\tmul " << res.GetRegName() << ", " << lhs.GetRegName() << ", " << rhs.GetRegName() << endl;
        break;
    case KOOPA_RBO_DIV:
        cout << "\tdiv " << res.GetRegName() << ", " << lhs.GetRegName() << ", " << rhs.GetRegName() << endl;
        break;
    case KOOPA_RBO_MOD:
        cout << "\trem " << res.GetRegName() << ", " << lhs.GetRegName() << ", " << rhs.GetRegName() << endl;
        break;
    default:
        cout << "Unknown op:" << binary.op << endl;
        assert(false);
    }
    ReleaseRegs(lhs);
    ReleaseRegs(rhs);
    return res;
}
Reg FindReg(const koopa_raw_value_t &value)
{
    for (int i = 0; i < regs.size(); i++)
    {
        if (regs[i].stats == 0)
        {
            regs[i].stats = 1;
            valueMap[value] = regs[i];
            return regs[i];
        }
    }
    assert(false);
}
void InitRegs()
{
    init = true;
    for (int i = 0; i < regNames.size(); i++)
    {
        Reg reg(i);
        reg.stats = 0;
        regs.push_back(reg);
    }
}
void ReleaseRegs(Reg &reg)
{
    regs[reg.GetReg()].stats = 0;
}