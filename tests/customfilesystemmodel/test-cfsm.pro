TARGET = test-cfsm
QT = core widgets

HEADERS	+=						\
	../../src/customfilesystemmodel.h		\
	scenario-num.h					\
	testCFSM.h

SOURCES	+= test-cfsm.cpp				\
	../../src/customfilesystemmodel.cpp		\
	testCFSM.cpp

include(../tests-include.pro)
