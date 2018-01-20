/*
** $Id: lopcodes.h,v 1.125.1.1 2007/12/27 13:02:25 roberto Exp $
** Opcodes for Lua virtual machine
** See Copyright Notice in lua.h
*/

#ifndef lopcodes_h
#define lopcodes_h

#include "llimits.h"


/*===========================================================================
  We assume that instructions are unsigned numbers.
  All instructions have an opcode in the first 6 bits.
  Instructions can have the following fields:
	`A' : 8 bits
	`B' : 9 bits
	`C' : 9 bits
	`Bx' : 18 bits (`B' and `C' together)
	`sBx' : signed Bx

  A signed argument is represented in excess K; that is, the number
  value is the unsigned value minus K. K is exactly the maximum value
  for that argument (so that -max is represented by 0, and +max is
  represented by 2*max), which is half the maximum for the corresponding
  unsigned argument.
===========================================================================*/
/*===========================================================================
	我们假定指令都是无符号数字
	所有指令都有一个操作码 (占6位)
	指令可能以下的字段:
	`A' : 8 位
	`B' : 9 位
	`C' : 9 位
	`Bx' : 18 位 (`B' 和 `C' 合并)
	`sBx' : 有符号的 Bx

  A signed argument is represented in excess K; that is, the number
  value is the unsigned value minus K. K is exactly the maximum value
  for that argument (so that -max is represented by 0, and +max is
  represented by 2*max), which is half the maximum for the corresponding
  unsigned argument.
===========================================================================*/


// 基本指令格式
enum OpMode {iABC, iABx, iAsBx};  /* basic instruction format */


/*
** size and position of opcode arguments.
*/
#define SIZE_C		9
#define SIZE_B		9
#define SIZE_Bx		(SIZE_C + SIZE_B) // 18
#define SIZE_A		8

#define SIZE_OP		6

// 字段起始位
#define POS_OP		0
#define POS_A		(POS_OP + SIZE_OP)  // 6
#define POS_C		(POS_A + SIZE_A)		 // 14
#define POS_B		(POS_C + SIZE_C)		 // 23
#define POS_Bx		POS_C					 // 14


/*
** limits for opcode arguments.		// 操作码参数限制
** we use (signed) int to manipulate most arguments,  
** so they must fit in LUAI_BITSINT-1 bits (-1 for sign)
*/
#if SIZE_Bx < LUAI_BITSINT-1
#define MAXARG_Bx        ((1<<SIZE_Bx)-1)			// Bx的最大值
#define MAXARG_sBx        (MAXARG_Bx>>1)         /* `sBx' is signed */	// sBx最大值
#else
#define MAXARG_Bx        MAX_INT
#define MAXARG_sBx        MAX_INT
#endif


#define MAXARG_A        ((1<<SIZE_A)-1)		// A最大值
#define MAXARG_B        ((1<<SIZE_B)-1)		// B最大值
#define MAXARG_C        ((1<<SIZE_C)-1)		// C最大值


/* creates a mask with `n' 1 bits at position `p' */		// 创建一个掩码 从第p位开始n个位为1.如 MASK1(3,5) = 00 00 00 - 1110 0000
#define MASK1(n,p)	((~((~(Instruction)0)<<n))<<p)

/* creates a mask with `n' 0 bits at position `p' */// 创建一个掩码 从第p位开始有n个位为0.如 MASK1(3,5) = FF FF FF - 0001 1111
#define MASK0(n,p)	(~MASK1(n,p))

/*
** the following macros help to manipulate instructions
** 下面这些宏帮助操作指令
*/

// 获取操作码
#define GET_OPCODE(i)	(cast(OpCode, ((i)>>POS_OP) & MASK1(SIZE_OP,0)))
// 设置操作码
#define SET_OPCODE(i,o)	((i) = (((i)&MASK0(SIZE_OP,POS_OP)) | \
		((cast(Instruction, o)<<POS_OP)&MASK1(SIZE_OP,POS_OP))))
