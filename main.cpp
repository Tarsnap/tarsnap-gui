#include "coreapplication.h"
#include "debug.h"

int main(int argc, char *argv[])
{
    Debug::instance().initialize();
    CoreApplication app(argc, argv);
    return app.exec();
}
