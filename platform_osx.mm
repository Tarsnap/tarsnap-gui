#include <QUrl>

#include <Foundation/NSAutoreleasePool.h>
#include <Foundation/NSURL.h>
#include <Foundation/NSString.h>

namespace Utils {
namespace Platform {

/* Workaround file drops from Finder, QTBUG-40449 */
QUrl osxRefToUrl(const QUrl &url)
{
    QUrl ret = url;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSURL *nsurl = url.toNSURL();
    if ([nsurl isFileReferenceURL])
        ret = QUrl::fromNSURL([nsurl filePathURL]);
    [pool release];
    return ret;
}

} // Internal
} // Utils
