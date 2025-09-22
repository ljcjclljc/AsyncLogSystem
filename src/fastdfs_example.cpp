#include "include/FastDFSClient.h"
#include <iostream>
#include <fstream>
#include <cstring>

void testFileUploadDownload() {
    std::cout << "=== 测试文件上传下载 ===" << std::endl;
    
    // 创建 FastDFS 客户端
    FastDFSClient client("/etc/fdfs/client.conf");
    
    // 初始化客户端
    if (!client.initialize()) {
        std::cerr << "初始化失败: " << client.getLastError() << std::endl;
        return;
    }
    
    std::cout << "FastDFS 客户端初始化成功" << std::endl;
    
    // 创建测试文件
    std::string test_file = "/tmp/fastdfs_test.txt";
    std::ofstream ofs(test_file);
    ofs << "这是一个 FastDFS 测试文件\n";
    ofs << "测试时间: " << time(nullptr) << std::endl;
    ofs.close();
    
    // 上传文件
    std::cout << "上传文件: " << test_file << std::endl;
    std::string file_id = client.uploadFile(test_file);
    if (file_id.empty()) {
        std::cerr << "上传失败: " << client.getLastError() << std::endl;
        return;
    }
    
    std::cout << "上传成功，文件ID: " << file_id << std::endl;
    
    // 获取文件信息
    int64_t file_size;
    time_t create_time;
    int crc32;
    if (client.getFileInfo(file_id, &file_size, &create_time, &crc32)) {
        std::cout << "文件信息:" << std::endl;
        std::cout << "  大小: " << file_size << " 字节" << std::endl;
        std::cout << "  创建时间: " << create_time << std::endl;
        std::cout << "  CRC32: " << crc32 << std::endl;
    }
    
    // 检查文件是否存在
    if (client.fileExists(file_id)) {
        std::cout << "文件存在确认" << std::endl;
    }
    
    // 下载文件
    std::string download_file = "/tmp/fastdfs_downloaded.txt";
    std::cout << "下载文件到: " << download_file << std::endl;
    if (client.downloadFile(file_id, download_file)) {
        std::cout << "下载成功" << std::endl;
        
        // 验证下载的文件内容
        std::ifstream ifs(download_file);
        std::string content((std::istreambuf_iterator<char>(ifs)),
                           std::istreambuf_iterator<char>());
        std::cout << "下载文件内容:" << std::endl << content << std::endl;
    } else {
        std::cerr << "下载失败: " << client.getLastError() << std::endl;
    }
    
    // 删除文件
    std::cout << "删除远程文件" << std::endl;
    if (client.deleteFile(file_id)) {
        std::cout << "删除成功" << std::endl;
    } else {
        std::cerr << "删除失败: " << client.getLastError() << std::endl;
    }
    
    // 清理本地文件
    remove(test_file.c_str());
    remove(download_file.c_str());
}

void testBufferUploadDownload() {
    std::cout << "\n=== 测试缓冲区上传下载 ===" << std::endl;
    
    FastDFSClient client;
    
    if (!client.initialize()) {
        std::cerr << "初始化失败: " << client.getLastError() << std::endl;
        return;
    }
    
    // 准备测试数据
    std::string test_data = "这是通过缓冲区上传的测试数据\n包含多行内容\n测试完成";
    
    // 上传缓冲区
    std::cout << "上传缓冲区数据，大小: " << test_data.size() << " 字节" << std::endl;
    std::string file_id = client.uploadBuffer(test_data.c_str(), test_data.size(), "txt");
    
    if (file_id.empty()) {
        std::cerr << "缓冲区上传失败: " << client.getLastError() << std::endl;
        return;
    }
    
    std::cout << "缓冲区上传成功，文件ID: " << file_id << std::endl;
    
    // 下载到缓冲区
    char* download_buffer = nullptr;
    int64_t download_size = 0;
    
    std::cout << "下载到缓冲区" << std::endl;
    if (client.downloadToBuffer(file_id, &download_buffer, &download_size)) {
        std::cout << "下载成功，大小: " << download_size << " 字节" << std::endl;
        std::cout << "下载内容:" << std::endl;
        std::cout.write(download_buffer, download_size);
        std::cout << std::endl;
        
        // 释放缓冲区内存
        if (download_buffer) {
            free(download_buffer);
        }
    } else {
        std::cerr << "下载到缓冲区失败: " << client.getLastError() << std::endl;
    }
    
    // 清理
    client.deleteFile(file_id);
}

void testErrorHandling() {
    std::cout << "\n=== 测试错误处理 ===" << std::endl;
    
    FastDFSClient client;
    
    // 测试未初始化的操作
    std::string result = client.uploadFile("/tmp/nonexistent.txt");
    if (result.empty()) {
        std::cout << "未初始化错误处理正确: " << client.getLastError() << std::endl;
    }
    
    // 初始化后测试不存在的文件
    if (client.initialize()) {
        result = client.uploadFile("/tmp/definitely_nonexistent_file.txt");
        if (result.empty()) {
            std::cout << "文件不存在错误处理正确: " << client.getLastError() << std::endl;
        }
        
        // 测试下载不存在的文件
        bool success = client.downloadFile("group1/M00/00/00/nonexistent.txt", "/tmp/test.txt");
        if (!success) {
            std::cout << "下载不存在文件错误处理正确: " << client.getLastError() << std::endl;
        }
    }
}

int main() {
    std::cout << "FastDFS 客户端封装类测试程序" << std::endl;
    std::cout << "================================" << std::endl;
    
    try {
        // 测试文件上传下载
        testFileUploadDownload();
        
        // 测试缓冲区上传下载
        testBufferUploadDownload();
        
        // 测试错误处理
        testErrorHandling();
        
        std::cout << "\n所有测试完成!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "测试过程中发生异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}