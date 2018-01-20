/*
** $Id: lobject.h,v 2.20.1.2 2008/08/06 13:29:48 roberto Exp $
** Type definitions for Lua objects
** See Copyright Notice in lua.h
*/


#ifndef lobject_h
#define lobject_h


#include <stdarg.h>


#include "llimits.h"
#include "lua.h"


/* tags for values visible from Lua */
#define LAST_TAG	LUA_TTHREAD

#define NUM_TAGS	(LAST_TAG+1)


/*
** Extra tags for non-values
*/
#define LUA_TPROTO	(LAST_TAG+1)
#define LUA_TUPVAL	(LAST_TAG+2)
#define LUA_TDEADKEY	(LAST_TAG+3)


/*
** Union of all collectable objects
*/
typedef union GCObject GCObject;


/*
** Common Header for all collectable objects (in macro form, to be
** included in other objects)
*/
#define CommonHeader	GCObject *next; lu_byte tt; lu_byte marked


/*
** Common header in struct form
*/
typedef struct GCheader {
  CommonHeader;
} GCheader;




/*
** Union of all Lua values
*/
typedef union {
  GCObject *gc;		// 需要垃圾回收的值
  void *p;				// light userdata //不会被垃圾回收
  lua_Number n;	// double
  int b;					// bool
} Value;


/*
** Tagged Values
*/

#define TValuefields	Value value; int tt

typedef struct lua_TValue {
  TValuefields;
} TValue;


/* Macros to test type */
// 类型判断
#define ttisnil(o)	(ttype(o) == LUA_TNIL)
#define ttisnumber(o)	(ttype(o) == LUA_TNUMBER)
#define ttisstring(o)	(ttype(o) == LUA_TSTRING)
#define ttistable(o)	(ttype(o) == LUA_TTABLE)
#define ttisfunction(o)	(ttype(o) == LUA_TFUNCTION)
#define ttisboolean(o)	(ttype(o) == LUA_TBOOLEAN)
#define ttisuserdata(o)	(ttype(o) == LUA_TUSERDATA)
#define ttisthread(o)	(ttype(o) == LUA_TTHREAD)
#define ttislightuserdata(o)	(ttype(o) == LUA_TLIGHTUSERDATA)

/* Macros to access values */
#define ttype(o)	((o)->tt)
// TValue转GCObject
#define gcvalue(o)	check_exp(iscollectable(o), (o)->value.gc)
// TValue取p
#define pvalue(o)	check_exp(ttislightuserdata(o), (o)->value.p)
// TValue取n
#define nvalue(o)	check_exp(ttisnumber(o), (o)->value.n)
// TValue转TString
#define rawtsvalue(o)	check_exp(ttisstring(o), &(o)->value.gc->ts)
#define tsvalue(o)	(&rawtsvalue(o)->tsv)
// TValue转Udata
#define rawuvalue(o)	check_exp(ttisuserdata(o), &(o)->value.gc->u)
#define uvalue(o)	(&rawuvalue(o)->uv)
// TValue转闭包
#define clvalue(o)	check_exp(ttisfunction(o), &(o)->value.gc->cl)
// TValue转表
#define hvalue(o)	check_exp(ttistable(o), &(o)->value.gc->h)
// TValue转bool
#define bvalue(o)	check_exp(ttisboolean(o), (o)->value.b)
// TValue转Thread
#define thvalue(o)	check_exp(ttisthread(o), &(o)->value.gc->th)

#define l_isfalse(o)	(ttisnil(o) || (ttisboolean(o) && bvalue(o) == 0))

/*
** for internal debug only
*/
#define checkconsistency(obj) \
  lua_assert(!iscollectable(obj) || (ttype(obj) == (obj)->value.gc->gch.tt))

#define checkliveness(g,obj) \
  lua_assert(!iscollectable(obj) || \
  ((ttype(obj) == (obj)->value.gc->gch.tt) && !isdead(g, (obj)->value.gc)))


/* Macros to set values */
// 设置为NIL值
#define setnilvalue(obj) ((obj)->tt=LUA_TNIL)

// 设置为Number类型值
#define setnvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.n=(x); i_o->tt=LUA_TNUMBER; }

