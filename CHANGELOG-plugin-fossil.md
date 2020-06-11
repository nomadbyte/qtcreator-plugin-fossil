qtcreator-plugin-fossil ChangeLog  {#qtc-fossil-changelog}
=================================

## 4.11.2_1


## 4.7.0_1

- Adapted to `Qt Creator 4.7.0-rc1` `VcsBase` framework
- Change the default (300) and max number (1000) of history log entries shown
- Change the command execution's maximum timeout value (360s)


## 4.6.2_1 - 2018-07-08

- Adapted to `Qt Creator 4.6.2` `VcsBase` framework
- client min. version `fossil 1.33`
- `Annotate Current File`: support annotating a specific revision which can be
  selected from the context menu (thanks to `fossil annotate -r` option added
  in v2.4)
- Updated clone wizard icons to grayscale matching the current Qt Creator
  styling.


## 4.5.0_1 - 2018-02-11

- Adapted to `Qt Creator 4.5.0` `VcsBase` framework
- client min. version `fossil 1.33`


## 4.4.1_1 - 2017-11-01

- Adapted to `Qt Creator 4.4.1` `VcsBase` framework


## 4.3.1_2 - 2017-10-25

- __FIXED__:`isVcsFileOrDirectory()` is incorrect


## 4.3.1_1 - 2017-07-06

- Adapted to updated `Qt Creator 4.3.1` `VcsBase` framework
- Move clone wizard files into fossil qrc resource file
- client min. version `fossil 1.33`

- __NOTE__: Installation instructions changed -- no longer need to copy
  the clone wizard files, as they are now linked into the plugin lib


## 4.2.1_1 - 2017-04-10

- Adapted to updated `Qt Creator 4.2.1` `VcsBase` framework
- client min. version `fossil 1.33`


## 4.1.0_2 - 2017-04-03

- client min. version `fossil 1.33`
- `Annotate Current File`: jump to current source line.
- `Annotate Current File`: add a toggle to show a list of versions.


## 4.1.0_1 - 2016-11-11

- Adapted to updated `Qt Creator 4.1.0` `VcsBase` framework
- client min. version `fossil 1.33`
- __FIXED__:`Timeline Current File` ignores width `-W` option.
- __FIXED__:`Options` incorrect tab-order.


## 4.0.1_1 - 2016-08-21

- Adapted to updated `Qt Creator 4.0.1` `VcsBase` framework
- client min. version `fossil 1.33`


## 3.6.1_1 - 2016-04-22

- Adapted to updated `Qt Creator 3.6.1` `VcsBase` framework
- client min. version `fossil 1.33`


## 3.5.1_1 - 2015-12-16

- Adapted to updated `Qt Creator 3.5.1` `VcsBase` framework
- client min. version `fossil 1.33`
- Additional `fossil status` cases (`Set Exec`, `Set Symlink`)


## 3.0.1_3 - 2015-03-06

- client min. version `fossil 1.30`
- Consistent timeline view both for repository and for current file (thanks to
  `fossil timeline --path` option added in v1.30); falling back to `finfo`
  with legacy client versions.
- Apply custom syntax highlighter to timeline view

- __FIXED__:`Timeline Current File` shows complete timeline with file's changes
  from all branches, not only the current branch. `fossil` client currently has
  no facility to subset a file's timeline by branch and starting time
  (see `fossil help finfo`).


## 3.0.1_2 - 2014-07-17

- client min. version `fossil 1.29`
- Show current branch name indicator appended to the opened project name

- __FIXED__:`Diff` has no ignore white-space option. `fossil` client currently
  does not support this (see `fossil help diff`).


## 3.0.1_1 - 2014-03-25

- Adapted to updated `Qt Creator 3.0.1` VcsBase framework
- client min. version `fossil 1.28`


## 2.4.1_3 - 2014-02-19

- client min. version `fossil 1.28`
- Add support of legacy client versions.
- `Timeline`: allow control of the timeline entries width (see `fossil timeline -W`)
- `Annotate Current File`: add annotate/blame toggle (Show Committers)
  (see `fossil blame`)


## 2.4.1_2 - 2013-07-21

- client min. version `fossil 1.26`

- __FIXED__:`Diff` does not show contents of added/deleted files, only lists
  file status ADDED/DELETED. This is a limitation of the current `fossil` client
  version.


## 2.4.1_1 - 2013-02-16

- Based on `Qt Creator 2.4.1` VCSBase framework
- client min. version `fossil 1.25`
- __create__ local `fossil` repository
- __clone__ a remote `fossil` repository (via `New File or Project...`
  VCS clone wizard)
- __add/delete/rename__ tracked files
- show current __status__ of the repository and current files
- __commit__ changes made (__branch__ and __tag__ on commit)
- show __timeline__ of the commits
- __annotate__ file source lines with respective check-ins
- __diff__ changes
- __check-out/revert__ files in given revision
- __push/pull/update__ changes to/from remote repository

- __ISSUE__:`Diff` does not show contents of added/deleted files, only lists
  file status ADDED/DELETED. This is a limitation of the current `fossil` client
  version.
- __ISSUE__:`Diff` has no ignore white-space option. `fossil` client currently
  does not support this (see `fossil help diff`).
- __ISSUE__:`Timeline Current File` shows complete timeline with file's changes
  from all branches, not only the current branch. `fossil` client currently has
  no facility to subset a file's timeline by branch and starting time
  (see `fossil help finfo`).
- __ISSUE__:`Annotate Current File` does not allow re-annotation of revisions
  other than the currently checked-out. This is a limitation of
  `fossil annotate` client command. However prior revisions' changes are
  properly described/diff'ed.
- __ISSUE__:`Fossil Commit/Update` fail to auto-sync due to missing password.
  This is because `Fossil Clone` fails to save the specified password, since
  `fossil clone` client command can only show the save-password prompt
  interactively, which is not available in plugin context.
  - __WORKAROUND__:`Autosync` needs both the remote address and the login details.
    `Fossil Clone` saves remote address and username when given, yet fails to save
    the password. To save password manually after the cloning -- open the terminal
    and from the repository working directory execute `fossil update`, it should
    prompt for password and then ask again to save it; accept it and the `Autosync`
    should then work properly in plugin context.
- __ISSUE__:`Fossil Clone` does not bring up `SSH_ASKPASS` prompt. `fossil`
  client appears to not have support for this. So the password has to be
  specified explicitly `username:passwd@host`.
- __ISSUE__::`[Q_OS_WIN32]`:`Fossil Diff` incorrectly lists the contents of diff
  chunks, it inserts extra empty lines for each valid line of code. This issue is
  internal to `fossil diff` output, when dealing with Windows CR-LF endings. The
  resulting output contains mixed line-endings, which then are not getting
  properly stripped.
- __ISSUE__::`[Q_OS_WIN32]`:`Fossil Commit` shows error after adding newly created
  files: `"Abandoning commit due to CR/NL line-endings in .\filename.ext"`. This is
  specific to Windows where newline is CR-LF, `fossil` warns of potential
  incompatibilty with other platforms.
  - __WORKAROUND__:`fossil` has `crnl-glob` setting to allow CR-LF, CR, LF, or
    mixed line-endings in specified file names (see `fossil help settings`).
    To allow all files: `fossil set crnl-glob *,`. NOTE comma in `*,` -- this way
    it does not get expanded by cmd-shell.
