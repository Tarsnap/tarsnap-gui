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

/*!
 * \ingroup misc
 * \brief The Translator translates strings in the app.
 */
class Translator
{
public:
    //! The singleton instance of this class
    static Translator &instance();
    //! Translates the app.
    void translateApp(QCoreApplication *app, QString lang);
    //! The available languages.
    QStringList languageList();

private:
    Translator();
    Translator(Translator const &);
    Translator &operator=(Translator const &);
    ~Translator();

    QMap<QString, QString> _languageMap;
    QTranslator *_qtTranslator;
    QTranslator *_appTranslator;

    void remove_translators();
};

#endif // TRANSLATOR_H
