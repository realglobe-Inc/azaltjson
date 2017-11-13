#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <azprolog.h>
#include <jansson.h>

#define PARG(n,i)       (next_var_cell - (n) + (i))

#define AZALTJSON__FUNCTOR_FS  ("fs")
#define AZALTJSON__FUNCTOR_STR ("str")
#define AZALTJSON__CALL        ("azaltjson__makefs")

/* ---------- AZ-Prolog 文字列to節 コーディング ここから ---------- */
// #define BACKWARD_COMPAT

static int
string_to_term(Frame* Env, TERM* t, char* s)
{
#ifdef BACKWARD_COMPAT
  AZ_EXTERN struct _Stream* bufferstream;
  AZ_EXTERN TERM* gvar_top;

  AZ_EXTERN void Stream_ClearUngetBuff(struct _Stream *stream);
  AZ_EXTERN void Stream_ResetBuffer(struct _Stream *stream);
  AZ_EXTERN int  Stream_buf_put(int c, struct _Stream *s);
  AZ_EXTERN int  Read_ReadGeneric(Frame *Env, struct _Stream *stream);
#endif

  char* p;
  char* end;
  TERM *sv, *unifyp;
  int r;
#ifndef BACKWARD_COMPAT
  int v;
#endif

  end = s + strlen(s);

  Stream_ClearUngetBuff(bufferstream);
  Stream_ResetBuffer(bufferstream);

  p = s;
  while (p < end) {
    Stream_buf_put((int )*p, bufferstream);

    p++;
  }

  Stream_buf_put(' ', bufferstream);
  Stream_buf_put('.', bufferstream);
  Stream_buf_put(0,   bufferstream);
  Stream_ResetBuffer(bufferstream);

  PUSH_STACK(t);
  sv = next_var_cell;
#ifdef BACKWARD_COMPAT
  r = Read_ReadGeneric(Env, bufferstream);
#else
  r = Read_ReadGeneric(Env, bufferstream, &v);
#endif
  if (r == 1) {
    unifyp = next_var_cell - 1;
    next_var_cell = sv;
    POP_STACK(t);
    r = unifyE_ex(Env, t, unifyp);
    if (r != 0) return 0;
    return -1;
  }
  next_var_cell = sv - 1;

  return 1;
}

/* ---------- AZ-Prolog 文字列to節 コーディング ここまで ---------- */

/* ---------- AZ-Prolog コールバック コーディング ここから ---------- */
#define ATOM_COMMA		(ATOM_NIL+7)
#define ATOM_CUT		(ATOM_NIL+4)

#define PUTTERM(ari,body,t)		SETTAG(t,term_tag); NO(t)=ari; BODY(t)=body

#define   CALL_BACK_ATOM_BEGIN(env,fname)                   \
  { TERM *gcell;                                            \
    env.Link=CallbackEnv;                                   \
    LINK1(((Frame*)&env),3,3);                              \
    gcell=env.Global;                                       \
    PUTTERM(2,gcell,next_var_cell);                         \
    next_var_cell++ ;                                       \
    PUTATOM(ATOM_COMMA,gcell);                              \
    PUTATOM(fname,     gcell+1);                            \
    PUTATOM(ATOM_CUT,  gcell+2); }

#define  CALL_BACK_CALL(env,getcell)                                    \
  { register pred result;                                               \
    int      ret_status=0;                                              \
    TERM *my_local = next_var_cell-1;                                   \
    CALL_CDET(P1_call, &env, result, ret_status = -1 );                 \
    env.Global -= (getcell+4);                                          \
    env.Local  = my_local;                                              \
    env.nVars  =  0;                                                    \
    RELINKZ(((Frame*)&env));                                            \
    return ret_status; }

/* ***************************** */

static Frame* CallbackEnv;

static int
azros_callback(BASEINT callback_name)
{
  Frame Now       = {0};

  CALL_BACK_ATOM_BEGIN(Now,callback_name);
  CALL_BACK_CALL(Now,-1);
}

/* ---------- AZ-Prolog コールバック コーディング ここまで ---------- */

extern pred P3_azaltjson__json_term(Frame *Env);
extern pred P1_azaltjson__get_prefs(Frame *Env); // 非公開API
extern pred P1_azaltjson__set_fs(Frame *Env);    // 非公開API

static BASEINT TRUE_ATOM;
static BASEINT FALSE_ATOM;
static BASEINT NULL_ATOM;

static TERM *global_prefs_term = NULL;
static char global_fs_term_str[AZ_MAX_ATOM_LENGTH] = {0};

