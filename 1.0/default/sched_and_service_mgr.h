/*
 *  sched_and_ervice_mgr.h - The sced and service manager class declaration.
 *
 *  Copyright (C) 2015 Spreadtrum Communications Inc.
 *
 *  History:
 *  2019-6-24 Gloria.He
 *  Initial version.
 */
#ifndef SCHED_AND_SERVICE_MGR_H_
#define SCHED_AND_SERVICE_MGR_H_

#include "fd_hdl.h"

class SchedAndServiceManager : public FdHandler {
 public:
  enum RequestType {
    IRT_REGISTER = 0,
    IRT_UNREGISTER,
    IRT_QUIT,
  };

  struct Request {
    RequestType type;
    FdHandler* fd_hdl;
  };

  enum RespType {
    // Responses
    ITMT_REGISTER_RESULT = 0,
    ITMT_UNREGISTER_RESULT,
    ITMT_QUIT_RESULT,
    ITMT_UNKNOWN_REQ
  };
  SchedAndServiceManager(ConnectControl* ctrl, Multiplexer* multi);
  SchedAndServiceManager(const SchedAndServiceManager&) = delete;
  ~SchedAndServiceManager();

  SchedAndServiceManager& operator = (const SchedAndServiceManager&) = delete;

  int init(int fd);

  void process(int fd);

};


#endif  // !SCHED_AND_SERVICE_MGR_H_