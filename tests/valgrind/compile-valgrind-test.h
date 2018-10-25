#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMLEAKTEST(x)                                                         \
    {                                                                          \
        #x, x                                                                  \
    }

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