//获取操作数A
#define GETARG_A(i)	(cast(int, ((i)>>POS_A) & MASK1(SIZE_A,0)))
// 设置操作数A
#define SETARG_A(i,u)	((i) = (((i)&MASK0(SIZE_A,POS_A)) | \
		((cast(Instruction, u)<<POS_A)&MASK1(SIZE_A,POS_A))))

// 获取操作数B
#define GETARG_B(i)	(cast(int, ((i)>>POS_B) & MASK1(SIZE_B,0)))
// 设置操作数B
#define SETARG_B(i,b)	((i) = (((i)&MASK0(SIZE_B,POS_B)) | \
		((cast(Instruction, b)<<POS_B)&MASK1(SIZE_B,POS_B))))

// 获取操作数C
#define GETARG_C(i)	(cast(int, ((i)>>POS_C) & MASK1(SIZE_C,0)))
// 设置操作数C
#define SETARG_C(i,b)	((i) = (((i)&MASK0(SIZE_C,POS_C)) | \
		((cast(Instruction, b)<<POS_C)&MASK1(SIZE_C,POS_C))))

// 获取Bx
#define GETARG_Bx(i)	(cast(int, ((i)>>POS_Bx) & MASK1(SIZE_Bx,0)))
// 设置Bx
#define SETARG_Bx(i,b)	((i) = (((i)&MASK0(SIZE_Bx,POS_Bx)) | \
		((cast(Instruction, b)<<POS_Bx)&MASK1(SIZE_Bx,POS_Bx))))

// 获取sBx
#define GETARG_sBx(i)	(GETARG_Bx(i)-MAXARG_sBx)
// 设置sBx
#define SETARG_sBx(i,b)	SETARG_Bx((i),cast(unsigned int, (b)+MAXARG_sBx))

// 创建ABC格式指令
#define CREATE_ABC(o,a,b,c)	((cast(Instruction, o)<<POS_OP) \
			| (cast(Instruction, a)<<POS_A) \
			| (cast(Instruction, b)<<POS_B) \
			| (cast(Instruction, c)<<POS_C))

// 创建ABx格式指令
#define CREATE_ABx(o,a,bc)	((cast(Instruction, o)<<POS_OP) \
			| (cast(Instruction, a)<<POS_A) \
			| (cast(Instruction, bc)<<POS_Bx))


/*
** Macros to operate RK indices
*/

/* this bit 1 means constant (0 means register) */
#define BITRK		(1 << (SIZE_B - 1))

// 是否是常量
/* test whether value is a constant */
#define ISK(x)		((x) & BITRK)
// 获取常量索引
/* gets the index of the constant */
#define INDEXK(r)	((int)(r) & ~BITRK)

#define MAXINDEXRK	(BITRK - 1)

/* code a constant index as a RK value */
#define RKASK(x)	((x) | BITRK)


/*
** invalid register that fits in 8 bits
*/
#define NO_REG		MAXARG_A


/*
** R(x) - register	// 寄存器
** Kst(x) - constant (in constant table) // 常量表
** RK(x) == if ISK(x) then Kst(INDEXK(x)) else R(x)	// 如果是常量则获取其索引后到常量表取值，否则去寄存器取值
*/


