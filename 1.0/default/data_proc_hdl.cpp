/*
 *  data_proc_hdl.cpp - The socket data handler class implementation.
 *
 *  Copyright (C) 2015 Spreadtrum Communications Inc.
 *
 *  History:
 *  2019-6-24 Gloria.He
 *  Initial version.
 */

#include <unistd.h>
#include <errno.h>

#include "data_proc_hdl.h"

DataProcessHandler::DataProcessHandler(int sock, ConnectControl* ctrl, Multiplexer* multiplexer, size_t buf_len)
    : FdHandler{sock, ctrl, multiplexer},
      m_buffer{new char[buf_len], buf_len, 0, 0, -1} {}

void DataProcessHandler::process(int /*fd*/) {
  // Client socket connection readable

  size_t free_size = m_buffer.buf_size - m_buffer.data_start;
  ssize_t n = read(m_fd, m_buffer.buffer + m_buffer.data_start, free_size);

  if (!n) {
    // The connection is closed by the peer
    process_conn_closed();
  } else if (-1 == n) {
    if (EAGAIN != errno && EINTR != errno) {
      process_conn_error(errno);
    }
  } else {
    // Data read
    m_buffer.data_len += n;
    process_data();
  }
}
