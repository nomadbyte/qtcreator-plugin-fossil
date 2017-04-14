
qtcreator-plugin-fossil: Fossil SCM plugin for Qt Creator {#main}
=========================================================

Overview
--------

`qtcreator-plugin-fossil` is a version control plugin for [Qt Creator][qtcreator]
IDE. It implements support for [fossil SCM][fossil-scm], which is a free and
open-source distributed version control system (DVCS) and as such it adds to the
same toolbox as [git][git-scm], [Bazaar][bzr-scm], [Mercurial][hg-scm] etc.

`fossil` SCM is designed and developed by D. Richard Hipp, the creator of
[SQLite][sqlite]; in fact, the official repository for
[SQLite source code][sqlite-src] is managed with `fossil`, so is its own.
Some highlights of `fossil` SCM: it's _lightweight_ (just a single stand-alone
executable), it presents an _intuitive yet potent_ command-line interface, it
yields a _reasonable performance_ as one may expect from `SQLite` (`fossil` uses
it internally), it includes a _built-in HTTP-server_. On top of this, it also
features a built-in _web-interface_, _issue-tracking_, and _project-wiki_ pages
... it also __fixes bugs in your code on check-in__ (no, it doesn't -- but it
_fossilizes them for posterity_ :)

Most importantly, `fossil` is portable and available on the mainstream platforms.
In a way, `fossil` offers to version control, what `SQLite` does to
database realm, making it easy to introduce version control practices to teams
and projects of any size. In any case, we needed to make our existing `fossil`
repositories work with `Qt Creator` -- so here it is.

`qtcreator-plugin-fossil` is built with `Qt Creator` VCS framework (similar
to official `Bazaar` plugin) and makes use of `fossil` command-line interface.
This directly integrates `fossil` into `Qt Creator` project flow. It supports
base set of version control operations (excluding merge). The guiding intent is
not to provide a full-blown GUI client to `fossil` (it has its web-interface
already), but to marry the IDE convenience with the general version control flow.

`qtcreator-plugin-fossil` is a free software; the use, copy, and distribution
rights are granted under the terms of the
@ref qtc-fossil-license | [MIT License][qtc-fossil-license].

[qtc-fossil]: https://github.com/nomadbyte/qtcreator-plugin-fossil "qtcreator-plugin-fossil project space"
[qtc-fossil-license]: LICENSE-plugin-fossil.md "MIT License"
[qtc-fossil-changelog]: CHANGELOG-plugin-fossil.md "qtcreator-plugin-fossil ChangeLog"
[qtcreator]: https://www.qt.io/ide "Qt Creator IDE"
[fossil-scm]: http://fossil-scm.org  "fossil distributed SCM"
[git-scm]: http://git-scm.com  "git distributed SCM"
[bzr-scm]: http://bazaar.canonical.com "Bazaar version control system"
[hg-scm]: http://www.mercurial-scm.org "Mercurial SCM"
[sqlite]: https://sqlite.org "SQLite database engine"
[sqlite-src]: http://www.sqlite.org/src "SQLite source code in Fossil"
[qtcreator-build]: http://wiki.qt.io/Building-Qt-Creator-from-Git "Building Qt Creator from Git"


Features
--------

`qtcreator-plugin-fossil` directly integrates `fossil` into `Qt Creator` project
flow similarly to `git` and `Bazaar`. It supports base set of version control
operations:

- __create__ local `fossil` repository
- __clone__ a remote `fossil` repository
- __add/delete/rename__ tracked files
- show current __status__ of the repository and current files
- __commit__ changes made (__branch__ and __tag__ on commit)
- show __timeline__ of the commits
- __annotate__ file source lines with respective check-ins
- __diff__ changes
- __check-out/revert__ files in given revision
- __push/pull/update__ changes to/from remote repository

This covers routine operations in code development context (stashing would
be nice too, well, maybe in the future). The rest (repository management, merge,
etc.) could be achieved via `fossil` command-line or web-interface.

Refer to _"Using Version Control Systems"_ `Qt Creator` help page for more
details.

Updates and details about the current version listed in
@ref qtc-fossil-changelog | [qtcreator-plugin-fossil ChangeLog][qtc-fossil-changelog].


Quick Start
-----------

In short, it's a `Qt Creator` plugin, so those familiar with plugin building
may just skip through to fetch the sources. Otherwise, read on.

