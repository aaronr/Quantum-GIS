TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt thread rtti debug console

LIBS	+= $(GDAL)\lib\gdal_i.lib $(POSTGRESQL)\src\interfaces\libpq\Release\libpq.lib $(GEOS)\lib\geos.lib

DEFINES	+= QGISDEBUG

INCLUDEPATH	+= . $(GDAL)\include $(POSTGRESQL)\src\interfaces\libpq $(POSTGRESQL)\src\include $(GEOS)\include

HEADERS	+= qgscolortable.h

SOURCES	+= main.cpp \
	qgisapp.cpp \
	qgisiface.cpp \
	qgisinterface.cpp \
	qgsattributeaction.cpp \
	qgsattributeactiondialog.cpp \
	qgsattributedialog.cpp \
	qgsattributetable.cpp \
	qgsattributetabledisplay.cpp \
	qgscontcoldialog.cpp \
	qgscontinuouscolrenderer.cpp \
	qgscoordinatetransform.cpp \
	qgscustomsymbol.cpp \
	qgsdatasource.cpp \
	qgsdbsourceselect.cpp \
	qgsdlgvectorlayerproperties.cpp \
	qgsfeature.cpp \
	qgsfeatureattribute.cpp \
	qgsfield.cpp \
	qgsgraduatedmarenderer.cpp \
	qgsgraduatedsymrenderer.cpp \
	qgsgramadialog.cpp \
	qgsgramaextensionwidget.cpp \
	qgsgrasydialog.cpp \
	qgshelpviewer.cpp \
	qgsidentifyresults.cpp \
	qgslegend.cpp \
	qgslegenditem.cpp \
	qgslinestyledialog.cpp \
	qgslinesymbol.cpp \
	qgsmapcanvas.cpp \
	qgsmaplayer.cpp \
	qgsmaplayerregistry.cpp \
	qgsmapserverexport.cpp \
	qgsmarkerdialog.cpp \
	qgsmarkersymbol.cpp \
	qgsnewconnection.cpp \
	qgsoptions.cpp \
	qgspatterndialog.cpp \
	qgspgquerybuilder.cpp \
	qgspluginitem.cpp \
	qgspluginmanager.cpp \
	qgspluginmetadata.cpp \
	qgspluginregistry.cpp \
	qgspoint.cpp \
	qgspolygonsymbol.cpp \
	qgsproject.cpp \
	qgsprojectproperties.cpp \
	qgsprovidermetadata.cpp \
	qgsproviderregistry.cpp \
	qgsrangerenderitem.cpp \
	qgsrasterlayer.cpp \
	qgsrasterlayerproperties.cpp \
	qgsrect.cpp \
	qgsrenderitem.cpp \
	qgsscalecalculator.cpp \
	qgssimadialog.cpp \
	qgssimarenderer.cpp \
	qgssinglesymrenderer.cpp \
	qgssisydialog.cpp \
	qgssymbol.cpp \
	qgssymbologyutils.cpp \
	qgsvectorlayer.cpp \
	qgssvgcache.cpp \
	splashscreen.cpp \
	qgsacetateobject.cpp \
	qgslabeldialog.cpp \
	qgslabel.cpp \
	qgslabelattributes.cpp \
	qgsacetaterectangle.cpp \
	qgsuvaldialog.cpp \
	qgsludialog.cpp \
	qgsuniquevalrenderer.cpp \
	qgsuvalmadialog.cpp \
	qgsuvalmarenderer.cpp \
	qgsvectorfilewriter.cpp \
	qgsgeomtypedialog.cpp

FORMS	= qgisappbase.ui \
	qgsabout.ui \
	qgsattributetablebase.ui \
	qgsattributeactiondialogbase.ui \
	qgsattributedialogbase.ui \
	qgscontcoldialogbase.ui \
	qgsdbsourceselectbase.ui \
	qgsdlgvectorlayerpropertiesbase.ui \
	qgsgramadialogbase.ui \
	qgsgrasydialogbase.ui \
	qgshelpviewerbase.ui \
	qgsidentifyresultsbase.ui \
	qgslegenditembase.ui \
	qgslinestyledialogbase.ui \
	qgsmapserverexportbase.ui \
	qgsmarkerdialogbase.ui \
	qgsmessageviewer.ui \
	qgsnewconnectionbase.ui \
	qgsoptionsbase.ui \
	qgspatterndialogbase.ui \
	qgspgquerybuilderbase.ui \
	qgspluginmanagerbase.ui \
	qgsprojectpropertiesbase.ui \
	qgsrasterlayerpropertiesbase.ui \
	qgslabeldialogbase.ui \
	qgscustomprojectiondialogbase.ui

