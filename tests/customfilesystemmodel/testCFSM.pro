QT += core widgets

INCLUDEPATH += ../../src

HEADERS = testCFSM.h \
	scenario-num.h \
	../../src/customfilesystemmodel.h \

SOURCES = testCFSM.cpp \
	../../src/customfilesystemmodel.cpp \
	main.cpp

test.commands = ./testCFSM -platform offscreen
test.depends = testCFSM
QMAKE_EXTRA_TARGETS += test
