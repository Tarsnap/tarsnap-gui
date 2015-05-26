#include "coreapplication.h"
#include "debug.h"

int main(int argc, char *argv[])
{
    Debug::instance().initialize();
    CoreApplication app(argc, argv);
    int ret = app.initialize();
    if(ret != 0)
        return ret;
    else
        return app.exec();
}
