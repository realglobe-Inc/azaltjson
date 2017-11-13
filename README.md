# azaltjson

使用例
======

入力例
```
?-
dlib_require(azaltjson),
INPUT = '{"aa":{"bb":[123,{"cc":"456","10":0.789}]}}',
atom_codes(INPUT, INPUTC),
json_term({option2fs:true, str2atom:true}, INPUT, T1),
json_term({option2fs:true, str2atom:true}, INPUTC, T2),
json_term({option2fs:false, str2atom:true}, INPUT, T3),
json_term({option2fs:true, str2atom:false}, INPUT, T4),
json_term({option2fs:false, str2atom:false}, INPUT, T5),
json_term(INPUT, T6).
```

出力例
```
INPUT	= '{"aa":{"bb":[123,{"cc":"456","10":0.789}]}}',
INPUTC	= [123,34,97,97,34,58,123,34,98,98,34,58,91,49,50,51,44,123,34,99,99,34,58,34,52,53,54,34,44,34,49,48,34,58,48,46,55,56,57,125,93,125,125],
T1	= {aa:{bb:[123,{cc:'456','10':0.789}]}},
T2	= {aa:{bb:[123,{cc:'456','10':0.789}]}},
T3	= fs([aa:fs([bb:[123,fs([cc:'456','10':0.789])]])]),
T4	= {aa:{bb:[123,{cc:str([52,53,54]),'10':0.789}]}},
T5	= fs([aa:fs([bb:[123,fs([cc:str([52,53,54]),'10':0.789])]])]),
T6	= fs([aa:fs([bb:[123,fs([cc:str([52,53,54]),'10':0.789])]])])
```

