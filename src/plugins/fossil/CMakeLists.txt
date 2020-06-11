add_qtc_plugin(Fossil
  PLUGIN_DEPENDS Core TextEditor ProjectExplorer VcsBase
  SOURCES
    annotationhighlighter.cpp annotationhighlighter.h
    branchinfo.cpp branchinfo.h
    commiteditor.cpp commiteditor.h
    configuredialog.cpp configuredialog.h configuredialog.ui
    constants.h
    fossil.qrc
    fossilclient.cpp fossilclient.h
    fossilcommitpanel.ui
    fossilcommitwidget.cpp fossilcommitwidget.h
    fossilcontrol.cpp fossilcontrol.h
    fossileditor.cpp fossileditor.h
    fossilplugin.cpp fossilplugin.h
    fossilsettings.cpp fossilsettings.h
    optionspage.cpp optionspage.h optionspage.ui
    pullorpushdialog.cpp pullorpushdialog.h pullorpushdialog.ui
    revertdialog.ui
    revisioninfo.cpp revisioninfo.h
    wizard/fossiljsextension.cpp wizard/fossiljsextension.h
)
