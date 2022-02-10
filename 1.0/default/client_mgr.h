/*
 *  client_mgr.h - The client manager class declaration.
 *
 *  Copyright (C) 2015 Spreadtrum Communications Inc.
 *
 *  History:
 *  2019-6-24 Gloria.He
 *  Initial version.
 */
#ifndef CLIENT_MGR_H_
#define CLIENT_MGR_H_

#include "client_hdl.h"
#include "ConnectControl.h"

using vendor::sprd::hardware::cplog_connmgr::V1_0::implementation::ConnectControl;

class ClientManager : public FdHandler {
 public:
  ClientManager(ConnectControl* ctrl, Multiplexer* multi);
  ClientManager(const ClientManager&) = delete;
  ~ClientManager();

  ClientManager& operator = (const ClientManager&) = delete;

  int init(const char* serv_name);

  void process(int fd);

  void process_client_disconn(ClientHandler* client);

};

#endif  // !CLIENT_MGR_H_
