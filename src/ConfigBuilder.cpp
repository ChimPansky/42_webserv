#include "ConfigBuilder.h"

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

const Config& ConfigBuilder::Parse()
{
    Config config(settings_);
    config_ = config;

    return config_;
}

void ConfigBuilder::PrintSettings()
{
    for (std::vector<setting>::iterator it = settings_.begin(); it != settings_.end(); it++) {
        std::cout << "[" << it->first << "] : [" << it->second << "]" << std::endl;
    }
}

std::vector<setting> ConfigBuilder::ProcessFile(std::ifstream& _config_file)
{
    setting parsed_setting;

    std::string content;
    while (std::getline(_config_file >> std::ws, content)) {
        content.erase(content.find_last_not_of(" \t") + 1);
        if (content.empty() || content[0] == '#') {  // TODO: handle comments after a setting
            continue;
        }
        parsed_setting = MakePair(content);
        if (content.find(';') == content.size() - 1) {
            ParseDirective(parsed_setting);
        } else if (content.find('{') == content.size() - 1) {
            ParseNesting(parsed_setting);
        } else if (content == "}" && !nesting_.empty()) {
            nesting_.pop();
            settings_.push_back(parsed_setting);
        } else {
            throw std::runtime_error("Invalid configuration file: invalid content: " + content);
        }
    }
    if (!nesting_.empty()) {
        throw std::runtime_error("Invalid configuration file: opened paranthesis.");
    }
    return settings_;
}

void ConfigBuilder::ParseDirective(setting& parsed_setting)
{
    std::vector<std::string> valid_tokens;

    if (nesting_.empty()) {
        valid_tokens = Config::GetMainTokens();
    } else if (nesting_.top() == "http") {
        valid_tokens = Config::GetHttpTokens();
    } else if (nesting_.top() == "server") {
        valid_tokens = ServerBlock::GetTokens();
    } else if (nesting_.top() == "location") {
        valid_tokens = LocationBlock::GetTokens();
    } else {
        throw std::runtime_error("Invalid configuration file: invalid nesting.");
    }
    if (std::find(valid_tokens.begin(), valid_tokens.end(), parsed_setting.first) ==
        valid_tokens.end()) {
        throw std::runtime_error("Invalid configuration file: invalid directive: " +
                                 parsed_setting.first);
    }
    // TODO : check if duplicates override the value or are they invalid
    settings_.push_back(parsed_setting);
}

void ConfigBuilder::ParseNesting(setting& parsed_setting)
{
    if (parsed_setting.second != "{" && parsed_setting.first != "location") {
        throw std::runtime_error("Invalid configuration file: invalid content: " +
                                 parsed_setting.second);
    } else if (parsed_setting.first == "http") {
        if (!nesting_.empty()) {
            throw std::runtime_error("Invalid configuration file: http block outside main block.");
        }
    } else if (parsed_setting.first == "server") {
        if (nesting_.empty() || nesting_.top() != "http") {
            throw std::runtime_error(
                "Invalid configuration file: server block outside http block.");
        }

    } else if (parsed_setting.first == "location") {
        if (nesting_.empty() || nesting_.top() != "server") {
            throw std::runtime_error(
                "Invalid configuration file: location block outside server block.");
        }
    } else {
        throw std::runtime_error("Invalid configuration file: invalid block: " +
                                 parsed_setting.first);
    }
    nesting_.push(parsed_setting.first);
    settings_.push_back(parsed_setting);
}

setting ConfigBuilder::MakePair(const std::string& line)
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
