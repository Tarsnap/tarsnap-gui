#ifndef COMPAT_H
#define COMPAT_H

#if(QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#define KEEP_EMPTY_PARTS Qt::KeepEmptyParts
#define SKIP_EMPTY_PARTS Qt::SkipEmptyParts
#else
#define KEEP_EMPTY_PARTS QString::KeepEmptyParts
#define SKIP_EMPTY_PARTS QString::SkipEmptyParts
#endif

#if(QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#define QT_ENDL Qt::endl
#else
#define QT_ENDL endl
#endif

#endif /* !COMPAT_H */
