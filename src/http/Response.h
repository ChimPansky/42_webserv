#ifndef WS_HTTP_RESPONSE_H
#define WS_HTTP_RESPONSE_H

#include <vector>
#include <multiplexers/ICallback.h>
#include <ReadSocket.h>

namespace http {

enum ResponseCode {
    OK                      = 200,
    NOT_FOUND               = 403,
    SERVER_INTERNAL_ERROR   = 503,
};

class Response {
  public:
    class ResponseBuilder {
      public:
        ResponseBuilder(utils::unique_ptr<Response> rs) : rs_(rs), complete_(true) {};
        ResponseBuilder(utils::unique_ptr<Response> rs, utils::unique_ptr<c_api::ReadSocket> bodygen_sock) : rs_(rs), bodygen_socket_(bodygen_sock), complete_(false) {};
        const utils::unique_ptr<c_api::ReadSocket>& bodygen_sock() const {return bodygen_socket_;}
        bool Complete() const {return complete_;};
        void AddToBody(size_t sz, const char* buf) {
            rs_->body_.reserve(rs_->body_.size() + sz);
            std::copy(buf, buf + sz, std::back_inserter(rs_->body_));
        }
        void Finalize() {
            complete_ = true;
        }
        utils::unique_ptr<Response> GetResponse() {
            if (complete_) {
                return rs_;
            } else {
                return  utils::unique_ptr<Response>();
            }
        }
      private:
        utils::unique_ptr<Response> rs_;
        utils::unique_ptr<c_api::ReadSocket> bodygen_socket_;
        bool complete_;
    };
    Response(ResponseCode code, std::vector<char> body) : code_(code), body_(body) {}
  public:
    std::vector<char> Dump() const;
  private:
    ResponseCode code_;
    std::vector<char> body_;
};

}  // namespace http

#endif  // WS_HTTP_RESPONSE_H
