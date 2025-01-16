#include <iostream>
#include <CLI/CLI.hpp>
#include "midjourney_client.hpp"
#include "config.hpp"

void printHelp() {
    std::cout << "可用命令:\n"
              << "  imagine <提示词>            - 创建新图像\n"
              << "  upscale <任务ID> <索引>     - 放大指定图像\n"
              << "  variation <任务ID> <索引>   - 创建变体\n"
              << "  describe <图片URL>          - 描述图像\n"
              << "  blend <图片URL1> <图片URL2> - 混合图像\n"
              << "  reset <任务ID>              - 重置任务\n"
              << "  zoom <任务ID> <百分比>      - 缩放图像\n"
              << "  status <任务ID>             - 查询任务状态\n"
              << "  config                      - 设置API配置\n"
              << "  exit                        - 退出程序\n";
}

#include <conio.h> // 用于_getch()

void configureClient(Config& config) {
    std::string base_url, api_key;
    
    std::cout << "请输入Base URL (例如: https://api.openai.com/): ";
    std::getline(std::cin, base_url);
    
    std::cout << "请输入API Key: ";
    // 隐藏式输入API Key
    char ch;
    while ((ch = _getch()) != '\r') {  // 读取直到回车键
        if (ch == '\b') {  // 退格键
            if (!api_key.empty()) {
                api_key.pop_back();
                std::cout << "\b \b";  // 删除显示的*
            }
        }
        else {
            api_key += ch;
            std::cout << '*';  // 显示*代替实际字符
        }
    }
    std::cout << std::endl;
    
    config.setBaseUrl(base_url);
    config.setApiKey(api_key);
    config.save();
}

