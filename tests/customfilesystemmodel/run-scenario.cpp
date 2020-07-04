#include "run-scenario.h"

WARNINGS_DISABLE
#include <QChar>
#include <QCharRef>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QTextStream>
#include <QVariant>
#include <Qt>
WARNINGS_ENABLE

#include <stdio.h>

#include "../qtest-platform.h"

RunScenario::RunScenario()
    : _rootDir(QDir::currentPath() + QDir::separator() + "dirs")
{
    // Set up model.
    _model.setReadOnly(true);
    _model.setRootPath(_rootDir);

    // In the GUI, we cannot select a file or directory until its parent
    // directory has been loaded.  This is not a perfect imitation of that
    // scenario, but it is better than nothing.
    WAIT_UNTIL(!_model.needToReadSubdirs(_rootDir));
}

// The format of these lines in the scenario file is:
// X filename
// where X is a single character, followed by a space.
QString RunScenario::getRelname(const QString &line)
{
    QString relname = line.right(line.size() - 2);
    return relname;
}

QModelIndex RunScenario::getIndex(const QString &line)
{
    QString     relname  = getRelname(line);
    QString     filename = QDir(_rootDir).filePath(relname);
    QModelIndex index    = _model.index(filename);
    return index;
}

int RunScenario::getLineState(const QString &line)
{
    int state = line[0].digitValue();
    return state;
}

int RunScenario::getCheckedStateInt(const QString &line)
{
    QString     relname  = line.right(line.size() - 2);
    QString     filename = QDir(_rootDir).filePath(relname);
    QModelIndex index    = _model.index(filename);
    int         state    = _model.data(index, Qt::CheckStateRole).toInt();
    return state;
}

int RunScenario::processActions(QTextStream &in)
{
    while(!in.atEnd())
    {
        QString line = in.readLine();
        // Blank lines and comments.
        if((line.size() == 0) || (line[0] == QChar('#')))
            continue;
        else if(line == "actions:")
            continue;
        else if(line == "results:")
            break;
        else if(line[0] == QChar('+'))
            _model.setData(getIndex(line), Qt::Checked, Qt::CheckStateRole);
        else if(line[0] == QChar('-'))
            _model.setData(getIndex(line), Qt::Unchecked, Qt::CheckStateRole);
        else
        {
            QTextStream console(stdout);
            console << "ERROR: Scenario file is broken; parsing died on line:"
                    << endl
                    << line << endl;
            // Don't try to recover.
            QCoreApplication::exit(1);
        }
    }
    return (0);
}

// Returns 0 if success, 1 if a model error, 2 if an emit error.
int RunScenario::processResults(QTextStream &in)
{
    while(!in.atEnd())
    {
        QString line = in.readLine();
        // Blank lines and comments.
        if((line.size() == 0) || (line[0] == QChar('#')))
            continue;
        else if(line == "results:")
            continue;
        else if((line[0] == QChar('0')) || (line[0] == QChar('1'))
                || (line[0] == QChar('2')))
        {
            QString lineRelname  = getRelname(line);
            int     desiredState = getLineState(line);
            int     modelState   = getCheckedStateInt(line);
            if(desiredState != modelState)
            {
                // Test failed!
                return (1);
            }
        }
        else
        {
            QTextStream console(stdout);
            console << "ERROR: Scenario file is broken; parsing died on line:"
                    << endl
                    << line << endl;
            // Don't try to recover.
            QCoreApplication::exit(1);
        }
    }
    return (0);
}

int RunScenario::runScenario(const int num)
{
    int result = -1;
    _model.reset();

    QString scenarioFilename =
        QString("scenario-%1.txt").arg(num, 2, 10, QChar('0'));
    QFile inputFile(scenarioFilename);
    if(inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        processActions(in);
        result = processResults(in);
        if(result != 0)
        {
            // Test failed!
            QTextStream console(stdout);
            console << "--" << endl
                    << "Test failed: " << scenarioFilename << endl;
            if(result == 1)
            {
                console << "Model internal state does not match "
                        << "desired state.  Model data:" << endl;
                printModel();
            }
            console << "--" << endl << endl;
        }
    }
    else
    {
        QTextStream console(stdout);
        console << "ERROR: could not read file: " << scenarioFilename << endl;
        // Don't try to recover.
        QCoreApplication::exit(1);
    }
    inputFile.close();

    return (result);
}

void RunScenario::printDir(const QString &dirname, const int depth)
{
    QTextStream console(stdout);
    QModelIndex index;
    QModelIndex dir;

    dir = _model.index(dirname);
    for(int i = 0; i < _model.rowCount(dir); i++)
    {
        index = _model.index(i, dir.column(), dir);
        console << _model.data(index, Qt::CheckStateRole).toInt() << "\t";
        // Add indents to show the directory structure.
        for(int j = 0; j < depth; j++)
            console << "\t";
        console << _model.fileName(index) << endl;
        // Recursively print the subdirectory.
        if(_model.isDir(index))
        {
            printDir(_model.filePath(index), depth + 1);
        }
    }
}

void RunScenario::printModel()
{
    printDir(_model.rootPath(), 0);
}
