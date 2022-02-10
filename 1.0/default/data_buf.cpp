/*
 *  data_buf.cpp - data buffer functions.
 *
 *  Copyright (C) 2016 Spreadtrum Communications Inc.
 *
 *  History:
 *  2019-06-24 Gloria.He
 */

#include "data_buf.h"

DataBuffer::DataBuffer()
    :DataBuffer{nullptr, 0, 0, 0, -1} {}

DataBuffer::DataBuffer(char* buf, size_t sz,
                       size_t ds, size_t dl,
                       int doff)
    :buffer{buf},
     buf_size{sz},
     data_start{ds},
     data_len{dl},
     dst_offset{doff} {}

DataBuffer::~DataBuffer() {
  if (buffer) {
    delete [] buffer;
  }
}
