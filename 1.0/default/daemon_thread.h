/*
 *  daemon_thread.h - daemon thread basic functions declaration.
 *
 *  Copyright (C) 2016 Spreadtrum Communications Inc.
 *
 *  History:
 *  2019-6-29 Gloria He
 *  Initial version.
 */
#ifndef _DAEMON_THREAD_H_
#define _DAEMON_THREAD_H_

#include <pthread.h>
#include <hidl/LegacySupport.h>
#include <hidl/HidlTransportSupport.h>
#include "ConnectControl.h"

using vendor::sprd::hardware::cplog_connmgr::V1_0::implementation::ConnectControl;

class FdHandler;
class Multiplexer;
class SchedAndServiceManager;

class DaemonThread {
 public:
  // request
  enum RequestType {
    IRT_REGISTER = 0,
    IRT_UNREGISTER,
    IRT_QUIT,
  };

  struct Request;

  struct Request {
    RequestType type;
    FdHandler* fd_hdl;
  };
    // Commands and responses to the I/O thread
  enum RespType {
    // Responses
    ITMT_REGISTER_RESULT = 0,
    ITMT_UNREGISTER_RESULT,
    ITMT_QUIT_RESULT,
    ITMT_UNKNOWN_REQ
  };

  DaemonThread(ConnectControl* ctrl, Multiplexer* multiplexer);
  ~DaemonThread();

  int register_request(FdHandler* fd_hdl);
  int unregister_request(FdHandler* fd_hdl);
  int quit_request();
  void shall_quit() { run_ = false; }
  bool is_quit() const { return run_; }

  int init();
  void stop();


 private:
  bool run_;
  bool inited_;
  int fd_;
  int thread_sock_;
  pthread_t thread_;
  SchedAndServiceManager* sched_service_mgr_;
  ConnectControl* connect_ctrl_;
  Multiplexer* multiplexer_;
  static void* daemon_thread_func(void* param);
};

#endif  //!_DAEMON_THREAD_H_