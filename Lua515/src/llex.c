/*
** $Id: llex.c,v 2.20.1.2 2009/11/23 14:58:22 roberto Exp $
** Lexical Analyzer
** See Copyright Notice in lua.h
*/


#include <ctype.h>
#include <locale.h>
#include <string.h>

#define llex_c
#define LUA_CORE

#include "lua.h"

#include "ldo.h"
#include "llex.h"
#include "lobject.h"
#include "lparser.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "lzio.h"


// 读下一个字符
#define next(ls) (ls->current = zgetc(ls->z))



#define currIsNewline(ls)	(ls->current == '\n' || ls->current == '\r')


// lua 命令关键字
/* ORDER RESERVED */
const char *const luaX_tokens [] = {
    "and", "break", "do", "else", "elseif",
    "end", "false", "for", "function", "if",
    "in", "local", "nil", "not", "or", "repeat",
    "return", "then", "true", "until", "while",
    "..", "...", "==", ">=", "<=", "~=",
    "<number>", "<name>", "<string>", "<eof>",
    NULL
};


// 保存当前字符并读下一个字符
#define save_and_next(ls) (save(ls, ls->current), next(ls))

// 把字符c保存在ls->buff中
static void save (LexState *ls, int c) {
  Mbuffer *b = ls->buff;
  if (b->n + 1 > b->buffsize) {
    size_t newsize;
    if (b->buffsize >= MAX_SIZET/2)
      luaX_lexerror(ls, "lexical element too long", 0);
    newsize = b->buffsize * 2;
    luaZ_resizebuffer(ls->L, b, newsize);
  }
  b->buffer[b->n++] = cast(char, c);
}


// 关键字名初始化
void luaX_init (lua_State *L) {
  int i;
  for (i=0; i<NUM_RESERVED; i++) {
    TString *ts = luaS_new(L, luaX_tokens[i]);
    luaS_fix(ts);  /* reserved words are never collected */
    lua_assert(strlen(luaX_tokens[i])+1 <= TOKEN_LEN);
    ts->tsv.reserved = cast_byte(i+1);  /* reserved word */
  }
}


#define MAXSRC          80

// toke转成字符串
const char *luaX_token2str (LexState *ls, int token) {
  if (token < FIRST_RESERVED) { // 字符本身
    lua_assert(token == cast(unsigned char, token));
    return (iscntrl(token)) ? luaO_pushfstring(ls->L, "char(%d)", token) :
                              luaO_pushfstring(ls->L, "%c", token);
  }
  else
    return luaX_tokens[token-FIRST_RESERVED];		// 关键字
}

// token返回对应字符串
static const char *txtToken (LexState *ls, int token) {
  switch (token) {
    case TK_NAME:
    case TK_STRING:
    case TK_NUMBER:
      save(ls, '\0');
      return luaZ_buffer(ls->buff);
    default:
      return luaX_token2str(ls, token);
  }
}


void luaX_lexerror (LexState *ls, const char *msg, int token) {
  char buff[MAXSRC];
  luaO_chunkid(buff, getstr(ls->source), MAXSRC);
  msg = luaO_pushfstring(ls->L, "%s:%d: %s", buff, ls->linenumber, msg);
  if (token)
    luaO_pushfstring(ls->L, "%s near " LUA_QS, msg, txtToken(ls, token));
  luaD_throw(ls->L, LUA_ERRSYNTAX);
}


void luaX_syntaxerror (LexState *ls, const char *msg) {
  luaX_lexerror(ls, msg, ls->t.token);
}

// 
TString *luaX_newstring (LexState *ls, const char *str, size_t l) {
  lua_State *L = ls->L;
  TString *ts = luaS_newlstr(L, str, l);
  TValue *o = luaH_setstr(L, ls->fs->h, ts);  /* entry for `str' */
  if (ttisnil(o)) {
    setbvalue(o, 1);  /* make sure `str' will not be collected */
    luaC_checkGC(L);
  }
  return ts;
}

// 增加行号
static void inclinenumber (LexState *ls) {
  int old = ls->current;
  lua_assert(currIsNewline(ls));
  next(ls);  /* skip `\n' or `\r' */	// 跳过'\n' '\r'
  if (currIsNewline(ls) && ls->current != old)
    next(ls);  /* skip `\n\r' or `\r\n' */// 跳过'\n\r'或 '\r\n'
  if (++ls->linenumber >= MAX_INT) // 增加行号
    luaX_syntaxerror(ls, "chunk has too many lines");
}

