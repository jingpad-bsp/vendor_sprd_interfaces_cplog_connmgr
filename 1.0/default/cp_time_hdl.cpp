/*
 *  cp_time_hdl.cpp - The cp time sync class implementation.
 *
 *  Copyright (C) 2015 Spreadtrum Communications Inc.
 *
 *  History:
 *  2019-6-24 Gloria.He
 *  Initial version.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>

#include <cutils/sockets.h>
#include "cp_time_hdl.h"
#include "connect_cmn.h"
#include "multiplexer.h"
#include "ConnectControl.h"
#include "ConnectControlCallback.h"


CpTimeHandler::CpTimeHandler(int src_sock, int fd, ConnectControl* ctrl,
                             Multiplexer* multi, const char* sock_name)
    : FdHandler(fd, ctrl, multi),
    src_sock_{src_sock},
    des_sock_{sock_name} {}

CpTimeHandler::~CpTimeHandler() { }

void CpTimeHandler::init() {
  multiplexer()->register_fd(this, POLLIN);
}

void CpTimeHandler::process(int /*fd*/) {
  // Server socket readable
  time_info_sync cp_time;
  int n = read(m_fd, &cp_time, sizeof(struct time_info_sync));
  if (n > 0) {
    info_log("get time info sys_cnt=%d,uptime=%d,n=%d",
             cp_time.sys_cnt,cp_time.uptime,n);

    vendor::sprd::hardware::cplog_connmgr::V1_0::time_sync timeinfo;
    timeinfo.sys_cnt = cp_time.sys_cnt;
    timeinfo.uptime = cp_time.uptime;
    process_time(src_sock_,timeinfo,sizeof(timeinfo));
  } else if (n == 0) {
    process_close();
  } else if (-1 == n) {
    if (EAGAIN != errno && EINTR != errno) {
      process_close();
    }
  }
}

void CpTimeHandler::process_time(int sock,
                                  const ::vendor::sprd::hardware::cplog_connmgr::V1_0::time_sync& time,
                                  int len) {

  info_log("send time data to callback:sys_cnt=%d, uptime=%d",
           time.sys_cnt, time.uptime);

  sp<IConnectControlCallback> callback = controller()->get_callback();

  if (callback != NULL) {
    Return<void> status = callback->onTimeresp(sock,time,len);
    if (!status.isOk()) {
      err_log("send time status bad");
    } else {
      info_log("send time to callback success");
    }
  } else {
      err_log("callback is null");
  }
}

void CpTimeHandler::process_reconn() {
  int fd = -1;
  if (!des_sock_) {
    return;
  }

  while (1) {
    fd = socket_local_client(des_sock_, ANDROID_SOCKET_NAMESPACE_ABSTRACT,
                                 SOCK_STREAM);
    if (fd < 0) {
      err_log("open %s failed: %s\n", des_sock_, strerror(errno));
      usleep(200*1000);
    } else {
      long flags = fcntl(fd, F_GETFL);
      flags |= O_NONBLOCK;
      int err = fcntl(fd, F_SETFL, flags);
      if (-1 == err) {
        ::close(fd);
        err_log("set socket O_NONBLOCK fail.");
        continue;
      }
      break;
    }
  }
  m_fd = fd;
  multiplexer()->register_fd(this,POLLIN);
}

void CpTimeHandler::process_close() {
  multiplexer()->unregister_fd(this);
  close();
  //no use, connect again if refnotify restart
  //  process_reconn();
}