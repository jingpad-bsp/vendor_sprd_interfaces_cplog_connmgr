/*
 *  connect_cmn.cpp - Common functions.
 *
 *  Copyright (C) 2015 Spreadtrum Communications Inc.
 *
 *  History:
 *  2019-6-26 Gloria He
 *  Initial version.
 */

#include <cutils/properties.h>
#include <sys/types.h>
#include <unistd.h>
#include "connect_cmn.h"

int get_sys_gsi_flag(char* value) {
  property_get("ro.product.device", value, "");
  return 0;
}
