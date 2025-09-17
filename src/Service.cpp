#include"Service.h"
#include"Config.h"
#include<sstream>
#include<iomanip>
#include<fstream>
#include<cstring>
#include<unistd.h>
#include"HttpConnection.h"
#include"base64.h"  // 添加base64头文件

namespace storage
{
    extern DataManager *data_;

    Service::Service()
    {
    }

    void Service::Upload(std::shared_ptr<HttpConnection> conn)
    {
        mylog::GetLogger("asynclogger")->Info("Upload");
        const auto& request_body = conn->_request.body();
        std::string content = beast::buffers_to_string(request_body.data());
        if(content.empty())
        {
            mylog::GetLogger("asynclogger")->Error("Upload:content is empty");
            return;
        }

        size_t len=content.size();
        mylog::GetLogger("asynclogger")->Info("Upload:len:%d",len);
        if(len==0)
        {
            mylog::GetLogger("asynclogger")->Error("Upload:len is 0");
            conn->_response.result(http::status::bad_request);
            conn->_response.set(http::field::content_type, "text/plain");
            beast::ostream(conn->_response.body()) << "Empty request body";
            return;
        }

        std::string filename;
        // 添加Filename请求头处理
        if (conn->_request.count("Filename"))
        {
            std::string encoded_filename = std::string(conn->_request["Filename"]);
            filename = base64_decode(encoded_filename);  // base64解码
            std::cout<<"decoded filename:"<<filename<<std::endl;
        }
        else
        {
            mylog::GetLogger("asynclogger")->Error("Upload:Filename header missing");
            conn->_response.result(http::status::bad_request);
            conn->_response.set(http::field::content_type, "text/plain");
            beast::ostream(conn->_response.body()) << "Filename header missing";
            return;
        }

        std::string storage_type;
        if (conn->_request.count("StorageType"))
        {
            storage_type = std::string(conn->_request["StorageType"]);
            std::cout<<"storage_type:"<<storage_type<<std::endl;
        }
        else
        {
            mylog::GetLogger("asynclogger")->Error("Upload:StorageType header missing");
            conn->_response.result(http::status::bad_request);
            conn->_response.set(http::field::content_type, "text/plain");
            beast::ostream(conn->_response.body()) << "StorageType header missing";
            return;
        }

        std::string storage_path;
        if(storage_type=="low")
        {
            storage_path=Config::GetInstance()->GetLowStorageDir();
        }
        else if(storage_type=="deep")
        {
            storage_path=Config::GetInstance()->GetDeepStorageDir();
        }
        else
        {
           mylog::GetLogger("asynclogger")->Error("Upload:storage_type error");
            conn->_response.result(http::status::not_found);
            conn->_response.set(http::field::content_type, "text/plain");
            beast::ostream(conn->_response.body()) << "Invalid storage type";
            return;
        }

        FileUtil dirCreate(storage_path);
        dirCreate.CreatDirectory();

        storage_path+=filename;
        FileUtil fu(storage_path);
        if(storage_path.find("low_storage")!=std::string::npos)
        {
            if(fu.SetContent(content.c_str(),len)==false)
            {
                mylog::GetLogger("asynclogger")->Error("Upload:fu.SetContent error");
                conn->_response.result(http::status::internal_server_error);
                conn->_response.set(http::field::content_type, "text/plain");
                beast::ostream(conn->_response.body()) << "Failed to save file";
                conn->WriteResponse();
                return;
            }
            else
            {
                mylog::GetLogger("asynclogger")->Info("Upload:fu.SetContent success");
            }
        }
        else
        {
            if(fu.Compress(content.c_str(),Config::GetInstance()->GetBundleFormat())==false)
            {
                mylog::GetLogger("asynclogger")->Error("Upload:fu.Compress error");
                conn->_response.result(http::status::internal_server_error);
                conn->_response.set(http::field::content_type, "text/plain");
                beast::ostream(conn->_response.body()) << "Failed to compress file";
                conn->WriteResponse();
                return;
            }
            else
            {
                mylog::GetLogger("asynclogger")->Info("Upload:fu.Compress success");
            }
        }
        //将文件写入data
        StorageInfo info;
        info.NewStorageInfo(storage_path);
        data_->Insert(info);

        conn->_response.result(http::status::ok);
        conn->_response.set(http::field::content_type, "text/plain");
        conn->_response.set(http::field::server, "GateServer");
        beast::ostream(conn->_response.body()) << "Upload successful";
        conn->WriteResponse();
        mylog::GetLogger("asynclogger")->Info("Upload:success");
    }

