set(IDE_VERSION "4.84.0")                             # The IDE version.
set(IDE_VERSION_COMPAT "4.84.0")                      # The IDE Compatibility version.
string(TIMESTAMP SNAPSHOT_TODAY "%Y-%m-%d")
set(IDE_VERSION_DISPLAY "5.0.0-rc1 snapshot-${SNAPSHOT_TODAY}")                     # The IDE display version.
set(IDE_COPYRIGHT_YEAR "2021")                        # The IDE current copyright year.

set(IDE_SETTINGSVARIANT "QtProject")                  # The IDE settings variation.
set(IDE_DISPLAY_NAME "Qt Creator")                    # The IDE display name.
set(IDE_ID "qtcreator")                               # The IDE id (no spaces, lowercase!)
set(IDE_CASED_ID "QtCreator")                         # The cased IDE id (no spaces!)
set(IDE_BUNDLE_IDENTIFIER "org.qt-project.${IDE_ID}") # The macOS application bundle identifier.

set(PROJECT_USER_FILE_EXTENSION .user)
set(IDE_DOC_FILE "qtcreator/qtcreator.qdocconf")
set(IDE_DOC_FILE_ONLINE "qtcreator/qtcreator-online.qdocconf")
