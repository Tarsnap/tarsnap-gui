#ifndef TESTCFSM_H
#define TESTCFSM_H

#include <QObject>
#include <QString>
#include <QTextStream>

#include "customfilesystemmodel.h"

class TestCFSM : public QObject
{
    Q_OBJECT

public:
    TestCFSM();
    ~TestCFSM();

public slots:
    // Run the specified scenario.  Return 0 on success.
    int runScenario(const int num);

private:
    // The model we are testing.
    CustomFileSystemModel _model;
    // The root directory of the model.
    QString _rootDir;

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
    // Returns 0 if success, 1 if a model error, 2 if an emit error.
    int processResults(QTextStream &in);

    // For debugging and/or notifying of failed tests.
    void printModel();
    void printDir(const QString dirname, const int depth);
};

#endif