// 设置为LightUserData类型值
#define setpvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.p=(x); i_o->tt=LUA_TLIGHTUSERDATA; }

// 设置为Boolean类型值
#define setbvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.b=(x); i_o->tt=LUA_TBOOLEAN; }

// 设置为字符串类型值
#define setsvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TSTRING; \
    checkliveness(G(L),i_o); }

// 设置为userdata类型值
#define setuvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TUSERDATA; \
    checkliveness(G(L),i_o); }

// 设置为线程类型值
#define setthvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TTHREAD; \
    checkliveness(G(L),i_o); }

// 设置为函数类型值
#define setclvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TFUNCTION; \
    checkliveness(G(L),i_o); }

// 设置为表类型值
#define sethvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TTABLE; \
    checkliveness(G(L),i_o); }

// 设置为Proto类型值
#define setptvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TPROTO; \
    checkliveness(G(L),i_o); }




#define setobj(L,obj1,obj2) \
  { const TValue *o2=(obj2); TValue *o1=(obj1); \
    o1->value = o2->value; o1->tt=o2->tt; \
    checkliveness(G(L),o1); }


/*
** different types of sets, according to destination
*/

/* from stack to (same) stack */
#define setobjs2s	setobj
/* to stack (not from same stack) */
#define setobj2s	setobj
#define setsvalue2s	setsvalue
#define sethvalue2s	sethvalue
#define setptvalue2s	setptvalue
/* from table to same table */
#define setobjt2t	setobj
/* to table */
#define setobj2t	setobj
/* to new object */
#define setobj2n	setobj
#define setsvalue2n	setsvalue

#define setttype(obj, tt) (ttype(obj) = (tt))


// 可回收表
#define iscollectable(o)	(ttype(o) >= LUA_TSTRING)



typedef TValue *StkId;  /* index to stack elements */


/*
** String headers for string table
*/
// 字符串实际存在些结构之后
typedef union TString {
  L_Umaxalign dummy;  /* ensures maximum alignment for strings */
  struct {
    CommonHeader;
    lu_byte reserved;			// 预留字
    unsigned int hash;		// hash值
    size_t len;						// 字符串长度
  } tsv;
} TString;


// 获取实际字符串所在地址
#define getstr(ts)	cast(const char *, (ts) + 1)
// 通过TValue获取字符串所在地址
#define svalue(o)       getstr(rawtsvalue(o))



// UData数据库结构
// 创建一个userdata，其实也就是分配一块内存紧跟在Udata的后面
typedef union Udata {
  L_Umaxalign dummy;  /* ensures maximum alignment for `local' udata */
  struct {
    CommonHeader;
    struct Table *metatable;	// 原表
    struct Table *env;				// 环境表
    size_t len;							// 本结构后面空间大小
  } uv;
} Udata;




/*
** Function Prototypes		// 函数原型
*/
typedef struct Proto {
  CommonHeader;
  TValue *k;  /* constants used by the function */		// 给函数使用的常量
  Instruction *code;		// 指令数组
  struct Proto **p;  /* functions defined inside the function */	// 指向这个Proto内部的Proto列表
  int *lineinfo;  /* map from opcodes to source lines */	// 每条指令对应的行
  struct LocVar *locvars;  /* information about local variables */	// 存储local变量信息
  TString **upvalues;  /* upvalue names */		// 存储upvalue的信息
  TString  *source;	// 指向这个Proto所属的文件名
  int sizeupvalues;		// upvalue的大小
  int sizek;  /* size of `k' */	// 常量数组大小
  int sizecode;				// 指令数组大小
  int sizelineinfo;			// 行信息
  int sizep;  /* size of `p' */	 // 内部Proto列表大小
  int sizelocvars;					// local变量数组大小
  int linedefined;
  int lastlinedefined;
  GCObject *gclist;
  lu_byte nups;  /* number of upvalues */	// upvalues数量
  lu_byte numparams;			// 固定的参数的个数
  lu_byte is_vararg;					// 表示这个Proto是否是一个变参函数
  lu_byte maxstacksize;			// 表示最大stack大小
} Proto;


/* masks for new-style vararg */
#define VARARG_HASARG		1
#define VARARG_ISVARARG		2
#define VARARG_NEEDSARG		4


