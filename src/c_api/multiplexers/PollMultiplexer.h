#ifndef WS_C_API_MULTIPLEXERS_POLL_MULTIPLEXER_H
#define WS_C_API_MULTIPLEXERS_POLL_MULTIPLEXER_H

#include "AMultiplexer.h"

namespace c_api {

class PollMultiplexer : public AMultiplexer {
  public:
    PollMultiplexer(int timeout_ms);
    void CheckOnce();

  private:
    unsigned short timeout_ms_;
};

}  // namespace c_api

#endif  // WS_C_API_MULTIPLEXERS_POLL_MULTIPLEXER_H
