#pragma once
#include"DataManager.h"
#include<sys/queue.h>
#include<evhttp.h>
#include<event2/http.h>
#include<fcntl.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<regex>
#include"base64.h"

extern storage::DataManager *data_;

namespace storage
{
    class Service
    {
        private:
            int server_port;
            std::string server_ip;
            std::string download_prefix;
        public:
            Service()
            {
                server_port=Config::GetInstance()->GetServerPort();
                server_ip=Config::GetInstance()->GetServerIp();
                download_prefix=Config::GetInstance()->GetDownloadPrefix();
            }
            bool RunModule()
            {
                //初始化
                event_base *base=event_base_new();
                if(NULL==base)
                {
                    mylog::GetLogger("asynclogger")->Error("RunModule:base is null");
                    return false;
                }
                //设置端口地址
                sockaddr_in sin;
                memset(&sin,0,sizeof(sin));
                sin.sin_family=AF_INET;
                sin.sin_port=htons(server_port);
                //创建http服务器
                evhttp *http=evhttp_new(base);
                //绑定端口
                if(evhttp_bind_socket(http,"0.0.0.0",server_port)<0)
                {
                    mylog::GetLogger("asynclogger")->Error("RunModule:evhttp_bind_socket error");
                    return false;
                }
                //设置回调函数
                evhttp_set_gencb(http,GenHandler,NULL);
                if(base)
                {
                    if(-1==event_base_dispatch(base))
                    {
                        mylog::GetLogger("asynclogger")->Error("RunModule:event_base_dispatch error");
                        return false;
                    }
                }
                //std::cout<<"Service is over"<<std::endl;
                if(http)
                {
                    evhttp_free(http);
                }
                if(base)
                {
                    event_base_free(base);
                }
                return true;
            }
        private:
            static void GenHandler(struct evhttp_request *req,void *arg)
            {
                std::string path = evhttp_uri_get_path(evhttp_request_get_evhttp_uri(req));
                path=UrlDecode(path);
                mylog::GetLogger("asynclogger")->Info("GenHandler:path:%s",path.c_str());
                if(path.find("/download/")!=-1)
                {
                   Download(req,arg);
                }
                // 这里是上传
                else if (path == "/upload")
                {
                    Upload(req, arg);
                }
                // 这里就是显示已存储文件列表，返回一个html页面给浏览器
                else if (path == "/")
                {
                    ListShow(req, arg);
                }
                else
                {
                    evhttp_send_reply(req, HTTP_NOTFOUND, "Not Found", NULL);
                }
        }
        static void Upload(struct evhttp_request *req,void *arg)
        {
            mylog::GetLogger("asynclogger")->Info("Upload");
            struct evbuffer *buf = evhttp_request_get_input_buffer(req);
            if(buf==nullptr)
            {
                mylog::GetLogger("asynclogger")->Error("Upload:buf is null");
                return;
            }

            size_t len=evbuffer_get_length(buf);//获取缓冲区长度
            mylog::GetLogger("asynclogger")->Info("Upload:len:%d",len);
            if(len==0)
            {
                mylog::GetLogger("asynclogger")->Error("Upload:len is 0");
                return;
            }

            std::string content(len,0);
            if(evbuffer_copyout(buf,&content[0],len)!=len)
            {
                mylog::GetLogger("asynclogger")->Error("Upload:evbuffer_copyout error");
                return;
            }
            //获取文件名
            std::string filename =evhttp_find_header(req->input_headers,"Filename");
            filename=base64_decode(filename);//解码
            //获取储存类型
            std::string storage_type =evhttp_find_header(req->input_headers,"StorageType");
            
            //组织存储路径
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
                evhttp_send_reply(req, HTTP_NOTFOUND, "Not Found", NULL);
                return;
            }

            //组织文件路径
            FileUtil dirCreate(storage_path);
            dirCreate.CreatDirectory();

