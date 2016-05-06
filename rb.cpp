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

// execute the script
void exec_script(char *script_name, int argc, char **argv, bool show)
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
	sprintf(exec, "ruby -I%s %s", get_incdir(name), name);
	while (--argc) {
		strcat(exec, " ");
		strcat(exec, *++argv);
	}
	
	if (show) {
		puts(exec);		// show but not execute
	} else {
		system(exec);
	}
}

void usage()
{
	puts(
		"== rb v1.0 ==\n"
		"usage: rb [-show] script[.rb] [args]\n\n"
		"-show: shows the command line format without execution.\n\n"
		"* Envrionmental vaiable, RBPATH or PATH, is used to search the script file.\n"
		"* Or you can just rename rb.exe to <script>.exe to invoke the script file.\n"
	);
}

int main(int argc, char **argv)
{
	char	name[_MAX_PATH];
	bool	show = false;
	
	// change name of this exectable to .rb script file
	strcpy(name, get_scriptname(argv[0]));

	// rb.exe was used to invoke ruby script
	if (!stricmp(name, "rb.rb")) {
		if (argc > 1 && !stricmp(argv[1], "-show")) {
			show = true;
			argc--;
			argv++;
		}
		if (argc == 1) {
			usage();
			puts("no script file was specified.");
			exit(1);
		}
		strcpy(name, argv[1]);
		if (strlen(name) < 3 || stricmp(name + strlen(name) - 3, ".rb")) {
			strcat(name, ".rb");
		}
		argc--;
		argv++;
	} else if (argc > 1 && !stricmp(argv[1], "-show")) {
		argc--;
		argv++;
		show = true;
	}
	exec_script(name, argc, argv, show);
}
