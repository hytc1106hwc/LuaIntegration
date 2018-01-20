/*
** $Id: lparser.h,v 1.57.1.1 2007/12/27 13:02:25 roberto Exp $
** Lua Parser		�﷨����
** See Copyright Notice in lua.h
*/

#ifndef lparser_h
#define lparser_h

#include "llimits.h"
#include "lobject.h"
#include "lzio.h"


/*
** Expression descriptor 
** ���ʽ������
*/

typedef enum {
  VVOID,	/* no value */
  /******************************�������ʽ*******************************/
  VNIL,
  VTRUE,
  VFALSE,

// ������ʾһ��ͨ�������ϵĳ������ʽ��ʹ��info���洢��������ĳ���ֵ�ڳ������е�id 
// �ַ��������ͱ�ֱ�Ӵ�����VK���ͣ�Ȼ�����Ӧ���ַ���ֵ���浽�������У�����id���浽info�С�
  VK,		/* info = index of constant in `k' */	

	// �� VNUM��������֧����������ĳ����Ż�(constfolding)��������Ż����ˣ�Ҳ�Ͳ���Ҫ�ڳ������г���
  VKNUM,	/* nval = numerical value */		
  /***********************************************************************/

  /******************************�������ʽ*******************************/
  VLOCAL,	/* info = local register */	// ��info�б���ֲ�������Ӧ�ļĴ���id

  VUPVAL,       /* info = index of upvalue in `upvalues' */	 // ��info�б���upvalue��id

  VGLOBAL,	/* info = index of table; aux = index of global name in `k' */

  // �����һ������������ı������ʽ������a.b����a[1]��ʹ��ind�ṹ�屣�����ݡ�
 //  idx��������������key��id����������һ���Ĵ���id���߳���id��t���汻�������id����������һ���Ĵ���id����upvalue id��
 // vt��ʾt�������ǼĴ���id(VLOCAL)����upvalue id(VUPVAL)��
  VINDEXED,	/* info = table register; aux = index register (or `k') */
  /***********************************************************************/

  VJMP,		/* info = instruction pc */
  VRELOCABLE,	/* info = instruction pc */
  VNONRELOC,	/* info = result register */
  VCALL,	/* info = instruction pc */
  VVARARG	/* info = instruction pc */
} expkind;
// ���������ɴ�����м�ת���ṹ
typedef struct expdesc {
  expkind k;	// ��ʾ���ʽ������
  union {
    struct { int info, aux; } s;
    lua_Number nval;
  } u;	// ��ʾ��Ӧ���͵�����
  int t;  /* patch list of `exit when true' */ // t��f�ֱ��ʾ���ʽΪtrue��falseʱ����������תָ����±�
  int f;  /* patch list of `exit when false' */
} expdesc;


typedef struct upvaldesc {
  lu_byte k;
  lu_byte info;
} upvaldesc;


struct BlockCnt;  /* defined in lparser.c */


/* state needed to generate code for a given function */
// ����״̬��Ϣ
typedef struct FuncState {
  Proto *f;  /* current function header */
  Table *h;  /* table to find (and reuse) elements in `k' */
  struct FuncState *prev;  /* enclosing function */
  struct LexState *ls;  /* lexical state */
  struct lua_State *L;  /* copy of the Lua state */
  struct BlockCnt *bl;  /* chain of current blocks */
  int pc;  /* next position to code (equivalent to `ncode') */	// Proto��ָ������(code)Ԫ������
  int lasttarget;   /* `pc' of last `jump target' */
  int jpc;  /* list of pending jumps to `pc' */
  int freereg;  /* first free register */	// ��һ�����еļĴ���id��
  int nk;  /* number of elements in `k' */		// ��ǰProto�г�������(k)Ԫ������
  int np;  /* number of elements in `p' */
  short nlocvars;  /* number of elements in `locvars' */		// ����������
  lu_byte nactvar;  /* number of active local variables */
  upvaldesc upvalues[LUAI_MAXUPVALUES];  /* upvalues */
  unsigned short actvar[LUAI_MAXVARS];  /* declared-variable stack */
} FuncState;


LUAI_FUNC Proto *luaY_parser (lua_State *L, ZIO *z, Mbuffer *buff,
                                            const char *name);


#endif
