# AsyncLogSystem

## 一、项目简介
AsyncLogSystem 是一个基于异步日志系统的储存系统，它允许用户通过 HTTP 接口上传和下载文件，并支持不同的存储类型（如普通存储和深度存储）。该系统还提供了一个简单的 Web 界面，用于显示已存储的文件列表。

## 二、功能特性
1. **异步日志记录**：使用异步工作器处理日志记录，提高系统性能。
2. **文件上传与下载**：支持通过 HTTP 接口上传和下载文件。
3. **多种存储类型**：提供普通存储（low）和深度存储（deep）两种存储类型。
4. **文件列表展示**：通过 Web 界面显示已存储的文件列表，包括文件名、存储类型、文件大小和修改时间。
5. **配置文件管理**：通过 JSON 配置文件管理服务器端口、IP 地址、存储目录等参数。

## 三、项目结构
```
AsyncLogSystem-main/
├── README.md                    # 项目说明文档
├── log/                         # 日志系统模块
│   ├── AsyncBuffer.h            # 异步缓冲区
│   ├── AsyncLogger.h            # 异步日志器
│   ├── AsyncWorker.h            # 异步工作线程
│   ├── Level.h                  # 日志级别定义
│   ├── LogFlush.h               # 日志刷新器
│   ├── Manager.h                # 日志管理器
│   ├── Message.h                # 日志消息
│   ├── Mylog.h                  # 日志接口
│   ├── Threadpool.h             # 线程池
│   ├── Util.h                   # 工具函数
│   ├── backlog/                 # 备份日志相关
│   └── config.conf              # 日志配置文件
└── src/                         # 主程序模块
    ├── CMakeLists.txt           # CMake 配置文件
    ├── AsioIOServicePool.cpp    # Asio IO 服务池
    ├── CServer.cpp              # 服务器实现
    ├── HttpConnection.cpp       # HTTP 连接处理
    ├── LogicSystem.cpp          # 业务逻辑处理
    ├── Service.cpp              # 服务实现
    ├── base64.cpp               # Base64 编码解码
    ├── build/                   # 编译输出目录
    ├── include/                 # 头文件
    ├── index.html               # Web 界面
    ├── index1.html              # 备用 Web 界面
    ├── resource/                # 资源文件
    └── test.cpp                 # 测试程序
```

## 四、安装与配置
### 1. 依赖项
- CMake 3.10 及以上
- C++17 兼容编译器
- Boost 1.71.0 库 (system, filesystem, thread)
- libevent 库
- JsonCpp 库
- bundle 库
- base64 库
- pthread 库
- g++ 编译器
### 1. 克隆项目
```bash
git clone https://github.com/ljcjclljc/AsyncLogSystem.git
cd AsyncLogSystem
```

### 2. 配置文件
在 `src` 目录下，编辑配置文件（假设配置文件名为 `config.json`），设置服务器端口、IP 地址、存储目录等参数：
```json
{
    "server_port": 8080,
    "server_ip": "127.0.0.1",
    "download_prefix": "/download/",
    "storage_info": "storage_info.json",
    "deep_storage_dir": "deep_storage/",
    "low_storage_dir": "low_storage/",
    "bundle_format": 0
}
```

### 3. 编译项目
cd build文件夹下使用
```
cmake .. #生成对应的makefile文件
make     #编译项目
./test   #运行项目
```

## 五、使用方法
### 1. 启动服务器
```bash
./test
```

### 2. 访问 Web 界面
打开浏览器，访问 `http://<server_ip>:<server_port>`，即可看到文件上传和文件列表展示界面。

### 3. 上传文件
在 Web 界面中选择要上传的文件，并选择存储类型（普通存储或深度存储），点击“开始上传”按钮即可上传文件。

### 4. 下载文件
在文件列表中，点击文件对应的“下载”按钮，即可下载文件。

## 六、贡献与反馈
如果你对该项目有任何建议或发现了问题，请在 GitHub 上提交 Issue 或 Pull Request。

## 七、许可证
本项目遵循 [MIT 许可证](https://opensource.org/licenses/MIT)。
