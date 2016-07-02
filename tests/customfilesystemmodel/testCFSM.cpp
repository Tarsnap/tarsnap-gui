#include <assert.h>

#include <QApplication>
#include <QDir>
#include <QTextStream>

#include "testCFSM.h"
#include "scenario-num.h"

TestCFSM::TestCFSM()
{
    _rootDir = QDir::currentPath() + QDir::separator() + "dirs";

    // see comment for ::directoryLoaded()
    connect(&_model, &CustomFileSystemModel::directoryLoaded, this,
            &TestCFSM::directoryLoaded);
    connect(&_model, &CustomFileSystemModel::dataChanged, this,
            &TestCFSM::dataChanged);

    // Start reading from disk after setting up connections.
    _model.setReadOnly(true);
    _model.setRootPath(_rootDir);
}

TestCFSM::~TestCFSM()
{
}

// QFileSystemModule reads directories in a separate thread.  As a result,
// if we query it right after creating the module, it might tell us that a
// directory has no children, just because it hasn't started reading it.
// What's worse is that even if QFileSystemModule notifies us that it's
// finished reading "dir/", it has not necessarily read the subdirectories of
// "dir/".
//
// Therefore, we need to wait for a "directoryLoaded" signal, then check
// subdirectories of that path and tell the model to load those subdirs if
// they're not already read.
void TestCFSM::directoryLoaded(const QString &path)
{
    bool loadingMore = false;
    QModelIndex dir = _model.index(path);
    for (int i=0; i < _model.rowCount(dir); i++) {
        QModelIndex child = dir.child(i, dir.column());
        if (_model.isDir(child) && _model.canFetchMore(child)) {
            _model.fetchMore(child);
            loadingMore = true;
        }
    }
    // If we already know we need to wait longer, don't bother with the
    // next check.
    if (loadingMore)
        return;

    // The above is sufficient in our case,where the directory structure is
    //     dirs/dir-1/dir-2
    // but if there were any sibling directories, it wouldn't catch the case
    // of an unread sibling.  To be safe, we recursively check all
    // subdirectories, starting with the top directory.
    if (!needToReadSubdirs(_rootDir))
        start();
}

bool TestCFSM::needToReadSubdirs(const QString dirname)
{
    bool loadingMore = false;
    QModelIndex dir = _model.index(dirname);
    for (int i=0; i < _model.rowCount(dir); i++) {
        QModelIndex child = dir.child(i, dir.column());
        if (_model.isDir(child)) {
            if (_model.canFetchMore(child)) {
                _model.fetchMore(child);
                loadingMore = true;
            } else {
                if (needToReadSubdirs(_model.filePath(child)))
                    loadingMore = true;
            }
        }
    }
    return loadingMore;
}

void TestCFSM::dataChanged(const QModelIndex &topLeft,
    const QModelIndex &bottomRight, const QVector<int> &roles)
{
    QModelIndex index = topLeft;
    QString filename = _model.filePath(index);
    (void) roles;

    // Our CustomFileSystemModel outputs each file url separately.
    assert(topLeft == bottomRight);

    // We only care about names inside the model's root directory.
    if (filename.size() < _rootDir.size() + 1)
        return;

    // Store "relative name" and state in our hash.
    QString relname  = filename.right(filename.size() - _rootDir.size() - 1);
    int state = _model.data(index, Qt::CheckStateRole).toInt();
    _emittedHash[relname] = state;
}

// The format of these lines in the scenario file is:
// X filename
// where X is a single character, followed by a space.
QString TestCFSM::getRelname(const QString line)
{
    QString relname = line.right(line.size()-2);
    return relname;
}

QModelIndex TestCFSM::getIndex(const QString line)
{
    QString relname = getRelname(line);
    QString filename = QDir(_rootDir).filePath(relname);
    QModelIndex index = _model.index(filename);
    return index;
}

int TestCFSM::getLineState(const QString line)
{
    int state = line[0].digitValue();
    return state;
}

int TestCFSM::getCheckedStateInt(const QString line)
{
    QString relname = line.right(line.size()-2);
    QString filename = QDir(_rootDir).filePath(relname);
    QModelIndex index = _model.index(filename);
    int state = _model.data(index, Qt::CheckStateRole).toInt();
    return state;
}

