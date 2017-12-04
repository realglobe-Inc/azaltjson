#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <azprolog.h>
#include <jansson.h>

#define PARG(n,i)       (next_var_cell - (n) + (i))
#define ILL_ARG  9
#define EMPTY_LIST ("[]")
extern pred P4_azaltjson__json_term(Frame *Env);
extern pred P4_azaltjson__term_json(Frame *Env);

static BASEINT TRUE_ATOM;
static BASEINT FALSE_ATOM;
static BASEINT NULL_ATOM;

static BASEINT FUNCT_FS_ATOM;
static BASEINT FUNCT_STR_ATOM;

static BASEINT EMPTY_LIST_ATOM;

extern int initiate_plmodule(Frame *Env);
extern int initiate_azaltjson(Frame *Env) {
  initiate_plmodule(Env); // prologモジュール初期化（plmodule.c内部で定義）

  put_bltn("azaltjson__json_term", 4, P4_azaltjson__json_term);
  put_bltn("azaltjson__term_json", 4, P4_azaltjson__term_json);

  TRUE_ATOM  = PutSystemAtom(Env, "true");
  FALSE_ATOM = PutSystemAtom(Env, "false");
  NULL_ATOM  = PutSystemAtom(Env, "null");

  FUNCT_FS_ATOM  = PutSystemAtom(Env, "fs");
  FUNCT_STR_ATOM = PutSystemAtom(Env, "str");

  EMPTY_LIST_ATOM = PutSystemAtom(Env, EMPTY_LIST);

  return 1;
};

