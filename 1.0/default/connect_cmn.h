#ifndef _CONNECT_CMN_H_
#define _CONNECT_CMN_H_

#include <cstddef>
#include <cstdint>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <list>
#include <string>
#include <map>
#include <cutils/log.h>


using namespace std;

#define ConnectVector std::vector
#define LogList std::list
typedef std::map<int, std::string> SocketMap;

#define info_log(fmt, arg...) ALOGE("%s: " fmt "\n", __func__, ##arg)

#define err_log(fmt, arg...) \
  ALOGE("%s: " fmt " [%d(%s)]\n", __func__, ##arg, errno, strerror(errno))

struct time_info_sync {
  uint32_t sys_cnt;
  uint32_t uptime;
}__attribute__((__packed__));

template <typename C>
void remove_at(C& c, size_t index) {
  typename C::iterator it = c.begin();
  size_t i = 0;

  while (i < index) {
    ++it;
    ++i;
  }
  c.erase(it);
}

int get_sys_gsi_flag(char* value);

#endif  // !_CONNECT_CMN_H_
