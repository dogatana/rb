// rb.exe
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mbstring.h>
#include <io.h>
#include <windows.h>

// search the script from RBPATH or PATH
char *search_file(char *name)
{
	char	*env;
	
	// user RBPATH or PATH
	if ((env = getenv("RBPATH")) == NULL) {
		env = getenv("PATH");
	}
	char *value = _strdup(env);
	unsigned char *tok = (unsigned char *)strtok(value, ";");
	
	while (tok) {
		static char	path[_MAX_PATH];

		if (_mbsrchr(tok, '\\') == tok + strlen((char *)tok) - 1) {
			sprintf(path, "%s%s", tok, name);
		} else {
			sprintf(path, "%s\\%s", tok, name);
		}
		if (_access(path, 0) == 0) {
			free(value);
			return path;
		}
		tok = (unsigned char *)strtok(NULL, ";");
	}
	free(value);
	return NULL;
}


// extract basename of exe file
char *get_scriptname(char *arg = NULL)
{
	static char	name[_MAX_PATH];
	char		path[_MAX_PATH];
	
	if (arg == NULL) {
		GetModuleFileName(NULL, path, sizeof(path));
	} else {
		strcpy(path, arg);
	}
	_splitpath(path, NULL, NULL, name, NULL); 
	strcat(name, ".rb");
	return name;
}

// extract directory path of the script file
char *get_incdir(char *name)
{
	static char	path[_MAX_PATH];
	char	drive[4], dir[_MAX_PATH];
	
	_fullpath(path, name, sizeof(path));
	_splitpath(path, drive, dir, NULL, NULL);
	strcpy(path, drive);
	strcat(path, dir);
	return path;
}

void exec_script(char *script_name, int argc, char **argv, bool show, bool win)
// execute the script
{
	char	*name;
	
	if (_access(script_name, 0) == 0) {
		// search current folder
		name = script_name;
	} else if ((name = search_file(script_name)) == NULL) {
		// search RBPATH or PATH
		printf("could not find %s in %s\n", 
			script_name, getenv("RBPATH")? "RBPATH": "PATH");
		exit(1);
	}
		
	char	exec[4096];
	sprintf(exec, "ruby%s -I'%s' '%s'", (win? "w": ""), get_incdir(name), name);
	while (--argc) {
		strcat(exec, " ");
		strcat(exec, *++argv);
	}
	//printf("# exec %s\n", exec);
	//exit(1);
	if (show) {
		puts(exec);		// show but not execute
	} else {
		system(exec);
	}
}

void usage()
{
	puts(
		"== rb v1.2 ==\n"
		"usage: rb [-s|-w] script[.rb] [args]\n\n"
		"-s: shows the command line format without execution.\n"
		"-w: use rubyw.exe instead of ruby.exe.\n\n"
		"* Envrionmental vaiable, RBPATH or PATH, is used to search the script file.\n"
		"* Or you can just rename rb.exe to <script>.exe to invoke the script file.\n"
	);
}

int main(int argc, char **argv)
{
	char	name[_MAX_PATH];
	bool	show = false;
	bool	win = false;
	bool	cmd = true;
	
	// change name of this exectable to .rb script file
	strcpy(name, get_scriptname(argv[0]));

	// rb.exe was used to invoke ruby script
	if (!stricmp(name, "rb.rb")) {
		cmd = false;
		argc--;
		argv++;
	}
	while (argc && argv[0][0] == '-') {
		char *opt = *argv + 1;
		while (*opt) {
			switch (*opt++) {
			case 's':
			case 'S':
				show = true;
				break;
			case 'w':
			case 'W':
				win = true;
				break;
			default:
				usage();
				exit(1);
			}
		}
		argc--;
		argv++;
	}
	if (!argc) {
		usage();
		puts("no script file was specified.");
		exit(1);
	}
	strcpy(name, *argv);
	if (strlen(name) < 3 || stricmp(name + strlen(name) - 3, ".rb")) {
		strcat(name, ".rb");
	}
	
	if (cmd && (*argv)[strlen(*argv) - 1] == 'w') {
		win = true;
	}
	
//	printf("# name: %s\n", name);
//	for (int i = 0; i < argc; i++) {
//		printf("%d: %s\n", i, *argv++);
//	}
//	printf("show: %s\n", show ? "true": "false");
//	printf("win: %s\n", win? "true": "false");
//	printf("cmd: %s\n", cmd? "true": "false");
//	
	exec_script(name, argc, argv, show, win);
}
