#ifndef TESTCFSM_H
#define TESTCFSM_H

#include <QHash>
#include <QObject>
#include <QString>
#include <QTextStream>

#include "customfilesystemmodel.h"

class TestCFSM : public QObject
{
    Q_OBJECT

public:
    TestCFSM(int scenario_num);
    ~TestCFSM();

public slots:
    // CustomFileSystemModel changed some data;
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                     const QVector<int> &roles);

    // Start running the test scenarios.
    void start();

private:
    // The model we are testing.
    CustomFileSystemModel _model;
    // The root directory of the model.
    QString _rootDir;
    // A hash table of emitted data from the model.
    QHash<QString, int> _emittedHash;
    // Allows running a single scenario
    int _scenario_num;

    // Recursively checks for any unread subdirectories.
    // ASSUME: every directory must contain something.  See comment in .cpp
    // file.
    bool needToReadSubdirs(const QString dirname);

    // Helper functions for processing the scenario text files.
    // "relname" is the "relative name", i.e. relative to the _rootDir.
    QString getRelname(const QString line);
    QModelIndex getIndex(const QString line);
    int getLineState(const QString line);
    int getCheckedStateInt(const QString line);

    // Main scenario-handling functions.
    int processActions(QTextStream &in);
    // Return 0 if success, 1 if a model error, 2 if an emit error.
    int processResults(QTextStream &in);
    int runScenario(const int num);

    // For debugging and/or notifying of failed tests.
    void printModel();
    void printDir(const QString dirname, const int depth);
};

#endif
