TARGET = test-setupwizard

FORMS += ../../forms/setupdialog.ui

HEADERS  +=						\
	../../src/utils.h				\
	../../src/widgets/elidedlabel.h			\
	../../src/widgets/setupdialog.h			\
	../../tests/qtest-platform.h

SOURCES += test-setupwizard.cpp				\
	../../src/utils.cpp				\
	../../src/widgets/elidedlabel.cpp		\
	../../src/widgets/setupdialog.cpp

include(../tests-include.pro)
