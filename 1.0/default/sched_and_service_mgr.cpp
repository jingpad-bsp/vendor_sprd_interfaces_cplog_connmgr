/*
 *  sched_and_service_mgr.cpp - The schedule and service socket manager class implementation.
 *
 *  Copyright (C) 2015 Spreadtrum Communications Inc.
 *
 *  History:
 *  2019-6-24 Gloria.He
 *  Initial version.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>

#include <cutils/sockets.h>
#include "sched_and_service_mgr.h"
#include "connect_cmn.h"
#include "multiplexer.h"


SchedAndServiceManager::SchedAndServiceManager(ConnectControl* ctrl, Multiplexer* multi)
    : FdHandler(-1, ctrl, multi) {}

SchedAndServiceManager::~SchedAndServiceManager() { }

int SchedAndServiceManager::init(int fd) {
  int err = -1;
  m_fd = fd;

  if (m_fd >= 0) {
    long flags = fcntl(m_fd, F_GETFL);
    flags |= O_NONBLOCK;
    err = fcntl(m_fd, F_SETFL, flags);
    if (-1 == err) {
      ::close(m_fd);
      m_fd = -1;
      err_log("set server socket non-block error");
    } else {
      multiplexer()->register_fd(this, POLLIN);
    }
  } else {
    err_log("error socket");
  }

  return err;
}

void SchedAndServiceManager::process(int /*fd*/) {
  // Server socket readable
  Request req;
  ssize_t n = read(m_fd, &req, sizeof(Request));
  info_log("reqest type =%d, handle = %lu", req.type, req.fd_hdl);
  if (n > 0) {
    if (req.type == IRT_REGISTER
        && req.fd_hdl !=nullptr) {
      multiplexer()->register_fd(req.fd_hdl, POLLIN);
    } else if (req.type == IRT_QUIT) {
      RespType resp = ITMT_QUIT_RESULT;
      multiplexer()->shall_quit();
      write(m_fd,&resp,sizeof(RespType));
    } else if (req.type == IRT_UNREGISTER
               && req.fd_hdl !=nullptr) {
      RespType resp = ITMT_UNREGISTER_RESULT;
      multiplexer()->unregister_fd(req.fd_hdl);
      write(m_fd, &resp,sizeof(RespType));
    }
  }
}
