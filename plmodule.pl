
azaltjson__makefs:-
	azaltjson__get_prefs(PreFS),
	fs_list(FS, PreFS),
	azaltjson__set_fs(FS).
