#ifndef UPLOADER_H
#define UPLOADER_H

#include "config.h"

int upload_firmware();
int install_firmware();

void check_firmware_ready(const UploadConfig *cfg);
void check_cluster_sync(const UploadConfig *cfg);

#endif  // UPLOADER_H