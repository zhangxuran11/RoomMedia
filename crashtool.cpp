#include <string>
#include <memory>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include<QMap>
#define nullptr  NULL
#ifdef __linux
#include <execinfo.h>
#endif // __linux
static QMap<int,__sighandler_t> mapSighandler;
static int backtrace_xy(void **BUFFER, int SIZE)
{
    volatile int n = 0;
    volatile int *p;
    volatile int *q;
    volatile int ebp1;
    volatile int eip1;
    volatile int i = 0;
    p = &n;
    ebp1 = p[4];
    eip1 = p[6];
    fprintf(stderr, "======================= backtrace_xy addr: 0x%0x, param1: 0x%0x, param2: 0x%0x\n",
             (unsigned int)backtrace_xy, (unsigned int)&BUFFER, (unsigned int)&SIZE);
    fprintf(stderr, "n addr is 0x%0x\n", (unsigned int)&n);
    fprintf(stderr, "p addr is 0x%0x\n", (unsigned int)&p);
    for (i = 0; i < SIZE; i++)
    {
        fprintf(stderr, "ebp1 is 0x%0x, eip1 is 0x%0x\n", ebp1, eip1);
        BUFFER[i] = (void *)eip1;
        p = (int*)ebp1;
        q = p - 5;
        eip1 = q[5];
        ebp1 = q[2];
        if (ebp1 == 0 || eip1 == 0)
            break;
    }
    fprintf(stderr, "total level: %d\n", i);
    return i;
}
const int MAX_STACK_FRAMES = 32;
void OnCrashHandler(int signum)
{
    FILE* f = fopen("/crash.txt", "at");
    if (nullptr == f)
    {
        printf("err open /crash.txt");
        exit(1);
        mapSighandler[signum](signum);
        return;
    }
    try
   {
        printf("success open /crash.txt");
        char szLine[512] = {0, };
        time_t t = time(nullptr);
        tm* now = localtime(&t);
        sprintf(szLine,
                        "#########################################################\n[%04d-%02d-%02d %02d:%02d:%02d][crash signal number:%d]\n",
                        now->tm_year + 1900,
                        now->tm_mon + 1,
                        now->tm_mday,
                        now->tm_hour,
                        now->tm_min,
                        now->tm_sec,
                        signum);
         fwrite(szLine, 1, strlen(szLine), f);
#ifdef __linux
         void* array[MAX_STACK_FRAMES];
         size_t size = 0;
         char** strings = nullptr;
         size_t i;
         signal(signum, SIG_DFL);
         size = backtrace(array, MAX_STACK_FRAMES);
         strings = (char**)backtrace_symbols(array, size);
         fprintf(stderr, "oncrash;\n");
         for (i = 0; i < size; ++i)
         {
#if 0
              std::string symbol(strings[i]);
              std::string::size_type pos1 = symbol.find_first_of("[");
              std::string::size_type pos2 = symbol.find_last_of("]");
              std::string address = symbol.substr(pos1 + 1, pos2 - pos1 -1);
              char cmd[128] = {0, };
              sprintf(cmd, "addr2line -e /appbin/RoomMedia %p", address.c_str());
              system(cmd);
#endif
              char szLine[512] = {0, };
              sprintf(szLine, "%d %s\n", i, strings[i]);
              fwrite(szLine, 1, strlen(szLine), f);
              fprintf(stderr, "%d %s\n",i, strings[i]);
         }
         free(strings);
#endif // __linux
   }
   catch (...)
   {
       //
    }
    fflush(f);
    fclose(f);
    f = nullptr;
    mapSighandler[signum](signum);
    exit(1);
}

void EnableCrashHandler()
{
#ifdef __linux
   mapSighandler.insert(SIGSEGV,signal(SIGSEGV, OnCrashHandler));
   mapSighandler.insert(SIGABRT,signal(SIGABRT, OnCrashHandler));
   mapSighandler.insert(SIGTERM,signal(SIGTERM, OnCrashHandler));
   mapSighandler.insert(SIGSTOP,signal(SIGSTOP, OnCrashHandler));
   mapSighandler.insert(SIGTSTP,signal(SIGTSTP, OnCrashHandler));
   mapSighandler.insert(SIGKILL,signal(SIGKILL, OnCrashHandler));
   mapSighandler.insert(SIGINT ,signal(SIGINT , OnCrashHandler));
   mapSighandler.insert(SIGQUIT,signal(SIGQUIT, OnCrashHandler));
   mapSighandler.insert(SIGILL ,signal(SIGILL , OnCrashHandler));
   mapSighandler.insert(SIGTRAP,signal(SIGTRAP, OnCrashHandler));
   mapSighandler.insert(SIGBUS ,signal(SIGBUS , OnCrashHandler));
#endif // __linux
}

