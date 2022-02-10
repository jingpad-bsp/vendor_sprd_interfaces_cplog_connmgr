/*
 *  ConnectControl.cpp - The main function for the socket communication
 *
 *  Copyright (C) 2015 Spreadtrum Communications Inc.
 *
 *  History:
 *  2019-6-24 Gloria.He
 *  Initial version.
 */

#include <ctype.h>
#include <cutils/sockets.h>
#include <cutils/properties.h>
#include <fcntl.h>
#include <errno.h>
#include <log/log.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>

#include "client_mgr.h"
#include "cp_state_hdl.h"
#include "cp_time_hdl.h"
#include "connect_cmn.h"
#include "multiplexer.h"
#include "daemon_thread.h"
#include "ConnectControl.h"
#include "ConnectControlCallback.h"

namespace vendor {
namespace sprd {
namespace hardware {
namespace cplog_connmgr {
namespace V1_0 {
namespace implementation {

using android::sp;

ConnectControl::ConnectControl()
    : cli_mgr_{0},
      modem_state_{0},
      wcn_state_{0},
      time_hdl_{0},
      daemon_thread_{0},
      multiplexer_{0},
      timer_{nullptr} {}

ConnectControl::~ConnectControl() {
  info_log("ConnectControl deconstrucition.");
  if(multiplexer_ && timer_) {
    multiplexer_->timer_mgr().del_timer(timer_);
    timer_ = nullptr;
  }
  if (daemon_thread_) {
    daemon_thread_->stop();
  }
  if (cli_mgr_) {
    delete cli_mgr_;
  }
  if (modem_state_) {
    delete modem_state_;
  }
  if (wcn_state_) {
    delete wcn_state_;
  }
  if (time_hdl_) {
    delete time_hdl_;
  }
  if (daemon_thread_) {
    delete daemon_thread_;
  }
  if (multiplexer_) {
    delete multiplexer_;
  }
}

// Methods from IConnectControl follow.
Return<void> ConnectControl::setCallback(
    const sp<IConnectControlCallback>& callback) {
  // TODO implement
  call_back_ = callback;
  return Void();
}

sp<IConnectControlCallback> ConnectControl::get_callback() {
  // TODO implement
  return call_back_;
}

Return<void> ConnectControl::socketData(int desSock, const hidl_string& cmd,
                                        socketData_cb _hidl_cb) {

  const char *data = cmd.c_str();
  hidl_string ret;
  char response[256] = {0};
  int size = sizeof(response);

  int len = write(desSock,data,strlen(data));
  if (strlen(data) == len) {
    snprintf(response, size,"%s","write socket data success");
  } else {
    snprintf(response, size,"%s","write socket data fail");
  }
  ret.setToExternal(response, strlen(response));
  _hidl_cb(ret);

  return Void();
}

Return<int> ConnectControl::socketConnect(int srcSock, const hidl_string& desSocket) {

  int fd = -1;
  const char *socket = desSocket.c_str();
  info_log("connect server socket: %s", socket);

  fd = process_socket_connect(srcSock,socket);
  if (fd > 0) {
    info_log("socket connect success");
  } else {
    err_log("socket connect fail");
  }
  return fd;
}

Return<int> ConnectControl::socketClose(const hidl_string& desSocket) {
  const char *socket = desSocket.c_str();
  info_log("src sock socket close %s",socket);

  const char* des_sock = strstr(socket,"cp_time_sync_server");

  if (des_sock) {
    daemon_thread_->unregister_request(time_hdl_);
    delete time_hdl_;
    time_hdl_ = nullptr;
  } else {
    des_sock = nullptr;
    des_sock = strstr(socket,"modemd");
    if (des_sock) {
      daemon_thread_->unregister_request(modem_state_);
      delete modem_state_;
      modem_state_ = nullptr;
    } else {
      des_sock = nullptr;
      des_sock = strstr(socket,"wcnd");
      if (des_sock) {
        daemon_thread_->unregister_request(wcn_state_);
        delete wcn_state_;
        wcn_state_ = nullptr;
      }
    }
  }
  return 0;
}

void ConnectControl::socket_reconnect(void* param) {
  std::vector<int> sock_ids; 
  int ret = 0;

  if (!param){
    err_log("param is null");
    return;
  }

  ConnectControl* ctl = static_cast<ConnectControl*>(param);
  for (auto it = ctl->sockets_.begin(); it != ctl->sockets_.end(); ++it) {
    int src_sock = it->first;
    const char *des_socket = it->second.c_str();
    info_log("socket_reconnect src_sock=%d, des_socket=%s.",src_sock, des_socket);
    ret = ctl->process_socket_connect(src_sock, des_socket);
    if(ret != -1) {
      sock_ids.push_back(src_sock);
    }
  }

  for (auto it = sock_ids.begin(); it != sock_ids.end(); ++it) {
    auto element = ctl->sockets_.find(*it);
    if (element != ctl->sockets_.end()) {
      ctl->sockets_.erase(element);
      info_log("sockets_.size = %d.",ctl->sockets_.size());
    }
  }
}

int ConnectControl::process_socket_connect(int src_sock, const char *des_socket) {

  int fd = socket_local_client(des_socket, ANDROID_SOCKET_NAMESPACE_ABSTRACT,
                                 SOCK_STREAM);
  if (fd < 0) {
    err_log("open %s failed: %s", des_socket, strerror(errno));
    sockets_[src_sock] = des_socket;
    timer_ = multiplexer_->timer_mgr().create_timer(200, socket_reconnect, this);
    return -1;
  } else {
    long flags = fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK;
    int err = fcntl(fd, F_SETFL, flags);
    if (-1 == err) {
      close(fd);
      err_log("set socket O_NONBLOCK fail.");
      sockets_[src_sock] = des_socket;
      timer_ = multiplexer_->timer_mgr().create_timer(200, socket_reconnect, this);
      return -1;
    }
  }

  info_log("open socket %s success, src socket =%d, fd = %d",
           des_socket,src_sock,fd);

  const char* des_sock = strstr(des_socket,"cp_time_sync_server");

  if (des_sock) {
    time_hdl_ = new CpTimeHandler(src_sock, fd, this, multiplexer_,"cp_time_sync_server");
    daemon_thread_->register_request(time_hdl_);
  } else {
    des_sock = nullptr;
    des_sock = strstr(des_socket,"modemd");
    if (des_sock) {
      modem_state_ = new CpStateHandler(src_sock, fd, this, multiplexer_,"modemd");
      daemon_thread_->register_request(modem_state_);
    } else {
      des_sock = nullptr;
      des_sock = strstr(des_socket,"wcnd");
      if (des_sock) {
        wcn_state_ = new CpStateHandler(src_sock, fd, this, multiplexer_,"wcnd");
        daemon_thread_->register_request(wcn_state_);
      }
    }
  }
  return fd;
}

void ConnectControl::init() {
  multiplexer_ = new Multiplexer();
  cli_mgr_ = new ClientManager(this,multiplexer_);
  if (cli_mgr_->init("slogmodem") < 0) {
    delete cli_mgr_;
    cli_mgr_ = nullptr;
  }

  daemon_thread_ = new DaemonThread(this,multiplexer_);
  daemon_thread_->init();
}

// Methods from ::android::hidl::base::V1_0::IBase follow.
IConnectControl* HIDL_FETCH_IConnectControl(const char* /* name */) {
  char device[PROPERTY_VALUE_MAX] = {0};
  get_sys_gsi_flag(device);
  ConnectControl* service = new ConnectControl();
  if (strstr(device, "generic") != nullptr) {
    info_log("gsi version  %s return service,but not run ...", device);
    return service;
  }
  service ->init();
  return service;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace cplog_connmgr
}  // namespace hardware
}  // namespace sprd
}  // namespace vendor
