#include "koopa.h"
#include <iostream>
#include <cassert>
using namespace std;

void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &block);
void Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_return_t &ret);
void Visit(const koopa_raw_integer_t &integer);

void Visit(const koopa_raw_program_t &program)
{
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
    cout << "\t.global " << (function->name+1) << endl;
    cout << (function->name+1) << ":" << endl;
    Visit(function->bbs);
}
void Visit(const koopa_raw_basic_block_t &block)
{
    Visit(block->insts);
}
void Visit(const koopa_raw_value_t &value)
{
    switch (value->kind.tag)
    {
    case KOOPA_RVT_RETURN:
        Visit(value->kind.data.ret);
        break;
    case KOOPA_RVT_INTEGER:

        break;
    default:
        assert(false);
        break;
    }
}
void Visit(const koopa_raw_return_t &ret)
{
    cout << "\tli a0," << ret.value->kind.data.integer.value << endl;
    cout << "\tret" << endl;
}
void Visit(const koopa_raw_integer_t &integer)
{

}