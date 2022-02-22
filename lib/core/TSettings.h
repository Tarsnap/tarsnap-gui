#ifndef TSETTINGS_H
#define TSETTINGS_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QString>
#include <QVariant>
WARNINGS_ENABLE

/* Forward declaration(s). */
class QSettings;

/*!
 * \ingroup misc
 * \brief TSettings is QSettings with a globally-specified filename (unlike
 * <c>QSettings::setPath</c>).
 *
 * Usage:
 *
 *     TSettings::setFilename("my-program.conf");
 *     TSettings settings;
 *     ...
 *     TSettings::destroy();
 *
 * In the middle, you can interact with settings stored in
 * <c>my-program.conf</c>.  The file is stored as <c>QSettings::IniFormat</c>.
 *
 * This object is intended to act like QSettings (plus the ability to specify a
 * filename), but not all functionality is supported.
 *
 * \note
 * Qt's <c>QSettings::setPath()</c> acts in a somewhat surprising manner.
 * Given:
 *
 *     QSettings::setPath(format, scope, path)
 *
 * \note
 * this means
 *     <blockquote>
 *     If we're using \c format and \c scope, then use \c path as the
 *     <b>base directory</b> for the config file.
 *
 *     Qt will still <b>automatically</b> add the organization name as a
 *     subdirectory, name your config file based on your
 *     application name, and add a suffix that Qt thinks represents
 *     the <c>format</c>.
 *     </blockquote>
 * In other words, we can't do:
 *
 *     QSettings::setPath("mydir/my-program.conf");
 *     QSettings settings;
 * and have it use <c>mydir/my-program.conf</c>.
 */
class TSettings
{

public:
    //! Constructor.  If you want to specify a filename, call \ref setFilename
    //! first.
    TSettings();

    //! Specifies the config filename to use.  Only call this once.
    //! \warning Has no effect if it is called after a TSettings
    //! object is instantiated.
    static void setFilename(const QString &filename);

    //! Frees memory associated with TSettings.  This frees global state, so
    //! make sure you do not use TSettings after calling this (i.e. in a
    //! different thread).
    static void destroy();

    //! Returns a pointer to the internal QSettings object.
    QSettings *getQSettings();

    //!@{
    //! @name Functions like QSettings

    //! Returns \c true if there exists a setting called <c>key</c>.
    //! Exactly like <c>QSettings::contains()</c>.
    bool contains(const QString &key);

    //! Returns the value for \c key.
    //! Unlike <c>QSettings::value()</c>, \c defaultValue is not optional.
    QVariant value(const QString &key, const QVariant &defaultValue);

    //! Sets the value of \c key to <c>value</c>, overwriting as necessary.
    //! Exactly like <c>QSettings::setValue()</c>.
    void setValue(const QString &key, const QVariant &value);

    //! Remove the key and any sub-settings.  Exactly like
    //! <c>QSettings::remove()</c>.
    void remove(const QString &key);

    //! Clear all data.  Exactly like <c>QSettings::clear()</c>.
    void clear();

    //! Write any unsaved changes.  Exactly like <c>QSettings::sync()</c>.
    void sync();

    //!@}

private:
    static QSettings *_settings;
    static QString   *_filename_next;
};

#endif
