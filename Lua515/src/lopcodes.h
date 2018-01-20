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
	���Ǽٶ�ָ����޷�������
	����ָ���һ�������� (ռ6λ)
	ָ��������µ��ֶ�:
	`A' : 8 λ
	`B' : 9 λ
	`C' : 9 λ
	`Bx' : 18 λ (`B' �� `C' �ϲ�)
	`sBx' : �з��ŵ� Bx

  A signed argument is represented in excess K; that is, the number
  value is the unsigned value minus K. K is exactly the maximum value
  for that argument (so that -max is represented by 0, and +max is
  represented by 2*max), which is half the maximum for the corresponding
  unsigned argument.
===========================================================================*/


// ����ָ���ʽ
enum OpMode {iABC, iABx, iAsBx};  /* basic instruction format */


/*
** size and position of opcode arguments.
*/
#define SIZE_C		9
#define SIZE_B		9
#define SIZE_Bx		(SIZE_C + SIZE_B) // 18
#define SIZE_A		8

#define SIZE_OP		6

// �ֶ���ʼλ
#define POS_OP		0
#define POS_A		(POS_OP + SIZE_OP)  // 6
#define POS_C		(POS_A + SIZE_A)		 // 14
#define POS_B		(POS_C + SIZE_C)		 // 23
#define POS_Bx		POS_C					 // 14


/*
** limits for opcode arguments.		// �������������
** we use (signed) int to manipulate most arguments,  
** so they must fit in LUAI_BITSINT-1 bits (-1 for sign)
*/
#if SIZE_Bx < LUAI_BITSINT-1
#define MAXARG_Bx        ((1<<SIZE_Bx)-1)			// Bx�����ֵ
#define MAXARG_sBx        (MAXARG_Bx>>1)         /* `sBx' is signed */	// sBx���ֵ
#else
#define MAXARG_Bx        MAX_INT
#define MAXARG_sBx        MAX_INT
#endif


#define MAXARG_A        ((1<<SIZE_A)-1)		// A���ֵ
#define MAXARG_B        ((1<<SIZE_B)-1)		// B���ֵ
#define MAXARG_C        ((1<<SIZE_C)-1)		// C���ֵ


/* creates a mask with `n' 1 bits at position `p' */		// ����һ������ �ӵ�pλ��ʼn��λΪ1.�� MASK1(3,5) = 00 00 00 - 1110 0000
#define MASK1(n,p)	((~((~(Instruction)0)<<n))<<p)

/* creates a mask with `n' 0 bits at position `p' */// ����һ������ �ӵ�pλ��ʼ��n��λΪ0.�� MASK1(3,5) = FF FF FF - 0001 1111
#define MASK0(n,p)	(~MASK1(n,p))

/*
** the following macros help to manipulate instructions
** ������Щ���������ָ��
*/

// ��ȡ������
#define GET_OPCODE(i)	(cast(OpCode, ((i)>>POS_OP) & MASK1(SIZE_OP,0)))
// ���ò�����
#define SET_OPCODE(i,o)	((i) = (((i)&MASK0(SIZE_OP,POS_OP)) | \
		((cast(Instruction, o)<<POS_OP)&MASK1(SIZE_OP,POS_OP))))
//��ȡ������A
#define GETARG_A(i)	(cast(int, ((i)>>POS_A) & MASK1(SIZE_A,0)))
// ���ò�����A
#define SETARG_A(i,u)	((i) = (((i)&MASK0(SIZE_A,POS_A)) | \
		((cast(Instruction, u)<<POS_A)&MASK1(SIZE_A,POS_A))))

// ��ȡ������B
#define GETARG_B(i)	(cast(int, ((i)>>POS_B) & MASK1(SIZE_B,0)))
// ���ò�����B
#define SETARG_B(i,b)	((i) = (((i)&MASK0(SIZE_B,POS_B)) | \
		((cast(Instruction, b)<<POS_B)&MASK1(SIZE_B,POS_B))))

// ��ȡ������C
#define GETARG_C(i)	(cast(int, ((i)>>POS_C) & MASK1(SIZE_C,0)))
// ���ò�����C
#define SETARG_C(i,b)	((i) = (((i)&MASK0(SIZE_C,POS_C)) | \
		((cast(Instruction, b)<<POS_C)&MASK1(SIZE_C,POS_C))))

// ��ȡBx
#define GETARG_Bx(i)	(cast(int, ((i)>>POS_Bx) & MASK1(SIZE_Bx,0)))
// ����Bx
#define SETARG_Bx(i,b)	((i) = (((i)&MASK0(SIZE_Bx,POS_Bx)) | \
		((cast(Instruction, b)<<POS_Bx)&MASK1(SIZE_Bx,POS_Bx))))

// ��ȡsBx
#define GETARG_sBx(i)	(GETARG_Bx(i)-MAXARG_sBx)
// ����sBx
#define SETARG_sBx(i,b)	SETARG_Bx((i),cast(unsigned int, (b)+MAXARG_sBx))

// ����ABC��ʽָ��
#define CREATE_ABC(o,a,b,c)	((cast(Instruction, o)<<POS_OP) \
			| (cast(Instruction, a)<<POS_A) \
			| (cast(Instruction, b)<<POS_B) \
			| (cast(Instruction, c)<<POS_C))

// ����ABx��ʽָ��
#define CREATE_ABx(o,a,bc)	((cast(Instruction, o)<<POS_OP) \
			| (cast(Instruction, a)<<POS_A) \
			| (cast(Instruction, bc)<<POS_Bx))


