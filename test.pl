
:- dlib_require(azaltjson).

check(J, T):-
	check(J, T, succ).
check(J, T, R):-
	check(J, T, R, R).
check(J, T, R1, R2):-
	errorset(json_term(J, T), R1), !,
	(atom(J)-> OC = false; OC = true),
	errorset(term_json({output_codes: OC}, T, J), R2), !.

test(T, M):- errorset(T, R), !, write('test    ['), write(R), write(']: '), write(T), write(' % '), write(M), nl.

:-
	test(\+json_term('', _), '空文字列はparseエラーになること'),
	test(\+json_term("", _), '空文字列はparseエラーになること'),
	test((errorset(json_term(_, _), 9), !, nl), '変数はparseエラーになること'),
	test((errorset(json_term(aa(2), _), 9), !, nl), '複合項はparseエラーになること'),
	test((errorset(json_term(2, _), 9), !, nl), '数値型はparseエラーになること'),
	test((errorset(json_term([a], _), 9), !, nl), 'アトムを含むリスト型はparseエラーになること'),
	test((errorset(term_json(_, _), 9), !, nl), '変数はstringifyエラーになること'),
	test((errorset(term_json(aa(2), _), 9), !, nl), '複合項はstringifyエラーになること'),
	test(check('123', 123), '数値のみ'),
	test(check("123", 123), '数値のみ'),
	test(check('123.45', 123.45), '実数のみ'),
	test(check("123.45", 123.45), '実数のみ'),
	test(json_term('[ ]', []), '空リスト'),
	test(check("[]", []), '空リスト'),
	test(check('{}', {}), '空オブジェクト'),
	test(check("{}", {}), '空オブジェクト'),
	test(json_term('{"年齢": 43, "生年月日": {"元号": "昭和", "年": 56, "月":7,"日": 23}, "身長": 174.23}',
		       {年齢: 43, 生年月日: {元号: 昭和, 年: 56, 月:7,日: 23}, 身長: 174.23}),
	     '複合オブジェクト'),
	test(json_term('[{"cccc":"bbbb"}, 123, [], { }, [{"foo":22}, 1.23, {}], "aaaaa"]',
		       [{cccc:bbbb},123,[],{},[{foo:22},1.23,{}],aaaaa]),
	     '複合リスト'),
	write('----- test end -----'), nl.

% make -B && make install && echo "?-halt." | prolog_c -c test.pl | grep "^test "
