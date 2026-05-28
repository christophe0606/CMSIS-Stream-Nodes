#pragma once

#include <cstdio>

#define CMSISSTREAM_EVENT_QUEUE_LENGTH 20
#define CMSISSTREAM_MAX_NUMBER_EVENT_ARGUMENTS 8
#define CMSISSTREAM_TENSOR_MAX_DIMENSIONS 3

#define CMSISSTREAM_LOG_ERR(fmt, ...) std::fprintf(stderr, "[ERR] " fmt, ##__VA_ARGS__)
#define CMSISSTREAM_LOG_DBG(fmt, ...) std::fprintf(stderr, "[DBG] " fmt, ##__VA_ARGS__)
