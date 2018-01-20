/*
** $Id: lparser.h,v 1.57.1.1 2007/12/27 13:02:25 roberto Exp $
** Lua Parser		语法分析
** See Copyright Notice in lua.h
*/

#ifndef lparser_h
#define lparser_h

#include "llimits.h"
#include "lobject.h"
#include "lzio.h"


/*
** Expression descriptor 
** 表达式描述符
*/

typedef enum {
  VVOID,	/* no value */
  /******************************常量表达式*******************************/
  VNIL,
  VTRUE,
  VFALSE,

// 用来表示一个通常意义上的常量表达式，使用info来存储他所代表的常量值在常量表中的id 
// 字符串常量就被直接创建成VK类型，然后将其对应的字符串值保存到常量表中，并将id保存到info中。
  VK,		/* info = index of constant in `k' */	

	// 数 VNUM的作用是支持算数运算的常量优化(constfolding)，如果被优化掉了，也就不需要在常量表中出现
  VKNUM,	/* nval = numerical value */		
  /***********************************************************************/

  /******************************变量表达式*******************************/
  VLOCAL,	/* info = local register */	// 在info中保存局部变量对应的寄存器id

  VUPVAL,       /* info = index of upvalue in `upvalues' */	 // 在info中保存upvalue的id

  VGLOBAL,	/* info = index of table; aux = index of global name in `k' */

  // 代表对一个表进行索引的变量表达式，比如a.b或者a[1]，使用ind结构体保存数据。
 //  idx保存用来索引的key的id，他可能是一个寄存器id或者常量id；t保存被索引表的id，他可能是一个寄存器id或者upvalue id；
 // vt表示t的类型是寄存器id(VLOCAL)还是upvalue id(VUPVAL)。
  VINDEXED,	/* info = table register; aux = index register (or `k') */
  /***********************************************************************/

  VJMP,		/* info = instruction pc */
  VRELOCABLE,	/* info = instruction pc */
  VNONRELOC,	/* info = result register */
  VCALL,	/* info = instruction pc */
  VVARARG	/* info = instruction pc */
} expkind;
// 编译器生成代码的中间转换结构
typedef struct expdesc {
  expkind k;	// 表示表达式的类型
  union {
    struct { int info, aux; } s;
    lua_Number nval;
  } u;	// 表示对应类型的数据
  int t;  /* patch list of `exit when true' */ // t和f分别表示表达式为true和false时，待回填跳转指令的下标
  int f;  /* patch list of `exit when false' */
} expdesc;


typedef struct upvaldesc {
  lu_byte k;
  lu_byte info;
} upvaldesc;


struct BlockCnt;  /* defined in lparser.c */


/* state needed to generate code for a given function */
// 函数状态信息
typedef struct FuncState {
  Proto *f;  /* current function header */
  Table *h;  /* table to find (and reuse) elements in `k' */
  struct FuncState *prev;  /* enclosing function */
  struct LexState *ls;  /* lexical state */
  struct lua_State *L;  /* copy of the Lua state */
  struct BlockCnt *bl;  /* chain of current blocks */
  int pc;  /* next position to code (equivalent to `ncode') */	// Proto中指令数组(code)元素数量
  int lasttarget;   /* `pc' of last `jump target' */
  int jpc;  /* list of pending jumps to `pc' */
  int freereg;  /* first free register */	// 第一个空闲的寄存器id。
  int nk;  /* number of elements in `k' */		// 当前Proto中常量数组(k)元素数量
  int np;  /* number of elements in `p' */
  short nlocvars;  /* number of elements in `locvars' */		// 变量的数量
  lu_byte nactvar;  /* number of active local variables */
  upvaldesc upvalues[LUAI_MAXUPVALUES];  /* upvalues */
  unsigned short actvar[LUAI_MAXVARS];  /* declared-variable stack */
} FuncState;


LUAI_FUNC Proto *luaY_parser (lua_State *L, ZIO *z, Mbuffer *buff,
                                            const char *name);


#endif
