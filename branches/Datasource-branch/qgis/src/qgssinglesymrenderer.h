/***************************************************************************
                         qgssinglesymrenderer.h  -  description
                             -------------------
    begin                : Oct 2003
    copyright            : (C) 2003 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSINGLESYMRENDERER_H
#define QGSSINGLESYMRENDERER_H

#include "qgsrenderer.h"
#include "qgsrenderitem.h"
#include "qgspoint.h"
#include <ogrsf_frmts.h>
#include <ogr_geometry.h>
#include "qpainter.h"
#include "qgscoordinatetransform.h"

/**Render class to display all the features with a single QgsSymbol*/
class QgsSingleSymRenderer: public QgsRenderer
{
 public:
    QgsSingleSymRenderer();
    ~QgsSingleSymRenderer();
    /**Replaces the current m_item by ri*/
    void addItem(QgsRenderItem ri);
    /**Returns a pointer to m_item*/
    QgsRenderItem* item();
    /**Renders an OGRFeature*/
    void renderFeature(QPainter* p, OGRFeature* f, QgsCoordinateTransform* t, int endian);
 protected:
    QgsRenderItem m_item;
};

inline QgsRenderItem* QgsSingleSymRenderer::item()
{
    return &m_item;
}

#endif
