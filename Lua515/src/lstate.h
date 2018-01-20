/*
** $Id: lstate.h,v 2.24.1.2 2008/01/03 15:20:39 roberto Exp $
** Global State
** See Copyright Notice in lua.h
*/

#ifndef lstate_h
#define lstate_h

#include "lua.h"

#include "lobject.h"
#include "ltm.h"
#include "lzio.h"



struct lua_longjmp;  /* defined in ldo.c */


/* table of globals */
#define gt(L)	(&L->l_gt)

/* registry */
#define registry(L)	(&G(L)->l_registry)


/* extra stack space to handle TM calls and some other extras */
#define EXTRA_STACK   5


#define BASIC_CI_SIZE           8

#define BASIC_STACK_SIZE        (2*LUA_MINSTACK)



typedef struct stringtable {
  GCObject **hash;
  lu_int32 nuse;  /* number of elements */
  int size;
} stringtable;


/*
** informations about a call
*/
typedef struct CallInfo {
  StkId base;  /* base for this function */		//�հ����õ�ջ��
  StkId func;  /* function index in the stack */		// Ҫ���õıհ���ջ�ϵ�λ��
  StkId	top;  /* top for this function */		// �հ���ջʹ������, ����lua_push*��ʱ��ÿ��ŵ�, push̫��ͳ���, ����lua_checkstack����
  const Instruction *savedpc;		// ����ڱ��հ����ٴε��ñ�ıհ�, ��ô��ֵ�ͱ�����һ��ָ���Ա��ڷ���ʱ����ִ��
  int nresults;  /* expected number of results from this function */	// �հ�Ҫ���ص�ֵ����
  int tailcalls;  /* number of tail calls lost under this entry */	// β�ݹ���
} CallInfo;



#define curr_func(L)	(clvalue(L->ci->func))
#define ci_func(ci)	(clvalue((ci)->func))
#define f_isLua(ci)	(!ci_func(ci)->c.isC)
#define isLua(ci)	(ttisfunction((ci)->func) && f_isLua(ci))


/*
** `global state', shared by all threads of this state
*/
typedef struct global_State {
  stringtable strt;  /* hash table for strings */		// �ַ�����
  lua_Alloc frealloc;  /* function to reallocate memory */
  void *ud;         /* auxiliary data to `frealloc' */
  lu_byte currentwhite;				// ��ǰ�İ�ɫ�� 0 �ͻ��� 1 ��
  lu_byte gcstate;  /* state of garbage collector */	// �������յ�״̬
  int sweepstrgc;  /* position of sweep in `strt' */
  GCObject *rootgc;  /* list of all collectable objects */
  GCObject **sweepgc;  /* position of sweep in `rootgc' */
  GCObject *gray;  /* list of gray objects */
  GCObject *grayagain;  /* list of objects to be traversed atomically */
  GCObject *weak;  /* list of weak tables (to be cleared) */
  GCObject *tmudata;  /* last element of list of userdata to be GC */
  Mbuffer buff;  /* temporary buffer for string concatentation */
  lu_mem GCthreshold;
  lu_mem totalbytes;  /* number of bytes currently allocated */
  lu_mem estimate;  /* an estimate of number of bytes actually in use */
  lu_mem gcdept;  /* how much GC is `behind schedule' */
  int gcpause;  /* size of pause between successive GCs */
  int gcstepmul;  /* GC `granularity' */
  lua_CFunction panic;  /* to be called in unprotected errors */
  TValue l_registry;
  struct lua_State *mainthread;				// udata List	 // ��������userdata
  UpVal uvhead;  /* head of double-linked list of all open upvalues */  // ����lua�������������ջ��upvalue�����ͷ
  struct Table *mt[NUM_TAGS];  /* metatables for basic types */
  TString *tmname[TM_N];  /* array with tag-method names */	// ԭ������������
} global_State;