int main() {
    try {
        Config config;
        if (!config.load()) {
            std::cout << "未找到配置文件，请先进行配置。\n";
            configureClient(config);
        }
        
        MidjourneyClient client(config.getBaseUrl(), config.getApiKey());
        std::string command;
        
        std::cout << "Midjourney CLI 客户端已启动。输入 'help' 查看可用命令。\n";
        
        while (true) {
            std::cout << "> ";
            std::getline(std::cin, command);
            
            if (command == "help") {
                printHelp();
                continue;
            }
            
            if (command == "exit") {
                break;
            }
            
            if (command == "config") {
                configureClient(config);
                client = MidjourneyClient(config.getBaseUrl(), config.getApiKey());
                continue;
            }
            
            // 解析命令
            std::istringstream iss(command);
            std::string cmd;
            iss >> cmd;
            
            try {
                if (cmd == "imagine") {
                    std::string prompt;
                    std::string mode;
                    std::getline(iss >> std::ws, prompt);
                    
                    // 检查是否有模式参数
                    size_t modePos = prompt.rfind(" --mode=");
                    if (modePos != std::string::npos) {
                        mode = prompt.substr(modePos + 7);
                        prompt = prompt.substr(0, modePos);
                    }
                    
                    if (prompt.empty()) {
                        std::cout << "错误: 请提供提示词\n";
                        continue;
                    }
                    
                    // 验证模式是否有效
                    if (!mode.empty() &&
                        mode != "RELAX" &&
                        mode != "FAST" &&
                        mode != "TURBO") {
                        std::cout << "错误: 无效的模式。可用模式: RELAX, FAST, TURBO\n";
                        continue;
                    }
                    
                    std::string taskId = client.imagine(prompt, mode);
                    if (taskId.empty()) {
                        continue;
                    }
                    
                    if (mode.empty()) {
                        std::cout << "提示: 可以使用 --mode=RELAX|FAST|TURBO 来指定生成模式\n";
                    }
                }
                else if (cmd == "US" || cmd == "UC" ||  // Upscale
                         cmd == "VS" || cmd == "VL" || cmd == "VR" ||  // Vary
                         cmd == "Z2" || cmd == "Z1.5" || cmd == "ZC" ||  // Zoom
                         cmd == "LEFT" || cmd == "RIGHT" || cmd == "UP" || cmd == "DOWN" ||  // Pan
                         cmd == "LIKE") {  // Bookmark
                    client.handleButton(cmd);
                }
                else if (cmd == "blend") {
                    std::vector<std::string> image_paths;
                    std::string path;
                    while (iss >> path) {
                        if (path.starts_with("--mode=")) {
                            break;
                        }
                        image_paths.push_back(path);
                    }
                    
                    std::string mode;
                    if (!path.empty() && path.starts_with("--mode=")) {
                        mode = path.substr(7);
                    }
                    
                    if (image_paths.size() < 2) {
                        std::cout << "错误: blend命令需要至少2个图片路径\n";
                        continue;
                    }
                    
                    if (!mode.empty() &&
                        mode != "RELAX" &&
                        mode != "FAST" &&
                        mode != "TURBO") {
                        std::cout << "错误: 无效的模式。可用模式: RELAX, FAST, TURBO\n";
                        continue;
                    }
                    
                    client.blend(image_paths, mode);
                }
                else if (cmd == "describe") {
                    std::string image_path;
                    iss >> image_path;
                    
                    std::string mode;
                    std::string modeArg;
                    if (iss >> modeArg && modeArg.starts_with("--mode=")) {
                        mode = modeArg.substr(7);
                    }
                    
                    if (image_path.empty()) {
                        std::cout << "错误: describe命令需要一个图片路径\n";
                        continue;
                    }
                    
                    if (!mode.empty() &&
                        mode != "RELAX" &&
                        mode != "FAST" &&
                        mode != "TURBO") {
                        std::cout << "错误: 无效的模式。可用模式: RELAX, FAST, TURBO\n";
                        continue;
                    }
                    
                    client.describe(image_path, mode);
                }
                else if (cmd == "reset") {
                    client.reset();  // 重置当前会话
                }
                else {
                    std::cout << "未知的命令。可用命令:\n"
                              << "- imagine <提示词> [--mode=RELAX|FAST|TURBO]\n"
                              << "- blend <图片1> <图片2> [--mode=RELAX|FAST|TURBO]\n"
                              << "- describe <图片> [--mode=RELAX|FAST|TURBO]\n"
                              << "- US (放大-细致)\n"
                              << "- UC (放大-创意)\n"
                              << "- VS (变化-细微)\n"
                              << "- VL (变化-明显)\n"
                              << "- VR (区域变化)\n"
                              << "- Z2 (2x缩放)\n"
                              << "- Z1.5 (1.5x缩放)\n"
                              << "- ZC (自定义缩放)\n"
                              << "- LEFT/RIGHT/UP/DOWN (平移)\n"
                              << "- LIKE (收藏)\n"
                              << "- reset (重置会话)\n";
                }
                else if (cmd == "upscale") {
                    std::string task_id;
                    int index;
                    iss >> task_id >> index;
                    if (task_id.empty() || !(index >= 1 && index <= 4)) {
                        std::cout << "错误: 请提供有效的任务ID和索引(1-4)\n";
                        continue;
                    }
                    client.upscale(task_id, index);
                }
                else if (cmd == "variation") {
                    std::string task_id;
                    int index;
                    iss >> task_id >> index;
                    if (task_id.empty() || !(index >= 1 && index <= 4)) {
                        std::cout << "错误: 请提供有效的任务ID和索引(1-4)\n";
                        continue;
                    }
                    client.variation(task_id, index);
                }
                else if (cmd == "describe") {
                    std::string image_url;
                    iss >> image_url;
                    if (image_url.empty()) {
                        std::cout << "错误: 请提供图片URL\n";
                        continue;
                    }
                    client.describe(image_url);
                }
                else if (cmd == "blend") {
                    std::string url1, url2;
                    iss >> url1 >> url2;
                    if (url1.empty() || url2.empty()) {
                        std::cout << "错误: 请提供两个图片URL\n";
                        continue;
                    }
                    client.blend({url1, url2});
                }
                else if (cmd == "status") {
                    std::string task_id;
                    iss >> task_id;
                    if (task_id.empty()) {
                        std::cout << "错误: 请提供任务ID\n";
                        continue;
                    }
                    auto status = client.getTaskStatus(task_id);
                    std::cout << status.dump(2) << std::endl;
                }
                else {
                    std::cout << "未知命令。输入 'help' 查看可用命令。\n";
                }
            }
            catch (const std::exception& e) {
                std::cout << "错误: " << e.what() << std::endl;
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "程序错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}