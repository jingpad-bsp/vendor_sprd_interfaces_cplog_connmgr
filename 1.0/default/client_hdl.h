/*
 *  client_hdl.h - The base class declaration for file descriptor
 *                 handler.
 *
 *  Copyright (C) 2015 Spreadtrum Communications Inc.
 *
 *  History:
 *  2019-6-24 Gloria.He
 *  Initial version.
 */

#ifndef CLIENT_HDL_H_
#define CLIENT_HDL_H_

#include "data_proc_hdl.h"

using vendor::sprd::hardware::cplog_connmgr::V1_0::implementation::ConnectControl;


class ClientHandler : public DataProcessHandler {
 public:

  ClientHandler(int sock, ConnectControl* ctrl,Multiplexer* multiplexer);
  ClientHandler(const ClientHandler&) = delete;
  ~ClientHandler();

  ClientHandler& operator = (const ClientHandler&) = delete;


 private:
  int process_data();
  void process_conn_closed();
  void process_conn_error(int err);

  static const size_t CLIENT_BUF_SIZE = 256;

};

#endif  // !CLIENT_HDL_H_
