#include "ConfigBuilder.h"

ConfigBuilder::ConfigBuilder(const std::string& config_path)
{
    if (config_path.length() < 6 ||
        config_path.substr(config_path.find_last_of('.') + 1) != "conf") {
        throw std::runtime_error("Invalid configuration file suffix.");
    }

    std::ifstream _config_file(config_path.c_str());
    if (!_config_file.is_open()) {
        throw std::runtime_error("Failed to open configuration file.");
    }
    settings_ = ProcessFile(_config_file);
}

const Config ConfigBuilder::Parse()
{
    return Config(settings_);
}

void ConfigBuilder::PrintSettings()
{
    for (std::vector<S>::iterator it = settings_.begin(); it != settings_.end(); it++) {
        std::cout << "[" << it->first << "] : [" << it->second << "]" << std::endl;
    }
}

std::vector<ConfigBuilder::S> ConfigBuilder::ProcessFile(std::ifstream& _config_file)
{
    S parsed_setting;
    std::stack<std::string> nesting_level_;

    std::string content;
    while (std::getline(_config_file >> std::ws, content)) {
        if (content.empty() || content[0] == '#') {
            continue;
        }
        if (content.find('#') != std::string::npos) {
            content.erase(content.find('#'));
        }
        content.erase(content.find_last_not_of(" \t") + 1);
        parsed_setting = MakePair(content);
        if (content.find(';') == content.size() - 1) {
            ParseDirective(parsed_setting, nesting_level_);
        } else if (content.find('{') == content.size() - 1) {
            ParseNesting(parsed_setting, nesting_level_);
        } else if (content == "}" && !nesting_level_.empty()) {
            nesting_level_.pop();
            settings_.push_back(parsed_setting);
        } else {
            throw std::runtime_error("Invalid configuration file: invalid content: " + content);
        }
    }
    if (!nesting_level_.empty()) {
        throw std::runtime_error("Invalid configuration file: opened paranthesis.");
    }
    return settings_;
}

void ConfigBuilder::ParseDirective(S& parsed_setting, std::stack<std::string>& nesting_level_)
{
    std::vector<std::string> valid_tokens =
        GetTokensByLevel(nesting_level_.empty() ? "" : nesting_level_.top());

    if (std::find(valid_tokens.begin(), valid_tokens.end(), parsed_setting.first) ==
        valid_tokens.end()) {
        throw std::runtime_error("Invalid configuration file: invalid directive: " +
                                 parsed_setting.first);
    }
    // TODO : check if duplicates override the value or are they invalid
    parsed_setting.second.erase(parsed_setting.second.find(';'));
    settings_.push_back(parsed_setting);
}

void ConfigBuilder::ParseNesting(S& parsed_setting, std::stack<std::string>& nesting_level_)
{
    if (parsed_setting.second != "{" && parsed_setting.first != "location") {
        throw std::runtime_error("Invalid configuration file: invalid content: " +
                                 parsed_setting.second);
    } else if (parsed_setting.first == "http") {
        if (!nesting_level_.empty()) {
            throw std::runtime_error("Invalid configuration file: http block outside main block.");
        }
    } else if (parsed_setting.first == "server") {
        if (nesting_level_.empty() || nesting_level_.top() != "http") {
            throw std::runtime_error(
                "Invalid configuration file: server block outside http block.");
        }

    } else if (parsed_setting.first == "location") {
        if (nesting_level_.empty() || nesting_level_.top() != "server") {
            throw std::runtime_error(
                "Invalid configuration file: location block outside server block.");
        }
    } else {
        throw std::runtime_error("Invalid configuration file: invalid block: " +
                                 parsed_setting.first);
    }
    nesting_level_.push(parsed_setting.first);
    settings_.push_back(parsed_setting);
}

ConfigBuilder::S ConfigBuilder::MakePair(const std::string& line)
{
    if (line == "}") {
        return std::make_pair(line, "");
    }
    size_t pos = line.find_first_of(" \t");
    if (pos == std::string::npos) {
        throw std::runtime_error("Invalid configuration file: no value specified.");
    }
    std::string keyword = line.substr(0, pos);

    size_t start = line.find_first_not_of(" \t", pos);
    if (start == std::string::npos) {
        throw std::runtime_error("Invalid configuration file: no value specified.");
    }
    std::string value = line.substr(start, line.size() - start);
    return std::make_pair(keyword, value);
}

const std::vector<std::string> ConfigBuilder::GetTokensByLevel(const std::string& lvl)
{
    std::vector<std::string> tokens;

    if (lvl.empty()) {
        tokens.push_back("use");
        tokens.push_back("error_log");
        return tokens;
    } else if (lvl == "http") {
        tokens.push_back("keepalive_timeout");
        tokens.push_back("client_max_body_size");
        tokens.push_back("error_page");
    } else if (lvl == "server") {
        tokens.push_back("listen");
        tokens.push_back("server_name");
        tokens.push_back("access_log");
        tokens.push_back("error_log");
    } else if (lvl == "location") {
        tokens.push_back("allow_methods");
        tokens.push_back("return");
        tokens.push_back("cgi_path");
        tokens.push_back("cgi_extension");
    } else {
        throw std::runtime_error("Invalid configuration file: invalid nesting.");
    }
    tokens.push_back("root");
    tokens.push_back("index");
    tokens.push_back("autoindex");

    return tokens;
}
