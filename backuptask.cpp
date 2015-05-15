#include "backuptask.h"

BackupTask::BackupTask():_uuid(QUuid::createUuid()), _optionPreservePaths(true),
    _status(TaskStatus::Initialized)
{

}

BackupTask::~BackupTask()
{

}

