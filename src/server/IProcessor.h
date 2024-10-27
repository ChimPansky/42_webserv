#ifndef WS_SERVER_IPROCESSOR_H
#define WS_SERVER_IPROCESSOR_H

#include <Request.h>
#include <Response.h>
#include <unique_ptr.h>

namespace config {
class ServerBlock;
class LocationBlock;
}

class IProcessor {
  public:
    // virtual void ReadNext();  // accept and add to rq or return body chank?
    // virtual bool IsDone();
    // virtual bool HasFailed();  // return error code or always internal?
    virtual ~IProcessor() {};

    // class FileProcessorCallback : public c_api::ICallback {
    //   public:
    //     void Call(int fd) {
    //       fp->ReadNext();
    //       if (fp->IsDone()) {
    //         if (fp->HasFailed()) {
    //           // replace response with error response
    //         }
    //         client.SendResponse(rs_);
    //         c_api::EventManager::ClearEvent_(fd);
    //       }
    //     }
    //   private:
    //     utils::unique_ptr<IProcessor> fp_;
    //     utils::unique_ptr<Response> rs_;
    //     ClientSession& client_;
    // }
};

#include <fcntl.h>
#include <unistd.h>
class FileProcessor : public IProcessor {
  public:
    // change back to config
    FileProcessor(const std::string& location, const http::Request&, http::Response&) {fd_ = open(location.c_str(), O_RDONLY);}
    ~FileProcessor() {close(fd_);};
  private:
    int fd_;
};

class CgiProcessor : public IProcessor {
  public:
    CgiProcessor(const config::LocationBlock&, const http::Request& rq, http::Response& rs);
};

class DirProcessor : public IProcessor {
  public:
    DirProcessor(const config::LocationBlock&, const http::Request& rq, http::Response& rs);
};

// + redirection
// + error pages

utils::unique_ptr<IProcessor> GetReadInterface(const config::LocationBlock&, const http::Request& rq, http::Response& rs) {
  return utils::unique_ptr<IProcessor>(new FileProcessor("www/index.html", rq, rs));
}


// mb in cgi/fs later
// class ServerGenerateBodyCallback : c_api::ICallback {
//   public:
//     ServerGenerateBodyCallback(http::Response&);
//     void Call(int fd);
//   private:
//     http::Response& rs_;
// };

#endif  // WS_SERVER_IPROCESSOR_H