/*
** grep "ORDER OP" if you change these enums
*/
// 以R开头表示从寄存器中取,但是其实在Lua中并没有寄存器这一概念,只有前面说的Lua栈,所以其实这里的寄存器指代的是Lua栈.
// UpValue表示从当前函数的Upval数组中取值.
//	Kst开头的表示从常量数组中取值,常量数组的存放在Proto结构体的成员变量k中.
//	Gbl开头的表示从全局变量表中取值,存放在LClosure结构体的env变量中.
typedef enum {
/*----------------------------------------------------------------------
name		args	description
------------------------------------------------------------------------*/
OP_MOVE,/*	A B	R(A) := R(B)					*/				// MOVE指令,用于存放在栈上的变量之间的相互赋值.
OP_LOADK,/*	A Bx	R(A) := Kst(Bx)					*/		// 从以Bx为索引,从常量数组中取出一个值赋值到A指向的栈位置
OP_LOADBOOL,/*	A B C	R(A) := (Bool)B; if (C) pc++			*/		// 将布尔值B赋值到A指向的栈位置中,如果C为真,则PC指针加1
OP_LOADNIL,/*	A B	R(A) := ... := R(B) := nil			*/		// 将从A到B的栈数据,全部赋值为nil
OP_GETUPVAL,/*	A B	R(A) := UpValue[B]				*/	// 以B为索引从Upval数组中取值赋值给A指向的栈数据

OP_GETGLOBAL,/*	A Bx	R(A) := Gbl[Kst(Bx)]				*/	// 以B为索引,从常量数组中取值,作为索引,再到全局数组中取值赋值给A指向的栈数
OP_GETTABLE,/*	A B C	R(A) := R(B)[RK(C)]				*/		// 以B为索引从寄存器获取表R(B), 并以C为索引从寄存器或常量数组中获取对应值，作为key获取前面表中的值,放到R(A)中

OP_SETGLOBAL,/*	A Bx	Gbl[Kst(Bx)] := R(A)				*/		// 把R(A)赋值给Gbl[Kst(Bx)]
OP_SETUPVAL,/*	A B	UpValue[B] := R(A)				*/		// 把R(A)赋值给UpValue[B]
OP_SETTABLE,/*	A B C	R(A)[RK(B)] := RK(C)				*/ // RK(C)赋值给R(A)[RK(B)]

OP_NEWTABLE,/*	A B C	R(A) := {} (size = B,C)				*/	// 创建新表大小为B,C 赋值给R(A)

OP_SELF,/*	A B C	R(A+1) := R(B); R(A) := R(B)[RK(C)]		*/ // R(B)赋值给R(A+1), R(B)[RK(C)]赋值给R(A)

OP_ADD,/*	A B C	R(A) := RK(B) + RK(C)				*/ // R(A)=RK(B) + RK(C)
OP_SUB,/*	A B C	R(A) := RK(B) - RK(C)				*/ // R(A)=RK(B) + RK(C)
OP_MUL,/*	A B C	R(A) := RK(B) * RK(C)				*/ // R(A)=RK(B) * RK(C)
OP_DIV,/*	A B C	R(A) := RK(B) / RK(C)				*/ // R(A)=RK(B) / RK(C)
OP_MOD,/*	A B C	R(A) := RK(B) % RK(C)				*/ // R(A)=RK(B) % RK(C)
OP_POW,/*	A B C	R(A) := RK(B) ^ RK(C)				*/ // R(A)=RK(B) ^ RK(C)
OP_UNM,/*	A B	R(A) := -R(B)					*/	// R(A)= -R(B) 
OP_NOT,/*	A B	R(A) := not R(B)				*/ // R(A)= not R(B) 
OP_LEN,/*	A B	R(A) := length of R(B)				*/ // R(A)= length of  R(B) 

OP_CONCAT,/*	A B C	R(A) := R(B).. ... ..R(C)			*/  // R(A) = R(B) .. R(C)

OP_JMP,/*	sBx	pc+=sBx					*/	// pc += sBx	 指令跳转

OP_EQ,/*	A B C	if ((RK(B) == RK(C)) ~= A) then pc++		*/
OP_LT,/*	A B C	if ((RK(B) <  RK(C)) ~= A) then pc++  		*/
OP_LE,/*	A B C	if ((RK(B) <= RK(C)) ~= A) then pc++  		*/

OP_TEST,/*	A C	if not (R(A) <=> C) then pc++			*/ 
OP_TESTSET,/*	A B C	if (R(B) <=> C) then R(A) := R(B) else pc++	*/ 

OP_CALL,/*	A B C	R(A), ... ,R(A+C-2) := R(A)(R(A+1), ... ,R(A+B-1)) */
OP_TAILCALL,/*	A B C	return R(A)(R(A+1), ... ,R(A+B-1))		*/
OP_RETURN,/*	A B	return R(A), ... ,R(A+B-2)	(see note)	*/

OP_FORLOOP,/*	A sBx	R(A)+=R(A+2);
			if R(A) <?= R(A+1) then { pc+=sBx; R(A+3)=R(A) }*/
OP_FORPREP,/*	A sBx	R(A)-=R(A+2); pc+=sBx				*/

OP_TFORLOOP,/*	A C	R(A+3), ... ,R(A+2+C) := R(A)(R(A+1), R(A+2)); 
                        if R(A+3) ~= nil then R(A+2)=R(A+3) else pc++	*/ 
OP_SETLIST,/*	A B C	R(A)[(C-1)*FPF+i] := R(A+i), 1 <= i <= B	*/

OP_CLOSE,/*	A 	close all variables in the stack up to (>=) R(A)*/
OP_CLOSURE,/*	A Bx	R(A) := closure(KPROTO[Bx], R(A), ... ,R(A+n))	*/

OP_VARARG/*	A B	R(A), R(A+1), ..., R(A+B-1) = vararg		*/
} OpCode;


