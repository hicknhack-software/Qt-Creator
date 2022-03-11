add_qtc_plugin(GitVcsChanges
     DEPENDS LibGit2Cpp
     PLUGIN_DEPENDS Core TextEditor ProjectExplorer
     SOURCES
     gitvcschangesplugin.cpp gitvcschangesplugin.h
     gitscrollbarhighlighter.cpp gitscrollbarhighlighter.h)
