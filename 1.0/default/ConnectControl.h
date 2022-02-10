/*
 *  ConnectControl.h - ConnectControl class declaration.
 *
 *  Copyright (C) 2015 Spreadtrum Communications Inc.
 *
 *  History:
 *  2019-6-24 Gloria.He
 *  Initial version.
 */
 #ifndef VENDOR_SPRD_HARDWARE_CPLOGCONNMGR_V1_0_CONNECTCONTROL_H
#define VENDOR_SPRD_HARDWARE_CPLOGCONNMGR_V1_0_CONNECTCONTROL_H

#include <vendor/sprd/hardware/cplog_connmgr/1.0/types.h>
#include <vendor/sprd/hardware/cplog_connmgr/1.0/IConnectControl.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <poll.h>
#include "connect_cmn.h"
#include "timer_mgr.h"

class Multiplexer;
class ClientManager;
class CpStateHandler;
class CpTimeHandler;
class DaemonThread;

namespace vendor {
namespace sprd {
namespace hardware {
namespace cplog_connmgr {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

class ConnectControl : public IConnectControl {
  // Methods from ILogControl follow.
 public:
  ConnectControl();
  ~ConnectControl();
  Return<void> setCallback(const sp<IConnectControlCallback>& callback) override;
  Return<int> socketConnect(int srcSock, const hidl_string& desSocket) override;
  Return<int> socketClose(const hidl_string& desSocket) override;
  Return<void> socketData(int desSock, const hidl_string& cmd,
                          socketData_cb _hidl_cb) override;

  sp<IConnectControlCallback> get_callback();

  int process_socket_connect(int src_sock, const char *des_socket);
  int process_socket_data(const char *socket, const char *cmd, char *response, int size);

  void init();

 protected:
  int max_poll_num = 0;

 private:
  sp<IConnectControlCallback> call_back_;

  ClientManager* cli_mgr_;
  CpStateHandler* modem_state_;
  CpStateHandler* wcn_state_;
  CpTimeHandler* time_hdl_;
  DaemonThread* daemon_thread_;
  Multiplexer* multiplexer_;
  SocketMap sockets_;
  TimerManager::Timer* timer_;

  static void socket_reconnect(void* param);

};

extern "C" IConnectControl* HIDL_FETCH_IConnectControl(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace slogmodemconnmgr
}  // namespace hardware
}  // namespace sprd
}  // namespace vendor

#endif  // VENDOR_SPRD_HARDWARE_SLOGMODEMCONNMGR_V1_0_CONNECTCONTROL_H