static int json2term(Frame* Env, TERM* term, json_t* jv, int flag_str2comp) {
  int ret = 1;

  switch (json_typeof(jv)) {
  case JSON_OBJECT: {
    // オブジェクト
    const char *key;
    json_t *value;

    MakeUndef(Env);
    TERM *prefs_term = next_var_cell - 1;
    TERM *list_head_term = prefs_term;
    TERM *list_tail_term = prefs_term;

    json_object_foreach(jv, key, value) {
      MakeUndef(Env);
      TERM *key_term = next_var_cell - 1; // キーアトム
      ret = unify_atom(key_term, Asciz2Atom(Env, (char *)key));
      if (!ret) { return ret; }

      MakeUndef(Env);
      TERM *val_term = next_var_cell - 1; // バリュー
      ret = json2term(Env, val_term, value, flag_str2comp);
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
    ret = UnifyCompTerm(Env, term, FUNCT_FS_ATOM, 1, prefs_term);
    if (!ret) { return ret; }
    break;
  }
  case JSON_ARRAY: {
    // 配列
    int i, len;
    len = (int )json_array_size(jv);

    TERM *list_head_term = term;
    TERM *list_tail_term = term;

    for (i = 0; i < len; i++) {
      json_t *ev = json_array_get(jv, (size_t )i);

      MakeUndef(Env);
      TERM *val_term = next_var_cell - 1;
      ret = json2term(Env, val_term, ev, flag_str2comp);
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
    // 文字列
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
      TERM *list_tail_term = codes_term;

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

      ret = UnifyCompTerm(Env, term, FUNCT_STR_ATOM, 1, codes_term);
      if (!ret) { return ret; }
    }
    break;
  }
  case JSON_INTEGER: {
    // 整数
    json_int_t v = json_integer_value(jv);
    ret = unify_int(term, (SBASEINT )v);
    break;
  }
  case JSON_REAL: {
    // 浮動小数点数
    double v = json_real_value(jv);
    ret = UnifyDouble(Env, term, v);
    break;
  }
  case JSON_TRUE: { // true
    ret = unify_atom(term, TRUE_ATOM);
    break;
  }
  case JSON_FALSE: { // false
    ret = unify_atom(term, FALSE_ATOM);
    break;
  }
  case JSON_NULL: { // null
    ret = unify_atom(term, NULL_ATOM);
    break;
  }
  default:
    fprintf(stderr, "unrecognized JSON type %d\n", json_typeof(jv));
    return 0;
  }
  return ret;
}

pred P4_azaltjson__json_term(Frame *Env) {
  int argc = 4;

  json_t *jx;
  json_error_t error;
  int ret;
  char *s;
  char buf[512];

  TERM *isfile = PARG(argc, 0);
  TERM *opt = PARG(argc, 1);
  TERM *ain = PARG(argc, 2);
  TERM *out = PARG(argc, 3);

  int flag_str2comp = 0;
  int flag_input_atom = 0;
  char flag_str2comp_key[] = "str2comp";
  char flag_input_atom_key[] = "input_atom";

  // オプション解析
  if (!IsNil(opt) && !IsCons(opt)) {
    YIELD(FAIL);
  }
  while (!IsNil(opt)) {
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
    } else if (strcmp(key_str, flag_input_atom_key) == 0) {
      // 空リスト許容フラグ
      flag_input_atom = unify_atom(val_term, TRUE_ATOM);
    } else {
      // 未知のオプションキー
      // 何もしない
    }
  }

  if (!(IsAtom(isfile) && GetAtom(isfile) == TRUE_ATOM)) {
    if (flag_input_atom && IsAtom(ain) && GetAtom(ain) == ATOM_NIL) {
      YIELD(UnifyAtomE(Env, out, ATOM_NIL));
    }
  }

  int len = az_term_to_cstring_length(Env, ain);
  if (len >= sizeof(buf)) {
    s = malloc(len + 1);
    if (s == 0) YIELD(FAIL);
  }
  else {
    s = buf;
  }

  (void )az_term_to_cstring(Env, ain, s, len + 1);
  // printf("[ %s ]\n", s);

  /*
    Jansson ver2.8
    - JSON_REJECT_DUPLICATES  : 重複キーが含まれている場合にデコードエラー
    - JSON_DECODE_ANY         : 任意の有効なJSON値を受け入れる（デフォルトでは配列またはオブジェクトのみ）
    - JSON_DISABLE_EOF_CHECK  : JSONテキストの終端以降の末尾の余分なデータを許容
    - JSON_DECODE_INT_AS_REAL : すべての数値を実数値（浮動小数点数）として解釈
    - JSON_ALLOW_NUL          : 文字列値の中で\ u0000エスケープを許可
  */
  if (IsAtom(isfile) && GetAtom(isfile) == TRUE_ATOM) {
    jx = json_load_file(s, JSON_REJECT_DUPLICATES | JSON_DECODE_ANY, &error);
  } else {
    jx = json_loads(s, JSON_REJECT_DUPLICATES | JSON_DECODE_ANY, &error);
  }
  if (jx == 0) {
    fprintf(stderr, "text:[%s]\n", error.text);
    fprintf(stderr, "%d:%d:%d, source:[%s]\n",
            error.line, error.column, error.position, error.source);
    if (s != buf) free(s);
    YIELD(FAIL);
  }

  ret = json2term(Env, out, jx, flag_str2comp);
  json_decref(jx);
  if (s != buf) free(s);
  if (ret == 0) {
    fprintf(stderr, "json2term returns 0\n");
    YIELD(FAIL);
  }

  YIELD(DET_SUCC);
}