            storage_path+=filename;
            FileUtil fu(storage_path);
            if(storage_path.find("low_storage")!=std::string::npos)
            {
                if(fu.SetContent(content.c_str(),len)==false)
                {
                    mylog::GetLogger("asynclogger")->Error("Upload:fu.SetContent error");
                    evhttp_send_reply(req, HTTP_INTERNAL, "Not Found", NULL);
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
                    evhttp_send_reply(req, HTTP_INTERNAL, "Not Found", NULL);
                    return;
                }
                else
                {
                    mylog::GetLogger("asynclogger")->Info("Upload:fu.Compress success");
                }
            }

            //添加储存文件信息
            StorageInfo info;
            info.NewStorageInfo(storage_path);
            data_->Insert(info);
            evhttp_send_reply(req, HTTP_OK, "OK", NULL);
            mylog::GetLogger("asynclogger")->Info("Upload:success");
        }   

        static std::string TimetoStr(time_t t)
        {
            std::string tmp=std::ctime(&t);
            return tmp;
        }
        //前段代码处理函数
        //在渲染函数中直接处理StorageInfo
        static std::string generateModernFileList(const std::vector<StorageInfo>& files)
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
        //文件大小格式化函数
        static std::string formatSize(uint16_t bytes)
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
        static void ListShow(struct evhttp_request *req,void* arg)
        {
            mylog::GetLogger("asynclogger")->Info("ListShow");
            //获取文件列表
            std::vector<StorageInfo> files;
            data_->GetAll(&files);
            mylog::GetLogger("asynclogger")->Info("ListShow:files.size:%d",files.size());
            //读取模板文件
            std::ifstream ifs("index.html");
            std::string templatecontent(
                (std::istreambuf_iterator<char>(ifs)),
                std::istreambuf_iterator<char>()
            );
            //替换模板文件中的内容
            templatecontent=std::regex_replace(templatecontent,
                std::regex("\\{\\{FILE_LIST\\}\\}"),
                generateModernFileList(files));

            //替换服务器地址
            templatecontent = std::regex_replace(templatecontent,
                std::regex("\\{\\{BACKEND_URL\\}\\}"),
               "http://"+storage::Config::GetInstance()->GetServerIp()+":"+std::to_string(storage::Config::GetInstance()->GetServerPort()));
               //获取请求的输出缓冲区
               struct evbuffer *buf = evhttp_request_get_output_buffer(req);
               auto response_body=templatecontent;
               //把HTML页面写到输出缓冲区，设置响应头，返回给浏览器
               evbuffer_add(buf, response_body.c_str(), response_body.size());
               evhttp_add_header(req->output_headers, "Content-Type", "text/html;charset=utf-8");
               evhttp_send_reply(req, HTTP_OK, "OK", buf);
               mylog::GetLogger("asynclogger")->Info("ListShow:success");
            }

            static std::string GetETag(const StorageInfo& info)
            {
                //自定义etag
                FileUtil fu(info.storage_path_);
                std::string etag=fu.FileName();
                etag+="-";
                etag+=std::to_string(info.fsize_);
                etag+="-";
                etag+=std::to_string(info.mtime_);
                return etag;
            }

