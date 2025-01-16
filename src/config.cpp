#include "config.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

Config::Config() {
    initConfigPath();
}

void Config::initConfigPath() {
    auto home = std::filesystem::path(std::getenv("USERPROFILE"));
    config_path_ = home / ".midjourney-cli" / "config.json";
}

bool Config::load() {
    try {
        if (!std::filesystem::exists(config_path_)) {
            return false;
        }

        std::ifstream file(config_path_);
        if (!file.is_open()) {
            return false;
        }

        nlohmann::json config;
        file >> config;

        if (config.contains("base_url") && config.contains("api_key")) {
            base_url_ = config["base_url"].get<std::string>();
            api_key_ = config["api_key"].get<std::string>();
            return true;
        }

        return false;
    }
    catch (const std::exception&) {
        return false;
    }
}

bool Config::save() const {
    try {
        // 确保配置目录存在
        std::filesystem::create_directories(config_path_.parent_path());

        nlohmann::json config = {
            {"base_url", base_url_},
            {"api_key", api_key_}
        };

        std::ofstream file(config_path_);
        if (!file.is_open()) {
            return false;
        }

        file << config.dump(2);
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}