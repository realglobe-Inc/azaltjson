
% ?- json_term(+INPUT, -TERM).
json_term(INPUT, TERM):-
	json_term({}, INPUT, TERM).

% ?- json_term(+OPTIONS, +INPUT, -TERM).
json_term(OPTIONS, INPUT, TERM):-
	fstructure(OPTIONS),
	fs_list(OPTIONS, OPTIONS_LIST),
	azaltjson__json_term(OPTIONS_LIST, INPUT, PRETERM),
	((OPTIONS = {option2fs: FALG_FS}, nonvar(FALG_FS), FALG_FS = true)->
	 azaltjson__changeterm(PRETERM, TERM);
	 PRETERM = TERM).

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
	% 素性構造のペアの元
	azaltjson__changeterm(PREVALUE, VALUE),
	fs_av(TERM, ATTR, VALUE).
azaltjson__changeterm(TERM, TERM). % それ以外

