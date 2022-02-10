/*
 *  fd_hdl.cpp - The file descriptor handler base class implementation.
 *
 *  Copyright (C) 2015 Spreadtrum Communications Inc.
 *
 *  History:
 *  2019-6-24 Gloria.He
 *  Initial version.
 */
#include <unistd.h>
#include "ConnectControl.h"
#include "fd_hdl.h"
#include "multiplexer.h"

FdHandler::FdHandler(int fd, ConnectControl* ctrl,Multiplexer* multiplexer)
    : m_fd(fd), m_log_ctrl(ctrl), m_multiplexer(multiplexer) {}

FdHandler::~FdHandler() {
  if (m_fd >= 0) {
    m_multiplexer->unregister_fd(this);
    ::close(m_fd);
  }
}

int FdHandler::close() {
  int ret = 0;

  if (m_fd >= 0) {
    ret = ::close(m_fd);
    m_fd = -1;
  }

  return ret;
}
