/*
 *  service.cpp - The connect control service in vendor side.
 *
 *  Copyright (C) 2015 Spreadtrum Communications Inc.
 *
 *  History:
 *  2019-06-24 Gloria.He
 *  Initial version.
 */

#include <vendor/sprd/hardware/cplog_connmgr/1.0/IConnectControl.h>
#include <hidl/LegacySupport.h>


using vendor::sprd::hardware::cplog_connmgr::V1_0::IConnectControl;
using android::hardware::defaultPassthroughServiceImplementation;

int main() {
  // Ignore SIGPIPE to avoid to be killed by the kernel
  // when writing to a socket which is closed by the peer.
  struct sigaction siga;

  memset(&siga, 0, sizeof siga);
  siga.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &siga, 0);
  return defaultPassthroughServiceImplementation<IConnectControl>();
}
