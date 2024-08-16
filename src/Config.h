#ifndef WS_CONFIG_H
#define WS_CONFIG_H

// hopefully will be trivially-copyable
class Config {
  private:
    Config();
  public:
    Config(const char* config_path);
};

#endif  // WS_CONFIG_H
