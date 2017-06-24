#include "rlog.h"
#include <unistd.h>

int main(int argc, char** argv)
{
    LOG_MEM_SET(30 * 1024 * 1024);
    LOG_INIT("log", "myname", 3);
    for (int i = 0;i < 1e3; ++i)
    {
        LOG_ERROR("my number is number my number is my number is my number is my number is my number is my number is %d", i);
    }
}
