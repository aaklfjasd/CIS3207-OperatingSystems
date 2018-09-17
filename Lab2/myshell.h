#ifndef MYSHELL_H_FILE
#define MYSHELL_H_FILE

char *Prompt_Read();
char **parser(char*);
int myexec(char **);
int prog(char **, int[]);
int progredirect(char **, int []);
void shellmain(char **);
int *flagger(char **);

int shell_cd(char **args);
int shell_clr();
int shell_dir(char **args);
int shell_environ();
int shell_echo(char **args);
int shell_help();
int shell_pause();
int shell_quit();
int (*builtinPointer[]) (char **);

#endif
