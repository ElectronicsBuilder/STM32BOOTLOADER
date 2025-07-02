#ifndef __COMMIT_INTERNAL_FLASH_H
#define __COMMIT_INTERNAL_FLASH_H

#include "boot_commit.h"

extern const BootCommitBackend commit_internal_backend;

bool commit_internal_init(const staging_binary_info_t *info);
bool commit_internal_write_chunk(uint32_t offset, const uint8_t* data, uint32_t len);
bool commit_internal_finalize(void);

#endif
