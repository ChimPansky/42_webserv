#ifndef WS_CONFIG_H
#define WS_CONFIG_H

class Config {
  private:
    Config();
    Config(const Config&);
    Config& operator=(const Config&);
  public:
    Config(const char* config_path);
  private:
    // ?
};

#endif  // WS_CONFIG_H
