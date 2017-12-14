QT += core widgets

INCLUDEPATH += ../../src

HEADERS = testCFSM.h \
	scenario-num.h \
	../../src/customfilesystemmodel.h \

SOURCES = testCFSM.cpp \
	../../src/customfilesystemmodel.cpp \
	main.cpp

osx {
    LIBS += -framework Foundation
}

test.commands = ./${TARGET} -platform offscreen
test.depends = ${TARGET}
QMAKE_EXTRA_TARGETS += test
