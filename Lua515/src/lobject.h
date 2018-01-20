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
  GCObject *gc;		// ��Ҫ�������յ�ֵ
  void *p;				// light userdata //���ᱻ��������
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
// �����ж�
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
// TValueתGCObject
#define gcvalue(o)	check_exp(iscollectable(o), (o)->value.gc)
// TValueȡp
#define pvalue(o)	check_exp(ttislightuserdata(o), (o)->value.p)
// TValueȡn
#define nvalue(o)	check_exp(ttisnumber(o), (o)->value.n)
// TValueתTString
#define rawtsvalue(o)	check_exp(ttisstring(o), &(o)->value.gc->ts)
#define tsvalue(o)	(&rawtsvalue(o)->tsv)
// TValueתUdata
#define rawuvalue(o)	check_exp(ttisuserdata(o), &(o)->value.gc->u)
#define uvalue(o)	(&rawuvalue(o)->uv)
// TValueת�հ�
#define clvalue(o)	check_exp(ttisfunction(o), &(o)->value.gc->cl)
// TValueת��
#define hvalue(o)	check_exp(ttistable(o), &(o)->value.gc->h)
// TValueתbool
#define bvalue(o)	check_exp(ttisboolean(o), (o)->value.b)
// TValueתThread
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
// ����ΪNILֵ
#define setnilvalue(obj) ((obj)->tt=LUA_TNIL)

// ����ΪNumber����ֵ
#define setnvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.n=(x); i_o->tt=LUA_TNUMBER; }

// ����ΪLightUserData����ֵ
#define setpvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.p=(x); i_o->tt=LUA_TLIGHTUSERDATA; }

// ����ΪBoolean����ֵ
#define setbvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.b=(x); i_o->tt=LUA_TBOOLEAN; }

// ����Ϊ�ַ�������ֵ
#define setsvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TSTRING; \
    checkliveness(G(L),i_o); }

// ����Ϊuserdata����ֵ
#define setuvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TUSERDATA; \
    checkliveness(G(L),i_o); }

// ����Ϊ�߳�����ֵ
#define setthvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TTHREAD; \
    checkliveness(G(L),i_o); }

// ����Ϊ��������ֵ
#define setclvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TFUNCTION; \
    checkliveness(G(L),i_o); }

// ����Ϊ������ֵ
#define sethvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TTABLE; \
    checkliveness(G(L),i_o); }

// ����ΪProto����ֵ
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


// �ɻ��ձ�
#define iscollectable(o)	(ttype(o) >= LUA_TSTRING)



typedef TValue *StkId;  /* index to stack elements */


/*
** String headers for string table
*/
// �ַ���ʵ�ʴ���Щ�ṹ֮��
typedef union TString {
  L_Umaxalign dummy;  /* ensures maximum alignment for strings */
  struct {
    CommonHeader;
    lu_byte reserved;			// Ԥ����
    unsigned int hash;		// hashֵ
    size_t len;						// �ַ�������
  } tsv;
} TString;


// ��ȡʵ���ַ������ڵ�ַ
#define getstr(ts)	cast(const char *, (ts) + 1)
// ͨ��TValue��ȡ�ַ������ڵ�ַ
#define svalue(o)       getstr(rawtsvalue(o))



// UData���ݿ�ṹ
// ����һ��userdata����ʵҲ���Ƿ���һ���ڴ������Udata�ĺ���
typedef union Udata {
  L_Umaxalign dummy;  /* ensures maximum alignment for `local' udata */
  struct {
    CommonHeader;
    struct Table *metatable;	// ԭ��
    struct Table *env;				// ������
    size_t len;							// ���ṹ����ռ��С
  } uv;
} Udata;




/*
** Function Prototypes		// ����ԭ��
*/
typedef struct Proto {
  CommonHeader;
  TValue *k;  /* constants used by the function */		// ������ʹ�õĳ���
  Instruction *code;		// ָ������
  struct Proto **p;  /* functions defined inside the function */	// ָ�����Proto�ڲ���Proto�б�
  int *lineinfo;  /* map from opcodes to source lines */	// ÿ��ָ���Ӧ����
  struct LocVar *locvars;  /* information about local variables */	// �洢local������Ϣ
  TString **upvalues;  /* upvalue names */		// �洢upvalue����Ϣ
  TString  *source;	// ָ�����Proto�������ļ���
  int sizeupvalues;		// upvalue�Ĵ�С
  int sizek;  /* size of `k' */	// ���������С
  int sizecode;				// ָ�������С
  int sizelineinfo;			// ����Ϣ
  int sizep;  /* size of `p' */	 // �ڲ�Proto�б��С
  int sizelocvars;					// local���������С
  int linedefined;
  int lastlinedefined;
  GCObject *gclist;
  lu_byte nups;  /* number of upvalues */	// upvalues����
  lu_byte numparams;			// �̶��Ĳ����ĸ���
  lu_byte is_vararg;					// ��ʾ���Proto�Ƿ���һ����κ���
  lu_byte maxstacksize;			// ��ʾ���stack��С
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
  TValue *v;  /* points to stack or to its own value */		// ָ��u.l.valueʱΪ�ر�,ָ��ջʱΪ��
  union {
    TValue value;  /* the value (when closed) */
    struct {  /* double linked list (when open) */
      struct UpVal *prev;
      struct UpVal *next;
    } l;
  } u;
} UpVal;


/*
** Closures -- �հ�
*/

// isCΪ1��ʾCClosure 0ΪLClosure
// upvalue- upval�ĸ���,���������
// gclist - ���ӵ�ȫ�ֵ�gc����
// env -���������Կ�������һ��table���͵ģ������汣����һЩȫ�ֱ�����
#define ClosureHeader \
	CommonHeader; lu_byte isC; lu_byte nupvalues; GCObject *gclist; \
	struct Table *env

// C�����հ�--CClosure��ʾ��c����,Ҳ���Ǻ�lua�ⲿ�������ݽ�����c�����Լ��ڲ���ʹ�õ�c����.
typedef struct CClosure {
  ClosureHeader;
  lua_CFunction f;		// ��ʾ��Ҫִ�е�c������ԭ��
  TValue upvalue[1];	// ������������Ҫ��һЩ����(����string ��match����,������Ҫ�ļ����������ᱣ����upvalue���� 
} CClosure;


// LClosure��ʾlua�ĺ���,��Щ��������lua��������й���
typedef struct LClosure {
  ClosureHeader;
  struct Proto *p;		// ���ָ������˺ܶ�����ԣ��������������Ƕ�׺����ȵ�
  UpVal *upvals[1];		// ������鱣����ָ���ⲿ�ı���Ҳ�������Ǳհ�����Ҫ�ľֲ�����
} LClosure;


// �հ�
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
  TValue i_val;	// ֵ
  TKey i_key;	// ��
} Node;


typedef struct Table {
  CommonHeader;
  lu_byte flags;  /* 1<<p means tagmethod(p) is not present */ 
  lu_byte lsizenode;  /* log2 of size of `node' array */	 // Node�����С��log2���ֵ
  struct Table *metatable;						// Ԫ��
  TValue *array;  /* array part */				// ����
  Node *node;										// hash��,Node����
  Node *lastfree;  /* any free position is before this position */		// Node�������һ��Ԫ��֮���λ�ã��൱��iterEnd
  GCObject *gclist;
  int sizearray;  /* size of `array' array */			// �����С
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