extern int initiate_plmodule(Frame *Env);
extern int initiate_azaltjson(Frame *Env) {
  initiate_plmodule(Env); // prologモジュール初期化（plmodule.c内部で定義）

  put_bltn("azaltjson__json_term", 3, P3_azaltjson__json_term);
  put_bltn("azaltjson__get_prefs", 1, P1_azaltjson__get_prefs);
  put_bltn("azaltjson__set_fs",    1, P1_azaltjson__set_fs);

  TRUE_ATOM  = PutSystemAtom(Env, "true");
  FALSE_ATOM = PutSystemAtom(Env, "false");
  NULL_ATOM  = PutSystemAtom(Env, "null");

  return 1;
};

static int make_prolog_value_from_json_value(Frame* Env, TERM* t, json_t* jv, int flag_fs, int flag_atom) {
  int r;

  switch (json_typeof(jv)) {
  case JSON_OBJECT: {
      size_t size;
      const char *key;
      json_t *value;

      size = json_object_size(jv);
      MakeUndef(Env);
      TERM *prefs_term = next_var_cell - 1;
      TERM *list_head_term = prefs_term;
      TERM *list_tail_term = NULL;

      json_object_foreach(jv, key, value) {
        MakeUndef(Env);
        TERM *key_term = next_var_cell - 1; // キーアトム
        r = unify_atom(key_term, Asciz2Atom(Env, (char *)key));
        if (!r) { return r; }

        MakeUndef(Env);
        TERM *val_term = next_var_cell - 1; // バリュー
        r = make_prolog_value_from_json_value(Env, val_term, value, flag_fs, flag_atom);
        if (!r) { return r; }

        MakeUndef(Env);
        TERM *fs_delimiter_term = next_var_cell - 1; // 区切り記号
        r = B2_fs_delimiter(Env, fs_delimiter_term, fs_delimiter_term);
        if (!r) { return r; }

        MakeUndef(Env);
        TERM *pair_term = next_var_cell - 1; // ペア複合項
        r = UnifyCompTerm(Env, pair_term, Asciz2Atom(Env, ":"), 2, key_term, val_term);
        if (!r) { return r; }

        MakeUndef(Env);
        list_tail_term = next_var_cell - 1; // リスト要素
        r = UnifyCons(Env, list_head_term, pair_term, list_tail_term);
        if (!r) { return r; }

        list_head_term = list_tail_term;
      }
      // リスト終端
      r = UnifyAtomE(Env, list_tail_term, ATOM_NIL);
      if (!r) { return r; }

      if (!flag_fs) {
        // object -> ペアリスト格納fs複合項
        r = UnifyCompTerm(Env, t, Asciz2Atom(Env, AZALTJSON__FUNCTOR_FS), 1, prefs_term);
        if (!r) { return r; }
      } else {
        // object -> 素性構造
        global_prefs_term = prefs_term;

        LINK0(Env);
        LINK1ZZ(Env);
        CallbackEnv = Env;
        int ret = azros_callback(Asciz2Atom(Env, AZALTJSON__CALL));
        if (ret != 0) { return 0; }

        int backup_kanji = kanji;
        kanji = 0;
        r = string_to_term(Env, t, global_fs_term_str);
        kanji = backup_kanji;
        if (r == 1) { // syntax error
          return 0;
        }
        r = 1; // 正常終了
      }
      break;
  }
  case JSON_ARRAY: {
    int i, len;
    len = (int )json_array_size(jv);

    TERM *list_head_term = t;
    TERM *list_tail_term = NULL;

    for (i = 0; i < len; i++) {
      json_t *ev = json_array_get(jv, (size_t )i);

      MakeUndef(Env);
      TERM *val_term = next_var_cell - 1;
      r = make_prolog_value_from_json_value(Env, val_term, ev, flag_fs, flag_atom);
      if (!r) { return r; }

      MakeUndef(Env);
      list_tail_term = next_var_cell - 1; // リスト要素
      r = UnifyCons(Env, list_head_term, val_term, list_tail_term);
      if (!r) { return r; }

      list_head_term = list_tail_term;
    }
    // リスト終端
    r = UnifyAtomE(Env, list_tail_term, ATOM_NIL);
    break;
  }
  case JSON_STRING: {
    BASEINT a;
    const char *s = json_string_value(jv);
    if (s == 0) return 0;
    if (flag_atom) {
      // string -> アトム
      r = unify_atom(t, Asciz2Atom(Env, (char* )s));
      if (!r) { return r; }
    } else {
      // string -> 文字コードリスト格納str複合項
      MakeUndef(Env);
      TERM *codes_term = next_var_cell - 1;
      TERM *list_head_term = codes_term;
      TERM *list_tail_term = NULL;

      int i = 0;
      for (i = 0; i < strlen(s); i++) {
        MakeUndef(Env);
        TERM *val_term = next_var_cell - 1; // 文字コード
        r = unify_int(val_term, (SBASEINT)s[i]);
        if (!r) { return r; }

        MakeUndef(Env);
        list_tail_term = next_var_cell - 1; // リスト要素
        r = UnifyCons(Env, list_head_term, val_term, list_tail_term);
        if (!r) { return r; }

        list_head_term = list_tail_term;
      }
      // リスト終端
      r = UnifyAtomE(Env, list_tail_term, ATOM_NIL);
      if (!r) { return r; }

      r = UnifyCompTerm(Env, t, Asciz2Atom(Env, AZALTJSON__FUNCTOR_STR), 1, codes_term);
      if (!r) { return r; }
    }
    break;
  }
  case JSON_INTEGER: {
    json_int_t v = json_integer_value(jv);
    r = unify_int(t, (SBASEINT )v);
    break;
  }
  case JSON_REAL: {
    double v = json_real_value(jv);
    r = UnifyDouble(Env, t, v);
    break;
  }
  case JSON_TRUE: {
    r = unify_atom(t, TRUE_ATOM);
    break;
  }
  case JSON_FALSE: {
    r = unify_atom(t, FALSE_ATOM);
    break;
  }
  case JSON_NULL: {
    r = unify_atom(t, NULL_ATOM);
    break;
  }
  default:
    fprintf(stderr, "unrecognized JSON type %d\n", json_typeof(jv));
  }
  return r;
}

