//***************************************************************************//
//* 版权所有  www.mediapro.cc
//*
//* 内容摘要：文件帧级读取，目前只支持单帧单Slice
//*	
//* 当前版本：V1.0		
//* 作    者：mediapro
//* 完成日期：2020-03-13
//**************************************************************************//
#include <string>
#include "SDLog.h" 
#include "SDFileParseCommon.h"

#define ERROR_SUCCESS						0L
#define ERROR_KERNEL_STREAM_INIT            3038

#include <assert.h>
#define srs_assert(expression) assert(expression)
using namespace std;

static bool srs_avc_startswith_annexb(SrsBuffer* stream, int* pnb_start_code)
{
    char* bytes = stream->data() + stream->pos();
    char* p = bytes;
    
    for (;;) {
        if (!stream->require((int)(p - bytes + 3))) {
            return false;
        }
        
        // not match
        if (p[0] != (char)0x00 || p[1] != (char)0x00) {
            return false;
        }
        
        // match N[00] 00 00 01, where N>=0
        if (p[2] == (char)0x01) {
            if (pnb_start_code) {
                *pnb_start_code = (int)(p - bytes) + 3;
            }
            return true;
        }
        
        p++;
    }
    
    return false;
}

bool srs_h264_startswith_annexb(char* h264_raw_data, int h264_raw_size, int* pnb_start_code)
{
    SrsBuffer stream;
    if (stream.initialize(h264_raw_data, h264_raw_size) != ERROR_SUCCESS) 
    {
        return false;
    }
    
    return srs_avc_startswith_annexb(&stream, pnb_start_code);
}
   

static bool srs_is_little_endian()
{
	// convert to network(big-endian) order, if not equals,
	// the system is little-endian, so need to convert the int64
	static int little_endian_check = -1;

	if (little_endian_check == -1) {
		union {
			int32_t i;
			int8_t c;
		} little_check_union;

		little_check_union.i = 0x01;
		little_endian_check = little_check_union.c;
	}

	return (little_endian_check == 1);
}


SrsBuffer::SrsBuffer()
{
	set_value(NULL, 0);
}

SrsBuffer::SrsBuffer(char* b, int nb_b)
{
	set_value(b, nb_b);
}

SrsBuffer::~SrsBuffer()
{
}

void SrsBuffer::set_value(char* b, int nb_b)
{
	p = bytes = b;
	nb_bytes = nb_b;

	// TODO: support both little and big endian.
	srs_assert(srs_is_little_endian());
}

int SrsBuffer::initialize(char* b, int nb)
{
	int ret = ERROR_SUCCESS;

	if (!b) {
		ret = ERROR_KERNEL_STREAM_INIT;
		SDLOG_PRINTF("H264FilePase", SD_LOG_LEVEL_ERROR, "stream param bytes must not be NULL. ret=%d", ret);
		return ret;
	}

	if (nb <= 0) {
		ret = ERROR_KERNEL_STREAM_INIT;
		SDLOG_PRINTF("H264FilePase", SD_LOG_LEVEL_ERROR, "stream param size must be positive. ret=%d", ret);
		return ret;
	}

	nb_bytes = nb;
	p = bytes = b;
	//SDLOG_PRINTF("H264FilePase", SD_LOG_LEVEL_DEBUG, "init stream ok, size=%d", size());

	return ret;
}

char* SrsBuffer::data()
{
	return bytes;
}

int SrsBuffer::size()
{
	return nb_bytes;
}

int SrsBuffer::pos()
{
	return (int)(p - bytes);
}

bool SrsBuffer::empty()
{
	return !bytes || (p >= bytes + nb_bytes);
}

bool SrsBuffer::require(int required_size)
{
	srs_assert(required_size >= 0);

	return required_size <= nb_bytes - (p - bytes);
}

void SrsBuffer::skip(int size)
{
	srs_assert(p);

	p += size;
}

int8_t SrsBuffer::read_1bytes()
{
	srs_assert(require(1));

	return (int8_t)*p++;
}

int16_t SrsBuffer::read_2bytes()
{
	srs_assert(require(2));

	int16_t value;
	char* pp = (char*)&value;
	pp[1] = *p++;
	pp[0] = *p++;

	return value;
}

int32_t SrsBuffer::read_3bytes()
{
	srs_assert(require(3));

	int32_t value = 0x00;
	char* pp = (char*)&value;
	pp[2] = *p++;
	pp[1] = *p++;
	pp[0] = *p++;

	return value;
}

int32_t SrsBuffer::read_4bytes()
{
	srs_assert(require(4));

	int32_t value;
	char* pp = (char*)&value;
	pp[3] = *p++;
	pp[2] = *p++;
	pp[1] = *p++;
	pp[0] = *p++;

	return value;
}

int64_t SrsBuffer::read_8bytes()
{
	srs_assert(require(8));

	int64_t value;
	char* pp = (char*)&value;
	pp[7] = *p++;
	pp[6] = *p++;
	pp[5] = *p++;
	pp[4] = *p++;
	pp[3] = *p++;
	pp[2] = *p++;
	pp[1] = *p++;
	pp[0] = *p++;

	return value;
}

string SrsBuffer::read_string(int len)
{
	srs_assert(require(len));

	std::string value;
	value.append(p, len);

	p += len;

	return value;
}

void SrsBuffer::read_bytes(char* data, int size)
{
	srs_assert(require(size));

	memcpy(data, p, size);

	p += size;
}

void SrsBuffer::write_1bytes(int8_t value)
{
	srs_assert(require(1));

	*p++ = value;
}

void SrsBuffer::write_2bytes(int16_t value)
{
	srs_assert(require(2));

	char* pp = (char*)&value;
	*p++ = pp[1];
	*p++ = pp[0];
}

void SrsBuffer::write_4bytes(int32_t value)
{
	srs_assert(require(4));

	char* pp = (char*)&value;
	*p++ = pp[3];
	*p++ = pp[2];
	*p++ = pp[1];
	*p++ = pp[0];
}

void SrsBuffer::write_3bytes(int32_t value)
{
	srs_assert(require(3));

	char* pp = (char*)&value;
	*p++ = pp[2];
	*p++ = pp[1];
	*p++ = pp[0];
}

void SrsBuffer::write_8bytes(int64_t value)
{
	srs_assert(require(8));

	char* pp = (char*)&value;
	*p++ = pp[7];
	*p++ = pp[6];
	*p++ = pp[5];
	*p++ = pp[4];
	*p++ = pp[3];
	*p++ = pp[2];
	*p++ = pp[1];
	*p++ = pp[0];
}

void SrsBuffer::write_string(string value)
{
	srs_assert(require((int)value.length()));

	memcpy(p, value.data(), value.length());
	p += value.length();
}

void SrsBuffer::write_bytes(char* data, int size)
{
	srs_assert(require(size));

	memcpy(p, data, size);
	p += size;
}

