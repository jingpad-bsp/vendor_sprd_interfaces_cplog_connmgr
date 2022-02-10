/*
 *  client_hdl.cpp - The base class implementation for file descriptor
 *                   handler.
 *
 *  Copyright (C) 2015-2017 Spreadtrum Communications Inc.
 *
 *  History:
 *  2019-6-24 Gloria.He
 *  Initial version.
 *
 */

#include <cstring>
#include <poll.h>
#include <unistd.h>

#include "client_hdl.h"
#include "connect_cmn.h"
#include "ConnectControl.h"
#include "ConnectControlCallback.h"

ClientHandler::ClientHandler(int sock, ConnectControl* ctrl, Multiplexer* multiplexer)
    : DataProcessHandler(sock, ctrl, multiplexer, CLIENT_BUF_SIZE) {}

ClientHandler::~ClientHandler() {}

int ClientHandler::process_data() {

  char return_buf[256] ={0};
  sp<IConnectControlCallback> callback = controller()->get_callback();
  if(callback != NULL) {
    std::string pStr;
    auto cb = [&](hidl_string atResp) {
      pStr = atResp.c_str();
    };
    m_buffer.buffer[m_buffer.data_len] = '\0';
    Return<void> status = callback->onCmdreq(fd(),m_buffer.buffer,
        m_buffer.data_len,cb);

    if (!status.isOk()) {
      err_log("status callback->onCmdreq bad");
      snprintf(return_buf, sizeof(return_buf) - 1, "ERROR\n");
    } else {
      snprintf(return_buf, sizeof(return_buf) - 1, "%s", pStr.c_str());
    }
    write(fd(),return_buf,strlen(return_buf));

    m_buffer.data_len = 0;
    return 0;
  } else {
    snprintf(return_buf, sizeof(return_buf) - 1, "ERROR\n");
    write(fd(),return_buf,strlen(return_buf));
    err_log("callback null");
  }
  return 0;
}

void ClientHandler::process_conn_closed() {
  // Inform ClientManager the connection is closed
  sp<IConnectControlCallback> callback = controller()->get_callback();
  if(callback != NULL) {
    Return<void> status = callback->onSocketclose(fd());
    if(!status.isOk()) {
        err_log("close status from callback bad");
    } else {
        err_log("close status from callback ok");
    }
  } else {
    err_log("callback is null,no result return");
  }
  delete this;
}

void ClientHandler::process_conn_error(int /*err*/) {
  ClientHandler::process_conn_closed();
}
