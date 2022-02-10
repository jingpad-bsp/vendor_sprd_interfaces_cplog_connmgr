/*
 *  multiplexer.cpp - The multiplexer implementation.
 *
 *  Copyright (C) 2015 Spreadtrum Communications Inc.
 *
 *  History:
 *  2019-7-3 Gloria.He
 *  Initial version.
 */
#include <unistd.h>

#include "multiplexer.h"

Multiplexer::Multiplexer()
    : run_{true},
      dirty_{true},
      poll_arr_size_{0},
      current_num_{0},
      current_fds_{0},
      current_handlers_{0} {}

Multiplexer::~Multiplexer() {
  delete [] current_fds_;
  delete [] current_handlers_;
}

size_t Multiplexer::find_handler(FdHandler* handler) {
  size_t i;

  for (i = 0; i < polling_hdl_.size(); ++i) {
    if (polling_hdl_[i].handler == handler) {
      break;
    }
  }

  return i;
}

int Multiplexer::register_fd(FdHandler* handler, int events) {
  // First find the entry
  size_t i = find_handler(handler);
  int ret = -1;

  if (i == polling_hdl_.size()) {
    PollingEntry e;

    e.handler = handler;
    e.events = events;
    polling_hdl_.push_back(e);
    dirty_ = true;
    ret = 0;
  }

  return ret;
}

void Multiplexer::unregister_fd(FdHandler* handler) {
  // First find the entry
  size_t i = find_handler(handler);

  if (i < polling_hdl_.size()) {
    remove_cur_handler(handler);
    remove_at< ConnectVector<PollingEntry> >(polling_hdl_, i);
    dirty_ = true;
  }
}

void Multiplexer::remove_cur_handler(FdHandler* handler) {
  // Don't change m_current_num, just reset the entry
  for (unsigned i = 0; i < current_num_; ++i) {
    if (current_handlers_[i] == handler) {
      current_handlers_[i] = nullptr;
      current_fds_[i].revents = 0;
      break;
    }
  }
}

void Multiplexer::prepare_polling_array() {
  if (polling_hdl_.size() > poll_arr_size_) {
    size_t step_times = (polling_hdl_.size() + POLL_ARRAY_INC_STEP - 1)
                          / POLL_ARRAY_INC_STEP;
    size_t new_size = step_times * POLL_ARRAY_INC_STEP;
    pollfd* new_poll = new pollfd[new_size];

    delete [] current_fds_;
    current_fds_ = new_poll;

    FdHandler** new_hdl = new FdHandler*[new_size];
    delete [] current_handlers_;
    current_handlers_ = new_hdl;

    poll_arr_size_ = new_size;
  }

  nfds_t i;
  size_t j = 0;

  for (i = 0; i < polling_hdl_.size(); ++i) {
    int events = polling_hdl_[i].events;

    if (events) {
      current_fds_[j].fd = polling_hdl_[i].handler->fd();
      current_fds_[j].events = events;
      current_fds_[j].revents = 0;
      current_handlers_[j] = polling_hdl_[i].handler;
      ++j;
    }
  }

  current_num_ = j;
}

int Multiplexer::run() {

  while (run_) {
    if (dirty_) {
      // Fill m_current_fds, m_current_handlers according to
      // m_polling_hdl
      prepare_polling_array();

      dirty_ = false;
    }
    int to;

    if (timer_mgr_.next_time(to) < 0) {
      to = -1;
    }

    int err = poll(current_fds_, current_num_, to);
    timer_mgr_.run();

    // Here process the events
    if (err > 0) {
      for (unsigned i = 0; i < current_num_; ++i) {
        short revents = current_fds_[i].revents;
        if (revents) {
          current_handlers_[i]->process(current_fds_[i].fd);
          if (!run_) {
            break;
          }
        }
      }
    }
  }

  return 0;
}
