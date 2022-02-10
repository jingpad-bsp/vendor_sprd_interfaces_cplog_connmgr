/*
 *  data_buf.cpp - data buffer functions.
 *
 *  Copyright (C) 2016 Spreadtrum Communications Inc.
 *
 *  History:
 *  2019-06-24 Gloria.He
 */
#ifndef DATA_BUF_H_
#define DATA_BUF_H_

#include <cstddef>
#include <cstdint>

struct DataBuffer {
  char* buffer;
  size_t buf_size;
  size_t data_start;
  size_t data_len;
  int dst_offset;

  DataBuffer();
  DataBuffer(char* buf, size_t sz, size_t ds, size_t dl, int doff);
  DataBuffer(const DataBuffer&) = delete;
  ~DataBuffer();

  DataBuffer& operator = (const DataBuffer&) = delete;
};

#endif  //!DATA_BUF_H_
