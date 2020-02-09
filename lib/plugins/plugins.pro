TARGET		= tarsnap_plugins
CONFIG		+= plugin
TEMPLATE	= lib
QT		+= widgets uiplugin

HEADERS	=	plugins.h			\
		../core/warnings-disable.h	\
		../widgets/ElidedLabel.h	\
		../widgets/PathLineBrowse.h	\
		../widgets/TWizardPage.h	\
		ElidedLabelPlugin.h		\
		PathLineBrowsePlugin.h		\
		TWizardPagePlugin.h

SOURCES	=	plugins.cpp			\
		../widgets/ElidedLabel.cpp	\
		../widgets/PathLineBrowse.cpp	\
		../widgets/TWizardPage.cpp	\
		ElidedLabelPlugin.cpp		\
		PathLineBrowsePlugin.cpp	\
		TWizardPagePlugin.cpp

FORMS	=					\
		../forms/PathLineBrowse.ui

target.path	= $$[QT_INSTALL_PLUGINS]/designer
INSTALLS	+= target

INCLUDEPATH	+= ../core/ ../widgets/


### Pick up extra flags from the environment
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

### Shared build-dir

UI_DIR      = ../../build/plugins/
MOC_DIR     = ../../build/plugins/
RCC_DIR     = ../../build/plugins/
OBJECTS_DIR = ../../build/plugins/
DESTDIR	    = ../../build/plugins/
