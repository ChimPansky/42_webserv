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

std::pair<bool, int> ExtractStatus(const std::string& status_str)
{
    if (status_str.length() < 5) {
        return std::make_pair(false, 0);
    }
    std::pair<bool, int> status_or = utils::StrToNumericNoThrow<int>(status_str.substr(0, 3));
    if (!status_or.first || status_or.second < 100) {
        return std::make_pair(false, 0);
    }
    if ((status_str[4] != ' ' && status_str[4] != '\t') || !std::isalnum(status_str[5])) {
        return std::make_pair(false, 0);
    }
    return std::make_pair(true, status_or.second);
}

std::pair<bool, utils::unique_ptr<http::Response> > ParseCgiResponse(std::vector<char>& buf)
{
    std::pair<bool, utils::unique_ptr<http::Response> > res =
        std::make_pair(false, utils::unique_ptr<http::Response>(NULL));
    if (buf.empty() || buf[0] == '\n') {
        LOG(ERROR) << "Empty output or no headers in cgi output";
        return res;
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
            return res;
        }
        cur_pos = line_end + 1;
    }

    buf.erase(buf.begin(), cur_pos);
    http::ResponseCode rs_code = http::HTTP_OK;
    if (headers.find("Status") != headers.end()) {
        std::pair<bool, int> parsed_status = ExtractStatus(headers["Status"]);
        if (!parsed_status.first) {
            LOG(ERROR) << "Incorrect Status header formatting in CGI output: " << headers["Status"];
            return res;
        }
        rs_code = static_cast<http::ResponseCode>(parsed_status.second);
    }
    if (headers.find("Content-Length") == headers.end() && !buf.empty()) {
        headers["Content-Length"] = utils::NumericToString(buf.size());
    }

    return std::make_pair(true, utils::unique_ptr<http::Response>(
                                    new http::Response(rs_code, http::HTTP_1_1, headers, buf)));
}

std::vector<std::string> GetEnv(const std::string& script_path, const http::Request& rq)
{
    std::vector<std::string> env;

    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("SERVER_SOFTWARE=webserv/1.0");

    env.push_back("SERVER_PROTOCOL=" + std::string(HttpVerToStr(rq.version).second));
    env.push_back("SCRIPT_NAME=" + rq.rqTarget.path());
    env.push_back("REQUEST_METHOD=" + std::string(HttpMethodToStr(rq.method).second));

    if (rq.has_body) {
        env.push_back("CONTENT_LENGTH=" + rq.GetHeaderVal("Content-Length").second);
        env.push_back("CONTENT_TYPE=" + rq.GetHeaderVal("Content-Type").second);
    }
    env.push_back("QUERY_STRING=" + rq.rqTarget.query());
    // env.push_back("REMOTE_ADDR=" + utils::IPaddr);  TODO: pass Client Socket IP address here
    // env.push_back("SERVER_PORT=" + utils::port);  TODO: pass Master Socket port here
    env.push_back("REMOTE_HOST=" + rq.GetHeaderVal("Host").second);
    env.push_back("SERVER_NAME=" + rq.GetHeaderVal("Host").second);

    if (rq.GetHeaderVal("Authorization").first) {
        env.push_back("AUTH_TYPE=" + rq.GetHeaderVal("Authorization").second);
    }

    env.push_back("PATH_INFO=" + script_path);
    if (rq.GetHeaderVal("Authorization").first) {
        env.push_back("ACCEPT=" + rq.GetHeaderVal("Accept").second);
    }
    if (rq.GetHeaderVal("Accept").first) {
        env.push_back("ACCEPT=" + rq.GetHeaderVal("Accept").second);
    }
    env.push_back("PATH_TRANSLATED=" + script_path);
    // env.push_back("HTTP_COOKIE=" + rq.GetHeaderVal("Cookie").second); bonuses
    return env;
}

}  // namespace cgi