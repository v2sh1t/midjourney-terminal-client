#include "midjourney_client.hpp"
#include <iostream>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>

MidjourneyClient::MidjourneyClient(const std::string& base_url, const std::string& api_key)
    : base_url_(base_url), api_key_(api_key) {
    if (base_url_.empty() || api_key_.empty()) {
        throw std::runtime_error("Base URL和API Key不能为空");
    }
    // 确保base_url以/结尾
    if (base_url_.back() != '/') {
        base_url_ += '/';
    }
}

std::string MidjourneyClient::modeToString(MJMode mode) {
    switch (mode) {
        case MJMode::RELAX: return "RELAX";
        case MJMode::FAST: return "FAST";
        case MJMode::TURBO: return "TURBO";
        default: return "RELAX"; // 默认使用RELAX模式
    }
}

void MidjourneyClient::cleanup() {
    // 如果有当前任务，将其添加到历史记录
    if (!current_task_id_.empty()) {
        task_history_.push_back(current_task_id_);
    }
    // 清理当前状态
    current_task_id_.clear();
    current_buttons_ = nlohmann::json::array();
}

void MidjourneyClient::reset() {
    cleanup();
    // 清理历史记录
    task_history_.clear();
    std::cout << "已重置会话，可以开始新的绘画\n";
}

