TARGET = Fossil
TEMPLATE = lib
include(../../qtcreatorplugin.pri)
include(fossil_dependencies.pri)
SOURCES += \
    fossilclient.cpp \
    fossilcontrol.cpp \
    fossilplugin.cpp \
    optionspage.cpp \
    fossilsettings.cpp \
    commiteditor.cpp \
    fossilcommitwidget.cpp \
    fossileditor.cpp \
    annotationhighlighter.cpp \
    pullorpushdialog.cpp \
    branchinfo.cpp \
    clonewizardpage.cpp \
    clonewizard.cpp \
    cloneoptionspanel.cpp \
    clonerepositorypanel.cpp \
    configuredialog.cpp \
    revisioninfo.cpp
HEADERS += \
    fossilclient.h \
    constants.h \
    fossilcontrol.h \
    fossilplugin.h \
    optionspage.h \
    fossilsettings.h \
    commiteditor.h \
    fossilcommitwidget.h \
    fossileditor.h \
    annotationhighlighter.h \
    pullorpushdialog.h \
    branchinfo.h \
    clonewizard.h \
    clonewizardpage.h \
    cloneoptionspanel.h \
    clonerepositorypanel.h \
    configuredialog.h \
    revisioninfo.h
FORMS += \
    optionspage.ui \
    revertdialog.ui \
    fossilcommitpanel.ui \
    pullorpushdialog.ui \
    cloneoptionspanel.ui \
    clonerepositorypanel.ui \
    configuredialog.ui
RESOURCES += fossil.qrc
