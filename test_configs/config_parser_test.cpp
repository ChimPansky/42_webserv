#include <gtest/gtest.h>
#include <unistd.h>

#include <exception>
#include <iostream>
#include <stdexcept>
#include <filesystem>
#include <vector>

#include "config/ConfigBuilder.h"

class ConfigInvalidTest : public ::testing::TestWithParam<std::string> {};

TEST_P(ConfigInvalidTest, InvalidConfigShouldThrow)
{
    std::string file_path = GetParam();
    std::cout << "Testing invalid config: " << file_path << std::endl;
    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(file_path), std::exception);
}

std::vector<std::string> GetInvalidConfigFiles(const std::string& directory)
{
    std::vector<std::string> file_paths;
    DIR* dir;
    struct dirent* entry;

    dir = opendir(directory.c_str());
    if (dir == NULL) {
        perror("opendir");
        return file_paths;
    }

    while ((entry = readdir(dir)) != NULL) {
        std::string filename = entry->d_name;

        if (filename == "." || filename == "..")
            continue;

        if (filename.find(".conf") != std::string::npos) {
            file_paths.push_back(directory + "/" + filename);
        }
    }
    closedir(dir);
    return file_paths;
}

INSTANTIATE_TEST_SUITE_P(
    InvalidConfigTests,
    ConfigInvalidTest,
    ::testing::ValuesIn(GetInvalidConfigFiles("test_configs/invalid_configs"))
);

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
                    EXPECT_EQ(location_conf.client_max_body_size(), 5 << 20);  // 5 MB in bytes
                } else if (location_conf.route().first == "/error_pages/") {
                    EXPECT_EQ(location_conf.root_dir(), "/docs");
                    EXPECT_EQ(location_conf.dir_listing(), true);
                    EXPECT_EQ(location_conf.allowed_methods()[0],
                              config::LocationConfig::Method::GET);
                    EXPECT_EQ(location_conf.allowed_methods()[1],
                              config::LocationConfig::Method::POST);
                    EXPECT_EQ(location_conf.client_max_body_size(), 4 << 20);
                }
            }
        } else if (server_conf.listeners()[0].second == 8090) {
            EXPECT_EQ(server_conf.error_log_path(), "");
            EXPECT_EQ(server_conf.server_names()[0], "example.com");

            for (const config::LocationConfig& location_conf : server_conf.locations()) {
                if (location_conf.route().first == "/conf/") {
                    EXPECT_EQ(location_conf.redirect().first, 301);
                    EXPECT_EQ(location_conf.redirect().second, "/error_pages/404.html");
                    EXPECT_EQ(location_conf.client_max_body_size(), 1 << 20);
                } else if (location_conf.route().first == "/cgi-bin/") {
                    EXPECT_EQ(location_conf.root_dir(), "/cgi-bin");
                    EXPECT_EQ(location_conf.cgi_paths()[0], "/cgi-bin");
                    EXPECT_EQ(location_conf.cgi_extensions()[0], ".php");
                    EXPECT_EQ(location_conf.cgi_extensions()[1], ".py");
                    EXPECT_EQ(location_conf.client_max_body_size(), 1 << 20);
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

    const std::vector<config::ServerConfig>& server_configs = http_config.server_configs();
    ASSERT_EQ(server_configs.size(), 1);

    const config::ServerConfig& server_conf = server_configs[0];

    ASSERT_EQ(server_conf.listeners().size(), 1);
    EXPECT_EQ(server_conf.listeners()[0].second, 8080);

    EXPECT_EQ(server_conf.server_names().size(), 0);
    EXPECT_EQ(server_conf.error_log_path(), "");

    EXPECT_EQ(server_conf.locations().size(), 1);
}

/* TEST(ConfigTest, LoadInvalidConfigExtension)
{
    std::string invalid_file = "test_configs/invalid_extension.txt";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, LoadNonExistentConfigFile)
{
    std::string invalid_file = "test_configs/non_existent.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, LoadNonExistentConfigFile2)
{
    std::string invalid_file = "test_configs/.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, InvalidSyntax1)
{
    std::string invalid_file = "test_configs/invalid_syntax1.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, InvalidSyntax2)
{
    std::string invalid_file = "test_configs/invalid_syntax2.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, InvalidNesting1)
{
    std::string invalid_file = "test_configs/invalid_nesting1.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, InvalidNesting2)
{
    std::string invalid_file = "test_configs/invalid_nesting2.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, InvalidNesting3)
{
    std::string invalid_file = "test_configs/invalid_nesting3.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, EmptySetting)
{
    std::string invalid_file = "test_configs/empty_setting.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, UnknownSetting)
{
    std::string invalid_file = "test_configs/unknown_setting.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, NoListenSetting)
{
    std::string invalid_file = "test_configs/no_listen_setting.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, EmptyFile)
{
    std::string invalid_file = "test_configs/empty_file.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, InvalidUseSetting)
{
    std::string invalid_file = "test_configs/invalid_use.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, InvalidErrorLogSetting)
{
    std::string invalid_file = "test_configs/invalid_error_log.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, InvalidClientMaxBodySizeSetting)
{
    std::string invalid_file = "test_configs/invalid_clientmaxbodysize.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, InvalidErrorPageSetting)
{
    std::string invalid_file = "test_configs/invalid_errorpage.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, InvalidAutoIndexSetting)
{
    std::string invalid_file = "test_configs/invalid_autoindex.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, InvalidKeepAliveTimeoutSetting)
{
    std::string invalid_file = "test_configs/invalid_keepalive.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, InvalidListenSetting)
{
    std::string invalid_file = "test_configs/invalid_listen.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, InvalidServerNameSetting)
{
    std::string invalid_file = "test_configs/invalid_server_name.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, InvalidAccessLogSetting)
{
    std::string invalid_file = "test_configs/invalid_access_log.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, InvalidRootSetting)
{
    std::string invalid_file = "test_configs/invalid_root.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, InvalidRoute)
{
    std::string invalid_file = "test_configs/invalid_route.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, InvalidAllowMethodsSetting)
{
    std::string invalid_file = "test_configs/invalid_limit_except.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, InvalidCgiExtensionSetting)
{
    std::string invalid_file = "test_configs/invalid_cgi_extension.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, InvalidCgiPathSetting)
{
    std::string invalid_file = "test_configs/invalid_cgi_path.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
}

TEST(ConfigTest, InvalidReturnSetting)
{
    std::string invalid_file = "test_configs/invalid_return.conf";

    EXPECT_THROW(config::ConfigBuilder::GetConfigFromConfFile(invalid_file), std::exception);
} */

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
