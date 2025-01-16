#pragma once

#include <string>
#include <filesystem>

class Config {
public:
    Config();
    
    // 加载和保存配置
    bool load();
    bool save() const;
    
    // Getters
    std::string getBaseUrl() const { return base_url_; }
    std::string getApiKey() const { return api_key_; }
    
    // Setters
    void setBaseUrl(const std::string& url) { base_url_ = url; }
    void setApiKey(const std::string& key) { api_key_ = key; }
    
private:
    std::string base_url_;
    std::string api_key_;
    std::filesystem::path config_path_;
    
    // 初始化配置文件路径
    void initConfigPath();
};