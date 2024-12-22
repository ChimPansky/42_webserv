#include "cgi.h"

#include <ResponseCodes.h>
#include <logger.h>
#include <numeric_utils.h>
#include <str_utils.h>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

namespace cgi {

bool IsKeyValid(const std::string& key)
{
    if (key.empty()) {
        return false;
    }
    for (size_t i = 0; i < key.length(); ++i) {
        char c = key[i];
        if (!std::isalnum(c) && c != '-' && c != '_') {
            return false;
        }
    }
    return true;
}

bool IsValValid(const std::string& val)
{
    for (size_t i = 0; i < val.length(); ++i) {
        char c = val[i];
        if (c < ' ' || c > '~') {
            return false;
        }
    }
    return true;
}

bool ExtractHeaderToMap(std::map<std::string, std::string>& headers, std::string rs_line)
{
    size_t separator = rs_line.find(":");
    if (separator == std::string::npos) {
        return false;
    }
    std::string key = rs_line.substr(0, separator);
    if (!IsKeyValid(key)) {
        return false;
    }
    std::string value = rs_line.substr(separator + 1);
    value = utils::Trim(value, " \t");
    if (!IsValValid(value)) {
        return false;
    }
    if (!value.empty()) {
        headers[key] = value;
    }
    return true;
}

utils::maybe<int> ExtractStatus(const std::string& status_str)
{
    if (status_str.length() < 5) {
        return utils::maybe_not();
    }
    utils::maybe<int> status = utils::StrToNumericNoThrow<int>(status_str.substr(0, 3));
    if (!status || *status < 100) {
        return utils::maybe_not();
    }
    if ((status_str[4] != ' ' && status_str[4] != '\t') || !std::isalnum(status_str[5])) {
        return utils::maybe_not();
    }
    return *status;
}

utils::maybe<utils::unique_ptr<http::Response> > ParseCgiResponse(std::vector<char>& buf)
{
    if (buf.empty() || buf[0] == '\n') {
        LOG(ERROR) << "Empty output or no headers in cgi output";
        return utils::maybe_not();
    }

    std::map<std::string, std::string> headers;

    std::vector<char>::iterator cur_pos = buf.begin();
    while (true) {
        std::vector<char>::iterator line_end = std::find(cur_pos, buf.end(), '\n');
        if (line_end == buf.end()) {
            break;
        }

        size_t line_len = std::distance(cur_pos, line_end);
        if (line_len == 0) {
            ++cur_pos;
            break;
        }
        std::string cur_line = std::string(cur_pos.base(), line_len);
        if (!ExtractHeaderToMap(headers, cur_line)) {
            LOG(ERROR) << "Cannot parse header in CGI out: " << cur_line;
            return utils::maybe_not();
        }
        cur_pos = line_end + 1;
    }

    buf.erase(buf.begin(), cur_pos);
    http::ResponseCode rs_code = http::HTTP_OK;
    if (headers.find("Status") != headers.end()) {
        utils::maybe<int> parsed_status = ExtractStatus(headers["Status"]);
        if (!parsed_status) {
            LOG(ERROR) << "Incorrect Status header formatting in CGI output: " << headers["Status"];
            return utils::maybe_not();
        }
        // todo: check if we know http code
        rs_code = static_cast<http::ResponseCode>(*parsed_status);
    }
    if (headers.find("Content-Length") == headers.end() && !buf.empty()) {
        headers["Content-Length"] = utils::NumericToString(buf.size());
    }

    return utils::unique_ptr<http::Response>(
        new http::Response(rs_code, http::HTTP_1_1, headers, buf));
}

std::vector<std::string> GetEnv(const std::string& script_path, const http::Request& rq)
{
    std::vector<std::string> env;

    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("SERVER_SOFTWARE=webserv/1.0");

    env.push_back("SERVER_PROTOCOL=" + HttpVerToStr(rq.version));
    env.push_back("SCRIPT_NAME=" + rq.rqTarget.path());
    env.push_back("REQUEST_METHOD=" + HttpMethodToStr(rq.method));

    env.push_back("PATH_INFO=" + script_path);
    env.push_back("PATH_TRANSLATED=" + script_path);
    // env.push_back("HTTP_COOKIE=" + rq.GetHeaderVal("Cookie").second); bonuses

    env.push_back("QUERY_STRING=" + rq.rqTarget.query());
    // env.push_back("REMOTE_ADDR=" + utils::IPaddr);  TODO: pass Client Socket IP address here
    // env.push_back("SERVER_PORT=" + utils::port);  TODO: pass Master Socket port here
    env.push_back("REMOTE_HOST=" + rq.GetHeaderVal("Host").value());
    env.push_back("SERVER_NAME=" +
                  rq.GetHeaderVal("Host").value());  // TODO: REMOTE_HOST == SERVER_NAME?

    if (rq.has_body) {
        // todo: chanked?
        utils::maybe<std::string> cont_len = rq.GetHeaderVal("Content-Length");
        utils::maybe<std::string> cont_type = rq.GetHeaderVal("Content-Type");
        if (cont_len) {
            env.push_back("CONTENT_LENGTH=" + cont_len.value());
        }
        if (cont_type) {
            env.push_back("CONTENT_TYPE=" + cont_type.value());
        }
    }

    utils::maybe<std::string> auth = rq.GetHeaderVal("Authorization");
    utils::maybe<std::string> accept = rq.GetHeaderVal("Accept");
    if (auth) {
        env.push_back("AUTH_TYPE=" + *auth);
    }
    if (accept) {
        env.push_back("ACCEPT=" + *accept);
    }
    return env;
}

}  // namespace cgi
