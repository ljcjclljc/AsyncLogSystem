#ifndef FASTDFS_CLIENT_H
#define FASTDFS_CLIENT_H

#include <string>
#include <vector>
#include <memory>

extern "C" {
#include "fdfs_client.h"
}

/**
 * @brief FastDFS 客户端封装类
 * 
 * 提供文件上传、下载、删除等功能的封装接口
 */
class FastDFSClient {
public:
    /**
     * @brief 构造函数
     * @param config_file FastDFS 客户端配置文件路径
     */
    explicit FastDFSClient(const std::string& config_file = "/etc/fdfs/client.conf");
    
    /**
     * @brief 析构函数
     */
    ~FastDFSClient();
    
    /**
     * @brief 初始化 FastDFS 客户端
     * @return true 成功，false 失败
     */
    bool initialize();
    
    /**
     * @brief 清理资源
     */
    void cleanup();
    
    /**
     * @brief 上传文件
     * @param local_file_path 本地文件路径
     * @param group_name 组名（可选，为空则自动选择）
     * @return 文件ID，失败返回空字符串
     */
    std::string uploadFile(const std::string& local_filename, const std::string& file_ext);
    
    /**
     * @brief 上传文件缓冲区
     * @param file_buffer 文件缓冲区
     * @param buffer_size 缓冲区大小
     * @param file_ext 文件扩展名
     * @param group_name 组名（可选，为空则自动选择）
     * @return 文件ID，失败返回空字符串
     */
    std::string uploadBuffer(const char* file_buffer, int64_t buffer_size, 
                           const std::string& file_ext, const std::string& group_name = "");
    
    /**
     * @brief 下载文件
     * @param file_id 文件ID
     * @param local_file_path 本地保存路径
     * @return true 成功，false 失败
     */
    bool downloadFile(const std::string& file_id, const std::string& local_filename);
    
    /**
     * @brief 下载文件到缓冲区
     * @param file_id 文件ID
     * @param file_buffer 输出缓冲区指针
     * @param file_size 输出文件大小
     * @return true 成功，false 失败
     */
    bool downloadToBuffer(const std::string& file_id, char** file_buffer, int64_t* file_size);
    
    /**
     * @brief 删除文件
     * @param file_id 文件ID
     * @return true 成功，false 失败
     */
    bool deleteFile(const std::string& file_id);
    
    /**
     * @brief 获取文件信息
     * @param file_id 文件ID
     * @param file_size 输出文件大小
     * @param create_time 输出创建时间
     * @param crc32 输出CRC32校验值
     * @return true 成功，false 失败
     */
    bool getFileInfo(const std::string& file_id, int64_t& file_size, 
                    time_t& create_time, time_t& modify_time);
    
    /**
     * @brief 检查文件是否存在
     * @param file_id 文件ID
     * @return true 存在，false 不存在
     */
    bool fileExists(const std::string& file_id);
    
    /**
     * @brief 获取最后一次操作的错误信息
     * @return 错误信息字符串
     */
    std::string getLastError() const;
    
    /**
     * @brief 检查客户端是否已初始化
     * @return true 已初始化，false 未初始化
     */
    bool isInitialized() const;

private:
    std::string config_file_;       // 配置文件路径
    bool initialized_;              // 是否已初始化
    mutable std::string last_error_; // 最后一次错误信息
    TrackerServerInfo tracker_server_; // Tracker 服务器连接信息
    
    /**
     * @brief 解析文件ID，分离组名和文件名
     * @param file_id 完整文件ID
     * @param group_name 输出组名
     * @param remote_filename 输出远程文件名
     * @return true 成功，false 失败
     */
    bool parseFileId(const std::string& file_id, std::string& group_name, 
                    std::string& remote_filename) const;
    
    /**
     * @brief 设置错误信息
     * @param error 错误信息
     */
    void setError(const std::string& error) const;
    
    /**
     * @brief 获取 FastDFS 错误信息
     * @param error_code 错误码
     * @return 错误信息字符串
     */
    std::string getFdfsError(int error_code) const;
};

#endif // FASTDFS_CLIENT_H