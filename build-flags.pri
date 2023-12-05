include(check-version.pri)

# Treat Qt headers as system headers
# Only available for Qt 5.8+.
if(checkVersionAtLeast(5,8)) {
    QMAKE_CXXFLAGS += -isystem $$shell_quote($$[QT_INSTALL_HEADERS])
    for(module, QT) {
        equals(module, "testlib") {
            QMAKE_CXXFLAGS += 						\
                -isystem $$shell_quote($$[QT_INSTALL_HEADERS]/QtTest)
        } else {
            # Capitalize a first letter, result: -isystem .../include/QtCore
            moduleList = $$split(module, )
            QMAKE_CXXFLAGS += 						\
                -isystem $$shell_quote($$[QT_INSTALL_HEADERS]/Qt$$upper(\
                         $$take_first(moduleList))$$join(moduleList, ))
        }
    }
    unset(moduleList)
}

# Pick up extra flags from the environment
QMAKE_CXXFLAGS += $$(CXXFLAGS)
QMAKE_CFLAGS += $$(CFLAGS)
QMAKE_LFLAGS += $$(LDFLAGS)
env_CC = $$(QMAKE_CC)
!isEmpty(env_CC) {
	QMAKE_CC = $$(QMAKE_CC)
}
env_CXX = $$(QMAKE_CXX)
!isEmpty(env_CXX) {
	QMAKE_CXX = $$(QMAKE_CXX)
}
env_LINK = $$(QMAKE_LINK)
!isEmpty(env_LINK) {
	QMAKE_LINK = $$(QMAKE_LINK)
}
