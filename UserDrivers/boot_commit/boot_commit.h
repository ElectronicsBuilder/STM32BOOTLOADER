#ifndef __BOOT_COMMIT_H
#define __BOOT_COMMIT_H

#include <stdbool.h>
#include "boot_defs.h"

typedef struct {
    bool (*commit)(const staging_binary_info_t *info);
} BootCommitBackend;

bool boot_commit_from_staging(const staging_binary_info_t *info);

#endif /* __BOOT_COMMIT_H */