// 设置输入流
void luaX_setinput (lua_State *L, LexState *ls, ZIO *z, TString *source) {
  ls->decpoint = '.';
  ls->L = L;
  ls->lookahead.token = TK_EOS;  /* no look-ahead token */
  ls->z = z;
  ls->fs = NULL;
  ls->linenumber = 1;
  ls->lastline = 1;
  ls->source = source;
  luaZ_resizebuffer(ls->L, ls->buff, LUA_MINBUFFER);  /* initialize buffer */
  next(ls);  /* read first char */ // 读第一个字符
}



/*
** =======================================================
** LEXICAL ANALYZER		// 词法分析
** =======================================================
*/



static int check_next (LexState *ls, const char *set) {
  if (!strchr(set, ls->current))	// 查找字符串set中首次出现字符current的位置
    return 0; // 不存在
  save_and_next(ls);		// 存入buffer中
  return 1;
}


// 字符替换
static void buffreplace (LexState *ls, char from, char to) {
  size_t n = luaZ_bufflen(ls->buff);
  char *p = luaZ_buffer(ls->buff);
  while (n--)
    if (p[n] == from) p[n] = to;
}

// ls->buff 字符串转成LuaNumber
static void trydecpoint (LexState *ls, SemInfo *seminfo) {
  /* format error: try to update decimal point separator */
  struct lconv *cv = localeconv(); // 返回包含本地数字及货币信息格式的数组
  char old = ls->decpoint;
  ls->decpoint = (cv ? cv->decimal_point[0] : '.'); // 小数点字符
  buffreplace(ls, old, ls->decpoint);  /* try updated decimal separator */
  if (!luaO_str2d(luaZ_buffer(ls->buff), &seminfo->r)) {
    /* format error with correct decimal point: no more options */
    buffreplace(ls, ls->decpoint, '.');  /* undo change (for error message) */
    luaX_lexerror(ls, "malformed number", TK_NUMBER);
  }
}


/* LUA_NUMBER */
// 读一个数(整数、小数、浮点数)
static void read_numeral (LexState *ls, SemInfo *seminfo) {
  lua_assert(isdigit(ls->current));
  do {
    save_and_next(ls);
  } while (isdigit(ls->current) || ls->current == '.');
  if (check_next(ls, "Ee"))  /* `E'? */
    check_next(ls, "+-");  /* optional exponent sign */
  while (isalnum(ls->current) || ls->current == '_')
    save_and_next(ls);
  save(ls, '\0');
  buffreplace(ls, '.', ls->decpoint);  /* follow locale for decimal point */
  if (!luaO_str2d(luaZ_buffer(ls->buff), &seminfo->r))  /* format error? */
    trydecpoint(ls, seminfo); /* try to update decimal point separator */
}

// 跳过'['或']'中间的'='
static int skip_sep (LexState *ls) {
  int count = 0;
  int s = ls->current;
  lua_assert(s == '[' || s == ']');
  save_and_next(ls);
  while (ls->current == '=') {
    save_and_next(ls);
    count++;
  }
  return (ls->current == s) ? count : (-count) - 1;
}

// 读"[ ]"中的长字符串 
static void read_long_string (LexState *ls, SemInfo *seminfo, int sep) {
  int cont = 0;
  (void)(cont);  /* avoid warnings when `cont' is not used */
  save_and_next(ls);  /* skip 2nd `[' */ // 跳过第二个'['
  if (currIsNewline(ls))  /* string starts with a newline? */ // 是否要开始新行
    inclinenumber(ls);  /* skip it */	// 增加行号
  for (;;) {
    switch (ls->current) {
      case EOZ:
        luaX_lexerror(ls, (seminfo) ? "unfinished long string" :
                                   "unfinished long comment", TK_EOS);
        break;  /* to avoid warnings */
#if defined(LUA_COMPAT_LSTR)
      case '[': {
        if (skip_sep(ls) == sep) {
          save_and_next(ls);  /* skip 2nd `[' */
          cont++;
#if LUA_COMPAT_LSTR == 1
          if (sep == 0)
            luaX_lexerror(ls, "nesting of [[...]] is deprecated", '[');
#endif
        }
        break;
      }
#endif
      case ']': {
        if (skip_sep(ls) == sep) {
          save_and_next(ls);  /* skip 2nd `]' */	 //跳过第二个']'
#if defined(LUA_COMPAT_LSTR) && LUA_COMPAT_LSTR == 2
          cont--;
          if (sep == 0 && cont >= 0) break;
#endif
          goto endloop;
        }
        break;
      }
      case '\n':
      case '\r': {
        save(ls, '\n');
        inclinenumber(ls);
        if (!seminfo) luaZ_resetbuffer(ls->buff);  /* avoid wasting space */
        break;
      }
      default: {
        if (seminfo) save_and_next(ls);
        else next(ls);
      }
    }
  } endloop:
  if (seminfo)
    seminfo->ts = luaX_newstring(ls, luaZ_buffer(ls->buff) + (2 + sep),
                                     luaZ_bufflen(ls->buff) - 2*(2 + sep));
}

