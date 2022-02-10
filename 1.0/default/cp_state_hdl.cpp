/*
 *  cp_state_hdl.cpp - The cp state class implementation.
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
#include "connect_cmn.h"
#include "cp_state_hdl.h"
#include "multiplexer.h"
#include "ConnectControl.h"
#include "ConnectControlCallback.h"


CpStateHandler::CpStateHandler(int src_sock, int fd, ConnectControl* ctrl,
                               Multiplexer* multi, const char* sock_name)
    : FdHandler(fd, ctrl,multi),
    src_sock_{src_sock},
    des_sock_{sock_name},
    timer_{0} {}

CpStateHandler::~CpStateHandler() {
  if(timer_) {
    multiplexer()->timer_mgr().del_timer(timer_);
    timer_ = 0;
  }
}

void CpStateHandler::init() {
  multiplexer()->register_fd(this, POLLIN);
}

void CpStateHandler::process(int /*fd*/) {
  // Server socket readable
  char buffer[256] = {0};
  int n = read(m_fd, buffer, sizeof(buffer));
  if (n > 0) {
    info_log("get data from server fd=%d, buffer=%s,n=%d",
             m_fd,buffer,n);
    process_state(src_sock_,(const char*)buffer,sizeof(buffer));
  } else if (n == 0) {
    process_close();
  } else if (-1 == n) {
    if (EAGAIN != errno && EINTR != errno) {
      process_close();
    }
  }
}

void CpStateHandler::process_state(int sock, const char *data, int len) {

  info_log("new cmd comes:%s", data);

  sp<IConnectControlCallback> callback = controller()->get_callback();
  if (callback != nullptr) {
    std::string pStr;
    auto cb = [&](hidl_string resp) {
      pStr = resp.c_str();
    };
    Return<void> status = callback->onCmdresp(sock,data,len,cb);
    if (!status.isOk()) {
      err_log("send command status bad");
    } else {
      info_log(" response from callback %s", pStr.c_str());
    }
  }
}

void CpStateHandler::process_reconn() {
  if (!des_sock_) {
    return;
  }

  m_fd = socket_local_client(des_sock_,
                             ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);

  if (-1 == m_fd) {
    timer_ = multiplexer()->timer_mgr().create_timer(500, connect_server, this);
  } else {
    long flags = fcntl(m_fd, F_GETFL);
    flags |= O_NONBLOCK;
    int err = fcntl(m_fd, F_SETFL, flags);
    if (-1 == err) {
      close();
      err_log("set time sync socket O_NONBLOCK fail.");
      return ;
    } else {
      multiplexer()->register_fd(this, POLLIN);
    }
  }
}

void CpStateHandler::connect_server(void* param) {
  CpStateHandler* p = static_cast<CpStateHandler*>(param);

  p->timer_ = 0;
  p->process_reconn();
}

void CpStateHandler::process_close() {
  multiplexer()->unregister_fd(this);
  close();
  process_reconn();
}