static json_t* term2json(Frame* Env, TERM* term) {
  json_t* jv;

  if (IsInt(term)) {
    // 整数
    json_int_t val = (json_int_t )GetInt(term);
    jv = json_integer(val);
    if (jv == 0) {
      fprintf(stderr, "json_integer: [%" JSON_INTEGER_FORMAT "]\n", val);
      return NULL;
    }
  } else if (IsDouble(term)) {
    // 浮動小数点数
    double val = GetDouble(term);
    jv = json_real(val);
    if (jv == 0) {
      fprintf(stderr, "json_real: [%lf]\n", val);
      return NULL;
    }
  } else if (IsAtom(term)) {
    // アトム
    BASEINT atom = INT_BODY(term);

    if        (atom == TRUE_ATOM) { // true
      jv = json_true();
    } else if (atom == FALSE_ATOM) { // false
      jv = json_false();
    } else if (atom == NULL_ATOM) { // null
      jv = json_null();
    } else if (atom == EMPTY_LIST_ATOM) { // []
      jv = json_array(); // 空リスト（空文字列""とは認識しない）
    } else {
      // それ以外のアトム -> 文字列と推測
      char buf[256];
      char *s;
      int len;

      len = az_term_to_cstring_length(Env, term);
      if (len >= sizeof(buf)) {
        s = malloc(len + 1);
        if (s == 0) return NULL;
      } else {
        s = buf;
      }

      (void )az_term_to_cstring(Env, term, s, len + 1);
      jv = json_string_nocheck(s);
      if (jv == 0) {
        fprintf(stderr, "json_string_nocheck: [%s]\n", s);
        if (s != buf) free(s);
        return NULL;
      }
      if (s != buf) free(s);
    }
  } else if (IsCons(term)) {
    // リスト
    int ret;

    jv = json_array();

    while (IsCons(term)) {
      REALVALUE(term);
      term = BODY(term);

      json_t *ev = term2json(Env, term);
      if (ev == NULL) {
        return NULL;
      }
      ret = json_array_append_new(jv, ev);
      if (ret != 0) {
        fprintf(stderr, "json_array_append_new() fail.\n");
        json_decref(jv);
        return NULL;
      }
      term++;
    }
  } else if (IsCompTerm(term)) {
    // 複合項
    BASEINT functor;
    GetFunctor(term, &functor);

    if (functor == FUNCT_FS_ATOM) {
      // 素性構造
      jv = json_object();
      if (jv == 0) {
        return NULL;
      }

      GetArg(term, 1);
      TERM *pairlist_term = next_var_cell - 1; // ペアリスト

      while (!IsNil(pairlist_term)) {
        char key_str[AZ_MAX_ATOM_LENGTH] = {0};
        if (! IsCons(pairlist_term)) { return NULL; }
        GetCons(pairlist_term);
        TERM *elem_term = next_var_cell - 2;
        pairlist_term   = next_var_cell - 1;

        if (! IsCompTerm(elem_term)) { return NULL; }
        GetArg(elem_term, 1);
        TERM *key_term = next_var_cell - 1; // 名前
        GetArg(elem_term, 2);
        TERM *val_term = next_var_cell - 1; // 値

        json_t *ev = term2json(Env, val_term);
        if (ev == NULL) {
          return NULL;
        }
        Atom2Asciz(GetAtom(key_term), key_str);

        int ret = json_object_set_new(jv, key_str, ev);
        if (ret != 0) {
          fprintf(stderr, "json_object_set_new: [%s]\n", key_str);
          return NULL;
        }
      }

    } else if (functor == FUNCT_STR_ATOM) {
      // 文字列（文字コードリスト形式）
      GetArg(term, 1);
      TERM *list_term = next_var_cell - 1; // 文字コードリスト

      char buf[256];
      char *s;

      int len = az_term_to_cstring_length(Env, list_term);
      if (len >= sizeof(buf)) {
        s = malloc(len + 1);
        if (s == 0) return NULL;
      } else {
        s = buf;
      }

      (void )az_term_to_cstring(Env, list_term, s, len + 1);
      jv = json_string_nocheck(s);
      if (jv == 0) {
        fprintf(stderr, "json_string_nocheck: [%s]\n", s);
        if (s != buf) free(s);
        return NULL;
      }
      if (s != buf) free(s);

    } else {
      char functor_str[AZ_MAX_ATOM_LENGTH] = {0};
      Atom2Asciz(functor, functor_str);
      fprintf(stderr, "Unknown functor: [%s]\n", functor_str);
      return NULL;
    }
  } else {
    fprintf(stderr, "Unknown term\n");
    return NULL;
  }

  return jv;
}

