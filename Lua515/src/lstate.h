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
  StkId base;  /* base for this function */		//闭包调用的栈基
  StkId func;  /* function index in the stack */		// 要调用的闭包在栈上的位置
  StkId	top;  /* top for this function */		// 闭包的栈使用限制, 就是lua_push*的时候得看着点, push太多就超了, 可以lua_checkstack来扩
  const Instruction *savedpc;		// 如果在本闭包中再次调用别的闭包, 那么该值就保存下一条指令以便在返回时继续执行
  int nresults;  /* expected number of results from this function */	// 闭包要返回的值个数
  int tailcalls;  /* number of tail calls lost under this entry */	// 尾递归用
} CallInfo;



#define curr_func(L)	(clvalue(L->ci->func))
#define ci_func(ci)	(clvalue((ci)->func))
#define f_isLua(ci)	(!ci_func(ci)->c.isC)
#define isLua(ci)	(ttisfunction((ci)->func) && f_isLua(ci))


/*
** `global state', shared by all threads of this state
*/
typedef struct global_State {
  stringtable strt;  /* hash table for strings */		// 字符串表
  lua_Alloc frealloc;  /* function to reallocate memory */
  void *ud;         /* auxiliary data to `frealloc' */
  lu_byte currentwhite;				// 当前的白色是 0 型还是 1 型
  lu_byte gcstate;  /* state of garbage collector */	// 垃圾回收的状态
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
  struct lua_State *mainthread;				// udata List	 // 用来管理userdata
  UpVal uvhead;  /* head of double-linked list of all open upvalues */  // 整个lua虚拟机里面所有栈的upvalue链表的头
  struct Table *mt[NUM_TAGS];  /* metatables for basic types */
  TString *tmname[TM_N];  /* array with tag-method names */	// 原方法名字数组
} global_State;


/*
** `per thread' state
*/
struct lua_State {
  CommonHeader;
  lu_byte status;
  StkId top;  /* first free slot in the stack */		// 表示在这个栈上的第一个空闲的slot 
  StkId base;  /* base of current function */		//表示当前所在函数的base。这个base可以说就是栈底。只不过是当前函数的。 
  global_State *l_G;
  CallInfo *ci;  /* call info for current function */			// 当前被调用的闭包的调用信息
  const Instruction *savedpc;  /* `savedpc' of current function */
  StkId stack_last;  /* last free slot in the stack */	// 在栈上的最后一个空闲的slot 
  StkId stack;  /* stack base */								// 栈的base，这个是整个栈的栈底
  CallInfo *end_ci;  /* points after end of ci array*/	// 指向CallInfo数组最一个元素
  CallInfo *base_ci;  /* array of CallInfo's */			// 指向CallInfo数组第一个元素
  int stacksize;											// 栈的大小
  int size_ci;  /* size of array `base_ci' */					// 记录CallInfo数组当前的大小
  unsigned short nCcalls;  /* number of nested C calls */
  unsigned short baseCcalls;  /* nested C calls when resuming coroutine */
  lu_byte hookmask;
  lu_byte allowhook;
  int basehookcount;
  int hookcount;
  lua_Hook hook;

  // 些全局(这个状态)用到的东西
  TValue l_gt;  /* table of globals */
  TValue env;  /* temporary place for environments */

  // gc相关
  GCObject *openupval;  /* list of open upvalues in this stack */	// 这个域也就是当前的栈上的所有打开的upval
  GCObject *gclist;

  // 错误处理
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
// GCObject转 tstring
#define rawgco2ts(o)	check_exp((o)->gch.tt == LUA_TSTRING, &((o)->ts))
#define gco2ts(o)	(&rawgco2ts(o)->tsv)
// GCObject 转UserData
#define rawgco2u(o)	check_exp((o)->gch.tt == LUA_TUSERDATA, &((o)->u))
#define gco2u(o)	(&rawgco2u(o)->uv)
// GCObject 转 Function
#define gco2cl(o)	check_exp((o)->gch.tt == LUA_TFUNCTION, &((o)->cl))
// GCObject转Table
#define gco2h(o)	check_exp((o)->gch.tt == LUA_TTABLE, &((o)->h))
// GCObject转Proto
#define gco2p(o)	check_exp((o)->gch.tt == LUA_TPROTO, &((o)->p))
//GCObject转 UpVal
#define gco2uv(o)	check_exp((o)->gch.tt == LUA_TUPVAL, &((o)->uv))

#define ngcotouv(o) \
	check_exp((o) == NULL || (o)->gch.tt == LUA_TUPVAL, &((o)->uv))

// GCObject转Thread
#define gco2th(o)	check_exp((o)->gch.tt == LUA_TTHREAD, &((o)->th))

/* macro to convert any Lua object into a GCObject */
// Lua Object转GCObject
#define obj2gco(v)	(cast(GCObject *, (v)))


LUAI_FUNC lua_State *luaE_newthread (lua_State *L);
LUAI_FUNC void luaE_freethread (lua_State *L, lua_State *L1);

#endif

