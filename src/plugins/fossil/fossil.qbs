import qbs.base 1.0

import QtcPlugin

QtcPlugin {
    name: "Fossil"
    provider: "Nomadbyte"

    Depends { name: "Qt.widgets" }
    Depends { name: "Core" }
    Depends { name: "TextEditor" }
    Depends { name: "ProjectExplorer" }
    Depends { name: "Find" }
    Depends { name: "VcsBase" }
    Depends { name: "Locator" }


    files: [
        "annotationhighlighter.cpp",
        "annotationhighlighter.h",
        "branchinfo.cpp",
        "branchinfo.h",
        "cloneoptionspanel.cpp",
        "cloneoptionspanel.h",
        "cloneoptionspanel.ui",
        "clonerepositorypanel.cpp",
        "clonerepositorypanel.h",
        "clonerepositorypanel.ui",
        "clonewizard.cpp",
        "clonewizard.h",
        "clonewizardpage.cpp",
        "clonewizardpage.h",
        "commiteditor.cpp",
        "commiteditor.h",
        "configuredialog.cpp",
        "configuredialog.h",
        "configuredialog.ui",
        "constants.h",
        "fossil.qrc",
        "fossilclient.cpp",
        "fossilclient.h",
        "fossilcommitpanel.ui",
        "fossilcommitwidget.cpp",
        "fossilcommitwidget.h",
        "fossilcontrol.cpp",
        "fossilcontrol.h",
        "fossileditor.cpp",
        "fossileditor.h",
        "fossilplugin.cpp",
        "fossilplugin.h",
        "fossilsettings.cpp",
        "fossilsettings.h",
        "optionspage.cpp",
        "optionspage.h",
        "optionspage.ui",
        "pullorpushdialog.cpp",
        "pullorpushdialog.h",
        "pullorpushdialog.ui",
        "revertdialog.ui",
        "revisioninfo.cpp",
        "revisioninfo.h",
        "images/fossil.png",
    ]
}