    std::string Service::TimetoStr(time_t t)
    {
        std::string tmp=std::ctime(&t);
        return tmp;
    }

    std::string Service::generateModernFileList(const std::vector<StorageInfo>& files)
    {
        std::stringstream ss;
        ss<<"<div class='file-list'><h3>已上传文件</h3>";
        for(const auto& file:files)
        {
            std::string filename =FileUtil(file.storage_path_).FileName();
            std::string storage_type="low";
            if(file.storage_path_.find("deep")!=std::string::npos)
            {
                storage_type="deep";
            }
            ss << "<div class='file-item'>"
               << "<div class='file-info'>"
               << "<span>📄" << filename << "</span>"
               << "<span class='file-type'>"
               << (storage_type == "deep" ? "深度存储" : "普通存储")
               << "</span>"
               << "<span>" << formatSize(file.fsize_) << "</span>"
               << "<span>" << TimetoStr(file.mtime_) << "</span>"
               << "</div>"
               << "<button onclick=\"window.location='" << file.url_ << "'\">⬇️ 下载</button>"
               << "</div>";
        }

        ss<<"</div>";
        return ss.str();
    }

    std::string Service::formatSize(uint16_t bytes)
    {
        const char *suffix[] = {"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
        int unit_index=0;
        double size=bytes;
        while(size>1024)
        {
            size/=1024;
            unit_index++;
        }
        std::stringstream ss;
        ss<<std::fixed<<std::setprecision(2)<<size<<' '<<suffix[unit_index];
        return ss.str();
    }

    void Service::ListShow(std::shared_ptr<HttpConnection> conn)
    {
        mylog::GetLogger("asynclogger")->Info("ListShow");
        std::vector<StorageInfo> files;
        data_->GetAll(&files);
        mylog::GetLogger("asynclogger")->Info("ListShow:files.size:%d",files.size());

        std::ifstream ifs("index.html");
        if (!ifs.is_open())
        {
            mylog::GetLogger("asynclogger")->Error("ListShow:Failed to open index.html");
            conn->_response.result(http::status::internal_server_error);
            conn->_response.set(http::field::content_type, "text/plain");
            beast::ostream(conn->_response.body()) << "Failed to open template file";
            conn->WriteResponse();
            return;
        }
        std::string templatecontent(
            (std::istreambuf_iterator<char>(ifs)),
            std::istreambuf_iterator<char>());

        templatecontent=std::regex_replace(templatecontent,
            std::regex("\\{\\{FILE_LIST\\}\\}"),
            generateModernFileList(files));

        // 修复BACKEND_URL替换逻辑
        std::string backend_url = "http://" + storage::Config::GetInstance()->GetServerIp() + ":" + 
            std::to_string(storage::Config::GetInstance()->GetServerPort());
            std::cout<<"ListShow:backend_url:"<<backend_url<<std::endl;
        templatecontent = std::regex_replace(templatecontent,
            std::regex("\\{\\{BACKEND_URL\\}\\}"),
            backend_url);

        conn->_response.result(http::status::ok);
        conn->_response.set(http::field::content_type, "text/html;charset=utf-8");
        conn->_response.set(http::field::server, "GateServer");
        beast::ostream(conn->_response.body()) << templatecontent;
        conn->WriteResponse();
        mylog::GetLogger("asynclogger")->Info("ListShow:success");
    }

    std::string Service::GetETag(const StorageInfo& info)
    {
        FileUtil fu(info.storage_path_);
        std::string etag=fu.FileName();
        etag+="-";
        etag+=std::to_string(info.fsize_);
        etag+="-";
        etag+=std::to_string(info.mtime_);
        return etag;
    }

    void Service::Download(std::shared_ptr<HttpConnection> conn)
    {
        mylog::GetLogger("asynclogger")->Info("Download");
        StorageInfo info;
        std::string resource_path = conn->_request.target().to_string();
        resource_path=UrlDecode(resource_path);
        data_->GetOneByURL(resource_path,&info);
        mylog::GetLogger("asynclogger")->Info("Download:resource_path:%s",resource_path.c_str());
        std::string download_path=info.storage_path_;

        if(info.storage_path_.find(Config::GetInstance()->GetLowStorageDir())==std::string::npos)
        {
            mylog::GetLogger("asynclogger")->Info("Download:UnCompress");
            FileUtil fu(info.storage_path_);
            download_path=Config::GetInstance()->GetLowStorageDir()+
                std::string(download_path.begin()+download_path.find_last_of("/"),download_path.end());
            FileUtil dirCreate(Config::GetInstance()->GetLowStorageDir());
            dirCreate.CreatDirectory();
            fu.UnCompress(download_path);
        }

        mylog::GetLogger("asynclogger")->Info("Download:download_path:%s",download_path.c_str());
        FileUtil fu(download_path);

        if(fu.Exists()==false&&info.storage_path_.find("deep_storage")!=std::string::npos)
        {
            mylog::GetLogger("asynclogger")->Error("Download:fu.Exists error");
            conn->_response.result(http::status::internal_server_error);
            conn->_response.set(http::field::content_type, "text/plain");
            beast::ostream(conn->_response.body()) << "Not Found";
            conn->WriteResponse();
            return;
        }
        else if(fu.Exists()==false&&info.storage_path_.find("low_storage")!=std::string::npos)
        {
           mylog::GetLogger("asynclogger")->Info("Download:fu.Exists success");
            conn->_response.result(http::status::bad_request);
            conn->_response.set(http::field::content_type, "text/plain");
            beast::ostream(conn->_response.body()) << "Not Found";
            conn->WriteResponse();
            return;
        }

        bool retrans=false;
        std::string old_etag;
        auto const& if_range = conn->_request[http::field::if_range];
        if(!if_range.empty())
        {
            old_etag = std::string(if_range);
            if(old_etag == GetETag(info))
            {
                retrans=true;
                mylog::GetLogger("asynclogger")->Info("Download:retrans");
            }
        }

        if(fu.Exists()==false)
        {
            mylog::GetLogger("asynclogger")->Error("Download:fu.Exists error");
            download_path += " not found";
            conn->_response.result(http::status::not_found);
            conn->_response.set(http::field::content_type, "text/plain");
            beast::ostream(conn->_response.body()) << "Not Found";
            conn->WriteResponse();
            return;
        }

        int fd=open(download_path.c_str(),O_RDONLY);
        if(fd==-1)
        {
            mylog::GetLogger("asynclogger")->Error("Download:open error");
            conn->_response.result(http::status::not_found);
            conn->_response.set(http::field::content_type, "text/plain");
            beast::ostream(conn->_response.body()) << "Not Found";
            conn->WriteResponse();
            return;
        }

        if(fu.FileSize()==0)
        {
            mylog::GetLogger("asynclogger")->Error("Download:fu.FileSize==0");
            conn->_response.result(http::status::not_found);
            conn->_response.set(http::field::content_type, "text/plain");
            beast::ostream(conn->_response.body()) << "Not Found";
            close(fd);
            conn->WriteResponse();
            return;
        }

        std::vector<char> buffer(fu.FileSize());
        ssize_t bytes_read = read(fd, buffer.data(), buffer.size());
        std::string content(buffer.begin(), buffer.end());
        //std::cout<<"content:"<<content<<std::endl;
        if(bytes_read==-1)
        {
            int err = errno;
            mylog::GetLogger("asynclogger")->Error("Download:read failed, errno=%d, msg=%s", err, strerror(err));
            conn->_response.result(http::status::not_found);
            conn->_response.set(http::field::content_type, "text/plain");
            beast::ostream(conn->_response.body()) << "Not Found";
            close(fd);
            conn->WriteResponse();
            return;
        }
        close(fd);

        beast::ostream(conn->_response.body()).write(buffer.data(), bytes_read);

        conn->_response.set(http::field::accept_ranges, "bytes");
        conn->_response.set(http::field::etag, GetETag(info));
        conn->_response.set(http::field::content_type, "application/octet-stream");
        conn->_response.prepare_payload();
        mylog::GetLogger("asynclogger")->Info("Download:retrans success");
        //conn->_response.content_length(buffer.size());
        if(retrans)
        {
            conn->_response.result(http::status::ok);
        }
        else
        {
            conn->_response.result(http::status::partial_content);
            mylog::GetLogger("asynclogger")->Info("Download:success");
        }
        conn->WriteResponse();

        if(download_path!=info.storage_path_)
        {
            remove(download_path.c_str());
        }
    }
}