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
AsyncLogSystem/
├── log/
│   ├── AsyncBuffer.h
│   ├── AsyncLogger.h
│   ├── AsyncWorker.h
│   ├── backlog/
│   │   ├── ServerBackupLog.cpp
│   │   └── ServerBackupLog.h
│   └── Util.h
├── src/
│   ├── Config.h
│   ├── Service.h
│   ├── index1.html
│   └── text.cpp
└── README.md
```

## 四、安装与配置
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
由于没有提供具体的编译脚本，假设使用 g++ 进行编译：
```bash
g++ src/*.cpp -o AsyncLogSystem -levhttp -levent -lboost_system -lboost_thread -lpthread
```

## 五、使用方法
### 1. 启动服务器
```bash
./AsyncLogSystem
```

### 2. 访问 Web 界面
打开浏览器，访问 `http://<server_ip>:<server_port>`，即可看到文件上传和文件列表展示界面。

### 3. 上传文件
在 Web 界面中选择要上传的文件，并选择存储类型（普通存储或深度存储），点击“开始上传”按钮即可上传文件。

### 4. 下载文件
在文件列表中，点击文件对应的“下载”按钮，即可下载文件。

## 六、代码示例
### 1. 启动服务模块
```cpp
#include "Service.h"

void service_module()
{
    storage::Service s;
    mylog::GetLogger("asynclogger")->Info("service step in RunModule");
    s.RunModule();
}

int main()
{
    service_module();
    return 0;
}
```

### 2. 异步日志记录
```cpp
#include "AsyncLogger.h"

int main()
{
    std::vector<mylog::LogFlush::ptr> flushs;
    mylog::LoggerBuilder builder;
    builder.BuildLoggerName("asynclogger");
    builder.BuildLopperType(mylog::AsyncType::ASYNC_SAFE);
    builder.BuildLoggerFlush<mylog::StdoutFlush>();
    mylog::AsyncLogger::ptr logger = builder.Build();

    logger->Info(__FILE__, __LINE__, "This is an info log message");
    logger->Error(__FILE__, __LINE__, "This is an error log message");

    return 0;
}
```

## 七、贡献与反馈
如果你对该项目有任何建议或发现了问题，请在 GitHub 上提交 Issue 或 Pull Request。

## 八、许可证
本项目遵循 [MIT 许可证](https://opensource.org/licenses/MIT)。
