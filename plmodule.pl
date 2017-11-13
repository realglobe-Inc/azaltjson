
% ?- json_term(+INPUT, -TERM).
json_term(INPUT, TERM):-
	json_term({}, INPUT, TERM).

% ?- json_term(+OPTIONS, +INPUT, -TERM).
json_term(OPTIONS, INPUT, TERM):-
	fstructure(OPTIONS),
	fs_list(OPTIONS, OPTIONS_LIST),
	azaltjson__json_term(OPTIONS_LIST, INPUT, TERM).

% 非公開API
azaltjson__makefs:-
	azaltjson__get_prefs(PreFS),
	fs_list(FS, PreFS),
	azaltjson__set_fs(FS).
