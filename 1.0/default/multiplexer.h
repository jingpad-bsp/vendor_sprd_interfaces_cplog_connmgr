/*
 *  multiplexer.h - The multiplexer declaration.
 *
 *  Copyright (C) 2015 Spreadtrum Communications Inc.
 *
 *  History:
 *  2019-7-6 Gloria He
 *  Initial version.
 */
#ifndef _MULTIPLEXER_H_
#define _MULTIPLEXER_H_

#include "connect_cmn.h"
#include "fd_hdl.h"
#include "timer_mgr.h"
#include <poll.h>

class Multiplexer {
 public:
  Multiplexer();
  ~Multiplexer();

  /*
   * register_fd - Register a new handler
   *
   * Return Value:
   *   Return 0 if successful, return -1 if the handler array is
   *   full.
   */
  int register_fd(FdHandler* handler, int events);
  void unregister_fd(FdHandler* handler);
  void shall_quit() { run_ = false; }
  TimerManager& timer_mgr() { return timer_mgr_; }

  int run();

 private:

  static const unsigned POLL_ARRAY_INC_STEP = 32;

  struct PollingEntry {
    FdHandler* handler;
    int events;
  };
  bool run_;
  bool dirty_;
  size_t poll_arr_size_;
  nfds_t current_num_;
  pollfd* current_fds_;
  ConnectVector<PollingEntry> polling_hdl_;
  FdHandler** current_handlers_;
  // Timer manager
  TimerManager timer_mgr_;

  // Find handler
  size_t find_handler(FdHandler* handler);
  void prepare_polling_array();
  void remove_cur_handler(FdHandler* handler);
};

#endif  // !_MULTIPLEXER_H_