// 读一个字符串; ' ' 或 " "内的数据
static void read_string (LexState *ls, int del, SemInfo *seminfo) {
  save_and_next(ls);
  while (ls->current != del) {
    switch (ls->current) {
      case EOZ:
        luaX_lexerror(ls, "unfinished string", TK_EOS);
        continue;  /* to avoid warnings */
      case '\n':
      case '\r':
        luaX_lexerror(ls, "unfinished string", TK_STRING);
        continue;  /* to avoid warnings */
      case '\\': { // 转义字符
        int c;
        next(ls);  /* do not save the `\' */
        switch (ls->current) {
          case 'a': c = '\a'; break;
          case 'b': c = '\b'; break;
          case 'f': c = '\f'; break;
          case 'n': c = '\n'; break;
          case 'r': c = '\r'; break;
          case 't': c = '\t'; break;
          case 'v': c = '\v'; break;
          case '\n':  /* go through */
          case '\r': save(ls, '\n'); inclinenumber(ls); continue;
          case EOZ: continue;  /* will raise an error next loop */
          default: {
            if (!isdigit(ls->current))
              save_and_next(ls);  /* handles \\, \", \', and \? */	
			else {  /* \xxx */
					//a = 'alo\n123"'
					//a = "alo\n123\""
					//a = '\97lo\10\04923"'
              int i = 0;
              c = 0;
              do {
                c = 10*c + (ls->current-'0');
                next(ls);
              } while (++i<3 && isdigit(ls->current));
              if (c > UCHAR_MAX)
                luaX_lexerror(ls, "escape sequence too large", TK_STRING);
              save(ls, c);
            }
            continue;
          }
        }
        save(ls, c);
        next(ls);
        continue;
      }
      default:
        save_and_next(ls);
    }
  }
  save_and_next(ls);  /* skip delimiter */
  seminfo->ts = luaX_newstring(ls, luaZ_buffer(ls->buff) + 1,
                                   luaZ_bufflen(ls->buff) - 2);
}

