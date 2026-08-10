#ifndef CD_H
#define CD_H

#include <psxcd.h>
#include <sys/types.h>

void cd_init(void);

char* cd_load_file(const char *filename, u_long *length);
void cd_unload_file(char *buffer);

#endif
