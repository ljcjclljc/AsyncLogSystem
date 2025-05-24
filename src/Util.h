#pragma once
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/device/file.hpp>
#include "jsoncpp/json/json.h"
#include <cassert>
#include <sstream>
#include <memory>
#include "bundle.h"
#include "Config.h"
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <vector>
#include <fstream>
#include "../log/Mylog.h"
namespace storage{
    namespace fs=boost::filesystem;
    namespace io=boost::iostreams;
    //转化成16进制字符
    static unsigned char ToHex(unsigned char x)
    {
        // 0..9 => '0'..'9', 10..15 => 'A'..'F'
        return  x > 9 ? x + 55 : x + 48;
    }
    //将16进制字符转化成对应的数值
    static unsigned char FromHex(unsigned char x)
    {
        unsigned char y;
        if( x >= 'A' && x <= 'Z' )
            y = x - 'A' + 10;
        else if( x >= 'a' && x <= 'z' )
            y = x - 'a' + 10;
        else
            y = x - '0';
        return y;
    }

    static std::string UrlDecode(const std::string &str)
    {
        std::string result;
        result.reserve(str.size());
        
        for (size_t i = 0; i < str.size(); ++i) {
            if (str[i] == '%') {
                if (i + 2 < str.size()) {
                    unsigned char high = FromHex(static_cast<unsigned char>(str[++i]));
                    unsigned char low = FromHex(static_cast<unsigned char>(str[++i]));
                    result += high * 16 + low;
                } else {
                    mylog::GetLogger("asynclogger")->Info("UrlDecode: Incomplete percent encoding");
                    result += '%'; // 处理不完整的百分号编码
                }
            } else {
                result += str[i];
            }
        }
        
        return result;
    }

    class FileUtil
    {
        public:
        FileUtil(const std::string &path):path_(path){
            //std::cout<<path_<<std::endl;
        }

        const fs::path& path() const{return path_;}
        int64_t FileSize() const{
            try{
                return fs::file_size(path_);
            }catch(const fs::filesystem_error& e){
                mylog::GetLogger("asynclogger")->Info("FileSize: %s",e.what());
                return 0;
            }
        }

        time_t LastWriteTime() const{
            try{
                return fs::last_write_time(path_);
            }catch(const fs::filesystem_error& e){
                mylog::GetLogger("asynclogger")->Info("LastWriteTime: %s",e.what());
                return 0;
            }
        }

        time_t LastReadTime() const{
            try{
                return fs::last_write_time(path_);
            }catch(const fs::filesystem_error& e){
                mylog::GetLogger("asynclogger")->Info("LastWriteTime: %s",e.what());
                return 0;
            }
        }

        std::string FileName() const
        {
            return path_.filename().string();
        }

        bool GetPostLen(std::string *content,size_t pos,size_t len) const
        {
            try{
                if(pos+len>FileSize())
                {
                    mylog::GetLogger("asynclogger")->Info("GetPostLen: pos+len>FileSize()");
                    return false;
                }

                std::ifstream ifs(path_.string(),std::ios::binary);
                if(!ifs)
                {
                    mylog::GetLogger("asynclogger")->Info("GetPostLen: ifs");
                    return false;
                }

                ifs.seekg(pos,std::ios::beg);
                content->resize(len);
                ifs.read(&(*content)[0],len);

                if(!ifs)
                {
                    mylog::GetLogger("asynclogger")->Info("GetPostLen: ifs");
                    return false;
                }
                return true;
            }catch(const fs::filesystem_error& e){
                mylog::GetLogger("asynclogger")->Info("GetPostLen: %s",e.what());
                return false;
            }
        }

        bool GetContent(std::string *content) const
        {
            GetPostLen(content,0,FileSize());
            return true;
        }

        bool SetContent(const std::string &content,size_t len) const
        {
            try{
                std::ofstream ofs(path_.string(),std::ios::binary);
                if(!ofs)
                {
                    mylog::GetLogger("asynclogger")->Info("SetContent: ofs");
                    return false;
                }

                ofs.write(&content[0],len);
                if(!ofs)
                {
                    mylog::GetLogger("asynclogger")->Info("SetContent: ofs");
                    return false;
                }
                return true;
            }catch(const fs::filesystem_error& e){
                mylog::GetLogger("asynclogger")->Info("SetContent: %s",e.what());
                return false;
            }
        }

