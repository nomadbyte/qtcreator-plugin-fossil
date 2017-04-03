qtcreator-plugin-fossil ChangeLog  {#qtc-fossil-changelog}
=================================

## 3.5.1_3 - 2017-04-03

- client min. version `fossil 1.33`
- `Annotate Current File`: jump to current source line.
- `Annotate Current File`: add a toggle to show a list of versions.


## 3.5.1_2 - 2016-11-09

- client min. version `fossil 1.33`
- __FIXED__:`Timeline Current File` ignores width `-W` option.
- __FIXED__:`Options` incorrect tab-order.


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
