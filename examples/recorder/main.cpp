#include "stream_init.hpp"

#include <cstdio>

int main()
{
    std::printf("Recorder POSIX CMSIS-Stream\n");

    stream_configure_and_start();
    stream_wait_for_threads_end();
    stream_free_all(false);

    return 0;
}
