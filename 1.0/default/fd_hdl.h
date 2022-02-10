/*
 *  fd_hdl.h - The file descriptor handler base class declaration.
 *
 *  Copyright (C) 2015 Spreadtrum Communications Inc.
 *
 *  History:
 *  2019-06-24 Gloria.He
 *  Initial version.
 */
#ifndef FD_HDL_H_
#define FD_HDL_H_

#include <hidl/LegacySupport.h>
#include <hidl/HidlTransportSupport.h>
#include "ConnectControl.h"

using vendor::sprd::hardware::cplog_connmgr::V1_0::implementation::ConnectControl;

class Multiplexer;

class FdHandler {
 public:
  FdHandler(int fd, ConnectControl* ctrl, Multiplexer* multiplexer);
  FdHandler(const FdHandler&) = delete;

  virtual ~FdHandler();

  FdHandler& operator = (const FdHandler&) = delete;

  int fd() const { return m_fd; }
  ConnectControl* controller() const { return m_log_ctrl; }
  Multiplexer* multiplexer() const { return m_multiplexer; }
  int close();

  // Events handler
  virtual void process(int fd) = 0;

 protected:
  int m_fd;

 private:
  ConnectControl* m_log_ctrl;
  Multiplexer* m_multiplexer;
};

#endif  // !FD_HDL_H_
