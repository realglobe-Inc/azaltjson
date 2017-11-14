#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <azprolog.h>
#include <jansson.h>

#define PARG(n,i)       (next_var_cell - (n) + (i))

#define AZALTJSON__FUNCTOR_FS  ("fs")
#define AZALTJSON__FUNCTOR_STR ("str")

extern pred P3_azaltjson__json_term(Frame *Env);

static BASEINT TRUE_ATOM;
static BASEINT FALSE_ATOM;
static BASEINT NULL_ATOM;

extern int initiate_plmodule(Frame *Env);
extern int initiate_azaltjson(Frame *Env) {
  initiate_plmodule(Env); // prologモジュール初期化（plmodule.c内部で定義）

  put_bltn("azaltjson__json_term", 3, P3_azaltjson__json_term);

  TRUE_ATOM  = PutSystemAtom(Env, "true");
  FALSE_ATOM = PutSystemAtom(Env, "false");
  NULL_ATOM  = PutSystemAtom(Env, "null");

  return 1;
};

static int make_prolog_value_from_json_value(Frame* Env, TERM* term, json_t* jv, int flag_str2comp) {
  int ret = 1;

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
        ret = unify_atom(key_term, Asciz2Atom(Env, (char *)key));
        if (!ret) { return ret; }

        MakeUndef(Env);
        TERM *val_term = next_var_cell - 1; // バリュー
        ret = make_prolog_value_from_json_value(Env, val_term, value, flag_str2comp);
        if (!ret) { return ret; }

        MakeUndef(Env);
        TERM *fs_delimiter_term = next_var_cell - 1; // 区切り記号
        ret = B2_fs_delimiter(Env, fs_delimiter_term, fs_delimiter_term);
        if (!ret) { return ret; }

        MakeUndef(Env);
        TERM *pair_term = next_var_cell - 1; // ペア複合項
        ret = UnifyCompTerm(Env, pair_term, Asciz2Atom(Env, ":"), 2, key_term, val_term);
        if (!ret) { return ret; }

        MakeUndef(Env);
        list_tail_term = next_var_cell - 1; // リスト要素
        ret = UnifyCons(Env, list_head_term, pair_term, list_tail_term);
        if (!ret) { return ret; }

        list_head_term = list_tail_term;
      }
      // リスト終端
      ret = UnifyAtomE(Env, list_tail_term, ATOM_NIL);
      if (!ret) { return ret; }

      // object -> ペアリスト格納fs複合項
      ret = UnifyCompTerm(Env, term, Asciz2Atom(Env, AZALTJSON__FUNCTOR_FS), 1, prefs_term);
      if (!ret) { return ret; }
      break;
  }
  case JSON_ARRAY: {
    int i, len;
    len = (int )json_array_size(jv);

    TERM *list_head_term = term;
    TERM *list_tail_term = NULL;

    for (i = 0; i < len; i++) {
      json_t *ev = json_array_get(jv, (size_t )i);

      MakeUndef(Env);
      TERM *val_term = next_var_cell - 1;
      ret = make_prolog_value_from_json_value(Env, val_term, ev, flag_str2comp);
      if (!ret) { return ret; }

      MakeUndef(Env);
      list_tail_term = next_var_cell - 1; // リスト要素
      ret = UnifyCons(Env, list_head_term, val_term, list_tail_term);
      if (!ret) { return ret; }

      list_head_term = list_tail_term;
    }
    // リスト終端
    ret = UnifyAtomE(Env, list_tail_term, ATOM_NIL);
    break;
  }
  case JSON_STRING: {
    BASEINT a;
    const char *s = json_string_value(jv);
    if (s == 0) return 0;
    if (!flag_str2comp) {
      // string -> アトム
      ret = unify_atom(term, Asciz2Atom(Env, (char* )s));
      if (!ret) { return ret; }
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
        ret = UnifyIntE(Env, val_term, (unsigned char)s[i]);
        if (!ret) { return ret; }

        MakeUndef(Env);
        list_tail_term = next_var_cell - 1; // リスト要素
        ret = UnifyCons(Env, list_head_term, val_term, list_tail_term);
        if (!ret) { return ret; }

        list_head_term = list_tail_term;
      }
      // リスト終端
      ret = UnifyAtomE(Env, list_tail_term, ATOM_NIL);
      if (!ret) { return ret; }

      ret = UnifyCompTerm(Env, term, Asciz2Atom(Env, AZALTJSON__FUNCTOR_STR), 1, codes_term);
      if (!ret) { return ret; }
    }
    break;
  }
  case JSON_INTEGER: {
    json_int_t v = json_integer_value(jv);
    ret = unify_int(term, (SBASEINT )v);
    break;
  }
  case JSON_REAL: {
    double v = json_real_value(jv);
    ret = UnifyDouble(Env, term, v);
    break;
  }
  case JSON_TRUE: {
    ret = unify_atom(term, TRUE_ATOM);
    break;
  }
  case JSON_FALSE: {
    ret = unify_atom(term, FALSE_ATOM);
    break;
  }
  case JSON_NULL: {
    ret = unify_atom(term, NULL_ATOM);
    break;
  }
  default:
    fprintf(stderr, "unrecognized JSON type %d\n", json_typeof(jv));
    return 0;
  }
  return ret;
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

  int flag_str2comp = 0;
  char flag_str2comp_key[] = "str2comp";

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
    if (strcmp(key_str, flag_str2comp_key) == 0) {
      // アトムフラグ
      flag_str2comp = unify_atom(val_term, TRUE_ATOM);
    } else {
      // 未知のオプションキー
      // 何もしない
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

  r = make_prolog_value_from_json_value(Env, out, jx, flag_str2comp);
  json_object_clear(jx);
  if (s != buf) free(s);
  if (r == 0) {
    fprintf(stderr, "make_prolog_value_from_json_value returns 0\n");
    YIELD(FAIL);
  }

  YIELD(DET_SUCC);
}

