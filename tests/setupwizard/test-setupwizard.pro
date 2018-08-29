TARGET = test-setupwizard

FORMS += ../../forms/setupdialog.ui

HEADERS  +=						\
	../../tests/qtest-platform.h			\
	../../src/utils.h				\
	../../src/widgets/elidedlabel.h			\
	../../src/widgets/setupdialog.h

SOURCES += test-setupwizard.cpp				\
	../../src/utils.cpp				\
	../../src/widgets/elidedlabel.cpp		\
	../../src/widgets/setupdialog.cpp

include(../tests-include.pro)
