#include "translator.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>
WARNINGS_ENABLE

Translator *global_translator = nullptr;

void Translator::initializeTranslator()
{
    if(global_translator == nullptr)
        global_translator = new Translator();
}

void Translator::destroy()
{
    Q_ASSERT(global_translator != nullptr);

    // Clean up.
    delete global_translator;
    global_translator = nullptr;
}

Translator::Translator() : _qtTranslator(nullptr), _appTranslator(nullptr)
{
    _languageMap["English"]  = "en";
    _languageMap["Romanian"] = "ro";
}

Translator::~Translator()
{
    remove_translators();
}

void Translator::remove_translators()
{
    QCoreApplication *app = QCoreApplication::instance();
    if(app)
    {
        app->removeTranslator(_qtTranslator);
        app->removeTranslator(_appTranslator);
    }
    delete _qtTranslator;
    _qtTranslator = nullptr;
    delete _appTranslator;
    _appTranslator = nullptr;
}

void Translator::translateApp(QCoreApplication *app, const QString &language)
{
    QString lang(language);

    if(!app)
        return;

    if(lang == LANG_AUTO)
        lang = QLocale::languageToString(QLocale().language());

    if(_languageMap.find(lang) == _languageMap.end())
        return;

    if(_qtTranslator && _appTranslator)
        remove_translators();

    _qtTranslator = new QTranslator();
    _qtTranslator->load("qt_" + _languageMap[lang],
                        QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app->installTranslator(_qtTranslator);

    _appTranslator = new QTranslator();
    _appTranslator->load("tarsnap-gui_" + _languageMap[lang], ":/translations");
    app->installTranslator(_appTranslator);
}

QStringList Translator::languageList()
{
    return _languageMap.keys();
}
