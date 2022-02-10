/*
 *  cp_time_hdl.h - The cp time sync class declaration.
 *
 *  Copyright (C) 2015 Spreadtrum Communications Inc.
 *
 *  History:
 *  2019-6-24 Gloria.He
 *  Initial version.
 */
#ifndef CP_TIME_HDL_H_
#define CP_TIME_HDL_H_

#include "fd_hdl.h"

using vendor::sprd::hardware::cplog_connmgr::V1_0::implementation::ConnectControl;

class CpTimeHandler : public FdHandler {
 public:
  CpTimeHandler(int src_sock, int fd, ConnectControl* ctrl,
                Multiplexer* multi, const char* sock_name);

  CpTimeHandler(const CpTimeHandler&) = delete;
  ~CpTimeHandler();

  CpTimeHandler& operator = (const CpTimeHandler&) = delete;

  void init();
  void process(int fd);

 private:
  int src_sock_;
  const char* des_sock_;
  void process_time(int sock, const ::vendor::sprd::hardware::cplog_connmgr::V1_0::time_sync& time,
                    int len);
  void process_close();
  void process_reconn();
};

#endif  // !CP_TIME_HDL_H_