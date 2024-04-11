#ifndef _WISH_U_H_
#define _WISH_U_H_

void execute_exit(char *args);
void execute_cd(char *newpath);
void execute_path(char newpaths, char ***mypath);
int wish_launch_redirect(char **args, char *file);

#endif /* WISH_UTILS_H */