pred P4_azaltjson__term_json(Frame *Env) {
  int argc = 4;

  json_t *jv;
  int ret;

  TERM *isfile = PARG(argc, 0);
  TERM *opt = PARG(argc, 1);
  TERM *ain = PARG(argc, 2);
  TERM *out = PARG(argc, 3);

  char flag_output_codes_key[] = "output_codes";
  int flag_output_codes = 0;

  size_t flags_json_dumps = JSON_PRESERVE_ORDER | JSON_ENCODE_ANY; // default

  // オプション解析
  if (!IsNil(opt) && !IsCons(opt)) {
    YIELD(FAIL);
  }
  while (!IsNil(opt)) {
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

    if (strcmp(key_str, flag_output_codes_key) == 0) {
      // 文字コードフラグ
      flag_output_codes = unify_atom(val_term, TRUE_ATOM);
    }
    else if (strcmp(key_str, "json_indent") == 0) {
      if (IsInt(val_term)) {
        flags_json_dumps |= JSON_INDENT(GetInt(val_term));
      }
    }
    else if (strcmp(key_str, "json_compact") == 0) {
      if (unify_atom(val_term, TRUE_ATOM)) {
        flags_json_dumps |= JSON_COMPACT;
      }
    }
    else if (strcmp(key_str, "json_sort_keys") == 0) {
      if (unify_atom(val_term, TRUE_ATOM)) {
        flags_json_dumps |= JSON_SORT_KEYS;
      }
    }
    else if (strcmp(key_str, "json_escape_slash") == 0) {
      if (unify_atom(val_term, TRUE_ATOM)) {
        flags_json_dumps |= JSON_ESCAPE_SLASH;
      }
    }
    else if (strcmp(key_str, "json_real_precision") == 0) {
      if (IsInt(val_term)) {
        flags_json_dumps |= JSON_REAL_PRECISION(GetInt(val_term));
      }
    }
  }

  jv = term2json(Env, ain);
  if (jv == 0) {
    // fprintf(stderr, "term2json returns 0\n");
    AZ_ERROR(ILL_ARG);
  }

  /*
    Jansson ver2.8
    - JSON_INDENT(n)          : 改行とn個の空白を使ってインデントすることで、結果を見やすくする
    - JSON_COMPACT            : コンパクトな表現を可能に
    - JSON_ENSURE_ASCII       : 出力はASCII文字のみで構成されていることが保証される
    - JSON_SORT_KEYS          : 出力内のすべてのオブジェクトがキーでソートされる
    - JSON_PRESERVE_ORDER     : (Deprecated)
    - JSON_ENCODE_ANY         : 任意のJSON値をエンコード可能
    - JSON_ESCAPE_SLASH       : 文字列中の/文字をエスケープする
    - JSON_REAL_PRECISION(n)  : 最大n桁の精度ですべての実数を出力する
  */
  if (IsAtom(isfile) && GetAtom(isfile) == TRUE_ATOM) {
    char buf[256];
    char *s;
    int len;

    len = az_term_to_cstring_length(Env, out);
    if (len >= sizeof(buf)) {
      s = malloc(len + 1);
      if (s == 0) YIELD(FAIL);
    } else {
      s = buf;
    }

    (void )az_term_to_cstring(Env, out, s, len + 1);
    if (json_dump_file(jv, s, flags_json_dumps)) {
      fprintf(stderr, "json_dump_file: [%s]\n", s);
      if (s != buf) free(s);
      YIELD(FAIL);
    }
    if (s != buf) free(s);

    YIELD(DET_SUCC);
  } else {
    char *s = json_dumps(jv, flags_json_dumps);
    json_decref(jv);
    if (s == 0) {
      YIELD(FAIL);
    }

    if (!flag_output_codes) {
      // string -> アトム
      ret = unify_atom(out, Asciz2Atom(Env, (char* )s));
      if (!ret) { free(s); YIELD(FAIL); }
    } else {
      // string -> 文字コードリスト格納str複合項
      TERM *list_head_term = out;
      TERM *list_tail_term = out;

      int i = 0;
      for (i = 0; i < strlen(s); i++) {
        MakeUndef(Env);
        TERM *val_term = next_var_cell - 1; // 文字コード
        ret = UnifyIntE(Env, val_term, (unsigned char)s[i]);
        if (!ret) { free(s); YIELD(FAIL); }

        MakeUndef(Env);
        list_tail_term = next_var_cell - 1; // リスト要素
        ret = UnifyCons(Env, list_head_term, val_term, list_tail_term);
        if (!ret) { free(s); YIELD(FAIL); }

        list_head_term = list_tail_term;
      }
      // リスト終端
      ret = UnifyAtomE(Env, list_tail_term, ATOM_NIL);
      if (!ret) { free(s); YIELD(FAIL); }
    }
    free(s);
    YIELD(DET_SUCC);
  }

  YIELD(DET_SUCC);
}
