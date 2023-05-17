//***************************************************************************//
//* 版权所有  www.mediapro.cc
//*
//* 内容摘要：视频文件帧级读取
//*	
//* 当前版本：V1.0		
//* 作    者：mediapro
//* 完成日期：2020-03-13
//**************************************************************************//
#ifndef __SD_FILE_PARSE_COMMON_H__
#define __SD_FILE_PARSE_COMMON_H__
#include <sys/types.h>
#include <sys/stat.h>
#include "SDLog.h"
#include <stdint.h>

/**
 * bytes utility, used to:
 * convert basic types to bytes,
 * build basic types from bytes.
 * @remark the buffer never mange the bytes, user must manage it.
 */
class SrsBuffer
{
private:
    // current position at bytes.
    char* p;
    // the bytes data for stream to read or write.
    char* bytes;
    // the total number of bytes.
    int nb_bytes;
public:
    SrsBuffer();
    SrsBuffer(char* b, int nb_b);
    virtual ~SrsBuffer();
private:
    virtual void set_value(char* b, int nb_b);
public:
    /**
     * initialize the stream from bytes.
     * @b, the bytes to convert from/to basic types.
     * @nb, the size of bytes, total number of bytes for stream.
     * @remark, stream never free the bytes, user must free it.
     * @remark, return error when bytes NULL.
     * @remark, return error when size is not positive.
     */
    virtual int initialize(char* b, int nb);
    // get the status of stream
public:
    /**
     * get data of stream, set by initialize.
     * current bytes = data() + pos()
     */
    virtual char* data();
    /**
     * the total stream size, set by initialize.
     * left bytes = size() - pos().
     */
    virtual int size();
    /**
     * tell the current pos.
     */
    virtual int pos();
    /**
     * whether stream is empty.
     * if empty, user should never read or write.
     */
    virtual bool empty();
    /**
     * whether required size is ok.
     * @return true if stream can read/write specified required_size bytes.
     * @remark assert required_size positive.
     */
    virtual bool require(int required_size);
    // to change stream.
public:
    /**
     * to skip some size.
     * @param size can be any value. positive to forward; nagetive to backward.
     * @remark to skip(pos()) to reset stream.
     * @remark assert initialized, the data() not NULL.
     */
    virtual void skip(int size);
public:
    /**
     * get 1bytes char from stream.
     */
    virtual int8_t read_1bytes();
    /**
     * get 2bytes int from stream.
     */
    virtual int16_t read_2bytes();
    /**
     * get 3bytes int from stream.
     */
    virtual int32_t read_3bytes();
    /**
     * get 4bytes int from stream.
     */
    virtual int32_t read_4bytes();
    /**
     * get 8bytes int from stream.
     */
    virtual int64_t read_8bytes();
    /**
     * get string from stream, length specifies by param len.
     */
    virtual std::string read_string(int len);
    /**
     * get bytes from stream, length specifies by param len.
     */
    virtual void read_bytes(char* data, int size);
public:
    /**
     * write 1bytes char to stream.
     */
    virtual void write_1bytes(int8_t value);
    /**
     * write 2bytes int to stream.
     */
    virtual void write_2bytes(int16_t value);
    /**
     * write 4bytes int to stream.
     */
    virtual void write_4bytes(int32_t value);
    /**
     * write 3bytes int to stream.
     */
    virtual void write_3bytes(int32_t value);
    /**
     * write 8bytes int to stream.
     */
    virtual void write_8bytes(int64_t value);
    /**
     * write string to stream
     */
    virtual void write_string(std::string value);
    /**
     * write bytes to stream
     */
    virtual void write_bytes(char* data, int size);
};

bool srs_h264_startswith_annexb(char* h264_raw_data, int h264_raw_size, int* pnb_start_code);

#endif	// __SD_H265_FILE_PARSE_H__