std::string MidjourneyClient::imagine(const std::string& prompt, const std::string& mode) {
    // 清理之前的状态
    cleanup();
    
    nlohmann::json modes = nlohmann::json::array();
    if (!mode.empty()) {
        modes.push_back(mode);
    }
    
    nlohmann::json payload = {
        {"prompt", prompt},
        {"accountFilter", {
            {"modes", modes}
        }}
    };
    
    try {
        auto response = makeRequest("mj/submit/imagine", payload);
        
        if (response["code"].get<int>() == 1) {
            current_task_id_ = response["result"].get<std::string>();
            std::cout << "已提交Imagine任务，任务ID: " << current_task_id_ << std::endl;
            
            // 开始监控任务状态
            monitorTask(current_task_id_);
            return current_task_id_;
        } else {
            std::cerr << "提交失败: " << response["description"].get<std::string>() << std::endl;
            return "";
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Imagine请求失败: " << e.what() << std::endl;
        return "";
    }
}

std::string MidjourneyClient::blend(const std::vector<std::string>& image_paths, const std::string& mode) {
    // 检查图片数量
    if (image_paths.size() < 2) {
        throw std::runtime_error("Blend至少需要2张图片");
    }
    
    // 转换所有图片为base64
    std::vector<std::string> base64Array;
    for (const auto& path : image_paths) {
        try {
            base64Array.push_back("data:image/png;base64," + imageToBase64(path));
        }
        catch (const std::exception& e) {
            throw std::runtime_error("图片处理失败: " + std::string(e.what()));
        }
    }
    
    // 构建请求体
    nlohmann::json payload = {
        {"botType", "MID_JOURNEY"},
        {"base64Array", base64Array},
        {"dimensions", "SQUARE"},
        {"accountFilter", {
            {"channelId", ""},
            {"instanceId", ""},
            {"modes", nlohmann::json::array()},
            {"remark", ""},
            {"remix", true},
            {"remixAutoConsidered", true}
        }},
        {"notifyHook", ""},
        {"state", ""}
    };
    
    // 添加模式
    if (!mode.empty()) {
        payload["accountFilter"]["modes"] = {mode};
    }
    
    try {
        auto response = makeRequest("mj/submit/blend", payload);
        
        if (response["code"].get<int>() == 1) {
            cleanup();  // 清理之前的状态
            current_task_id_ = response["result"].get<std::string>();
            std::cout << "已提交Blend任务，任务ID: " << current_task_id_ << std::endl;
            
            // 开始监控任务状态
            monitorTask(current_task_id_);
            return current_task_id_;
        } else {
            std::cerr << "提交失败: " << response["description"].get<std::string>() << std::endl;
            return "";
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Blend请求失败: " << e.what() << std::endl;
        return "";
    }
}

std::string MidjourneyClient::describe(const std::string& image_path, const std::string& mode) {
    // 清理之前的状态
    cleanup();
    
    try {
        // 转换图片为base64
        std::string base64 = "data:image/png;base64," + imageToBase64(image_path);
        
        // 构建请求体
        nlohmann::json payload = {
            {"botType", "MID_JOURNEY"},
            {"base64", base64},
            {"accountFilter", {
                {"channelId", ""},
                {"instanceId", ""},
                {"modes", nlohmann::json::array()},
                {"remark", ""},
                {"remix", true},
                {"remixAutoConsidered", true}
            }},
            {"notifyHook", ""},
            {"state", ""}
        };
        
        // 添加模式
        if (!mode.empty()) {
            payload["accountFilter"]["modes"] = {mode};
        }
        
        auto response = makeRequest("mj/submit/describe", payload);
        
        if (response["code"].get<int>() == 1) {
            current_task_id_ = response["result"].get<std::string>();
            std::cout << "已提交Describe任务，任务ID: " << current_task_id_ << std::endl;
            
            // 开始监控任务状态
            monitorTask(current_task_id_);
            return current_task_id_;
        } else {
            std::cerr << "提交失败: " << response["description"].get<std::string>() << std::endl;
            return "";
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Describe请求失败: " << e.what() << std::endl;
        return "";
    }
}
    
    // 转换所有图片为base64
    std::vector<std::string> base64Array;
    for (const auto& path : image_paths) {
        try {
            base64Array.push_back("data:image/png;base64," + imageToBase64(path));
        }
        catch (const std::exception& e) {
            throw std::runtime_error("图片处理失败: " + std::string(e.what()));
        }
    }
    
    // 构建请求体
    nlohmann::json payload = {
        {"botType", "MID_JOURNEY"},
        {"base64Array", base64Array},
        {"dimensions", "SQUARE"},
        {"accountFilter", {
            {"channelId", ""},
            {"instanceId", ""},
            {"modes", nlohmann::json::array()},
            {"remark", ""},
            {"remix", true},
            {"remixAutoConsidered", true}
        }},
        {"notifyHook", ""},
        {"state", ""}
    };
    
    // 添加模式
    if (!mode.empty()) {
        payload["accountFilter"]["modes"] = {mode};
    }
    
    try {
        auto response = makeRequest("mj/submit/blend", payload);
        
        if (response["code"].get<int>() == 1) {
            cleanup();  // 清理之前的状态
            current_task_id_ = response["result"].get<std::string>();
            std::cout << "已提交Blend任务，任务ID: " << current_task_id_ << std::endl;
            
            // 开始监控任务状态
            monitorTask(current_task_id_);
            return current_task_id_;
        } else {
            std::cerr << "提交失败: " << response["description"].get<std::string>() << std::endl;
            return "";
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Blend请求失败: " << e.what() << std::endl;
        return "";
    }
}

bool MidjourneyClient::handleButton(const std::string& action) {
    if (current_task_id_.empty()) {
        std::cout << "错误: 没有活动的任务，请先使用imagine命令创建新图像\n";
        return false;
    }

    std::string customId = findCustomId(action);
    if (customId.empty()) {
        std::cout << "错误: 找不到对应的操作按钮\n";
        return false;
    }

    nlohmann::json payload = {
        {"chooseSameChannel", true},
        {"customId", customId},
        {"taskId", current_task_id_},
        {"accountFilter", {
            {"channelId", ""},
            {"instanceId", ""},
            {"modes", nlohmann::json::array()},
            {"remark", ""},
            {"remix", true},
            {"remixAutoConsidered", true}
        }},
        {"notifyHook", ""},
        {"state", ""}
    };

    try {
        auto response = makeRequest("mj/submit/action", payload);
        if (response["code"].get<int>() == 1) {
            cleanup();  // 清理旧的任务ID
            current_task_id_ = response["result"].get<std::string>();
            std::cout << "已提交" << action << "任务，新任务ID: " << current_task_id_ << std::endl;
            monitorTask(current_task_id_);
            return true;
        } else {
            std::cerr << "操作失败: " << response["description"].get<std::string>() << std::endl;
            return false;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "操作请求失败: " << e.what() << std::endl;
        return false;
    }
}

void MidjourneyClient::monitorTask(const std::string& taskId) {
    std::cout << "正在等待任务完成...\n";
    
    while (true) {
        try {
            nlohmann::json payload = {
                {"ids", {taskId}}
            };
            auto taskStatus = makeRequest("mj/task/list-by-condition", payload);
            
            if (taskStatus.empty() || taskStatus[0].empty()) {
                std::cout << "无法获取任务状态，将重试...\n";
                std::this_thread::sleep_for(std::chrono::seconds(2));
                continue;
            }

            auto& task = taskStatus[0];
            std::string status = task["status"].get<std::string>();
            std::string progress = task["progress"].get<std::string>();

            if (status == "SUCCESS") {
                std::string action = task["action"].get<std::string>();
                if (action == "DESCRIBE") {
                    std::cout << "\n描述结果:\n";
                    if (task.contains("properties") &&
                        task["properties"].contains("prompt")) {
                        std::cout << task["properties"]["prompt"].get<std::string>() << std::endl;
                    }
                } else {
                    displayTaskResult(task);
                }
                break;
            } else if (status == "FAILED") {
                std::cerr << "任务失败: " << task["failReason"].get<std::string>() << std::endl;
                break;
            } else {
                std::cout << "\r进度: " << progress << std::flush;
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
        }
        catch (const std::exception& e) {
            std::cerr << "监控任务状态失败: " << e.what() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }
}

void MidjourneyClient::displayTaskResult(const nlohmann::json& task) {
    std::cout << "\n任务完成！\n";
    std::cout << "图片链接: " << task["imageUrl"].get<std::string>() << "\n\n";
    
    // 保存按钮信息供后续使用
    current_buttons_ = task["buttons"];
    
    std::cout << "可用操作:\n";
    displayButtons(current_buttons_);
}

void MidjourneyClient::displayButtons(const nlohmann::json& buttons) {
    std::cout << "\n可用操作:\n";
    for (const auto& button : buttons) {
        std::string label = button["label"].get<std::string>();
        std::string emoji = button["emoji"].get<std::string>();
        
        if (!label.empty()) {
            if (label == "Upscale (Subtle)") {
                std::cout << "US - " << label << "\n";
            }
            else if (label == "Upscale (Creative)") {
                std::cout << "UC - " << label << "\n";
            }
            else if (label == "Vary (Subtle)") {
                std::cout << "VS - " << label << "\n";
            }
            else if (label == "Vary (Strong)") {
                std::cout << "VL - " << label << "\n";
            }
            else if (label == "Vary (Region)") {
                std::cout << "VR - " << label << "\n";
            }
            else if (label == "Zoom Out 2x") {
                std::cout << "Z2 - " << label << "\n";
            }
            else if (label == "Zoom Out 1.5x") {
                std::cout << "Z1.5 - " << label << "\n";
            }
            else if (label == "Custom Zoom") {
                std::cout << "ZC - " << label << "\n";
            }
        }
        else if (!emoji.empty()) {
            if (emoji == "⬅️") std::cout << "LEFT - 向左平移\n";
            else if (emoji == "➡️") std::cout << "RIGHT - 向右平移\n";
            else if (emoji == "⬆️") std::cout << "UP - 向上平移\n";
            else if (emoji == "⬇️") std::cout << "DOWN - 向下平移\n";
            else if (emoji == "❤️") std::cout << "LIKE - 收藏图片\n";
        }
    }
    
    std::cout << "\n使用方式：输入对应的命令代码（例如：US, VL, Z2等）\n";
}

std::string MidjourneyClient::findCustomId(const std::string& action) {
    if (current_buttons_.empty()) {
        return "";
    }

    for (const auto& button : current_buttons_) {
        std::string label = button["label"].get<std::string>();
        std::string emoji = button["emoji"].get<std::string>();
        
        // 匹配命令
        bool matches = false;
        if (action == "US" && label == "Upscale (Subtle)") matches = true;
        else if (action == "UC" && label == "Upscale (Creative)") matches = true;
        else if (action == "VS" && label == "Vary (Subtle)") matches = true;
        else if (action == "VL" && label == "Vary (Strong)") matches = true;
        else if (action == "VR" && label == "Vary (Region)") matches = true;
        else if (action == "Z2" && label == "Zoom Out 2x") matches = true;
        else if (action == "Z1.5" && label == "Zoom Out 1.5x") matches = true;
        else if (action == "ZC" && label == "Custom Zoom") matches = true;
        else if (action == "LEFT" && emoji == "⬅️") matches = true;
        else if (action == "RIGHT" && emoji == "➡️") matches = true;
        else if (action == "UP" && emoji == "⬆️") matches = true;
        else if (action == "DOWN" && emoji == "⬇️") matches = true;
        else if (action == "LIKE" && emoji == "❤️") matches = true;
        
        if (matches) {
            return button["customId"].get<std::string>();
        }
    }
    return "";
}

std::string MidjourneyClient::imageToBase64(const std::string& image_path) {
    std::ifstream file(image_path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("无法打开图片文件: " + image_path);
    }
    
    // 读取文件内容
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(file), {});
    file.close();
    
    // 转换为base64
    static const std::string base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
        
    std::string base64;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    
    for (unsigned char c : buffer) {
        char_array_3[i++] = c;
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for (i = 0; i < 4; i++) {
                base64 += base64_chars[char_array_4[i]];
            }
            i = 0;
        }
    }
    
    if (i) {
        for (j = i; j < 3; j++) {
            char_array_3[j] = '\0';
        }
        
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        
        for (j = 0; j < i + 1; j++) {
            base64 += base64_chars[char_array_4[j]];
        }
        
        while (i++ < 3) {
            base64 += '=';
        }
    }
    
    return base64;
}

nlohmann::json MidjourneyClient::makeRequest(
    const std::string& endpoint,
    const nlohmann::json& payload,
    const std::string& method
) {
    auto url = base_url_ + endpoint;
    auto headers = buildHeaders();
    cpr::Response response;
    
    if (method == "GET") {
        response = cpr::Get(
            cpr::Url{url},
            headers
        );
    } else {
        response = cpr::Post(
            cpr::Url{url},
            headers,
            cpr::Body{payload.dump()}
        );
    }
    
    if (response.status_code != 200) {
        throw std::runtime_error("HTTP请求失败: " + std::to_string(response.status_code) + 
                               "\n响应: " + response.text);
    }
    
    auto json_response = nlohmann::json::parse(response.text);
    if (!validateResponse(json_response)) {
        throw std::runtime_error("API响应验证失败");
    }
    
    return json_response;
}

bool MidjourneyClient::validateResponse(const nlohmann::json& response) {
    return response.is_object() && !response.empty();
}

cpr::Header MidjourneyClient::buildHeaders() const {
    return cpr::Header{
        {"Authorization", "Bearer " + api_key_},
        {"Content-Type", "application/json"},
        {"Accept", "application/json"}
    };
}