As with any `Qt Creator` plugins, `qtcreator-plugin-fossil` sources need to be
integrated into `Qt Creator` source tree, then built as part of `Qt Creator`
build process. Thus the __plugin sources must target a specific `Qt Creator`
version__. In the course of the `qtcreator-plugin-fossil` project we naturally
adapted it for selected `Qt Creator` updates, mostly targeting the long-term
support versions. It may be possible to try building it with interim releases,
though some tweaking may be needed to account for the underlying VCS framework
changes.

So the Quick start is not that quick as it requires some preparations and, yes,
the full `Qt Creator` build, before the `qtcreator-plugin-fossil` library could
be harvested and installed for use with the existing `Qt Creator` instance of
__matching__ or compatible version.

For reference: official instructions on how to
[build Qt Creator from sources][qtcreator-build], also see `Qt Creator` own
README.

In our experience, we normally would already have an active `Qt Creator` instance
(from `Qt SDK` installation), and would fetch the sources for that version, then
use that instance to build another one from sources with the
`qtcreator-plugin-fossil` integrated. This leverages the IDE to help track
possible wrinkles (mostly missing dependencies) or source incompatibilities.

Cloning `Qt Creator` repository is ok too, but be ready to accept over 250MB of
its size. Alternatively, this repository contains forks for selected
`Qt Creator` releases with `qtcreator-plugin-fossil` sources integrated
(see `qtcreator-fossil-*` branches, subject to `Qt Creator` license). We used these
directly to build and test `qtcreator-plugin-fossil`.

- Fetch the `Qt Creator` sources for the needed `${version}` either from official `git`
  repository; optionally fetch the sources for the `qbs` module `${qbs_version}`

        wget --output-document qt-creator-${version}.tar.gz https://github.com/qtproject/qt-creator/archive/v${version}.tar.gz
        wget --output-document qt-creator-${version}-qbs.tar.gz https://github.com/qt-labs/qbs/archive/v${qbs_version}.tar.gz

- Fetch the `qtcreator-plugin-fossil` sources for the matching version `${plugin_fossil_version}`

        wget --output-document qt-creator-${version}-fossil.tar.gz https://github.com/nomadbyte/qtcreator-plugin-fossil/archive/v${plugin_fossil_version}.tar.gz

- Merge source trees

        mkdir qtcreator-fossil-${version}
        cd qtcreator-fossil-${version}

        tar xvf ../qt-creator-${version}.tar.gz  --strip-components=1
        tar xvf ../qt-creator-${version}-qbs.tar.gz  --strip-components=1 -C src/shared/qbs/
        tar xvf ../qt-creator-${version}-fossil.tar.gz  --strip-components=1

- __OR__
  Fetch the `Qt Creator` sources with integrated `qtcreator-plugin-fossil`

        wget --output-document qtcreator-fossil-${version}.tar.gz https://github.com/nomadbyte/qtcreator-plugin-fossil/archive/qtcreator-fossil-${version}.tar.gz

        mkdir qtcreator-fossil-${version}
        cd qtcreator-fossil-${version}

        tar xvf ../qtcreator-fossil-${version}.tar.gz  --strip-components=1

- Launch the installed `Qt Creator` instance and open the `qtcreator.pro` project

        qtcreator-fossil-${version}/qtcreator.pro

- The integrated `qtcreator-plugin-fossil` project

        qtcreator-fossil-${version}/src/plugins/fossil/fossil.pro

- Build the `Qt Creator` in _Release_ configuration with your selected Kit

__This assumes all needed dependencies are installed__

It will take ~some~ time... There will likely be a few warnings in Issues pane.
Once have got __successful green build__ -- it should be ready to launch.

- In `mode:Projects>Run` configuration add run parameters to avoid messing up
  settings of your active `Qt Creator` instance. Create these directories

        Working dir: <build-dir>/work
        Parameters: -settingspath ../settings

- Run the built `Qt Creator` instance and confirm the presence of the `Fossil`
  plugin

        menu:Help>About Plugins..>Version Control::Fossil
        menu:Tools>Options>Version Control::Fossil

- Download [fossil SCM client][fossil-scm] and install `fossil` executable file
  in your `PATH`

- Create a directory for local `fossil` repositories and remote clones

        mkdir ~/fossils/qt

