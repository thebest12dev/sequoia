#pragma once
#include "SequoiaExport.h"
extern "C" {
    SEQUOIA_EXPORT void* sequoia_openWorld(const char* path);
    SEQUOIA_EXPORT bool sequoia_closeWorld(void* world);
    SEQUOIA_EXPORT int sequoia_getBackupCount(void* world);
    SEQUOIA_EXPORT void* sequoia_backupPropertyAt(int idx, const char* tagName);
}