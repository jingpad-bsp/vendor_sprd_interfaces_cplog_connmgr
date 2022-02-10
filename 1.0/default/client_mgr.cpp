/*
 *  client_mgr.cpp - The client manager class implementation.
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
#include "client_mgr.h"
#include "connect_cmn.h"
#include "ConnectControl.h"
#include "multiplexer.h"


ClientManager::ClientManager(ConnectControl* ctrl, Multiplexer* multi)
    : FdHandler(-1, ctrl, multi) {}

ClientManager::~ClientManager() { }

int ClientManager::init(const char* serv_name) {
  int err = -1;

  m_fd = socket_local_server(serv_name, ANDROID_SOCKET_NAMESPACE_ABSTRACT,
                             SOCK_STREAM);

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
    err_log("can not create server socket");
  }

  return err;
}

void ClientManager::process(int /*fd*/) {
  // Server socket readable

  int sock = accept(m_fd, 0, 0);

  if (sock >= 0) {  // Client connection established
    // The socket returned by accept does not inherit file
    // status flags such O_NONBLOCK on Linux, so we has to
    // set it manually.
    long flags = fcntl(sock, F_GETFL);
    flags |= O_NONBLOCK;
    int err = fcntl(sock, F_SETFL, flags);
    if (-1 == err) {
      err_log("set accepted socket to O_NONBLOCK error");
    }
    ClientHandler* client =
        new ClientHandler(sock, controller(), multiplexer());
    multiplexer()->register_fd(client, POLLIN);
  }
}
