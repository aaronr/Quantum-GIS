/***************************************************************************
                          qgslegend.cpp  -  description
                             -------------------
    begin                : Sun Jul 28 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
               Romans 3:23=>Romans 6:23=>Romans 10:9,10=>Romans 12
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <map>
#include <qstring.h>
#include <qpainter.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qlistview.h>
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgslegenditem.h"
#include "qgslegend.h"

QgsLegend::QgsLegend(QListView * lv, QWidget * parent, const char *name):QWidget(parent, name), listView(lv)
{
}

QgsLegend::~QgsLegend()
{
}


void QgsLegend::setMapCanvas(QgsMapCanvas * canvas)
{
	map = canvas;
}

void QgsLegend::update()
{
// clear the legend
	listView->clear();

// Get the list of layers in order from the
// map canvas and add legenditems to the legend


	for (int idx = 0; idx < map->layerCount(); idx++) {
		QgsMapLayer *lyr = map->getZpos(idx);
		QgsLegendItem *lvi = new QgsLegendItem(lyr, listView);	// lyr->name(), QCheckListItem::CheckBox );
		//lvi->setOn(lyr->visible());
//  QgsLegendItem *li = new QgsLegendItem(lyr, legendContainer);
		//addChild(li,0,idx*60);
		int foo = 1;
		//repaint();
	}
}