int TestCFSM::processActions(QTextStream &in)
{
    while (!in.atEnd())
    {
        QString line = in.readLine();
        // Blank lines and comments.
        if ((line.size() == 0) || (line[0] == QChar('#')))
            continue;
        else if (line == "actions:")
            continue;
        else if (line == "results:")
            break;
        else if (line[0] == QChar('+'))
            _model.setData(getIndex(line), Qt::Checked, Qt::CheckStateRole);
        else if (line[0] == QChar('-'))
            _model.setData(getIndex(line), Qt::Unchecked, Qt::CheckStateRole);
        else {
            QTextStream console(stdout);
            console << "ERROR: Scenario file is broken; parsing died on line:"
                    << endl << line << endl;
            // Don't try to recover.
            QApplication::exit(1);
        }
    }
    return (0);
}

// Return 0 if success, 1 if a model error, 2 if an emit error.
int TestCFSM::processResults(QTextStream &in)
{
    while (!in.atEnd())
    {
        QString line = in.readLine();
        // Blank lines and comments.
        if ((line.size() == 0) || (line[0] == QChar('#')))
            continue;
        else if (line == "results:")
            continue;
        else if ((line[0] == QChar('0')) || line[0] == QChar('1') ||
                 (line[0] == QChar('2')))
        {
            QString lineRelname = getRelname(line);
            int desiredState = getLineState(line);
            int modelState = getCheckedStateInt(line);
            if (desiredState != modelState) {
                // Test failed!
                return (1);
            }
            // If we haven't received any info about this relname, assume it
            // was not checked.
            int emittedState = _emittedHash.value(lineRelname, 0);
            if (desiredState != emittedState) {
                // Test failed!
                return (2);
            }

        } else {
            QTextStream console(stdout);
            console << "ERROR: Scenario file is broken; parsing died on line:"
                    << endl << line << endl;
            // Don't try to recover.
            QApplication::exit(1);
        }
    }
    return (0);
}

int TestCFSM::runScenario(const int num)
{
    int result = -1;

    QString scenarioFilename = QString("scenario-%1.txt").arg(num, 2, 10,
                                                              QChar('0'));
    QFile inputFile(scenarioFilename);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        processActions(in);
        result = processResults(in);
        if (result != 0) {
            // Test failed!
            QTextStream console(stdout);
            console << "--" << endl << "Test failed: "
                    << scenarioFilename << endl;
            if (result == 1) {
                console << "Model internal state does not match "
                        << "desired state.  Model data:" << endl;
                printModel();
            } else if (result == 2) {
                console << "Model emitted state does not match "
                        << "desired state.  Emitted data:" << endl;
                QHashIterator<QString, int> iter(_emittedHash);
                while (iter.hasNext()) {
                    iter.next();
                    console << iter.value() << "\t" << iter.key() << endl;
                }
            }
            console << "--" << endl << endl;
        }
    } else {
        QTextStream console(stdout);
        console << "ERROR: could not read file: " << scenarioFilename << endl;
        // Don't try to recover.
        QApplication::exit(1);
    }
    inputFile.close();

    return (result);
}

void TestCFSM::printDir(const QString dirname, const int depth)
{
    QTextStream console(stdout);
    QModelIndex index;
    QModelIndex dir;

    dir = _model.index(dirname);
    for (int i=0; i < _model.rowCount(dir); i++) {
        index = dir.child(i, dir.column());
        console << _model.data(index, Qt::CheckStateRole).toInt() << "\t";
        // Add indents to show the directory structure.
        for (int j=0; j < depth; j++)
            console << "\t";
        console << _model.fileName(index) << endl;
        // Recursively print the subdirectory.
        if (_model.isDir(index)) {
            printDir(_model.filePath(index), depth + 1);
        }
    }
}

void TestCFSM::printModel()
{
    printDir(_model.rootPath(), 0);
}

void TestCFSM::start()
{
    // Run tests
    int num_errors = 0;
    for (int i = 0; i < NUM_SCENARIOS; i++) {
        _model.reset();
        _emittedHash.clear();
        if (runScenario(i) != 0)
            num_errors++;
    }

    // Notify user on stdout
    QTextStream console(stdout);
    if (num_errors == 0)
        console << "All " << NUM_SCENARIOS << " tests successful." << endl;
    else
        console << num_errors << " error(s) detected!" << endl;

    // Output exit code
    QApplication::exit(num_errors);
}
