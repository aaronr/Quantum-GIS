/***************************************************************************
                         qgscomposer.h
                             -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#ifndef QGSCOMPOSER_H
#define QGSCOMPOSER_H
#ifdef WIN32
#include "qgscomposerbase.h"
#else
#include "qgscomposerbase.uic.h"
#endif

#include "qgscomposerview.h"
#include "qgscomposition.h"

class QPrinter;
class QDomNode;
class QDomDocument;
class QgisApp;
class QgsComposerItem;

class QgsComposer: public QgsComposerBase
{
    Q_OBJECT

public:
    QgsComposer(QgisApp *qgis);
    ~QgsComposer();
    
    //! Open and show, set defaults if first time
    void open();

    //! Zoom to full extent of the paper
    void zoomFull();

    //! Zoom in
    void zoomIn();

    //! Zoom out 
    void zoomOut();

    //! Return pointer to map canvas
    QgsMapCanvas *mapCanvas( void );
    
    //! Return pointer to composer view
    QgsComposerView *view ( void );

    //! Return current composition
    QgsComposition *composition(void);

    //! Show composition options in widget
    void showCompositionOptions ( QWidget *w );
    
    //! Show item options in widget
    void showItemOptions ( QWidget *w );

    /** \brief stores statei in project */
    bool writeSettings ( void );

    /** \brief read state from project */
    bool readSettings ( void );

    //! Stores state in DOM node
    bool writeXML( QDomNode & node, QDomDocument & doc);

    //! Sets state from DOM document
    bool readXML( QDomNode & node );

public slots:
    //! Print the composition
    void print(void);
    
    //! Select item
    void selectItem(void);
    
    //! Add new map
    void addMap(void);

    //! Add new vector legend
    void addVectorLegend(void);
    
    //! Add new label
    void addLabel(void);

    //! read project
    void projectRead();

    //! New project
    void newProject();

private:
    //! remove widget childrens
    void removeWidgetChildren ( QWidget *w );

    //! Set buttons up
    void setToolActionsOff (void);

    //! Pointer to composer view
    QgsComposerView *mView;

    //! Current composition
    QgsComposition *mComposition;
    
    //! Printer
    QPrinter *mPrinter;

    //! Pointer to QGIS application
    QgisApp *mQgis;

    //! The composer was opened first time (-> set defaults)
    bool mFirstTime;
};

#endif