/*
** Macros to operate RK indices
*/

/* this bit 1 means constant (0 means register) */
#define BITRK		(1 << (SIZE_B - 1))

// �Ƿ��ǳ���
/* test whether value is a constant */
#define ISK(x)		((x) & BITRK)
// ��ȡ��������
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
** R(x) - register	// �Ĵ���
** Kst(x) - constant (in constant table) // ������
** RK(x) == if ISK(x) then Kst(INDEXK(x)) else R(x)	// ����ǳ������ȡ�������󵽳�����ȡֵ������ȥ�Ĵ���ȡֵ
*/


/*
** grep "ORDER OP" if you change these enums
*/
// ��R��ͷ��ʾ�ӼĴ�����ȡ,������ʵ��Lua�в�û�мĴ�����һ����,ֻ��ǰ��˵��Luaջ,������ʵ����ļĴ���ָ������Luaջ.
// UpValue��ʾ�ӵ�ǰ������Upval������ȡֵ.
//	Kst��ͷ�ı�ʾ�ӳ���������ȡֵ,��������Ĵ����Proto�ṹ��ĳ�Ա����k��.
//	Gbl��ͷ�ı�ʾ��ȫ�ֱ�������ȡֵ,�����LClosure�ṹ���env������.
typedef enum {
/*----------------------------------------------------------------------
name		args	description
------------------------------------------------------------------------*/
OP_MOVE,/*	A B	R(A) := R(B)					*/				// MOVEָ��,���ڴ����ջ�ϵı���֮����໥��ֵ.
OP_LOADK,/*	A Bx	R(A) := Kst(Bx)					*/		// ����BxΪ����,�ӳ���������ȡ��һ��ֵ��ֵ��Aָ���ջλ��
OP_LOADBOOL,/*	A B C	R(A) := (Bool)B; if (C) pc++			*/		// ������ֵB��ֵ��Aָ���ջλ����,���CΪ��,��PCָ���1
OP_LOADNIL,/*	A B	R(A) := ... := R(B) := nil			*/		// ����A��B��ջ����,ȫ����ֵΪnil
OP_GETUPVAL,/*	A B	R(A) := UpValue[B]				*/	// ��BΪ������Upval������ȡֵ��ֵ��Aָ���ջ����

OP_GETGLOBAL,/*	A Bx	R(A) := Gbl[Kst(Bx)]				*/	// ��BΪ����,�ӳ���������ȡֵ,��Ϊ����,�ٵ�ȫ��������ȡֵ��ֵ��Aָ���ջ��
OP_GETTABLE,/*	A B C	R(A) := R(B)[RK(C)]				*/		// ��BΪ�����ӼĴ�����ȡ��R(B), ����CΪ�����ӼĴ������������л�ȡ��Ӧֵ����Ϊkey��ȡǰ����е�ֵ,�ŵ�R(A)��

OP_SETGLOBAL,/*	A Bx	Gbl[Kst(Bx)] := R(A)				*/		// ��R(A)��ֵ��Gbl[Kst(Bx)]
OP_SETUPVAL,/*	A B	UpValue[B] := R(A)				*/		// ��R(A)��ֵ��UpValue[B]
OP_SETTABLE,/*	A B C	R(A)[RK(B)] := RK(C)				*/ // RK(C)��ֵ��R(A)[RK(B)]

OP_NEWTABLE,/*	A B C	R(A) := {} (size = B,C)				*/	// �����±��СΪB,C ��ֵ��R(A)

OP_SELF,/*	A B C	R(A+1) := R(B); R(A) := R(B)[RK(C)]		*/ // R(B)��ֵ��R(A+1), R(B)[RK(C)]��ֵ��R(A)

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

OP_JMP,/*	sBx	pc+=sBx					*/	// pc += sBx	 ָ����ת

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
** bits 0-1: op mode		// �������ʽ
** bits 2-3: C arg mode	// C��������ʽ
** bits 4-5: B arg mode	 // B ��������ʽ
** bit 6: instruction set register A	// 
** bit 7: operator is a test
*/  

enum OpArgMask {
  OpArgN,  /* argument is not used */		// ����������
  OpArgU,  /* argument is used */				// ����������
  OpArgR,  /* argument is a register or a jump offset */	// ��������һ���Ĵ�������һ��ƫ��
  OpArgK   /* argument is a constant or register/constant */	// ��������һ���������߼Ĵ���
};

// ָ�����Ӧ�ĸ�ʽ����, ��������ָ���ʽ���������洢λ��
LUAI_DATA const lu_byte luaP_opmodes[NUM_OPCODES];

// ��ȡ�������ʽ
#define getOpMode(m)	(cast(enum OpMode, luaP_opmodes[m] & 3))
// ��ȡ�������Ӧ�Ĳ�������ʽ
#define getBMode(m)	(cast(enum OpArgMask, (luaP_opmodes[m] >> 4) & 3))
#define getCMode(m)	(cast(enum OpArgMask, (luaP_opmodes[m] >> 2) & 3))
#define testAMode(m)	(luaP_opmodes[m] & (1 << 6))
#define testTMode(m)	(luaP_opmodes[m] & (1 << 7))

// ����������
LUAI_DATA const char *const luaP_opnames[NUM_OPCODES+1];  /* opcode names */


/* number of list items to accumulate before a SETLIST instruction */
#define LFIELDS_PER_FLUSH	50


#endif
