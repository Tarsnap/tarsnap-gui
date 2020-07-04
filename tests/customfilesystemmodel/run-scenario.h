#ifndef RUN_SCENARIO_H
#define RUN_SCENARIO_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QModelIndex>
#include <QObject>
#include <QString>
WARNINGS_ENABLE

#include "customfilesystemmodel.h"

/* Forward declaration(s). */
class QTextStream;

class RunScenario : public QObject
{
    Q_OBJECT

public:
    RunScenario();

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
    bool needToReadSubdirs(const QString &dirname);

    // Helper functions for processing the scenario text files.
    // "relname" is the "relative name", i.e. relative to the _rootDir.
    QString     getRelname(const QString &line);
    QModelIndex getIndex(const QString &line);
    int         getLineState(const QString &line);
    int         getCheckedStateInt(const QString &line);

    // Main scenario-handling functions.
    int processActions(QTextStream &in);
    // Returns 0 if success, 1 if a model error, 2 if an emit error.
    int processResults(QTextStream &in);

    // For debugging and/or notifying of failed tests.
    void printModel();
    void printDir(const QString &dirname, const int depth);
};

#endif
