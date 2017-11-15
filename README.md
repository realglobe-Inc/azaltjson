
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

API
======

## json_term(+JSON, -TERM)

`json_term({}, +JSON, -TERM)` と同等（省略形）

## json_term(+OPTIONS, +JSON, -TERM)

OPTIONSは素性構造で指定する。指定方法は以下のとおり。

| 素性項目名 | 設定型 | デフォルト値 | 説明 |
| :--- | :--- | :--- | :--- |
| obj2comp | アトム（true / それ以外） | false | trueを指定した場合、オブジェクトをfsファンクタによる複合項とする。それ以外を指定した場合は素性構造とする。 |
| str2comp | アトム（true / それ以外） | false | trueを指定した場合、文字列をstrファンクタによる複合項とする。それ以外を指定した場合はアトムとする。 |

## term_json(+TERM, -JSON)

`term_json({}, TERM, JSON)` と同等（省略形）

## term_json(+OPTIONS, +TERM, -JSON)

OPTIONSは素性構造で指定する。指定方法は以下のとおり。
なお、json_で始まる項目のデフォルト値はJanssonの仕様に従う。

| 素性項目名 | 設定型 | デフォルト値 | 説明 |
| :--- | :--- | :--- | :--- |
| output_codes        | アトム（true / それ以外） | false | trueを指定した場合、結果JSONを文字コードリストとする。それ以外を指定した場合はアトムとする。 |
| json_indent         | 数値（0以上） | 0 | 1以上を指定した場合、改行と指定個数の半角スペースを使って結果を見やすくインデント（字下げ）する。0を指定した場合は改行およびインデントを行わない。 |
| json_compact        | アトム（true / それ以外） | false | trueを指定した場合、JSON表現をコンパクトにする。 |
| json_sort_keys      | アトム（true / それ以外） | false | trueを指定した場合、オブジェクト内の要素をキーでソートする。 |
| json_escape_slash   | アトム（true / それ以外） | false | trueを指定した場合、スラッシュ記号をエスケープする。 |
| json_real_precision | 数値（0以上） | 17 | 最大で指定桁数の精度で浮動小数点数（整数以外の実数）を出力する。 |
