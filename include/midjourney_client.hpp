#pragma once

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

enum class MJMode {
    RELAX,
    FAST,
    TURBO
};

class MidjourneyClient {
public:
    MidjourneyClient(const std::string& base_url, const std::string& api_key);
    
    // 将mode转换为字符串
    static std::string modeToString(MJMode mode);
    
    // 主要指令
    std::string imagine(const std::string& prompt, const std::string& mode = "RELAX");
    std::string blend(const std::vector<std::string>& image_paths, const std::string& mode = "RELAX");
    std::string describe(const std::string& image_path, const std::string& mode = "RELAX");
    
    // 图片处理
    static std::string imageToBase64(const std::string& image_path);
    
    // 处理按钮操作 (U1-U4, V1-V4, R)
    bool handleButton(const std::string& action);
    
    // 重置会话
    void reset();
    
    // 获取当前任务ID
    std::string getCurrentTaskId() const { return current_task_id_; }
    
private:
    std::string base_url_;
    std::string api_key_;
    std::string current_task_id_;     // 当前任务ID
    nlohmann::json current_buttons_;  // 当前任务的按钮信息
    std::vector<std::string> task_history_;  // 任务历史记录
    
    // 清理所有临时状态
    void cleanup();
    
    // 监控任务状态
    void monitorTask(const std::string& taskId);
    
    // 通用HTTP请求处理
    nlohmann::json makeRequest(const std::string& endpoint, 
                             const nlohmann::json& payload,
                             const std::string& method = "POST");
    
    // 验证响应
    bool validateResponse(const nlohmann::json& response);
    
    // 构建请求头
    cpr::Header buildHeaders() const;
    
    // 格式化显示任务结果
    void displayTaskResult(const nlohmann::json& task);
    
    // 格式化显示按钮选项
    void displayButtons(const nlohmann::json& buttons);
    
    // 根据操作找到对应的customId
    std::string findCustomId(const std::string& action);
};