// 词法分析程序
/*
在词法分析的过程中，lua会处理以下几种情况：
<1> ‘/r’， ‘/n’　这两个东西的任意组合，是换行，lua直接跳过，其中是用inclinenumber(ls)这个函数跳过的。
<2> -- 或 –--[ ] 注释，lua也会跳过，不过，lua是通过把注释当作字符串读取后跳过的。
<3>[==[  ]==]，中间是等量=的字符串，lua会读取字符串，通过read_long_string()。并返回：TK_STRING。
<4> “ ”，‘ ’　用引号引起来的字符串，lua会通过read_string()读取字符串。并返回：TK_STRING。
<5> ==，等于号，这是lua的保留符号，返回：TK_EQ。
<6> <，小于号，返回：TK_LT。
<7> >，大于号，返回：TK_GT。
<8> ~，返回~，如果下一个字符是=，如下：
<9> ~=，不等于号，返回：TK_NE。
<10> . , 返回：’.’。
<11> .. , 返回：TK_DOTS。
<12> … , 返回：TK_CONCAT。
<13> .(0~9) , 返回：TK_NUMBER。
<14> EOZ , 返回：TK_EOS。
<15> 如果是数字开头，则lua会读取数字，用函数read_numberal()，返回：TK_NUMBER。
<16> 如果是字母开头或_开头，则为变量、函数名或保留字，lua会区别开来，分别返回保留字的标志或TK_NAME。
<17> 如果是：+ - * /等，lua会返回其ASCII码。
注：
1、  如果遇到[=，lua会假设这是一个长字符串，并检查其后面的=的数目，并且如果=后面没有紧跟着[，则报错。
2、 有时候数字是小数，直接以.开始，如：.31415926。lua会把这个当作一个数字，也就是TK_NUMBER类型。
3、 因为在不同国家，小数点可能不同，lua会检查到这种情况，并作处理。
4、 以下划线_开头的文字，lua会当作保留字处理。
5、 当lua读取一个字符串的时候，会新建一个TString类型的数据，并存在G(L)中，以便进行垃圾回收。
6、 保留字是lua预先建好的TString类型的字符串，在lua初始化的时候存在了G(L)中。
7、 在lua中，字符串是常量，所以，当在程序中用到相同的字符串的时候，lua会让他们指向同一个G(L)中的字符串，以便节省内存
*/
static int llex (LexState *ls, SemInfo *seminfo) {
  luaZ_resetbuffer(ls->buff);
  for (;;) {
    switch (ls->current) {
      case '\n':
      case '\r': {
		  // 增加行号
        inclinenumber(ls);
        continue;
      }
      case '-': {
        next(ls);
        if (ls->current != '-') return '-';
        /* else is a comment */ // 接下来是注释
        next(ls);
        if (ls->current == '[') {
          int sep = skip_sep(ls);
          luaZ_resetbuffer(ls->buff);  /* `skip_sep' may dirty the buffer */
          if (sep >= 0) {
            read_long_string(ls, NULL, sep);  /* long comment */ // 长注释 
            luaZ_resetbuffer(ls->buff);
            continue;
          }
        }
        /* else short comment */ // 短注释
        while (!currIsNewline(ls) && ls->current != EOZ)
          next(ls);
        continue;
      }
      case '[': {
        int sep = skip_sep(ls);
        if (sep >= 0) {
			// 用"[["和"]]"括起来的多行字符串
          read_long_string(ls, seminfo, sep);
          return TK_STRING;
        }
        else if (sep == -1) return '[';
        else luaX_lexerror(ls, "invalid long string delimiter", TK_STRING);
      }
      case '=': {
        next(ls);
        if (ls->current != '=') return '=';	 // 赋值
        else { next(ls); return TK_EQ; }	// 相等
      }
      case '<': {
        next(ls);
        if (ls->current != '=') return '<'; // 小于
        else { next(ls); return TK_LE; }	 // 小于等于
      }
      case '>': {
        next(ls);
        if (ls->current != '=') return '>';	 // 大于
        else { next(ls); return TK_GE; }		// 大于等于
      }
      case '~': {
        next(ls);
        if (ls->current != '=') return '~';	// 非
        else { next(ls); return TK_NE; }	// 不等于
      }
      case '"':
      case '\'': {
        read_string(ls, ls->current, seminfo);		// 字符串 
        return TK_STRING;
      }
      case '.': {
        save_and_next(ls);
        if (check_next(ls, ".")) {
          if (check_next(ls, "."))
            return TK_DOTS;   /* ... */	// ...
          else return TK_CONCAT;   /* .. */		// 连接
        }
        else if (!isdigit(ls->current)) return '.'; // 不是数字
        else {
          read_numeral(ls, seminfo);	// 读个数
          return TK_NUMBER;
        }
      }
      case EOZ: {
        return TK_EOS;
      }
      default: {
        if (isspace(ls->current)) {	 // 是否为空格字符
          lua_assert(!currIsNewline(ls));
          next(ls);
          continue;
        }
        else if (isdigit(ls->current)) { // 数字
          read_numeral(ls, seminfo);	// 读个数
          return TK_NUMBER;
        }
        else if (isalpha(ls->current) || ls->current == '_') {	 // 是否为英文字母或下划线
          /* identifier or reserved word */
          TString *ts;
          do {
            save_and_next(ls);
          } while (isalnum(ls->current) || ls->current == '_');		// 字符变量c是否为字母或数字 或下划线
          ts = luaX_newstring(ls, luaZ_buffer(ls->buff),
                                  luaZ_bufflen(ls->buff));
          if (ts->tsv.reserved > 0)  /* reserved word? */	// 关键字
            return ts->tsv.reserved - 1 + FIRST_RESERVED;
          else {
            seminfo->ts = ts;	
            return TK_NAME;	// 名字
          }
        }
        else {
          int c = ls->current;
          next(ls);
          return c;  /* single-char tokens (+ - / ...) */	// 单字符
        }
      }
    }
  }
}


void luaX_next (LexState *ls) {
  ls->lastline = ls->linenumber;
  if (ls->lookahead.token != TK_EOS) {  /* is there a look-ahead token? */
    ls->t = ls->lookahead;  /* use this one */
    ls->lookahead.token = TK_EOS;  /* and discharge it */
  }
  else
    ls->t.token = llex(ls, &ls->t.seminfo);  /* read next token */
}


void luaX_lookahead (LexState *ls) {
  lua_assert(ls->lookahead.token == TK_EOS);
  ls->lookahead.token = llex(ls, &ls->lookahead.seminfo);
}