        //压缩
        bool Compress(const std::string &content,int format) const
        {
            try{
                std::string packed =bundle::pack(format,content);
                if(packed.empty())
                {
                    mylog::GetLogger("asynclogger")->Info("Compress: packed.empty()");
                    return false;
                }
                return SetContent(packed,packed.size());
            }catch(const fs::filesystem_error& e){
                mylog::GetLogger("asynclogger")->Info("Compress: %s",e.what());
                return false;
            }
        }

        //解压缩
        bool UnCompress(std::string &download_path) const
        {
            try{
                std::string content;
                if(!GetContent(&content))
                {
                    mylog::GetLogger("asynclogger")->Info("UnCompress: GetContent");
                    return false;
                }
                std::string unpacked =bundle::unpack(content);
                FileUtil fu(unpacked);
                if(!fu.SetContent(unpacked,unpacked.size()))
                {
                    mylog::GetLogger("asynclogger")->Info("UnCompress: SetContent");
                    return false;
                }
                return true;
            }catch(const fs::filesystem_error& e){
                mylog::GetLogger("asynclogger")->Info("UnCompress: %s",e.what());
                return false;
            }
        }

        bool Exists() const
        {
            return fs::exists(path_);
        }

        bool CreatDirectory() const
        {
                if(Exists())
            {
                return true;
            }
            return fs::create_directories(path_);
        }

        bool ScanDirectory(std::vector<std::string> *entries) const
        {
            try{
                if(!fs::is_directory(path_))
                {
                    mylog::GetLogger("asynclogger")->Info("ScanDirectory: !fs::is_directory(path_)");
                    return false;
                }

                entries->clear();
                for(auto &entry:fs::directory_iterator(path_))
                {
                    if(!fs::is_directory(entry.status()))
                    {
                        entries->push_back(entry.path().string());
                    }
                }

                return true;
            }catch(const fs::filesystem_error& e){
                mylog::GetLogger("asynclogger")->Info("ScanDirectory: %s",e.what());
                return false;
            }
        }
        private:
        fs::path path_;
    };

    class JsonUtil
    {
        public:
            static bool Serialize(const Json::Value &val,std::string *str)
            {
                try{
                    Json::StreamWriterBuilder swb;
                    swb["emitUTF8"] = true;
                    std::unique_ptr<Json::StreamWriter> usw(swb.newStreamWriter());
                    std::stringstream ss;
                    if(usw->write(val, &ss)!=0)
                    {
                        mylog::GetLogger("asynclogger")->Info("Serialize: usw->write");
                        return false;
                    }else{
                        mylog::GetLogger("asynclogger")->Info("Serialize: ss.str");
                        //std::cout<<ss.str();
                    }
                    *str = ss.str();
                    return true;
                }catch(const Json::LogicError& e){
                    mylog::GetLogger("asynclogger")->Info("Serialize: catch");
                    return false;
                }
            }
            
            static bool Unserialize(const std::string &str,Json::Value *val)
            {
                try{
                    Json::CharReaderBuilder rbuilder;
                    std::unique_ptr<Json::CharReader> r(rbuilder.newCharReader());
                    std::string errs;
                    if(r->parse(str.c_str(),str.c_str()+str.size(),val,&errs)==false)
                    {
                        mylog::GetLogger("asynclogger")->Info("Unserialize: r->parse");
                        return false;
                    }
                    return true;

                }catch(const Json::LogicError& e){
                    mylog::GetLogger("asynclogger")->Info("Unserialize: catch");
                    return false;
                }
            }
            //从文件加载Json文件
            static bool LoadFromFile(const std::string &filename,Json::Value *val)
            {
                try{
                    FileUtil fu(filename);
                    std::string content;
                    if(fu.GetContent(&content))
                    {
                        mylog::GetLogger("asynclogger")->Info("LoadFromFile: fu.GetContent");
                        return false;
                    }

                    return Unserialize(content,val);
                }catch(const fs::filesystem_error& e){
                    mylog::GetLogger("asynclogger")->Info("LoadFromFile: %s",e.what());
                    return false;
                }
            }

            static bool SaveToFile(const std::string &filename,const Json::Value &val)
            {
                try{
                    std::string content;
                    if(!Serialize(val,&content))
                    {
                        mylog::GetLogger("asynclogger")->Info("SaveToFile: Serialize");
                        return false;
                    }

                    FileUtil fu(filename);
                    return fu.SetContent(content,content.size());
                }catch(const fs::filesystem_error& e){
                    mylog::GetLogger("asynclogger")->Info("SaveToFile: %s",e.what());
                    return false;
                }
            }
    };
}