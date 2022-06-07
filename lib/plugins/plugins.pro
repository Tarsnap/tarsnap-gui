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
		../widgets/TTabWidget.h		\
		../widgets/TTextView.h		\
		../widgets/TWizardPage.h	\
		TBusyLabelPlugin.h		\
		TElidedLabelPlugin.h		\
		TOkLabelPlugin.h		\
		TPathComboBrowsePlugin.h	\
		TPathLineBrowsePlugin.h		\
		TPopupPushButtonPlugin.h	\
		TTabWidgetPlugin.h		\
		TTextViewPlugin.h		\
		TWizardPagePlugin.h

SOURCES	=	plugins.cpp			\
		../widgets/TBusyLabel.cpp	\
		../widgets/TElidedLabel.cpp	\
		../widgets/TOkLabel.cpp		\
		../widgets/TPathComboBrowse.cpp	\
		../widgets/TPathLineBrowse.cpp	\
		../widgets/TPopupPushButton.cpp	\
		../widgets/TTabWidget.cpp	\
		../widgets/TTextView.cpp	\
		../widgets/TWizardPage.cpp	\
		TBusyLabelPlugin.cpp		\
		TElidedLabelPlugin.cpp		\
		TOkLabelPlugin.cpp		\
		TPathComboBrowsePlugin.cpp	\
		TPathLineBrowsePlugin.cpp	\
		TPopupPushButtonPlugin.cpp	\
		TTabWidgetPlugin.cpp		\
		TTextViewPlugin.cpp		\
		TWizardPagePlugin.cpp

FORMS	=					\
		../forms/TPathComboBrowse.ui	\
		../forms/TPathLineBrowse.ui

RESOURCES	+=				\
		../resources/lib-resources.qrc

target.path	= $$[QT_INSTALL_PLUGINS]/designer
INSTALLS	+= target

INCLUDEPATH	+= ../core/ ../widgets/


include(../../build-flags.pri)

### Shared build-dir

UI_DIR      = ../../build/plugins/
MOC_DIR     = ../../build/plugins/
RCC_DIR     = ../../build/plugins/
OBJECTS_DIR = ../../build/plugins/
DESTDIR	    = ../../build/plugins/