            static void Download(struct evhttp_request *req,void* arg)  
            {
                //获取客户端的资源路径
                //根据资源路径获取文件信息
                StorageInfo info;
                //data_->GetOneByStoragePath(resource_path,&info);
                std::string resource_path=evhttp_uri_get_path(evhttp_request_get_evhttp_uri(req));
                resource_path=UrlDecode(resource_path);
                data_->GetOneByURL(resource_path,&info);
                mylog::GetLogger("asynclogger")->Info("Download:resource_path:%s",resource_path.c_str());
                std::string download_path=info.storage_path_;
                //如果压缩过了就解压缩到新文件给用户下载
                if(info.storage_path_.find(Config::GetInstance()->GetLowStorageDir())==std::string::npos)
                {
                    mylog::GetLogger("asynclogger")->Info("Download:UnCompress");
                    FileUtil fu(info.storage_path_);
                    download_path=Config::GetInstance()->GetLowStorageDir()+
                        std::string(download_path.begin()+download_path.find_last_of("/"),download_path.end());
                    FileUtil dirCreate(Config::GetInstance()->GetLowStorageDir());
                    dirCreate.CreatDirectory();
                    fu.UnCompress(download_path);//解压缩到新文件
                }
                mylog::GetLogger("asynclogger")->Info("Download:download_path:%s",download_path.c_str());
                FileUtil fu(download_path);
                //std::cout<<"---------------------------------"<<'\n';
                //std::cout<<fu.FileName()<<std::endl;
                if(fu.Exists()==false&&info.storage_path_.find("depp_storage")!=std::string::npos)
                {
                    mylog::GetLogger("asynclogger")->Error("Download:fu.Exists error");
                    evhttp_send_reply(req, HTTP_INTERNAL, "Not Found", NULL);
                    return;
                }
                else if(fu.Exists()==false&&info.storage_path_.find("low_storage")!=std::string::npos)
                {
                    mylog::GetLogger("asynclogger")->Info("Download:fu.Exists success");
                    evhttp_send_reply(req, HTTP_BADREQUEST, "Not Found", NULL);
                    return;
                }
                //3.确认文件是否需断点续传
                bool retrans=false;
                std::string old_etag;
                auto if_range=evhttp_find_header(req->input_headers,"If-Range");
                if(NULL!=if_range)
                {
                    old_etag=if_range;
                    // 有If-Range字段且，这个字段的值与请求文件的最新etag一致则符合断点续传
                    if(old_etag==GetETag(info))
                    {
                        retrans=true;
                        mylog::GetLogger("asynclogger")->Info("Download:retrans");
                    }
                }
                //4.读取文件数据，放入rsp。body
                if(fu.Exists()==false)
                {
                    mylog::GetLogger("asynclogger")->Error("Download:fu.Exists error");
                    download_path+="not found";
                    evhttp_send_reply(req, 404, "Not Found", NULL);
                    return;
                }
                evbuffer *buf = evhttp_request_get_output_buffer(req);
                int fd=open(download_path.c_str(),O_RDONLY);
                if(fd==-1)
                {
                    mylog::GetLogger("asynclogger")->Error("Download:open error");
                    evhttp_send_reply(req, 404, "Not Found", NULL);
                    return;
                }
                if(buf==NULL)
                {
                    mylog::GetLogger("asynclogger")->Error("Download:buf is null");
                    evhttp_send_reply(req, 404, "Not Found", NULL);
                    return;
                }
                if(fu.FileSize()==0)
                {
                    mylog::GetLogger("asynclogger")->Error("Download:fu.FileSize==0");
                    evhttp_send_reply(req, 404, "Not Found", NULL);
                    return;
                }
                if(-1==evbuffer_add_file(buf,fd,0,fu.FileSize()))
                {
                    int err = errno;  // 立即保存errno，避免被后续函数覆盖
                    mylog::GetLogger("asynclogger")->Error("Download:evbuffer_add_file failed, errno=%d, msg=%s", err, strerror(err));
                    evhttp_send_reply(req, 404, "Not Found", NULL);
                    return;
                }
                //5.设置响应头

                evhttp_add_header(req->output_headers,"Accept-Ranges","bytes");
                evhttp_add_header(req->output_headers, "ETag", GetETag(info).c_str());
                evhttp_add_header(req->output_headers, "Content-Type", "application/octet-stream");
                if(retrans)
                {
                    evhttp_send_reply(req, HTTP_OK, "Success", NULL);
                    mylog::GetLogger("asynclogger")->Info("Download:retrans success");
                }
                else
                {
                    evhttp_send_reply(req, 206, "breakpoint continue transmission", NULL);
                    mylog::GetLogger("asynclogger")->Info("Download:success");
                }
                if(download_path!=info.storage_path_)
                {
                    remove(download_path.c_str());
                }
            }
    };  
}