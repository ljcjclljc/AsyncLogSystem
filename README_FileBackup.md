# FastDFS 文件传输备份管理器

一个基于 FastDFS 的高可靠性文件传输备份解决方案，提供自动化的分布式文件备份、恢复和管理功能。

## 📁 项目结构

```
src/
├── include/
│   ├── FastDFSClient.h          # FastDFS 客户端封装类
│   ├── FileBackupManager.h      # 文件备份管理器
│   └── BackupConfig.h           # 备份策略配置
├── FastDFSClient.cpp            # FastDFS 客户端实现
├── FileBackupManager.cpp        # 备份管理器实现
├── BackupConfig.cpp             # 备份配置实现
├── backup_example.cpp           # 备份管理器使用示例
├── fastdfs_example.cpp          # 简单 FastDFS 示例
├── CMakeLists_backup.txt        # CMake 编译配置
└── build.sh                     # 快速编译脚本
```

## 🚀 核心功能

### 1. 多种备份策略
- **单副本备份**: 基础备份，适用于一般场景
- **多副本备份**: 同组内多副本，提高可靠性
- **多组备份**: 跨组备份，提供最高级别的数据安全
- **混合备份**: 结合多副本和多组的灵活策略

### 2. 智能配置管理
- **预定义模板**: 最小、标准、企业级、高性能、高安全等配置
- **自动优化**: 根据文件大小、可靠性需求、性能要求自动调整
- **配置验证**: 确保备份配置的有效性和合理性

### 3. 完整的生命周期管理
- **自动备份**: 文件上传时自动创建备份
- **完整性验证**: 校验和验证确保数据完整性
- **智能恢复**: 主文件损坏时自动从备份恢复
- **统计监控**: 备份状态、成功率、存储使用情况统计

## 🛠️ 编译和安装

### 前置依赖

```bash
# Ubuntu/Debian
sudo apt-get install libfastdfs-dev libfastcommon-dev cmake build-essential

# CentOS/RHEL
sudo yum install fastdfs-devel fastcommon-devel cmake gcc-c++
```

### 快速编译

```bash
# 使用提供的编译脚本
chmod +x build.sh
./build.sh

# 或者手动编译
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 编译目标

- `backup_example`: 备份管理器完整示例程序
- `fastdfs_example`: 简单 FastDFS 客户端示例
- `fastdfs_client`: FastDFS 客户端静态库
- `backup_manager`: 备份管理器静态库

## 📖 使用指南

### 1. 基础使用

```cpp
#include "include/FileBackupManager.h"
#include "include/BackupConfig.h"

// 创建备份管理器
FileBackupManager manager("/etc/fdfs/client.conf");

// 配置备份策略
BackupConfig config = BackupConfigFactory::createStandardBackup();

// 初始化
if (!manager.initialize(config)) {
    std::cerr << "初始化失败: " << manager.getLastError() << std::endl;
    return;
}

// 上传并备份文件
std::string backup_id;
std::string file_id = manager.uploadWithBackup("/path/to/file.txt", backup_id);

if (!file_id.empty()) {
    std::cout << "备份成功，ID: " << backup_id << std::endl;
    
    // 下载文件
    manager.downloadFromBackup(backup_id, "/path/to/downloaded.txt");
    
    // 删除备份
    manager.deleteBackup(backup_id);
}
```

### 2. 高级配置

```cpp
// 创建自定义配置
BackupConfig config;
config.strategy = BackupStrategy::MULTI_GROUP;
config.target_groups = {"group1", "group2", "group3"};
config.copy_count = 2;
config.enable_checksum = true;
config.enable_compression = true;
config.max_retry_count = 3;

// 或使用工厂方法
BackupConfig config = BackupConfigFactory::createMultiGroupConfig(
    {"group1", "group2"}, 2);

// 针对大文件优化
config = BackupStrategyOptimizer::optimizeForFileSize(
    100 * 1024 * 1024, config); // 100MB
```

### 3. 进度监控

```cpp
// 设置进度回调
manager.setProgressCallback([](const std::string& backup_id, 
                              int progress, 
                              BackupStatus status) {
    std::cout << "备份 " << backup_id 
              << " 进度: " << progress << "%" << std::endl;
});
```

### 4. 缓冲区操作

```cpp
// 直接上传内存数据
const char* data = "Hello, FastDFS!";
std::string backup_id;
std::string file_id = manager.uploadBufferWithBackup(
    data, strlen(data), ".txt", backup_id);

