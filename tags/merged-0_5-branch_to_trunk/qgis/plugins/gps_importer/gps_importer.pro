######################################################################
# Automatically generated by qmake (1.06c) Fri Jul 16 17:05:20 2004
######################################################################

TEMPLATE = lib
INCLUDEPATH += . ..\..\src
LIBS += ..\..\src\libqgis.lib

DLLDESTDIR= ..\..\win_build\lib\qgis
CONFIG += qt dll thread rtti

# Input

HEADERS += plugin.h \
           plugingui.h \
           pluginguibase.ui.h \
           shapefile.h \
           utils.h \
           waypointtoshape.h 
INTERFACES += pluginguibase.ui
SOURCES += dbfopen.c \
           plugin.cpp \
           plugingui.cpp \
           shpopen.c \
           utils.c \
           waypointtoshape.cpp 

           
