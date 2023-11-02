%code requires {
  #include <memory>
  #include <string>
  #include "AST.h"
}

%{
#include <iostream>
#include <memory>
#include <string>
#include "AST.h"
#include "Utilities.h"

// 声明 lexer 函数和错误处理函数 
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
  vector<unique_ptr<BaseAST>> *vec_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN LESS LARGER AND OR LESSEQ LARGEREQ EQ NE CONST
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt PrimaryExp UnaryExp Exp AddExp MulExp RelExp EqExp LAndExp LOrExp
%type <ast_val> Decl ConstDecl ConstDef ConstInitVal BlockItem ConstExp 
%type <int_val> Number AddOp MulOp RelOp EqOp LAndOp LOrOp BType
%type <str_val> UnaryOp LVal
%type <vec_val> BlockItemList ConstDefList
%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnit
  : FuncDef {
    auto comp=make_unique<CompUnitAST>();
    comp->funcDef=unique_ptr<BaseAST>($1);
    ast = move(comp);
  }
  ;

// FuncDef ::= FuncType IDENT '(' ')' Block;
// 我们这里可以直接写 '(' 和 ')', 因为之前在 lexer 里已经处理了单个字符的情况
// 解析完成后, 把这些符号的结果收集起来, 然后拼成一个新的字符串, 作为结果返回
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
// 你可能会问, FuncType, IDENT 之类的结果已经是字符串指针了
// 为什么还要用 unique_ptr 接住它们, 然后再解引用, 把它们拼成另一个字符串指针呢
// 因为所有的字符串指针都是我们 new 出来的, new 出来的内存一定要 delete
// 否则会发生内存泄漏, 而 unique_ptr 这种智能指针可以自动帮我们 delete
// 虽然此处你看不出用 unique_ptr 和手动 delete 的区别, 但当我们定义了 AST 之后
// 这种写法会省下很多内存管理的负担
FuncDef
  : FuncType IDENT '(' ')' Block {
    auto funcdef=new FuncDefAST();
    funcdef->funcType = unique_ptr<BaseAST>($1);
    funcdef->ident = *unique_ptr<string>($2);
    funcdef->block = unique_ptr<BaseAST>($5);
    
    $$ = funcdef;
  }
  ;

// 同上, 不再解释 
FuncType
  : INT {
    auto funcType=new FuncTypeAST();
    funcType->type="int";
    $$ = funcType;
  }
  ;
Decl
  :ConstDecl
  {
    auto decl = new DeclAST();
    decl->decl = unique_ptr<BaseAST>($1);
    $$ = decl;
  };
ConstDecl
  :CONST BType ConstDefList ';'
  {
    auto constDecl = new ConstDeclAST();
    constDecl->type = (EBType)($2);
    constDecl->defs = unique_ptr<vector<unique_ptr<BaseAST>>>($3);
    $$ = constDecl;
  };
BType
  :INT
  {
    $$ = 1;
  };
ConstDefList
  :ConstDef
  {
    auto list = new vector<unique_ptr<BaseAST>>();
    list->push_back(unique_ptr<BaseAST>($1));
    $$ = list;
  }
  |ConstDefList ',' ConstDef
  {
    auto list = $1;
    list->push_back(unique_ptr<BaseAST>($3));
    $$ = list;
  };
ConstDef
  :IDENT '=' ConstInitVal
  {
    auto def = new ConstDefAST();
    def->ident = (*unique_ptr<string>($1));
    def->initVal = unique_ptr<BaseAST>($3);
    $$ = def;
  };
ConstInitVal
  :ConstExp
  {
    auto init= new ConstInitValAST();
    init->constExp = unique_ptr<BaseAST>($1);
    $$ = init;
  };
ConstExp
  :Exp
  {
    auto exp = new ConstExpAST();
    exp->exp = unique_ptr<BaseAST>($1);
    $$ = exp;
  }
Block
  : '{' BlockItemList '}' {
    cout<<"new block"<<endl;
    auto block=new BlockAST();
    block->itemList = unique_ptr<vector<unique_ptr<BaseAST>>>($2);
    $$ = block;
  }
  ;
BlockItem
  :Decl
  {
    cout<<"new decl block"<<endl;
    auto item = new BlockItemAST();
    item->type = EBlockItem::Decl;
    item->item = unique_ptr<BaseAST>($1);
    $$ = item;
  }
  |Stmt
  {
    cout<<"new stmt block"<<endl;
    auto item = new BlockItemAST();
    item->type = EBlockItem::Stmt;
    item->item = unique_ptr<BaseAST>($1);
    $$ = item;
  };
BlockItemList
  :BlockItem
  {
    auto list = new vector<unique_ptr<BaseAST>>();
    list->push_back(unique_ptr<BaseAST>($1));
    cout<<"new block list"<<endl;
    $$ = list;
  }
  |BlockItemList  BlockItem
  {
    cout<<"list push back";
    auto list = $1;
    list->push_back(unique_ptr<BaseAST>($2));
    $$ = list;
  };
LVal
  :IDENT
  {
    $$ = $1;
  }         
                                                                       

Stmt
  : RETURN Exp ';' {
    cout<<"new stmt"<<endl;
    auto stmt=new StmtAST();
    stmt->exp = unique_ptr<BaseAST>($2);
    $$ = stmt;
  }
  ;

Number
  : INT_CONST {
    $$ = $1;
  }
  ;
Exp
  : LOrExp{
    auto exp = new ExpAST();
    exp->unaryExp = unique_ptr<BaseAST>($1);
    $$ = exp;
  };
UnaryExp
  : PrimaryExp
  {
    auto unaryExp = new UnaryExpAST();
    unaryExp->type = EUnaryExp::PrimaryExp;
    unaryExp->primaryExp = unique_ptr<BaseAST>($1);
    $$ = unaryExp;
  } 
  | UnaryOp UnaryExp
  {
    auto unaryExp = new UnaryExpAST();
    unaryExp->type = EUnaryExp::UnaryExp;
    unaryExp->unaryExp = unique_ptr<BaseAST>($2);
    unaryExp->op = (*unique_ptr<string>($1))[0];
    $$ = unaryExp;
  };
UnaryOp
  : '+'
  {
    $$ = new string("+");
  } 
  | '-' 
  {
    $$ = new string("-");
  } | '!'
  {
    $$ = new string("!");
  };

PrimaryExp
  : '(' Exp ')'
  {
    auto primaryExp = new PrimaryExpAST();
    primaryExp->type = EPrimaryExp::Exp;
    primaryExp->exp = unique_ptr<BaseAST>($2);
    $$ = primaryExp; 
  } 
  | Number
  {
    auto primaryExp = new PrimaryExpAST();
    primaryExp->type = EPrimaryExp::Number;
    primaryExp->num = $1;
    $$ = primaryExp;
  }
  |LVal
  {
    auto primaryExp = new PrimaryExpAST();
    primaryExp->type = EPrimaryExp::LVal;
    primaryExp->lval = (*unique_ptr<string>($1));
    $$ = primaryExp; 
  }
  ;
AddOp
  :'+'
  {
    $$ = 1;
  }
  |'-'
  {
    $$ = 2;
  }
MulOp
  :'*'
  {
    $$ = 3;
  }
  |'/'
  {
    $$ = 4;
  }
  |'%'
  {
    $$ = 5;
  }
MulExp
  :UnaryExp{
    auto mulExp = new MulExpAST();
    mulExp->type = EMulExp::Single;
    mulExp->unaryExp = unique_ptr<BaseAST>($1);
    $$ = mulExp;
  }
  |MulExp MulOp UnaryExp
  {
    auto mulExp = new MulExpAST();
    mulExp->type = EMulExp::Double;
    mulExp->lhs = unique_ptr<BaseAST>($1);
    mulExp->op = (EOp)($2);
    mulExp->rhs = unique_ptr<BaseAST>($3);
    $$ = mulExp;
  }
AddExp
  :MulExp
  {
    auto addExp = new AddExpAST();
    addExp->type = EAddExp::Single;
    addExp->mulExp = unique_ptr<BaseAST>($1);
    $$ = addExp;
  } 
  | AddExp AddOp MulExp
  {
    auto addExp = new AddExpAST();
    addExp->type = EAddExp::Double;
    addExp->lhs = unique_ptr<BaseAST>($1);
    addExp->op = (EOp)($2);
    addExp->rhs = unique_ptr<BaseAST>($3);
    $$ = addExp;
  }
RelOp
  :LESS
  {
    $$ = 6;
  }
  |LARGER
  {
    $$ = 7;
  }
  |LESSEQ
  {
    $$ = 8;
  }
  |LARGEREQ
  {
    $$ = 9;
  }
RelExp
  :AddExp 
  {
    auto singleExp = new RelExpAST();
    singleExp->type = ERelExp::Single;
    singleExp->single = unique_ptr<BaseAST>($1);
    $$ = singleExp;
  }
  |RelExp RelOp AddExp
  {
    auto doubleExp = new RelExpAST();
    doubleExp->type = ERelExp::Double;
    doubleExp->lhs = unique_ptr<BaseAST>($1);
    doubleExp->rhs = unique_ptr<BaseAST>($3);
    doubleExp->op = (EOp)($2);
    $$ = doubleExp;
  }
EqOp
  :EQ
  {
    $$ = 10;
  } 
  |NE
  {
    $$ = 11;
  }
EqExp
  :RelExp
  {
    auto singleExp = new EqExpAST();
    singleExp->type = EEqExp::Single;
    singleExp->single = unique_ptr<BaseAST>($1);
    
  }
  |EqExp EqOp RelExp
  {
    auto doubleExp= new EqExpAST();
    doubleExp->type = EEqExp::Double;
    doubleExp->lhs = unique_ptr<BaseAST>($1);
    doubleExp->rhs = unique_ptr<BaseAST>($3);
    doubleExp->op = (EOp)($2);
    $$ = doubleExp;
  }
LAndOp
  :AND
  {
    $$ = 12;
  }
LOrOp
  :OR
  {
    $$ = 13;
  }
LAndExp
  :EqExp
  {
    auto eqExp = new LAndExpAST();
    eqExp->type = ELAndExp::Single;
    eqExp->single = unique_ptr<BaseAST>($1);
    $$ = eqExp;
  }
  |LAndExp LAndOp EqExp
  {
    auto eqExp = new LAndExpAST();
    eqExp->type = ELAndExp::Double;
    eqExp->lhs = unique_ptr<BaseAST>($1);
    eqExp->rhs = unique_ptr<BaseAST>($3);
    eqExp->op = (EOp)($2);
    $$ = eqExp;
  }
LOrExp
  :LAndExp
  {
    auto lAndExp = new LOrExpAST();
    lAndExp->type = ELOrExp::Single;
    lAndExp->single = unique_ptr<BaseAST>($1);
    $$ = lAndExp;
  }
  |LOrExp LOrOp LAndExp
  {
    auto lAndExp = new LOrExpAST();
    lAndExp->type = ELOrExp::Double;
    lAndExp->lhs = unique_ptr<BaseAST>($1);
    lAndExp->rhs = unique_ptr<BaseAST>($3);
    lAndExp->op = (EOp)($2);
    $$ = lAndExp;
  }
%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
