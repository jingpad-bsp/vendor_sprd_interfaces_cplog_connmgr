/*
 *  daemon_thread.cpp - daemon thread basic functions.
 *
 *  Copyright (C) 2016 Spreadtrum Communications Inc.
 *
 *  History:
 *  2019-7-02 Gloria He
 *  Initial version.
 */

#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "daemon_thread.h"
#include "multiplexer.h"
#include "sched_and_service_mgr.h"

DaemonThread::DaemonThread(ConnectControl* ctrl, Multiplexer* multiplexer)
    :run_{true},
     inited_{false},
     fd_{-1},
     thread_sock_{-1},
     sched_service_mgr_{nullptr},
     connect_ctrl_{ctrl},
     multiplexer_{multiplexer} {}

DaemonThread::~DaemonThread() {
  if (inited_) {
    stop();
    ::close(thread_sock_);
    delete sched_service_mgr_;
  }
}

int DaemonThread::init() {
   if (inited_) {
    return -1;
  }

  int sock_fds[2];
  int err = socketpair(AF_LOCAL, SOCK_STREAM, 0, sock_fds);

  if (-1 == err) {
    err_log("socketpair error");
    return -1;
  }

  fd_ = sock_fds[0];
  thread_sock_ = sock_fds[1];

  sched_service_mgr_ = new SchedAndServiceManager(connect_ctrl_,multiplexer_);
  if (sched_service_mgr_) {
    sched_service_mgr_->init(thread_sock_);
  }

  err = pthread_create(&thread_, nullptr, daemon_thread_func, this);
  if (err) {
    err_log("pthread_create error %d", err);
    goto clSock;
  }

  inited_ = true;

  return 0;

clSock:
  delete sched_service_mgr_;
  ::close(sock_fds[0]);
  ::close(sock_fds[1]);
  fd_ = -1;
  thread_sock_ = -1;
  return -1;
}

void DaemonThread::stop() {
  if (!inited_) {
    return;
  }

  quit_request();

  pthread_join(thread_, nullptr);

}

int DaemonThread::register_request(FdHandler* fd_hdl) {
  Request  m_cur_req;
  m_cur_req.type = IRT_REGISTER;
  m_cur_req.fd_hdl = fd_hdl;

  size_t nwr = write(fd_, &m_cur_req, sizeof(Request));
  if (sizeof(Request) != static_cast<size_t>(nwr)) {
    err_log("send handle register request error");
    return -1;
  }

  return 0;
}

int DaemonThread::unregister_request(FdHandler* fd_hdl) {
  Request  m_cur_req;
  m_cur_req.type = IRT_UNREGISTER;
  m_cur_req.fd_hdl = fd_hdl;

  size_t nwr = write(fd_, &m_cur_req, sizeof(Request));
  if (sizeof(Request) != static_cast<size_t>(nwr)) {
    err_log("send handle register request error");
    return -1;
  }
  while (1) {
    RespType msg;
    ssize_t nr = 0;
    nr = read(fd_, &msg, sizeof(RespType));
    if (nr > 0) {
      if (ITMT_UNREGISTER_RESULT == msg ) {
        break;
      }
    }
  }

  return 0;
}


int DaemonThread::quit_request() {
  Request m_cur_req;
  m_cur_req.type = IRT_QUIT;
  m_cur_req.fd_hdl = nullptr;

  size_t nwr = write(fd_, &m_cur_req, sizeof(Request));
  if (sizeof(Request) != static_cast<size_t>(nwr)) {
    err_log("send quit request to error");
    return -1;
  }

  while (1) {
    RespType msg;
    ssize_t nr = 0;
    nr = read(fd_, &msg, sizeof(RespType));
    if (nr > 0) {
      if (ITMT_QUIT_RESULT == msg ) {
        shall_quit();
        break;
      }
    }
  }
  return 0;
}

void* DaemonThread::daemon_thread_func(void* param) {
  DaemonThread* io_chan =  static_cast<DaemonThread*>(param);
  while (io_chan->run_) {
    io_chan->multiplexer_->run();
  }
  return nullptr;
}
