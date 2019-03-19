#ifndef MACROS_VALGRIND_H
#define MACROS_VALGRIND_H

#define T_TEST_CONSOLE_BEGIN                                                   \
    int   argc = 1;                                                            \
    char *argv[1];                                                             \
    argv[0]               = strdup("./potential-memleaks");                    \
    QCoreApplication *app = new QCoreApplication(argc, argv);

#define T_TEST_CONSOLE_END                                                     \
    delete app;                                                                \
    free(argv[0]);

#endif
