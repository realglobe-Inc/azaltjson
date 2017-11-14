# azaltjson

使用例
======

入力例
```
?-
dlib_require(azaltjson),
INPUT = '{"aa":{"bb":[123,{"cc":"456","10":0.789}]}}',
atom_codes(INPUT, INPUTC),
json_term(INPUT, T1),
json_term(INPUTC, T2),
json_term({obj2comp: true}, INPUT, T3),
json_term({str2comp: true}, INPUT, T4),
json_term({obj2comp: true, str2comp: true}, INPUT, T5).
```

出力例
```
INPUT	= '{"aa":{"bb":[123,{"cc":"456","10":0.789}]}}',
INPUTC	= [123,34,97,97,34,58,123,34,98,98,34,58,91,49,50,51,44,123,34,99,99,34,58,34,52,53,54,34,44,34,49,48,34,58,48,46,55,56,57,125,93,125,125],
T1	= {aa:{bb:[123,{cc:'456','10':0.789}]}},
T2	= {aa:{bb:[123,{cc:'456','10':0.789}]}},
T3	= fs([aa:fs([bb:[123,fs([cc:'456','10':0.789])]])]),
T4	= {aa:{bb:[123,{cc:str([52,53,54]),'10':0.789}]}},
T5	= fs([aa:fs([bb:[123,fs([cc:str([52,53,54]),'10':0.789])]])])
yes
```
