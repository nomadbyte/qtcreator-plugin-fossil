include(../../qtcreatorplugin.pri)
SOURCES += \
    fossilclient.cpp \
    fossilplugin.cpp \
    optionspage.cpp \
    fossilsettings.cpp \
    commiteditor.cpp \
    fossilcommitwidget.cpp \
    fossileditor.cpp \
    annotationhighlighter.cpp \
    pullorpushdialog.cpp \
    branchinfo.cpp \
    configuredialog.cpp \
    revisioninfo.cpp \
    wizard/fossiljsextension.cpp
HEADERS += \
    fossilclient.h \
    constants.h \
    fossilplugin.h \
    optionspage.h \
    fossilsettings.h \
    commiteditor.h \
    fossilcommitwidget.h \
    fossileditor.h \
    annotationhighlighter.h \
    pullorpushdialog.h \
    branchinfo.h \
    configuredialog.h \
    revisioninfo.h \
    wizard/fossiljsextension.h
FORMS += \
    optionspage.ui \
    revertdialog.ui \
    fossilcommitpanel.ui \
    pullorpushdialog.ui \
    configuredialog.ui
RESOURCES += fossil.qrc