typedef struct LocVar {
  TString *varname;
  int startpc;  /* first point where variable is active */
  int endpc;    /* first point where variable is dead */
} LocVar;



/*
** Upvalues
*/

typedef struct UpVal {
  CommonHeader;
  TValue *v;  /* points to stack or to its own value */		// 指向u.l.value时为关闭,指向栈时为打开
  union {
    TValue value;  /* the value (when closed) */
    struct {  /* double linked list (when open) */
      struct UpVal *prev;
      struct UpVal *next;
    } l;
  } u;
} UpVal;


/*
** Closures -- 闭包
*/

// isC为1表示CClosure 0为LClosure
// upvalue- upval的个数,或参数个数
// gclist - 链接到全局的gc链表
// env -环境，可以看到它是一个table类型的，他里面保存了一些全局变量等
#define ClosureHeader \
	CommonHeader; lu_byte isC; lu_byte nupvalues; GCObject *gclist; \
	struct Table *env

// C函数闭包--CClosure表示是c函数,也就是和lua外部交互传递进来的c函数以及内部所使用的c函数.
typedef struct CClosure {
  ClosureHeader;
  lua_CFunction f;		// 表示所要执行的c函数的原型
  TValue upvalue[1];	// 函数运行所需要的一些参数(比如string 的match函数,它所需要的几个参数都会保存在upvalue里面 
} CClosure;


// LClosure表示lua的函数,这些函数是由lua虚拟机进行管理
typedef struct LClosure {
  ClosureHeader;
  struct Proto *p;		// 这个指针包含了很多的属性，比如变量，比如嵌套函数等等
  UpVal *upvals[1];		// 这个数组保存了指向外部的变量也就是我们闭包所需要的局部变量
} LClosure;


// 闭包
typedef union Closure {
  CClosure c;
  LClosure l;
} Closure;


#define iscfunction(o)	(ttype(o) == LUA_TFUNCTION && clvalue(o)->c.isC)
#define isLfunction(o)	(ttype(o) == LUA_TFUNCTION && !clvalue(o)->c.isC)


/*
** Tables
*/

typedef union TKey {
  struct {
    TValuefields;
    struct Node *next;  /* for chaining */
  } nk;
  TValue tvk;
} TKey;


typedef struct Node {
  TValue i_val;	// 值
  TKey i_key;	// 键
} Node;


typedef struct Table {
  CommonHeader;
  lu_byte flags;  /* 1<<p means tagmethod(p) is not present */ 
  lu_byte lsizenode;  /* log2 of size of `node' array */	 // Node数组大小的log2结果值
  struct Table *metatable;						// 元表
  TValue *array;  /* array part */				// 数组
  Node *node;										// hash表,Node数组
  Node *lastfree;  /* any free position is before this position */		// Node数组最后一个元素之后的位置，相当于iterEnd
  GCObject *gclist;
  int sizearray;  /* size of `array' array */			// 数组大小
} Table;



/*
** `module' operation for hashing (size is always a power of 2)
*/
#define lmod(s,size) \
	(check_exp((size&(size-1))==0, (cast(int, (s) & ((size)-1)))))


#define twoto(x)	(1<<(x))
#define sizenode(t)	(twoto((t)->lsizenode))


#define luaO_nilobject		(&luaO_nilobject_)

LUAI_DATA const TValue luaO_nilobject_;

#define ceillog2(x)	(luaO_log2((x)-1) + 1)

LUAI_FUNC int luaO_log2 (unsigned int x);
LUAI_FUNC int luaO_int2fb (unsigned int x);
LUAI_FUNC int luaO_fb2int (int x);
LUAI_FUNC int luaO_rawequalObj (const TValue *t1, const TValue *t2);
LUAI_FUNC int luaO_str2d (const char *s, lua_Number *result);
LUAI_FUNC const char *luaO_pushvfstring (lua_State *L, const char *fmt,
                                                       va_list argp);
LUAI_FUNC const char *luaO_pushfstring (lua_State *L, const char *fmt, ...);
LUAI_FUNC void luaO_chunkid (char *out, const char *source, size_t len);


#endif

