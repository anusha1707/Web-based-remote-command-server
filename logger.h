#ifndef WRC_LOGGER_H
#define WRC_LOGGER_H

#ifdef LOGGING_ENABLED

#define DEBUG(fmt, arg...)                                                 \
    do                                                                     \
    {                                                                      \
        fprintf(stdout,                                                    \
                "\x1b[36m[DEBUG]\x1b[0m \x1b[90m%s:%d\x1b[0m : " fmt "\n", \
                __FILE__,                                                  \
                __LINE__,                                                  \
                ##arg);                                                    \
        fflush(stdout);                                                    \
    } while (0)

#define INFO(fmt, arg...)                                                  \
    do                                                                     \
    {                                                                      \
        fprintf(stdout,                                                    \
                "\x1b[32m[INFO]\x1b[0m  \x1b[90m%s:%d\x1b[0m : " fmt "\n", \
                __FILE__,                                                  \
                __LINE__,                                                  \
                ##arg);                                                    \
        fflush(stdout);                                                    \
    } while (0)

#define WARN(fmt, arg...)                                                  \
    do                                                                     \
    {                                                                      \
        fprintf(stdout,                                                    \
                "\x1b[33m[WARN]\x1b[0m  \x1b[90m%s:%d\x1b[0m : " fmt "\n", \
                __FILE__,                                                  \
                __LINE__,                                                  \
                ##arg);                                                    \
        fflush(stdout);                                                    \
    } while (0)

#define ERROR(fmt, arg...)                                                 \
    do                                                                     \
    {                                                                      \
        fprintf(stdout,                                                    \
                "\x1b[31m[ERROR] %s:%d : " fmt "\x1b[0m\n",                \
                __FILE__,                                                  \
                __LINE__,                                                  \
                ##arg);                                                    \
        fflush(stdout);                                                    \
    } while (0)

#else

#define DEBUG(fmt, arg...)
#define INFO(fmt, arg...)
#define WARN(fmt, arg...)
#define ERROR(fmt, arg...)

#endif

#endif
