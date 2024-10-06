#include <gtest/gtest.h>
#include <unistd.h>

#include <exception>
#include <iostream>
#include <stdexcept>
#include <filesystem>
#include <vector>

#include "config/ConfigBuilder.h"

TEST(ConfigTest, LoadValidConfig)
{
    std::string valid_file = "./test_configs/valid_config.conf";
    char buf[1024];
    std::cout << getcwd(buf, sizeof(buf)) << std::endl;
    config::Config conf = config::ConfigBuilder::GetConfigFromConfFile(valid_file);

    EXPECT_EQ(conf.mx_type(), c_api::MultiplexType::MT_SELECT);
    EXPECT_EQ(conf.error_log_path(), "/log/error.log");
    EXPECT_EQ(conf.error_log_level(),
              Severity::INFO);  // Assuming 'info' is mapped to `Severity::INFO`

    const config::HttpConfig& http_config = conf.http_config();

    EXPECT_EQ(http_config.keepalive_timeout(), 65);
    EXPECT_EQ(http_config.client_max_body_size(), 1 << 20);  // 1 MB in bytes
    EXPECT_EQ(http_config.error_pages().at(404), "error_pages/404.html");
    EXPECT_EQ(http_config.error_pages().at(501), "error_pages/501.html");

    const std::vector<config::ServerConfig>& server_configs = http_config.server_configs();
    for (const config::ServerConfig& server_conf : server_configs) {
        if (server_conf.listeners()[0].second == 8080) {
            EXPECT_EQ(server_conf.error_log_path(), "");
            EXPECT_EQ(server_conf.server_names()[0], "www.example.com");

            for (const config::LocationConfig& location_conf : server_conf.locations()) {
                if (location_conf.route().first == "/docs/") {
                    EXPECT_EQ(location_conf.root_dir(), "/docs");
                    EXPECT_EQ(location_conf.allowed_methods()[0],
                              config::LocationConfig::Method::GET);
                    EXPECT_EQ(location_conf.allowed_methods()[1],
                              config::LocationConfig::Method::POST);
                } else if (location_conf.route().first == "/error_pages/") {
                    EXPECT_EQ(location_conf.root_dir(), "/docs");
                    EXPECT_EQ(location_conf.dir_listing(), true);
                    EXPECT_EQ(location_conf.allowed_methods()[0],
                              config::LocationConfig::Method::GET);
                    EXPECT_EQ(location_conf.allowed_methods()[1],
                              config::LocationConfig::Method::POST);
                }
            }
        } else if (server_conf.listeners()[0].second == 8090) {
            EXPECT_EQ(server_conf.error_log_path(), "");
            EXPECT_EQ(server_conf.server_names()[0], "example.com");

            for (const config::LocationConfig& location_conf : server_conf.locations()) {
                if (location_conf.route().first == "/conf/") {
                    EXPECT_EQ(location_conf.redirect().first, 301);
                    EXPECT_EQ(location_conf.redirect().second, "/error_pages/404.html");
                } else if (location_conf.route().first == "/cgi-bin/") {
                    EXPECT_EQ(location_conf.root_dir(), "/cgi-bin");
                    EXPECT_EQ(location_conf.cgi_paths()[0], "/cgi-bin");
                    EXPECT_EQ(location_conf.cgi_extensions()[0], ".php");
                    EXPECT_EQ(location_conf.cgi_extensions()[1], ".py");
                }
            }
        }
    }
}

TEST(ConfigTest, MinimumSettingsConfig)
{
    std::string valid_file = "test_configs/minimum_settings.conf";
    config::Config conf = config::ConfigBuilder::GetConfigFromConfFile(valid_file);

    EXPECT_EQ(conf.mx_type(), c_api::MultiplexType::MT_SELECT);

    EXPECT_EQ(conf.error_log_path(), "");
    EXPECT_EQ(conf.error_log_level(), Severity::INFO);

    const config::HttpConfig& http_config = conf.http_config();

    EXPECT_EQ(http_config.keepalive_timeout(), 65);
    EXPECT_EQ(http_config.client_max_body_size(), 2ul << 20);  // 2 MB in bytes

    const std::vector<config::ServerConfig>& server_configs = http_config.server_configs();
    ASSERT_EQ(server_configs.size(), 1);

    const config::ServerConfig& server_conf = server_configs[0];

    ASSERT_EQ(server_conf.listeners().size(), 1);
    EXPECT_EQ(server_conf.listeners()[0].second, 8080);

    EXPECT_EQ(server_conf.server_names().size(), 0);
    EXPECT_EQ(server_conf.error_log_path(), "");

    EXPECT_EQ(server_conf.locations().size(), 1);
}

std::vector<std::string> GetConfigFilesFromDirectory(const std::string& directory)
{
    std::vector<std::string> config_files;
    DIR* dir;
    struct dirent* entry;

    dir = opendir(directory.c_str());
    if (dir == NULL) {
        perror("opendir");
        return config_files;
    }

    while ((entry = readdir(dir)) != NULL) {
        std::string filename = entry->d_name;

        if (filename == "." || filename == "..")
            continue;

        if (filename.find(".conf") != std::string::npos) {
            std::string file_path = directory + "/" + filename;
            config_files.push_back(file_path);
        }
    }

    closedir(dir);
    return config_files;
}

class ConfigTest : public ::testing::TestWithParam<std::string> {};

TEST_P(ConfigTest, InvalidConfigFileTest)
{
    std::string file_path = GetParam();
    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(file_path), std::exception);
}
INSTANTIATE_TEST_SUITE_P(
    InvalidConfigTests,
    ConfigTest,
    ::testing::ValuesIn(GetConfigFilesFromDirectory("test_configs/invalid_configs"))
);

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
