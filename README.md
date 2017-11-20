
README
======

azaltjsonは、AZ-PrologからJansson(JSONライブラリ)を利用するための非公式拡張ライブラリである。
JSONとProlog節を相互変換できる。

必要なもの
======

* AZ-Prolog >= 9.64 (system/pl/fs_utility.plの素性構造型操作のユーティリティ(コンパイル組込述語）を使用可能なもの)
* Jansson


使用例
======

簡単な使用例
--------

```
| ?- dlib_require(azaltjson).
yes
| ?- json_term('{"aa":{"bb":[123,{"cc":"456","10":0.789}]}}', T).
T	= {aa:{bb:[123,{cc:'456','10':0.789}]}}
yes
| ?- term_json({aa:{bb:[123,{cc:'456','10':0.789}]}}, J).
J	= '{"aa": {"bb": [123, {"cc": "456", "10": 0.78900000000000003}]}}'
yes
```


オプション指定例
--------

```
| ?- dlib_require(azaltjson).
yes
| ?-
| A = '{"aa":{"bb":[123,{"cc":"456","10":0.789}]}}',
| json_term({str2comp: true}, A, T),
| term_json({json_indent: 4, json_real_precision: 10}, T, J),
| write(J), nl.
{
    "aa": {
        "bb": [
            123,
            {
                "cc": "456",
                "10": 0.789
            }
        ]
    }
}
A	= '{"aa":{"bb":[123,{"cc":"456","10":0.789}]}}',
T	= {aa:{bb:[123,{cc:str([52,53,54]),'10':0.789}]}},
J	= '{(\n)    "aa": {(\n)        "bb": [(\n)            123,(\n)            {(\n)                "cc": "456",(\n)                "10": 0.789(\n)            }(\n)        ](\n)    }(\n)}'
yes
```

API
======

## json_term(+JSON, -TERM)

`json_term({}, JSON, TERM)` と同等（省略形）

## json_term(+OPTIONS, +JSON, -TERM)

JSON文字列をProlog節へ変換する。

- OPTIONS: オプション指定
- JSON: JSON文字列（アトムまたは文字コードリスト）
- TERM: Prolog節

OPTIONSは素性構造で指定する。指定方法は以下のとおり。

| 素性項目名 | 設定型 | デフォルト値 | 説明 |
| :--- | :--- | :--- | :--- |
| obj2comp | true / false | false | trueを指定した場合、オブジェクトをfsファンクタによる複合項とする。<br/>falseを指定した場合は素性構造とする。 |
| str2comp | true / false | false | trueを指定した場合、文字列をstrファンクタによる複合項とする。<br/>falseを指定した場合はアトムとする。 |

## term_json(+TERM, -JSON)

`term_json({}, TERM, JSON)` と同等（省略形）

## term_json(+OPTIONS, +TERM, -JSON)

Prolog節をJSON文字列へ変換する。

- OPTIONS: オプション指定
- TERM: Prolog節
- JSON: JSON文字列（アトムまたは文字コードリスト）

OPTIONSは素性構造で指定する。指定方法は以下のとおり。
なお、json_で始まる項目のデフォルト値はJanssonの仕様に従う。

| 素性項目名 | 設定型 | デフォルト値 | 説明 |
| :--- | :--- | :--- | :--- |
| output_codes        | true / false | false | trueを指定した場合、結果JSONを文字コードリストとする。<br/>falseを指定した場合はアトムとする。 |
| json_indent         | 数値（0以上）  | 0     | 1以上を指定した場合、改行と指定個数の半角スペースを使って結果を見やすくインデント（字下げ）する。<br/>0を指定した場合は改行およびインデントを行わない。 |
| json_compact        | true / false | false | trueを指定した場合、JSON表現をコンパクトにする。 |
| json_sort_keys      | true / false | false | trueを指定した場合、オブジェクト内の要素をキーでソートする。 |
| json_escape_slash   | true / false | false | trueを指定した場合、スラッシュ記号 `/` をエスケープ `\/` する。 |
| json_real_precision | 数値（0以上）  | 17    | 最大で指定桁数の精度で浮動小数点数（整数以外の実数）を出力する。 |


azjsonとの比較
==============

parse
--------

azjson
```
| ?-
| json_parse('{"aa":{"bb":[123,{"cc":"456","10":0.789}]}}', T0),
| json_get_attribute(T0, aa, T1),
| json_get_attribute(T1, bb, [V0, T2]),
| json_get_attribute(T2, cc, V1),
| json_get_attribute(T2, '10', V2),
| json_free_object(T0).
T0	= '$JSON$'(11171440),
T1	= '$JSON$'(11171680),
V0	= 123,
T2	= '$JSON$'(11172064),
V1	= '456',
V2	= 0.789
```

azaltjson
```
| ?-
| json_term('{"aa":{"bb":[123,{"cc":"456","10":0.789}]}}', {aa:{bb:[V0,{cc:V1,'10':V2}]}}).
V0	= 123,
V1	= '456',
V2	= 0.789
```

stringify
--------

azjson
```
| ?-
| json_make_object(T2),
| json_set_attribute(T2, cc, '456'),
| json_set_attribute(T2, '10', 0.789),
| json_make_object(T1),
| json_set_attribute(T1, bb, [123, T2]),
| json_make_object(T0),
| json_set_attribute(T0, aa, T1),
| json_object_to_atom(T0, A),
| json_free_object(T0).
T2	= '$JSON$'(11171440),
T1	= '$JSON$'(11171680),
T0	= '$JSON$'(11172064),
A	= '{"aa": {"bb": [123, {"cc": "456", "10": 0.78900000000000003}]}}'
```

azaltjson
```
| ?-
| term_json({aa:{bb:[123,{cc:'456','10':0.789}]}}, A).
A	= '{"aa": {"bb": [123, {"cc": "456", "10": 0.78900000000000003}]}}'
```

