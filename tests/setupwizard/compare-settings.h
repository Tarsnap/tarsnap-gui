#ifndef COMPARE_SETTINGS_H
#define COMPARE_SETTINGS_H

/* Forward declaration(s). */
class QSettings;

//! Compares all the keys in two QSettings files.
bool compareSettings(QSettings *settings, QSettings *target);

#endif /* !COMPARE_SETTINGS_H */
