#ifndef WS_SERVER_IPROCESSOR_H
#define WS_SERVER_IPROCESSOR_H

#include "http/Request.h"
#include "http/Response.h"
#include "utils/unique_ptr.h"

namespace config {
class ServerBlock;
class LocationBlock;
}

class IProcessor {
  public:
    virtual void ReadNext();
    virtual bool IsDone();
    virtual bool HasFailed();
    virtual ~IProcessor();
};

class FileProcessor : IProcessor {
  public:
    FileProcessor(const config::LocationBlock&, const http::Request& rq, http::Response& rs);
};

class CgiProcessor : IProcessor {
  public:
    CgiProcessor(const config::LocationBlock&, const http::Request& rq, http::Response& rs);
};

class DirProcessor : IProcessor {
  public:
    DirProcessor(const config::LocationBlock&, const http::Request& rq, http::Response& rs);
};

// + redirection
// + error pages

utils::unique_ptr<IProcessor> GetReadInterface(const config::LocationBlock&, const http::Request& rq, http::Response& rs);


// mb in cgi/fs later
// class ServerGenerateBodyCallback : c_api::ICallback {
//   public:
//     ServerGenerateBodyCallback(http::Response&);
//     void Call(int fd);
//   private:
//     http::Response& rs_;
// };

#endif  // WS_SERVER_IPROCESSOR_H
