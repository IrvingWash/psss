#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include "cd.h"
#include <psxcd.h>
#include <sys/types.h>

#define SECTOR_MULTIPLIER 2048

void cd_init(void) {
    CdInit();
}

char* cd_load_file(const char *filename, u_long *length) {
    CdlFILE file;

    assert(CdSearchFile(&file, filename) != NULL);

    int sector_count = (file.size + (SECTOR_MULTIPLIER - 1)) / SECTOR_MULTIPLIER;

    char *buffer = (char *) malloc(SECTOR_MULTIPLIER * sector_count);

    CdControl(CdlSetloc, (u_char*) &file.pos, 0);
    CdRead(sector_count, (u_long*) buffer, CdlModeSpeed);
    CdReadSync(0, 0);

    *length = file.size;

    return buffer;
}

void cd_unload_file(char *buffer) {
    free(buffer);
};