#define NUM_OPCODES	(cast(int, OP_VARARG) + 1)



/*===========================================================================
  Notes:
  (*) In OP_CALL, if (B == 0) then B = top. C is the number of returns - 1,
      and can be 0: OP_CALL then sets `top' to last_result+1, so
      next open instruction (OP_CALL, OP_RETURN, OP_SETLIST) may use `top'.

  (*) In OP_VARARG, if (B == 0) then use actual number of varargs and
      set top (like in OP_CALL with C == 0).

  (*) In OP_RETURN, if (B == 0) then return up to `top'

  (*) In OP_SETLIST, if (B == 0) then B = `top';
      if (C == 0) then next `instruction' is real C

  (*) For comparisons, A specifies what condition the test should accept
      (true or false).

  (*) All `skips' (pc++) assume that next instruction is a jump
===========================================================================*/


/*
** masks for instruction properties. The format is:
** bits 0-1: op mode		// 操作码格式
** bits 2-3: C arg mode	// C操作数格式
** bits 4-5: B arg mode	 // B 操作数格式
** bit 6: instruction set register A	// 
** bit 7: operator is a test
*/  

enum OpArgMask {
  OpArgN,  /* argument is not used */		// 操作数无用
  OpArgU,  /* argument is used */				// 操作数有用
  OpArgR,  /* argument is a register or a jump offset */	// 操作数是一个寄存器或者一个偏移
  OpArgK   /* argument is a constant or register/constant */	// 操作数是一个常量或者寄存器
};

// 指令码对应的格式属性, 基本操作指令格式及操作数存储位置
LUAI_DATA const lu_byte luaP_opmodes[NUM_OPCODES];

// 获取操作码格式
#define getOpMode(m)	(cast(enum OpMode, luaP_opmodes[m] & 3))
// 获取操作码对应的操作数格式
#define getBMode(m)	(cast(enum OpArgMask, (luaP_opmodes[m] >> 4) & 3))
#define getCMode(m)	(cast(enum OpArgMask, (luaP_opmodes[m] >> 2) & 3))
#define testAMode(m)	(luaP_opmodes[m] & (1 << 6))
#define testTMode(m)	(luaP_opmodes[m] & (1 << 7))

// 操作码名字
LUAI_DATA const char *const luaP_opnames[NUM_OPCODES+1];  /* opcode names */


/* number of list items to accumulate before a SETLIST instruction */
#define LFIELDS_PER_FLUSH	50


#endif
