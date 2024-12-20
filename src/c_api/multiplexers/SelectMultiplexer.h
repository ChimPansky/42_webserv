#ifndef WS_C_API_MULTIPLEXERS_SELECT_MULTIPLEXER_H
#define WS_C_API_MULTIPLEXERS_SELECT_MULTIPLEXER_H

#include <sys/time.h>

#include "IMultiplexer.h"

namespace c_api {

class SelectMultiplexer : public IMultiplexer {
  public:
    SelectMultiplexer(int timeout_ms);
    void CheckOnce(const FdToCallbackMap& rd_sockets, const FdToCallbackMap& wr_sockets);

  private:
    struct timeval* GetTimeout_();

  private:
    static int kMaxSelectFds_() { return 1023; }

    int timeout_ms_;
    struct timeval timeout_storage_;
};

}  // namespace c_api

#endif  // WS_C_API_MULTIPLEXERS_SELECT_MULTIPLEXER_H