/*
** `per thread' state
*/
struct lua_State {
  CommonHeader;
  lu_byte status;
  StkId top;  /* first free slot in the stack */		// ��ʾ�����ջ�ϵĵ�һ�����е�slot 
  StkId base;  /* base of current function */		//��ʾ��ǰ���ں�����base�����base����˵����ջ�ס�ֻ�����ǵ�ǰ�����ġ� 
  global_State *l_G;
  CallInfo *ci;  /* call info for current function */			// ��ǰ�����õıհ��ĵ�����Ϣ
  const Instruction *savedpc;  /* `savedpc' of current function */
  StkId stack_last;  /* last free slot in the stack */	// ��ջ�ϵ����һ�����е�slot 
  StkId stack;  /* stack base */								// ջ��base�����������ջ��ջ��
  CallInfo *end_ci;  /* points after end of ci array*/	// ָ��CallInfo������һ��Ԫ��
  CallInfo *base_ci;  /* array of CallInfo's */			// ָ��CallInfo�����һ��Ԫ��
  int stacksize;											// ջ�Ĵ�С
  int size_ci;  /* size of array `base_ci' */					// ��¼CallInfo���鵱ǰ�Ĵ�С
  unsigned short nCcalls;  /* number of nested C calls */
  unsigned short baseCcalls;  /* nested C calls when resuming coroutine */
  lu_byte hookmask;
  lu_byte allowhook;
  int basehookcount;
  int hookcount;
  lua_Hook hook;

  // Щȫ��(���״̬)�õ��Ķ���
  TValue l_gt;  /* table of globals */
  TValue env;  /* temporary place for environments */

  // gc���
  GCObject *openupval;  /* list of open upvalues in this stack */	// �����Ҳ���ǵ�ǰ��ջ�ϵ����д򿪵�upval
  GCObject *gclist;

  // ������
  struct lua_longjmp *errorJmp;  /* current error recover point */
  ptrdiff_t errfunc;  /* current error handling function (stack index) */
};


#define G(L)	(L->l_G)


/*
** Union of all collectable objects
*/
union GCObject {
  GCheader gch;
  union TString ts;
  union Udata u;
  union Closure cl;
  struct Table h;
  struct Proto p;
  struct UpVal uv;
  struct lua_State th;  /* thread */
};


/* macros to convert a GCObject into a specific value */
// GCObjectת tstring
#define rawgco2ts(o)	check_exp((o)->gch.tt == LUA_TSTRING, &((o)->ts))
#define gco2ts(o)	(&rawgco2ts(o)->tsv)
// GCObject תUserData
#define rawgco2u(o)	check_exp((o)->gch.tt == LUA_TUSERDATA, &((o)->u))
#define gco2u(o)	(&rawgco2u(o)->uv)
// GCObject ת Function
#define gco2cl(o)	check_exp((o)->gch.tt == LUA_TFUNCTION, &((o)->cl))
// GCObjectתTable
#define gco2h(o)	check_exp((o)->gch.tt == LUA_TTABLE, &((o)->h))
// GCObjectתProto
#define gco2p(o)	check_exp((o)->gch.tt == LUA_TPROTO, &((o)->p))
//GCObjectת UpVal
#define gco2uv(o)	check_exp((o)->gch.tt == LUA_TUPVAL, &((o)->uv))

#define ngcotouv(o) \
	check_exp((o) == NULL || (o)->gch.tt == LUA_TUPVAL, &((o)->uv))

// GCObjectתThread
#define gco2th(o)	check_exp((o)->gch.tt == LUA_TTHREAD, &((o)->th))

/* macro to convert any Lua object into a GCObject */
// Lua ObjectתGCObject
#define obj2gco(v)	(cast(GCObject *, (v)))


LUAI_FUNC lua_State *luaE_newthread (lua_State *L);
LUAI_FUNC void luaE_freethread (lua_State *L, lua_State *L1);

#endif

