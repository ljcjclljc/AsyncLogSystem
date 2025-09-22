#ifndef MOCK_FASTDFS_CLIENT_H
#define MOCK_FASTDFS_CLIENT_H

#include <string>
#include <iostream>
#include <fstream>
#include <cstring>

/**
 * @brief 模拟 FastDFS 客户端类
 * 
 * 这个类提供了与 FastDFS 客户端相同的接口，但不依赖实际的 FastDFS 库
 * 主要用于开发和测试环境，当系统中没有安装 FastDFS 时使用
 */
class MockFastDFSClient {
private:
    std::string config_file_;
    std::string last_error_;
    bool initialized_;
    
public:
    /**
     * @brief 构造函数
     * @param config_file FastDFS 客户端配置文件路径
     */
    explicit MockFastDFSClient(const std::string& config_file) 
        : config_file_(config_file), initialized_(false) {
        std::cout << "[Mock] FastDFS 客户端创建，配置文件: " << config_file << std::endl;
    }
    
    /**
     * @brief 析构函数
     */
    ~MockFastDFSClient() {
        if (initialized_) {
            cleanup();
        }
    }
    
    /**
     * @brief 初始化客户端
     * @return 成功返回 true，失败返回 false
     */
    bool initialize() {
        std::cout << "[Mock] 初始化 FastDFS 客户端..." << std::endl;
        
        // 模拟检查配置文件
        std::ifstream config(config_file_);
        if (!config.is_open()) {
            last_error_ = "无法打开配置文件: " + config_file_;
            std::cout << "[Mock] " << last_error_ << std::endl;
            return false;
        }
        config.close();
        
        initialized_ = true;
        std::cout << "[Mock] FastDFS 客户端初始化成功" << std::endl;
        return true;
    }
    
    /**
     * @brief 清理资源
     */
    void cleanup() {
        if (initialized_) {
            std::cout << "[Mock] 清理 FastDFS 客户端资源" << std::endl;
            initialized_ = false;
        }
    }
    
    /**
     * @brief 上传文件
     * @param file_path 本地文件路径
     * @return 成功返回文件ID，失败返回空字符串
     */
    std::string uploadFile(const std::string& file_path) {
        if (!initialized_) {
            last_error_ = "客户端未初始化";
            return "";
        }
        
        std::cout << "[Mock] 上传文件: " << file_path << std::endl;
        
        // 检查文件是否存在
        std::ifstream file(file_path);
        if (!file.is_open()) {
            last_error_ = "文件不存在: " + file_path;
            std::cout << "[Mock] " << last_error_ << std::endl;
            return "";
        }
        file.close();
        
        // 模拟生成文件ID
        std::string file_id = "group1/M00/00/00/mock_file_" + std::to_string(time(nullptr)) + ".dat";
        std::cout << "[Mock] 文件上传成功，文件ID: " << file_id << std::endl;
        
        return file_id;
    }
    
    /**
     * @brief 上传缓冲区数据
     * @param buffer 数据缓冲区
     * @param buffer_size 缓冲区大小
     * @param file_ext 文件扩展名
     * @return 成功返回文件ID，失败返回空字符串
     */
    std::string uploadBuffer(const char* buffer, int64_t buffer_size, const std::string& file_ext = "") {
        if (!initialized_) {
            last_error_ = "客户端未初始化";
            return "";
        }
        
        if (!buffer || buffer_size <= 0) {
            last_error_ = "无效的缓冲区参数";
            return "";
        }
        
        std::cout << "[Mock] 上传缓冲区数据，大小: " << buffer_size << " 字节" << std::endl;
        
        // 模拟生成文件ID
        std::string file_id = "group1/M00/00/00/mock_buffer_" + std::to_string(time(nullptr)) + file_ext;
        std::cout << "[Mock] 缓冲区上传成功，文件ID: " << file_id << std::endl;
        
        return file_id;
    }
    
    /**
     * @brief 下载文件
     * @param file_id 文件ID
     * @param local_file 本地保存路径
     * @return 成功返回 true，失败返回 false
     */
    bool downloadFile(const std::string& file_id, const std::string& local_file) {
        if (!initialized_) {
            last_error_ = "客户端未初始化";
            return false;
        }
        
        std::cout << "[Mock] 下载文件: " << file_id << " -> " << local_file << std::endl;
        
        // 模拟创建下载文件
        std::ofstream file(local_file);
        if (!file.is_open()) {
            last_error_ = "无法创建本地文件: " + local_file;
            std::cout << "[Mock] " << last_error_ << std::endl;
            return false;
        }
        
        // 写入模拟内容
        file << "这是从 FastDFS 下载的模拟文件内容\n";
        file << "文件ID: " << file_id << "\n";
        file << "下载时间: " << time(nullptr) << std::endl;
        file.close();
        
        std::cout << "[Mock] 文件下载成功: " << local_file << std::endl;
        return true;
    }
    
    /**
     * @brief 下载文件到缓冲区
     * @param file_id 文件ID
     * @param buffer 输出缓冲区指针
     * @param buffer_size 输出缓冲区大小
     * @return 成功返回 true，失败返回 false
     */
    bool downloadToBuffer(const std::string& file_id, char** buffer, int64_t* buffer_size) {
        if (!initialized_) {
            last_error_ = "客户端未初始化";
            return false;
        }
        
        if (!buffer || !buffer_size) {
            last_error_ = "无效的缓冲区参数";
            return false;
        }
        
        std::cout << "[Mock] 下载文件到缓冲区: " << file_id << std::endl;
        
        // 模拟文件内容
        std::string content = "这是从 FastDFS 下载到缓冲区的模拟内容\n文件ID: " + file_id + "\n";
        
        *buffer_size = content.length();
        *buffer = (char*)malloc(*buffer_size);
        
        if (!*buffer) {
            last_error_ = "内存分配失败";
            return false;
        }
        
        memcpy(*buffer, content.c_str(), *buffer_size);
        
        std::cout << "[Mock] 文件下载到缓冲区成功，大小: " << *buffer_size << " 字节" << std::endl;
        return true;
    }
    
    /**
     * @brief 删除文件
     * @param file_id 文件ID
     * @return 成功返回 true，失败返回 false
     */
    bool deleteFile(const std::string& file_id) {
        if (!initialized_) {
            last_error_ = "客户端未初始化";
            return false;
        }
        
        std::cout << "[Mock] 删除文件: " << file_id << std::endl;
        std::cout << "[Mock] 文件删除成功" << std::endl;
        return true;
    }
    
    /**
     * @brief 获取最后的错误信息
     * @return 错误信息字符串
     */
    std::string getLastError() const {
        return last_error_;
    }
    
    /**
     * @brief 检查客户端是否已初始化
     * @return 已初始化返回 true，否则返回 false
     */
    bool isInitialized() const {
        return initialized_;
    }
    
    /**
     * @brief 获取文件信息
     * @param file_id 文件ID
     * @return 成功返回文件大小，失败返回 -1
     */
    int64_t getFileSize(const std::string& file_id) {
        if (!initialized_) {
            last_error_ = "客户端未初始化";
            return -1;
        }
        
        std::cout << "[Mock] 获取文件大小: " << file_id << std::endl;
        
        // 模拟返回文件大小
        int64_t size = 1024 + (time(nullptr) % 10000); // 随机大小
        std::cout << "[Mock] 文件大小: " << size << " 字节" << std::endl;
        
        return size;
    }
};

// 为了兼容性，定义一个别名
using FastDFSClient = MockFastDFSClient;

#endif // MOCK_FASTDFS_CLIENT_H