- Configure the `qtcreator-plugin-fossil` to use that directory

        menu:Tools>Options>Version Control::Fossil:Local Repositories Default path

Now you should be able to use `fossil` repositories with `Qt Creator`

- Create a new project and from wizard select `Fossil` as Version Control

Once the project opens, you should be able to use `fossil` from the main menu

      menu:Tools>Fossil::Status
      menu:Tools>Fossil::Commit
      menu:Tools>Fossil::Timeline

The newly created fossil repository should also be found in the configured
location. As expected, the newly created project may also be managed via `fossil`
SCM command-line:

      ls ~/fossils/qt/<new-project>.fossil

      cd <new-project-dir>
      fossil status

As a side-note, `fossil` already offers ways of sharing the repositories with
other users (see `fossil server` command), however as an alternative way to host
and serve your `fossil` repositories, check out
[Chisel Fossil SCM Hosting](http://chiselapp.com).

To clone a remote project repository, say, hosted at http://chiselapp.com

      menu:File>New File or Project...>Import Project::Fossil Clone
      wizard:Fossil Clone>Remote Repository Clone
      Remote Repository: https://<repouser>:<passwd>@chiselapp.com/user/<username>/repository/<reponame>

> __NOTE__: To use an external `fossil` repository, it needs to contain a valid
> `Qt Creator` project (`.pro`) file, which `Qt Creator` then loads in at clone
> completion.

Refer to _"Using Version Control Systems"_ `Qt Creator` help page for more
details.


Installation
------------

Once the `qtcreator-plugin-fossil` has been successfully built, it may be
harvested from the build directory to install it for use with your active
`Qt Creator` instance. Alternatively, install the newly built `Qt Creator` (refer
to `Qt Creator` own README)

> __NOTE__: Exit the installed `Qt Creator` instance, if currently running, prior
> to installing the built `qtcreator-plugin-fossil`. Otherwise `Qt Creator` may
> hang on exiting.

- Harvest the built `qtcreator-plugin-fossil` plugin files from `Qt Creator`
  build directory.
- Copy these files to location of the installed `Qt Creator` instance
- On _Linux_:

        sudo cp $QTC_BUILD_DIR/lib/qtcreator/plugins/*Fossil*  $QTC_INSTALL_DIR/lib/qtcreator/plugins/
        ## for versions prior to v4.3.0_1
        sudo cp -R $QTC_BUILD_DIR/share/qtcreator/templates/wizards/projects/vcs/fossil  $QTC_INSTALL_DIR/share/qtcreator/templates/wizards/projects/vcs/

- Optionally install the updated documentation

        cd $QTC_BUILD_DIR
        make docs

        sudo mv $QTC_INSTALLL_DIR/share/doc/qtcreator/qtcreator.qch $QTC_INSTALLL_DIR/share/doc/qtcreator/qtcreator.qch-fossil-uninstall
        sudo cp $QTC_BUILD_DIR/share/doc/qtcreator/qtcreator.qch $QTC_INSTALLL_DIR/share/doc/qtcreator/

Starting up `Qt Creator` should load the installed `qtcreator-plugin-fossil`.

> __NOTE__: In case the `qtcreator-plugin-fossil` fails to load into the
> `Qt Creator`, make sure that it was built with that `Qt Creator` source version
> AND compiler version (see `menu:Help>About Qt Creator...`).


Usage
-----

Refer to _"Using Version Control Systems"_ `Qt Creator` help page for more
details.


Support
-------

`qtcreator-plugin-fossil` has already gone through a number of iterations to keep
up with `Qt Creator` updates. `Qt Creator` VCS framework too got its share of
updates, though for the most part it remains stable. Other VCS plugins, already
included in official `Qt Creator` releases, are based on that framework
(`Bazaar plugin`, `Mercurial plugin` etc.) We intend to continue our practice of
maintaining `qtcreator-plugin-fossil` so that it works with upcoming long-term
support `Qt Creator` releases. There is obviously a great benefit of having it
also included in the official `Qt Creator` code base, if that would be deemed
possible.

Let us know of your experience, challenges or problems (bugs?) with it -- quite
possible we have dealt with these too and may have work-arounds or may suggest
some alternatives.

Please direct your feedback to
[qtcreator-plugin-fossil GitHub project page][qtc-fossil].

Your contribution is welcomed!

