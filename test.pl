
:- dlib_require(azaltjson).

check_disp(succ, succ, Exp, Exp):- !. % 成功して結果も一致
check_disp(succ, succ, Exp, Act):- !, % 成功して結果は不一致
	nl,
	write('test    '), write('  Exp: '), write(Exp), nl,
	write('test    '), write('  Act: '), write(Act), nl,
	fail.
check_disp(ExpResult, ExpResult, _, _). % 失敗結果が一致
check_disp(ExpResult, ActResult, _, _):- !, % 失敗結果が不一致
	nl,
	write('test    '), write('  ExpResult: '), write(ExpResult), nl,
	write('test    '), write('  ActResult: '), write(ActResult), nl,
	fail.
check(Func, Src, Exp):-
	check(Func, {}, Src, Exp).
check(Func, Opt, Src, Exp):-
	check(Func, Opt, Src, Exp, succ).
check(Func, Opt, Src, Exp, ExpResult):-
	check(Func, Opt, Src, Exp, _, ExpResult, _).
check(Func, Opt, Src, Exp, Act, ExpResult, ActResult):-
	Term =.. [Func, Opt, Src, Act],
	errorset(Term, ActResult), !,
	% write('test    TERM is '), write([ExpResult, ActResult, Exp, Act]),nl,
	check_disp(ExpResult, ActResult, Exp, Act).

bidcheck(J, T):-
	bidcheck(J, T, succ).
bidcheck(J, T, R):-
	bidcheck(J, T, R, R).
bidcheck(J, T, R1, R2):-
	check(json_term, {}, J, T, _, R1, _),
	(atom(J)-> OC = false; OC = true),
	check(term_json, {output_codes: OC}, T, J, _, R2, _).

test(T, M):-
	test(T, succ, M).
test(T, Exp, M):-
	errorset(T, Act), !,
	(Exp == Act-> Result = 'OK   '; Result = 'NG!!!'),
	nl,
	write('test    '), write('['), write(Result), write(']: '),
	write(T),
	write(' % '), write(M), nl.

:-
	test(json_term('', _),           fail, '空文字列はparseエラーになること'),
	test(json_term("", _),           fail, '空文字列はparseエラーになること'),
	test(json_term(_, _),            9, '変数はparseエラーになること'),
	test(json_term(aa(2), _),        9, '複合項はparseエラーになること'),
	test(json_term(2, _),            9, '数値型はparseエラーになること'),
	test(json_term([a], _),          9, 'アトムを含むリスト型はparseエラーになること'),
	test(term_json(_, _),            9, '変数はstringifyエラーになること'),
	test(term_json([1, 2, _, 3], _), 9, '変数を含むリストはstringifyエラーになること'),
	test(term_json(aa(2), _),        9, '複合項はstringifyエラーになること'),
	test(bidcheck('123', 123),       '数値のみ'),
	test(bidcheck("123", 123),       '数値のみ'),
	test(bidcheck('123.45', 123.45), '実数のみ'),
	test(bidcheck("123.45", 123.45), '実数のみ'),
	test(json_term('[ ]', []),    '空リスト'),
	test(bidcheck("[]", []),         '空リスト'),
	test(bidcheck('{}', {}),         '空オブジェクト'),
	test(bidcheck("{}", {}),         '空オブジェクト'),
	test(check(json_term,
		   '{"null": null, "bool": ["true", true, "false", false, "null", null]}',
		   {null:null,bool:[true,true,false,false,null,null]}
		  ), '種別網羅'),
	test(check(term_json,
		   {null:null,bool:[str("true"),true,str("false"),false,str("null"),null]},
		   '{"null": null, "bool": ["true", true, "false", false, "null", null]}'
		  ), '種別網羅'),
	test(check(json_term,
		   '{"年齢":43,"生年月日":{"元号":"昭和","年":56,"月":7,"日":23},"身長":174.23}',
		   {年齢: 43, 生年月日: {元号: 昭和, 年: 56, 月:7,日: 23}, 身長: 174.23}
		  ), '複合オブジェクト'),
	test(check(json_term,
		   '[{"cccc":"bbbb"},123,[],{},[{"foo":22},1.23,{}],"aaaaa"]',
		   [{cccc:bbbb},123,[],{},[{foo:22},1.23,{}],aaaaa]
		  ), '複合リスト'),
	write('----- test end -----'), nl.
