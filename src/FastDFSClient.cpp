#include "include/FastDFSClient.h"
#include <iostream>
#include <cstring>
#include <sstream>

FastDFSClient::FastDFSClient(const std::string& config_file)
    : config_file_(config_file), initialized_(false) {
    memset(&tracker_server_, 0, sizeof(tracker_server_));
}

FastDFSClient::~FastDFSClient() {
    cleanup();
}

bool FastDFSClient::initialize() {
    if (initialized_) {
        return true;
    }
    
    if (fdfs_client_init(config_file_.c_str()) != 0) {
        last_error_ = "Failed to initialize FastDFS client";
        return false;
    }
    
    initialized_ = true;
    return true;
}

void FastDFSClient::cleanup() {
    if (initialized_) {
        fdfs_client_destroy();
        initialized_ = false;
    }
}

std::string FastDFSClient::uploadFile(const std::string& local_filename, const std::string& file_ext) {
    if (!initialized_) {
        last_error_ = "FastDFS client not initialized";
        return "";
    }
    
    char file_id[128];
    int result;
    
    result = storage_upload_by_filename(nullptr, nullptr, 0,
                                      local_filename.c_str(), file_ext.c_str(),
                                      nullptr, 0, nullptr, file_id);
    
    if (result != 0) {
        last_error_ = "Failed to upload file: " + std::to_string(result);
        return "";
    }
    
    return std::string(file_id);
}

std::string FastDFSClient::uploadBuffer(const char* file_buffer, int64_t buffer_size, 
                                      const std::string& file_ext, const std::string& group_name) {
    if (!initialized_) {
        last_error_ = "FastDFS client not initialized";
        return "";
    }
    
    char file_id[128];
    int result;
    
    result = storage_upload_by_filebuff(nullptr, nullptr,
                                      0, file_buffer, buffer_size, file_ext.c_str(),
                                      nullptr, 0, nullptr, file_id);
    
    if (result != 0) {
        last_error_ = "Failed to upload buffer: " + std::to_string(result);
        return "";
    }
    
    return std::string(file_id);
}

bool FastDFSClient::downloadFile(const std::string& file_id, const std::string& local_file_path) {
    if (!initialized_) {
        last_error_ = "FastDFS client not initialized";
        return false;
    }
    
    std::string group_name, remote_filename;
    if (!parseFileId(file_id, group_name, remote_filename)) {
        return false;
    }
    
    int64_t file_size = 0;
    int result = storage_download_file_to_file(nullptr, nullptr,
                                             group_name.c_str(), remote_filename.c_str(),
                                             local_file_path.c_str(), &file_size);
    
    if (result != 0) {
        last_error_ = "Failed to download file: " + std::to_string(result);
        return false;
    }
    
    return true;
}

bool FastDFSClient::downloadToBuffer(const std::string& file_id, char** file_buffer, int64_t* file_size) {
    if (!initialized_) {
        last_error_ = "FastDFS client not initialized";
        return false;
    }
    
    std::string group_name, remote_filename;
    if (!parseFileId(file_id, group_name, remote_filename)) {
        return false;
    }
    
    int result = storage_download_file_to_buff(nullptr, nullptr,
                                             group_name.c_str(), remote_filename.c_str(),
                                             file_buffer, file_size);
    
    if (result != 0) {
        last_error_ = "Failed to download file to buffer: " + std::to_string(result);
        return false;
    }
    
    return true;
}

bool FastDFSClient::deleteFile(const std::string& file_id) {
    if (!initialized_) {
        last_error_ = "FastDFS client not initialized";
        return false;
    }
    
    std::string group_name, remote_filename;
    if (!parseFileId(file_id, group_name, remote_filename)) {
        return false;
    }
    
    int result = storage_delete_file(nullptr, nullptr,
                                   group_name.c_str(), remote_filename.c_str());
    
    if (result != 0) {
        last_error_ = "Failed to delete file: " + std::to_string(result);
        return false;
    }
    
    return true;
}

bool FastDFSClient::getFileInfo(const std::string& file_id, int64_t& file_size, time_t& create_time, time_t& modify_time) {
    if (!initialized_) {
        last_error_ = "FastDFS client not initialized";
        return false;
    }
    
    std::string group_name, remote_filename;
    if (!parseFileId(file_id, group_name, remote_filename)) {
        return false;
    }
    
    FDFSFileInfo file_info;
    int result = storage_query_file_info_ex(nullptr, nullptr,
                                          group_name.c_str(), remote_filename.c_str(),
                                          &file_info, false);
    
    if (result != 0) {
        last_error_ = "Failed to get file info: " + std::to_string(result);
        return false;
    }
    
    file_size = file_info.file_size;
    create_time = file_info.create_timestamp;
    modify_time = file_info.create_timestamp; // FastDFS 没有修改时间
    
    return true;
}

bool FastDFSClient::fileExists(const std::string& file_id) {
    int64_t file_size;
    time_t create_time, modify_time;
    return getFileInfo(file_id, file_size, create_time, modify_time);
}

std::string FastDFSClient::getLastError() const {
    return last_error_;
}

bool FastDFSClient::isInitialized() const {
    return initialized_;
}

bool FastDFSClient::parseFileId(const std::string& file_id, std::string& group_name, 
                               std::string& remote_filename) const {
    size_t pos = file_id.find('/');
    if (pos == std::string::npos) {
        setError("Invalid file ID format");
        return false;
    }
    
    group_name = file_id.substr(0, pos);
    remote_filename = file_id.substr(pos + 1);
    
    if (group_name.empty() || remote_filename.empty()) {
        setError("Invalid file ID format");
        return false;
    }
    
    return true;
}

void FastDFSClient::setError(const std::string& error) const {
    last_error_ = error;
    std::cerr << "FastDFS Error: " << error << std::endl;
}

std::string FastDFSClient::getFdfsError(int error_code) const {
    std::ostringstream oss;
    oss << "Error code: " << error_code;
    
    switch (error_code) {
        case 0: return "Success";
        case 2: return "No such file or directory";
        case 22: return "Invalid argument";
        case 28: return "No space left on device";
        default: 
            oss << " (" << strerror(error_code) << ")";
            return oss.str();
    }
}