// 下载到内存
char* buffer = nullptr;
int64_t size = 0;
if (manager.downloadToBufferFromBackup(backup_id, &buffer, &size)) {
    // 使用数据...
    free(buffer); // 记得释放内存
}
```

## ⚙️ 配置文件

### FastDFS 客户端配置 (`/etc/fdfs/client.conf`)

```ini
# 连接超时时间
connect_timeout = 30

# 网络超时时间
network_timeout = 60

# tracker 服务器地址
tracker_server = 192.168.1.100:22122
tracker_server = 192.168.1.101:22122

# 日志级别
log_level = info

# 日志文件路径
log_filename = /var/log/fdfs/client.log

# HTTP 设置
http.tracker_server_port = 80
```

### 备份策略配置示例

```cpp
// 最小备份配置 - 适用于测试环境
BackupConfig minimal = BackupTemplates::MINIMAL_BACKUP;
// - 单副本
// - 无校验和
// - 无压缩

// 标准备份配置 - 适用于生产环境
BackupConfig standard = BackupTemplates::STANDARD_BACKUP;
// - 2个副本
// - 启用校验和
// - 适中的重试次数

// 企业级备份配置 - 适用于关键业务
BackupConfig enterprise = BackupTemplates::ENTERPRISE_BACKUP;
// - 3个副本
// - 多组备份
// - 启用所有安全特性
```

## 📊 监控和统计

### 获取备份统计

```cpp
int total, successful, failed;
int64_t total_size;

manager.getBackupStatistics(total, successful, failed, total_size);

std::cout << "总备份数: " << total << std::endl;
std::cout << "成功率: " << (successful * 100.0 / total) << "%" << std::endl;
std::cout << "总大小: " << (total_size / 1024 / 1024) << " MB" << std::endl;
```

### 查看备份记录

```cpp
// 获取单个备份记录
auto record = manager.getBackupRecord(backup_id);
if (record) {
    std::cout << "原始文件: " << record->original_file << std::endl;
    std::cout << "主文件ID: " << record->primary_file_id << std::endl;
    std::cout << "备份数量: " << record->backup_file_ids.size() << std::endl;
}

// 获取所有备份记录
auto all_records = manager.getAllBackupRecords();
for (const auto& pair : all_records) {
    std::cout << "备份ID: " << pair.first << std::endl;
    // 处理备份记录...
}
```

## 🔧 故障排除

### 常见问题

1. **初始化失败**
   ```
   错误: 无法连接到 tracker 服务器
   解决: 检查 tracker_server 配置和网络连接
   ```

2. **上传失败**
   ```
   错误: 存储服务器空间不足
   解决: 清理存储空间或添加新的存储节点
   ```

3. **备份验证失败**
   ```
   错误: 校验和不匹配
   解决: 检查网络稳定性，重新上传文件
   ```

### 调试模式

```cpp
// 启用详细日志
#define DEBUG
#include "include/FileBackupManager.h"

// 或在编译时添加 -DDEBUG 标志
```

### 性能优化建议

1. **大文件处理**
   - 使用 `optimizeForFileSize()` 自动优化配置
   - 考虑启用压缩以减少网络传输
   - 适当增加网络超时时间

2. **高并发场景**
   - 使用连接池减少连接开销
   - 批量操作提高效率
   - 监控系统资源使用情况

3. **存储优化**
   - 定期清理过期备份
   - 使用合适的备份策略避免过度冗余
   - 监控存储空间使用情况

## 🔒 安全考虑

1. **网络安全**
   - 使用防火墙限制 FastDFS 端口访问
   - 考虑使用 VPN 或专用网络

2. **数据安全**
   - 启用校验和验证确保数据完整性
   - 定期验证备份文件的可用性
   - 考虑加密敏感文件

3. **访问控制**
   - 限制客户端配置文件的访问权限
   - 使用专用的服务账户运行程序

## 📈 扩展功能

### 自定义备份策略

```cpp
class CustomBackupStrategy {
public:
    static BackupConfig createCustomConfig() {
        BackupConfig config;
        config.strategy = BackupStrategy::CUSTOM;
        // 自定义配置逻辑...
        return config;
    }
};
```

### 备份生命周期管理

```cpp
// 实现自动清理过期备份
class BackupLifecycleManager {
public:
    void cleanupExpiredBackups(int days_to_keep);
    void archiveOldBackups(const std::string& archive_group);
};
```

## 📞 技术支持

如果您在使用过程中遇到问题，请：

1. 查看日志文件获取详细错误信息
2. 检查 FastDFS 服务器状态
3. 验证网络连接和配置文件
4. 参考示例代码和文档

## 📄 许可证

本项目采用 MIT 许可证，详见 LICENSE 文件。

---

**注意**: 在生产环境中使用前，请充分测试所有功能，并根据实际需求调整配置参数。