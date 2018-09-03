#include "coreapplication.h"
#include "debug.h"

extern "C" {
#include "optparse.h"
#include "warnp.h"
}

#include <stdlib.h>

int main(int argc, char *argv[])
{
    struct optparse *opt;

    WARNP_INIT;

    if((opt = optparse_parse(argc, argv)) == NULL)
        exit(1);

    ConsoleLog::instance().initializeConsoleLog();
    CoreApplication app(argc, argv, opt);
    optparse_free(opt);
    if(app.initializeCore())
        return app.exec();
    else
        return EXIT_SUCCESS;
}
