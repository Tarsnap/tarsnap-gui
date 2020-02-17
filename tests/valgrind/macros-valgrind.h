#ifndef MACROS_VALGRIND_H
#define MACROS_VALGRIND_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* clang-format off */
#define MEMLEAKTEST(x)                                                         \
    {                                                                          \
        #x, x                                                                  \
    }
/* clang-format on */

#define T_TEST_BEGIN                                                           \
    static const struct memleaktest                                            \
    {                                                                          \
        const char *const name;                                                \
        void (*volatile const func)(void);                                     \
    } tests[] = {

#define T_TEST_END                                                             \
    }                                                                          \
    ;                                                                          \
    static const int num_tests = sizeof(tests) / sizeof(tests[0]);             \
                                                                               \
    int main(int argc, char *argv[])                                           \
    {                                                                          \
        int i;                                                                 \
                                                                               \
        if(argc == 2)                                                          \
        {                                                                      \
            for(i = 0; i < num_tests; i++)                                     \
            {                                                                  \
                if((strcmp(argv[1], tests[i].name)) == 0)                      \
                    tests[i].func();                                           \
            }                                                                  \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            for(i = 0; i < num_tests; i++)                                     \
                printf("%s\n", tests[i].name);                                 \
        }                                                                      \
        exit(0);                                                               \
    }

#define T_APP_BEGIN_CONSOLE                                                    \
    int   argc = 1;                                                            \
    char *argv[1];                                                             \
    argv[0]               = strdup("./potential-memleaks");                    \
    QCoreApplication *app = new QCoreApplication(argc, argv)

#define T_APP_END                                                              \
    delete app;                                                                \
    free(argv[0])

#define T_APP_BEGIN_GUI                                                        \
    int   argc = 1;                                                            \
    char *argv[1];                                                             \
    argv[0]           = strdup("./potential-memleaks");                        \
    QApplication *app = new QApplication(argc, argv)

#define T_APP_END_GUI                                                          \
    delete app;                                                                \
    free(argv[0])

#endif
