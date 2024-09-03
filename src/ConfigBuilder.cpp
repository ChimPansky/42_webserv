#include "ConfigBuilder.h"

#include <stack>

ConfigBuilder::ConfigBuilder(const char* config_path) : config_(), nesting_(), settings_(0)
{
    std::string path(config_path);
    std::ifstream _config_file;

    if (path.length() < 6 || path.substr(path.find_last_of('.') + 1) != "conf") {
        throw std::runtime_error("Invalid configuration file suffix.");
    }

    _config_file.open(config_path);
    if (!_config_file.is_open()) {
        throw std::runtime_error("Failed to open configuration file.");
    }
    settings_ = ProcessFile(_config_file);
    _config_file.close();
}

std::vector<setting> ConfigBuilder::ProcessFile(std::ifstream& _config_file)
{
    setting parsed_setting;

    std::string content;
    while (std::getline(_config_file >> std::ws, content)) {
        content.erase(content.find_last_not_of(" \t") + 1);
        if (content.empty() || content[0] == '#') {  // TODO: handle comments after settings_
            continue;
        }
        parsed_setting = MakePair(content);
        if (content.find(';') == std::string::npos - 1) {
            ParseDirective(parsed_setting);
        } else if (content.find('{') == std::string::npos - 1) {
            ParseNesting(parsed_setting);
            // TODO : check if duplicates override the value or are they invalid
        } else if (content == "}" && !nesting_.empty()) {
            nesting_.pop();
        } else {
            throw std::runtime_error("Invalid configuration file: invalid content: " + content);
        }
    }
    if (!nesting_.empty()) {
        throw std::runtime_error("Invalid configuration file: opened paranthesis.");
    }
    return settings_;
}

const Config& ConfigBuilder::Parse()
{
    Config config(settings_);
    config_ = config;

    return config_;
}

void ConfigBuilder::ParseDirective(setting parsed_setting)
{
    AppendNestingPath(parsed_setting.first, nesting_);
    settings_.push_back(parsed_setting);
}

void ConfigBuilder::ParseNesting(setting parsed_setting)
{
    AppendNestingPath(parsed_setting.first, nesting_);
    settings_.push_back(parsed_setting);
}

setting ConfigBuilder::MakePair(const std::string& line)
{
    size_t pos = line.find_first_of(" \t");
    if (pos == std::string::npos) {
        throw std::runtime_error("Invalid configuration file: no value specified.");
    }
    std::string keyword = line.substr(0, pos);

    size_t start = line.find_first_not_of(" \t", pos);
    size_t end = line.find_last_not_of(" \t", pos);
    if (start == std::string::npos || end != std::string::npos - 1) {
        throw std::runtime_error("Invalid configuration file: no value specified.");
    }
    std::string value = line.substr(start, end - start);
    return std::make_pair(keyword, value);
}

void ConfigBuilder::AppendNestingPath(std::string& token, std::stack<std::string> temp_stack)
{
    std::string nesting_path;

    while (!temp_stack.empty()) {
        nesting_path += temp_stack.top() + ":";
        temp_stack.pop();
    }
    if (!nesting_path.empty()) {
        token = nesting_path + token;
    }
}

bool ConfigBuilder::HandleParanthesis(setting& setting, const std::string& line)
{
    if (setting.first.find_first_of("{;") != std::string::npos) {
        return false;
    } else if (line.find(';') != std::string::npos &&
               setting.second[setting.second.size() - 1] != ';') {
        return false;
    } else if (line.find('{') != std::string::npos) {
        if (setting.second[setting.second.size() - 1] != '{') {
            return false;
        } else if (setting.first != "location" && setting.second != "{") {
            return false;
        } else if (setting.first != "http" && setting.first != "server" &&
                   setting.first != "location") {
            return false;
        }
    }
    return true;
}

/*const std::vector<std::string>  SplitLine(const std::string& line)
{
    std::vector<std::string> setting;
    std::istringstream ss(line);

    std::string word;
    while (ss >> word) {
        setting.push_back(word);
    }
    if (setting.size() < 2) {
        throw std::runtime_error("Invalid configuration file: setting with no value found");
    }
    return setting;
}*/
