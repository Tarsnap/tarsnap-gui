#ifndef NOTIFICATION_INFO_H
#define NOTIFICATION_INFO_H

//! Type of notification message.
enum message_type
{
    NOTIFICATION_UNSPECIFIED,
    NOTIFICATION_NOT_ONLINE,
    NOTIFICATION_ARCHIVE_CREATING,
    NOTIFICATION_ARCHIVE_CREATED,
    NOTIFICATION_ARCHIVE_FAILED
};

//! Info about what a message is about.
struct message_info
{
    //! What type of message?
    message_type type;
    //! Additional data (e.g., archive name), if applicable.
    QString data;
};

#endif /* !NOTIFICATION_INFO_H */
