
qtcreator-plugin-fossil: Fossil SCM plugin for Qt Creator {#main}
=========================================================

Fossil
------

Update your [fossil SCM client][fossil-scm].

Fossil client minimum version is listed in
@ref qtc-fossil-changelog | [qtcreator-plugin-fossil ChangeLog][qtc-fossil-changelog].


Build
-----

> __NOTE__: `qtcreator-plugin-fossil` version has to match to the target
> `Qt Creator` version. Both should also be built with the same compiler version.

- Get `Qt Creator` sources for the target version with integrated
  `qtcreator-plugin-fossil` (branch `qtcreator-fossil-${version}`)
- Build `Qt Creator` (`qtcreator.pro`) -- may need to get dependencies
  (see [Qt Creator's own README][qtcreator-readme])

        ## Build
        mkdir build-qtcreator-fossil-${version}
        cd build-qtcreator-fossil-${version}
        qmake -r ../qtcreator-fossil-${version}/qtcreator.pro | tee qmake.log
        make | tee make.log

        ## Test-run
        mkdir settings
        mkdir fossils
        mkdir work
        cd work
        ../bin/qtcreator -settingspath ../settings

- Build docs (`make docs`)


[fossil-scm]: http://fossil-scm.org  "fossil distributed SCM"
[qtcreator-readme]: README "QtCreator README"
[qtc-fossil-license]: LICENSE-plugin-fossil.md "MIT License"
[qtc-fossil-changelog]: CHANGELOG-plugin-fossil.md "qtcreator-plugin-fossil ChangeLog"


Installation
------------

- Harvest the built `qtcreator-plugin-fossil` plugin files from `Qt Creator`
  build directory.
- Copy these files to location of the installed `Qt Creator` instance
- On _Linux_:

        sudo cp $QTC_BUILD_DIR/lib/qtcreator/plugins/Nokia/*Fossil*  $QTC_INSTALL_DIR/lib/qtcreator/plugins/Nokia/

- Optionally install the updated documentation

        sudo mv $QTC_INSTALLL_DIR/share/doc/qtcreator/qtcreator.qch $QTC_INSTALLL_DIR/share/doc/qtcreator/qtcreator.qch-fossil-uninstall
        sudo cp $QTC_BUILD_DIR/share/doc/qtcreator/qtcreator.qch $QTC_INSTALLL_DIR/share/doc/qtcreator/


Configuration
-------------

- Download [fossil SCM client][fossil-scm] and install `fossil` executable file
  in your `PATH`

- Create a directory for local `fossil` repositories and remote clones

        mkdir ~/fossils/qt

- Configure the `qtcreator-plugin-fossil` to use that directory

        menu:Options>Version Control::Fossil:Local Repositories Default path


Usage
-----

Refer to _"Using Version Control Systems"_ `Qt Creator` help page for more
details.


Support
-------

Updates and details about the current version listed in
@ref qtc-fossil-changelog | [qtcreator-plugin-fossil ChangeLog][qtc-fossil-changelog].

`qtcreator-plugin-fossil` is a free software; the use, copy, and distribution
rights are granted under the terms of the
@ref qtc-fossil-license | [MIT License][qtc-fossil-license].
