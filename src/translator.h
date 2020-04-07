#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QMap>
#include <QString>
#include <QTranslator>
WARNINGS_ENABLE

#define LANG_AUTO "Auto"

/* Set up global Translator. */
class Translator;
extern Translator *global_translator;

/*!
 * \ingroup misc
 * \brief The Translator translates strings in the app.
 */
class Translator
{
public:
    Translator();
    ~Translator();

    //! Initialize the global Translator object.
    static void initializeTranslator();

    //! Free the global Translator object.  Can only be called
    //! once per initializeTranslator().
    static void destroy();

    //! Translates the app.
    void translateApp(QCoreApplication *app, QString lang);
    //! The available languages.
    QStringList languageList();

private:
    QMap<QString, QString> _languageMap;
    QTranslator *          _qtTranslator;
    QTranslator *          _appTranslator;

    void remove_translators();
};

#endif // TRANSLATOR_H
