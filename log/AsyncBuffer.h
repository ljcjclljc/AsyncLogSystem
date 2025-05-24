#pragma
#include<cassert>
#include<string>
#include<boost/circular_buffer.hpp>
#include<utility>
#include"Util.h"

extern mylog::Util::JsonData* g_conf_data;

namespace mylog{
    class Buffer{
        public:
        //构造函数
        Buffer():write_pos_(0),read_pos_(0){
            buffer_.set_capacity(g_conf_data->buffer_size);
            };
        
            //写入数据
            void Push(const char* data,size_t len)
            {
                ToBeEnough(len);//确保容量足够
                //开始写如
                for(size_t i=0;i<len;i++)
                {
                    buffer_.push_back(data[i]);
                }
                write_pos_+=len;
            }

            char *ReadBegin(int len)
            {
                assert(len<=ReadableSize());
                return &buffer_[read_pos_];
            }
            //判断有五可读内容
            bool IsEmpty(){ return write_pos_ == read_pos_;}
            //交换数组
            void Swap(Buffer &buf)
            {
                buffer_.swap(buf.buffer_);
                std::swap(read_pos_,buf.read_pos_);
                std::swap(write_pos_,buf.write_pos_);
            }
            //写剩余的空间
            size_t WriteableSize()
            {
                return buffer_.capacity()-buffer_.size();
            }
            //读剩余空间
            size_t ReadableSize()
            {
                return write_pos_-read_pos_;
            }

            const char *Begin(){ return &buffer_[read_pos_];}

            void MoveWritePos(int len)
            {
                assert(len<=WriteableSize());
                write_pos_+=len;
            }

            void MoveReadPos(int len)
            {
                assert(len<=ReadableSize());
                read_pos_+=len;
            }

            void Reset(){
                buffer_.clear();
                write_pos_=0;
                read_pos_=0;
            }
        protected:
            void ToBeEnough(size_t len){
                int buffersize=buffer_.capacity();
                if(len>=WriteableSize()){
                    if(buffer_.capacity()<g_conf_data->threshold)
                    {
                        buffer_.set_capacity(2*buffer_.capacity()+buffersize);
                    }else{
                        buffer_.set_capacity(g_conf_data->linear_growth+buffersize);
                    }
                }
            }
        protected:
            boost::circular_buffer<char> buffer_;//缓冲区
            size_t write_pos_;//生产者位置
            size_t read_pos_;//消费者位置
    };
}