#ifndef WS_C_API_MULTIPLEXERS_POLL_MULTIPLEXER_H
#define WS_C_API_MULTIPLEXERS_POLL_MULTIPLEXER_H

#include "IMultiplexer.h"

namespace c_api {

class PollMultiplexer : public IMultiplexer {
  public:
    PollMultiplexer(int timeout_ms);
    void CheckOnce(const FdToCallbackMap& rd_sockets, const FdToCallbackMap& wr_sockets);

  private:
    unsigned short timeout_ms_;
};

}  // namespace c_api

#endif  // WS_C_API_MULTIPLEXERS_POLL_MULTIPLEXER_H
