#include <QApplication>
#include <QCommandLineParser>
#include <QStringList>

#include "testCFSM.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addPositionalArgument("scenario_num",
            QCoreApplication::translate("main", "Run a single scenario."));
    parser.process(app);

    // By default, run all scenarios
    int scenario_num = -1;
    const QStringList args = parser.positionalArguments();
    if (args.size() > 0)
        scenario_num = args.at(0).toInt();

    TestCFSM tester(scenario_num);
    return app.exec();
}
