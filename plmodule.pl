
% ?- json_term(+INPUT, -TERM).
json_term(INPUT, TERM):-
	json_term({}, INPUT, TERM).

% ?- json_term(+OPTIONS, +INPUT, -TERM).
json_term(OPTIONS, INPUT, TERM):-
	fstructure(OPTIONS),
	fs_list(OPTIONS, OPTIONS_LIST),
	azaltjson__json_term(OPTIONS_LIST, INPUT, PRETERM),
	((OPTIONS = {obj2comp: FLAG_FS}, nonvar(FLAG_FS), FLAG_FS = true)->
	 PRETERM = TERM;
	 azaltjson__changeterm(PRETERM, TERM)).

% ?- term_json(+INPUT, -TERM).
term_json(INPUT, TERM):-
	term_json({}, INPUT, TERM).

% ?- term_json(+OPTIONS, +INPUT, -TERM).
term_json(OPTIONS, INPUT, TERM):-
	fstructure(OPTIONS),
	fs_list(OPTIONS, OPTIONS_LIST),
	((OPTIONS = {without_fs: FLAG_WOFS}, nonvar(FLAG_WOFS), FLAG_WOFS = true)->
	 SANINPUT = INPUT;
	 azaltjson__sanitize_term(INPUT, SANINPUT)),
	azaltjson__term_json(OPTIONS_LIST, SANINPUT, TERM).


% 非公開API
azaltjson__changeterm([PRETERM | PRETERM残り], [TERM | TERM残り]):- !,
	% リスト
	azaltjson__changeterm(PRETERM, TERM),
	azaltjson__changeterm(PRETERM残り, TERM残り).
azaltjson__changeterm(PRETERM, TERM):- PRETERM = fs(PRETERM_PAIRLIST), !,
	% 素性構造の元
	azaltjson__changeterm(PRETERM_PAIRLIST, TERM_PAIRLIST),
	fs_list(TERM, TERM_PAIRLIST).
azaltjson__changeterm(PRETERM, TERM):- fs_av(PRETERM, ATTR, PREVALUE), !,
	% 素性構造のペア
	azaltjson__changeterm(PREVALUE, VALUE),
	fs_av(TERM, ATTR, VALUE).
azaltjson__changeterm(TERM, TERM). % それ以外


% 非公開API
azaltjson__sanitize_term([TERM | TERM残り], [SANTERM | SANTERM残り]):- nonver(TERM), !,
	% リスト
	azaltjson__sanitize_term(TERM, SANTERM),
	azaltjson__sanitize_term(TERM残り, SANTERM残り).
azaltjson__sanitize_term(TERM, SANTERM):- fstructure(TERM), !,
	% 素性構造
	fs_list(TERM, PAIRLIST),
	azaltjson__sanitize_term(PAIRLIST, SANPAIRLIST),
	SANTERM = fs(SANPAIRLIST).
azaltjson__sanitize_term(TERM, SANTERM):- fs_av(TERM, ATTR, VALUE), !,
	% 素性構造のペア
	azaltjson__sanitize_term(VALUE, SANVALUE),
	fs_av(SANTERM, ATTR, SANVALUE).
azaltjson__sanitize_term(TERM, TERM). % それ以外

