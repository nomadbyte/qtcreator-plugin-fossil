import qbs 1.0

QtcPlugin {
    name: "Fossil"

    Depends { name: "Qt.widgets" }
    Depends { name: "Utils" }

    Depends { name: "Core" }
    Depends { name: "TextEditor" }
    Depends { name: "ProjectExplorer" }
    Depends { name: "VcsBase" }

    files: [
        "annotationhighlighter.cpp",
        "annotationhighlighter.h",
        "branchinfo.cpp",
        "branchinfo.h",
        "commiteditor.cpp",
        "commiteditor.h",
        "configuredialog.cpp",
        "configuredialog.h",
        "configuredialog.ui",
        "constants.h",
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
        "wizard/fossiljsextension.cpp",
        "wizard/fossiljsextension.h",
    ]
}

