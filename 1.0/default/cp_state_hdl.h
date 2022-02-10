/*
 *  cp_state_hdl.h - The cp state class declaration.
 *
 *  Copyright (C) 2015 Spreadtrum Communications Inc.
 *
 *  History:
 *  2019-6-24 Gloria.He
 *  Initial version.
 */
#ifndef CP_STATE_HDL_H_
#define CP_STATE_HDL_H_

#include "fd_hdl.h"
#include "timer_mgr.h"

using vendor::sprd::hardware::cplog_connmgr::V1_0::implementation::ConnectControl;

class CpStateHandler : public FdHandler {
 public:
  CpStateHandler(int src_sock, int fd, ConnectControl* ctrl, Multiplexer* multi,
                 const char* sock_name);
  CpStateHandler(const CpStateHandler&) = delete;
  ~CpStateHandler();

  CpStateHandler& operator = (const CpStateHandler&) = delete;
  void init();
  void process(int fd);

 private:
  int src_sock_;
  const char* des_sock_;
  TimerManager::Timer* timer_;
  void process_state(int sock, const char *data, int len);
  void process_close();
  void process_reconn();
  static void connect_server(void* param);

};
#endif  // !CP_STATE_HDL_H_
