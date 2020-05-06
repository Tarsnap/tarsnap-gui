TARGET		= tarsnap_plugins
CONFIG		+= plugin
TEMPLATE	= lib
QT		+= widgets uiplugin

HEADERS	=	plugins.h			\
		../core/warnings-disable.h	\
		../widgets/TBusyLabel.h		\
		../widgets/TElidedLabel.h	\
		../widgets/TOkLabel.h		\
		../widgets/TPathComboBrowse.h	\
		../widgets/TPathLineBrowse.h	\
		../widgets/TPopupPushButton.h	\
		../widgets/TTextView.h		\
		../widgets/TWizardPage.h	\
		TBusyLabelPlugin.h		\
		TElidedLabelPlugin.h		\
		TOkLabelPlugin.h		\
		TPathComboBrowsePlugin.h	\
		TPathLineBrowsePlugin.h		\
		TPopupPushButtonPlugin.h	\
		TTextViewPlugin.h		\
		TWizardPagePlugin.h

SOURCES	=	plugins.cpp			\
		../widgets/TBusyLabel.cpp	\
		../widgets/TElidedLabel.cpp	\
		../widgets/TOkLabel.cpp		\
		../widgets/TPathComboBrowse.cpp	\
		../widgets/TPathLineBrowse.cpp	\
		../widgets/TPopupPushButton.cpp	\
		../widgets/TTextView.cpp	\
		../widgets/TWizardPage.cpp	\
		TBusyLabelPlugin.cpp		\
		TElidedLabelPlugin.cpp		\
		TOkLabelPlugin.cpp		\
		TPathComboBrowsePlugin.cpp	\
		TPathLineBrowsePlugin.cpp	\
		TPopupPushButtonPlugin.cpp	\
		TTextViewPlugin.cpp		\
		TWizardPagePlugin.cpp

FORMS	=					\
		../forms/TPathComboBrowse.ui	\
		../forms/TPathLineBrowse.ui

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
