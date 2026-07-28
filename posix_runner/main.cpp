#include "stream_init.hpp"

#include <cstdio>

int main()
{
    std::printf("POSIX Runner\n");

    int err = stream_configure_and_start();
    if (err != 0) {
        return err;
    }
    stream_wait_for_threads_end();
    stream_free_all(false);

    return 0;
}
