
:- dlib_require(azaltjson).

% チェックおよび結果表示
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

% 変換チェック
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

% 双方向変換チェック
bidcheck(term_json_term, T, J):-
	bidcheck(term_json_term, T, J, {}).
bidcheck(term_json_term, T, J, Opt):-
	(atom(J)-> OC = false; OC = true),
	Opt = {output_codes: OC},
	check(term_json, Opt, T, J, _, succ, _),
	check(json_term, Opt, J, T, _, succ, _).
bidcheck(json_term_json, J, T):-
	bidcheck(json_term_json, J, T, {}).
bidcheck(json_term_json, J, T, Opt):-
	(atom(J)-> OC = false; OC = true),
	Opt = {output_codes: OC},
	check(json_term, Opt, J, T, _, succ, _),
	check(term_json, Opt, T, J, _, succ, _).

% テスト
test(T, M):-
	test(T, succ, M).
test(T, Exp, M):-
	errorset(T, Act), !,
	(Exp == Act-> Result = 'OK   '; Result = 'NG!!!'),
	nl,
	write('test    '), write('['), write(Result), write(']: '),
	(Exp == Act->
	 (write(Exp), write(' <- '));
	 (write(' Exp='), write(Exp), write(' Act='), write(Act))),
	write(' '), write(T),
	write(' % '), write(M), nl.

alltest:-
	test(json_term('', _),           fail, '空文字列はparseエラーになること'),
	test(json_term("", _),           fail, '空文字列はparseエラーになること'),
	test(json_term(_, _),            9, '変数はparseエラーになること'),
	test(json_term(aa(2), _),        9, '複合項はparseエラーになること'),
	test(json_term(2, _),            9, '数値型はparseエラーになること'),
	test(json_term([a], _),          9, 'アトムを含むリスト型はparseエラーになること'),
	test(term_json(_, _),            9, '変数はstringifyエラーになること'),
	test(term_json([1, 2, _, 3], _), 9, '変数を含むリストはstringifyエラーになること'),
	test(term_json(aa(2), _),        9, '複合項はstringifyエラーになること'),
	test(bidcheck(json_term_json, '123', 123),       '数値のみ'),
	test(bidcheck(json_term_json, "123", 123),       '数値のみ'),
	test(bidcheck(json_term_json, '123.45', 123.45), '実数のみ'),
	test(bidcheck(json_term_json, "123.45", 123.45), '実数のみ'),
	test(json_term('[]', []),        fail,           '空文字列'),
	test(json_term({input_atom: true}, '[]', []),    '空リスト'),
	test(json_term('[ ]', []),                       '空リスト'),
	test(bidcheck(json_term_json, "[]", []),         '空リスト'),
	test(bidcheck(json_term_json, '{}', {}),         '空オブジェクト'),
	test(bidcheck(json_term_json, "{}", {}),         '空オブジェクト'),
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
	test(check(term_json,
		   {年齢: 43, 生年月日: {元号: 昭和, 年: 56, 月:7,日: 23}, 身長: 174.23},
		   '{"年齢": 43, "生年月日": {"元号": "昭和", "年": 56, "月": 7, "日": 23}, "身長": 174.22999999999999}'
		  ), '複合オブジェクト'),
	test(bidcheck(json_term_json,
		      '[{"cccc": "bbbb"}, 123, [], {}, [{"foo": 22}, 1.23, {}], "aaaaa"]',
		      [{cccc:bbbb},123,[],{},[{foo:22},1.23,{}],aaaaa]
		     ), '複合リスト'),
	test(bidcheck(term_json_term,
		      [{cccc:bbbb},123,[],{},[{foo:22},1.23,{}],aaaaa],
		      _結果1,
		      {json_indent: 4, json_compact: true}
		     ), '複合リスト'),
	write('test    '), write('期待値は改行を含み設定しづらいため無し、結果は目視確認'), nl,
	atom_codes(_結果アトム1, _結果1), write(_結果アトム1), nl,
	test(check(json_term,
		   '"_\"_\\_\/_\u65e5\u672c\u8a9e"',
		   '_"_\_/_日本語'
		  ), 'エスケープ特殊文字'),
	test(check(term_json,
		   '_"_\_/_日本語',
		   '"_\"_\\_/_日本語"' % Unicode文字エスケープシーケンスは未対応
		  ), 'エスケープ特殊文字'),
	nl.

:- alltest.
/*
:-
	repeat(C),
	alltest,
	Mod is C mod 100, Mod == 0, write(C), nl,
	C >= 10000, !.
*/