pred P3_azaltjson__json_term(Frame *Env) {
  int argc = 3;

  json_t *jx;
  json_error_t error;
  int len, r;
  char *s;
  char buf[512];

  TERM *opt = PARG(argc, 0);
  TERM *ain = PARG(argc, 1);
  TERM *out = PARG(argc, 2);

  int flag_fs = 0;
  int flag_atom = 0;
  char flag_fs_key[] = "option2fs";
  char flag_atom_key[] = "str2atom";

  // オプション解析
  if (UnifyAtomE(Env, opt, ATOM_NIL)) {
    // 何もしない
  } else if (!IsCons(opt)) {
    YIELD(FAIL);
  }
  while (! IsNil(opt)) {
    char key_str[AZ_MAX_ATOM_LENGTH] = {0};
    if (! IsCons(opt)) { YIELD(FAIL); }
    GetCons(opt);
    TERM *elem_term = next_var_cell - 2;
    opt             = next_var_cell - 1;

    if (! IsCompTerm(elem_term)) { YIELD(FAIL); }
    GetArg(elem_term, 1);
    TERM *key_term = next_var_cell - 1; // 名前
    GetArg(elem_term, 2);
    TERM *val_term = next_var_cell - 1; // 値

    Atom2Asciz(GetAtom(key_term), key_str);
    if        (strcmp(key_str, flag_fs_key) == 0) {
      // 素性構造フラグ
      flag_fs = unify_atom(val_term, TRUE_ATOM);
    } else if (strcmp(key_str, flag_atom_key) == 0) {
      // アトムフラグ
      flag_atom = unify_atom(val_term, TRUE_ATOM);
    } else {
      // 未知のオプションキー
      YIELD(FAIL);
    }
  }

  len = az_term_to_cstring_length(Env, ain);
  if (len >= 512) {
    s = malloc(len + 1);
    if (s == 0) YIELD(FAIL);
  }
  else {
    s = buf;
  }

  (void )az_term_to_cstring(Env, ain, s, len + 1);

  jx = json_loads(s, JSON_REJECT_DUPLICATES, &error);
  if (jx == 0) {
    fprintf(stderr, "text:[%s]\n", error.text);
    fprintf(stderr, "%d:%d:%d, source:[%s]\n",
            error.line, error.column, error.position, error.source);
    if (s != buf) free(s);
    YIELD(FAIL);
  }

  r = make_prolog_value_from_json_value(Env, out, jx, flag_fs, flag_atom);
  if (s != buf) free(s);
  if (r == 0) {
    json_object_clear(jx);
    fprintf(stderr, "make_prolog_value_from_json_value returns 0\n");
    YIELD(FAIL);
  }

  YIELD(DET_SUCC);
}

pred P1_azaltjson__get_prefs(Frame *Env) {
  int argc = 1;
  TERM *term = PARG(argc, 0);

  if (!UnifyE(Env, global_prefs_term, term)) {
    YIELD(FAIL);
  }
  YIELD(DET_SUCC);
}

pred P1_azaltjson__set_fs(Frame *Env) {
  int argc = 1;
  TERM *term = PARG(argc, 0);

  int backup_kanji = kanji;
  kanji = 0;
  strncpy(global_fs_term_str, az_term_conv_to_string(Env, term), AZ_MAX_ATOM_LENGTH - 1);
  kanji = backup_kanji;
  YIELD(DET_SUCC);
}