######################################################################
# Qmake project file for QGIS src directory
# This file is used by qmake to generate the Makefile for building
# QGIS on Windows
#
# src.pro,v 1.55 2004/10/15 21:47:19 gsherman Exp 
######################################################################
#
# For a WIN32 release build do the following:
# 1. comment out the DEFINES += QGISDEBUG line 
# 2. remove console and debug from the CONFIG += line
# 3. qmake
# 4. nmake clean
# 5. nmake
#
######################################################################

################# 
# GEOS Notes    #
###########################################################################
# Geos support is currenlty provided by a custom compiled library.        #
# The library is compiled with vc++ and statically linked with the        #
# ogr provider. A dll may be supplied at a later date. GEOS source        #
# used in creating the library is available at geos.refractions.net.      #
# To compile the windows version, set the GEOS environment variable       #
# to point to the directory containing the include and lib subdirectories.#
# The headers/lib can be downloaded from http://qgis.org/win32_geos.zip   #
###########################################################################

TARGET = qgis
DEFINES+= QGISDEBUG
DESTDIR = ../win_build
#CONFIG += qt thread rtti console
RC_FILE = qgis_win32.rc
# Input
HEADERS += qgis.h \
            qgisapp.h \
            qgisappbase.ui.h \
            qgisiface.h \
            qgisinterface.h \
            qgsabout.ui.h \
            qgsattributeaction.h \
            qgsattributeactiondialog.h \
            qgsattributetable.h \
            qgsattributetablebase.ui.h \
            qgsattributetabledisplay.h \
            qgsattributedialog.h \
            qgsconfig.h \
            qgscontcoldialog.h \
            qgscontinuouscolrenderer.h \
            qgscoordinatetransform.h \
            qgscustomsymbol.h \
            qgsdataprovider.h \
            qgsdatasource.h \
            qgsdbsourceselect.h \
            qgsdbsourceselectbase.ui.h \
            qgsdlgvectorlayerproperties.h \
            qgsfeature.h \
            qgsfeatureattribute.h \
            qgsfield.h \
            qgsgraduatedmarenderer.h \
            qgsgraduatedsymrenderer.h \
            qgsgramadialog.h \
            qgsgramaextensionwidget.h \
            qgsgrasydialog.h \
            qgshelpviewer.h \
            qgshelpviewerbase.ui.h \
            qgsidentifyresults.h \
            qgslegend.h \
            qgslegenditem.h \
            qgslinestyledialog.h \
            qgslinesymbol.h \
            qgsmapcanvas.h \
            qgsmaplayer.h \
            qgsmaplayerinterface.h \
            qgsmaplayerregistry.h \
            qgsmapserverexport.h \
            qgsmapserverexportbase.ui.h \
            qgsmarkerdialog.h \
            qgsmarkersymbol.h \
            qgsmessageviewer.ui.h \
            qgsnewconnection.h \
            qgsoptions.h \
            qgsoptionsbase.ui.h \
            qgspatterndialog.h \
            qgspgquerybuilder.h \
            qgspluginitem.h \
            qgspluginmanager.h \
            qgspluginmetadata.h \
            qgspluginregistry.h \
            qgspoint.h \
            qgspolygonsymbol.h \
            qgsproject.h \
            qgsprojectproperties.h \
            qgsprojectpropertiesbase.ui.h \
            qgsprovidermetadata.h \
            qgsproviderregistry.h \
            qgsrangerenderitem.h \
            qgsrasterlayer.h \
            qgsrasterlayerproperties.h \
            qgsrasterlayerpropertiesbase.ui.h \
            qgsrect.h \
            qgsrenderer.h \
            qgsrenderitem.h \
            qgsscalecalculator.h \
            qgssimadialog.h \
            qgssimarenderer.h \
            qgssinglesymrenderer.h \
            qgssisydialog.h \
            qgssymbol.h \
            qgssymbologyutils.h \
            qgstable.h \
            qgsvectordataprovider.h \
            qgsvectorlayer.h \
            qgssvgcache.h \
            splashscreen.h \
            qgsacetateobject.h \
            qgslabel.h \
            qgslabelattributes.h \
            qgslabeldialog.h \
            qgsacetaterectangle.h \
            qgsuvaldialog.h \
            qgsludialog.h \
            qgsuniquevalrenderer.h \
            qgsuvalmadialog.h \
            qgsuvalmarenderer.h \
            qgscolortable.h \
            qgsvectorfilewriter.h \
            qgsgeomtINTERFACES += qgisappbase.ui \
            qgsabout.ui \
            qgsattributetablebase.ui \
            qgsattributeactiondialogbase.ui \
            qgsattributedialogbase.ui \
            qgscontcoldialogbase.ui \
            qgsdbsourceselectbase.ui \
            qgsdlgvectorlayerpropertiesbase.ui \
            qgsgramadialogbase.ui \
            qgsgrasydialogbase.ui \
            qgshelpviewerbase.ui \
            qgsidentifyresultsbase.ui \
            qgslegenditembase.ui \
            qgslinestyledialogbase.ui \
            qgsmapserverexportbase.ui \
            qgsmarkerdialogbase.ui \
            qgsmessageviewer.ui \
            qgsnewconnectionbase.ui \
            qgsoptionsbase.ui \
            qgspatterndialogbase.ui \
            qgspgquerybuilderbase.ui \
            qgspluginmanagerbase.ui \
            qgsprojectpropertiesbase.ui \
            qgsrasterlayerpropertiesbase.ui \
            qgslabeldialogbase.ui \ 
            qgssimadialogbase.ui \
            qgssisydialogbase.ui \ 
            qgsludialogbase.ui \
            qgsuvaldialogbase.ui \
            qgsuvalmadialogbase.ui \
            qgsgeomtypediSOURCES += main.cpp \
            qgisapp.cpp \
            qgisiface.cpp \
            qgisinterface.cpp \
            qgsattributeaction.cpp \
            qgsattributeactiondialog.cpp \
            qgsattributedialog.cpp \
            qgsattributetable.cpp \
            qgsattributetabledisplay.cpp \
            qgscontcoldialog.cpp \
            qgscontinuouscolrenderer.cpp \
            qgscoordinatetransform.cpp \
            qgscustomsymbol.cpp \
            qgsdatasource.cpp \
            qgsdbsourceselect.cpp \
            qgsdlgvectorlayerproperties.cpp \
            qgsfeature.cpp \
            qgsfeatureattribute.cpp \
            qgsfield.cpp \
            qgsgraduatedmarenderer.cpp \
            qgsgraduatedsymrenderer.cpp \
            qgsgramadialog.cpp \
            qgsgramaextensionwidget.cpp \
            qgsgrasydialog.cpp \
            qgshelpviewer.cpp \
            qgsidentifyresults.cpp \
            qgslegend.cpp \
            qgslegenditem.cpp \
            qgslinestyledialog.cpp \
            qgslinesymbol.cpp \
            qgsmapcanvas.cpp \
            qgsmaplayer.cpp \
            qgsmaplayerregistry.cpp \
            qgsmapserverexport.cpp \
            qgsmarkerdialog.cpp \
            qgsmarkersymbol.cpp \
            qgsnewconnection.cpp \
            qgsoptions.cpp \
            qgspatterndialog.cpp \
            qgspgquerybuilder.cpp \
            qgspluginitem.cpp \
            qgspluginmanager.cpp \
            qgspluginmetadata.cpp \
            qgspluginregistry.cpp \
            qgspoint.cpp \
            qgspolygonsymbol.cpp \
            qgsproject.cpp \
            qgsprojectproperties.cpp \
            qgsprovidermetadata.cpp \
            qgsproviderregistry.cpp \
            qgsrangerenderitem.cpp \
            qgsrasterlayer.cpp \
            qgsrasterlayerproperties.cpp \
            qgsrect.cpp \
            qgsrenderitem.cpp \
            qgsscalecalculator.cpp \
            qgssimadialog.cpp \
            qgssimarenderer.cpp \
            qgssinglesymrenderer.cpp \
            qgssisydialog.cpp \
            qgssymbol.cpp \
            qgssymbologyutils.cpp \
            qgsvectorlayer.cpp \
            qgssvgcache.cpp \
            splashscreen.cpp \
            qgsacetateobject.cpp \
            qgslabeldialog.cpp \
            qgslabel.cpp \
            qgslabelattributes.cpp \
            qgsacetaterectangle.cpp \
            qgsuvaldialog.cpp \
            qgsludialog.cpp \
            qgsuniquevalrenderer.cpp \
            qgsuvalmadialog.cpp \
            qgsuvalmarenderer.cpp \
            qgscolortable.h \
            qgsvectorfilewriter.cpp \
            qgsgeomtypedialog.cpp

