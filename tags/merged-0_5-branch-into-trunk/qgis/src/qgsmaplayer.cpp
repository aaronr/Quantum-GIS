/***************************************************************************
                          qgsmaplayer.cpp  -  description
                             -------------------
    begin                : Fri Jun 28 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
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
#include <cfloat>
#include <iostream>
#include <qapplication.h>
#include <qdatetime.h>
#include <qpopupmenu.h>
#include <qlistview.h>
#include "qgsrect.h"
#include "qgssymbol.h"
#include "qgsmaplayer.h"
#include <qpainter.h>

QgsMapLayer::QgsMapLayer(int type, QString lyrname, QString source):internalName(lyrname), layerType(type), dataSource(source),
m_legendItem(0)
{
  // assume the layer is valid (data source exists and can be used)
  // until we learn otherwise
  valid = true;
  m_visible = true;
  mShowInOverview = false;
  mShowInOverviewItemId =0;
  // Set the display name = internal name
  layerName = internalName;

  // Generate the unique ID of this layer
  QDateTime dt = QDateTime::currentDateTime();
  ID = lyrname + dt.toString("yyyyMMddhhmmsszzz");

#ifdef WIN32
QString PKGDATAPATH = qApp->applicationDirPath() + "/qgis/share";
#endif
  mInOverviewPixmap.load(QString(PKGDATAPATH) + QString("/images/icons/inoverview.png"));
  mEditablePixmap.load(QString(PKGDATAPATH) + QString("/images/icons/editable.png"));

}

QgsMapLayer::~QgsMapLayer()
{
}
const int QgsMapLayer::type()
{
  return layerType;
}

/** Get this layer's unique ID */
QString QgsMapLayer::getLayerID()
{
  return ID;
}

/** Write property of QString layerName. */
void QgsMapLayer::setLayerName(const QString & _newVal)
{
  layerName = _newVal;
}

/** Read property of QString layerName. */
const QString QgsMapLayer::name()
{
  return layerName;
}

QString QgsMapLayer::source()
{
  return dataSource;
}

QString QgsMapLayer::sourceName()
{
  return internalName;
}

const QgsRect QgsMapLayer::extent()
{
  return layerExtent;
}

QgsRect QgsMapLayer::calculateExtent()
{
  //just to prevent any crashes
  QgsRect rect(DBL_MAX, DBL_MAX, DBL_MIN, DBL_MIN);
  return rect;
}
void QgsMapLayer::draw(QPainter *, QgsRect * viewExtent, int yTransform)
{
//  std::cout << "In QgsMapLayer::draw" << std::endl;
}

void QgsMapLayer::draw(QPainter *, QgsRect *, QgsCoordinateTransform *,QPaintDevice * )
{
//  std::cout << "In QgsMapLayer::draw" << std::endl;
}

void QgsMapLayer::drawLabels(QPainter *, QgsRect *, QgsCoordinateTransform *,QPaintDevice * )
{
//  std::cout << "In QgsMapLayer::draw" << std::endl;
}

/** Read property of QString labelField. */
const QString & QgsMapLayer::labelField()
{
  return m_labelField;
}

/** Write property of QString labelField. */
void QgsMapLayer::setLabelField(const QString & _newVal)
{
  m_labelField = _newVal;
}

bool QgsMapLayer::isValid()
{
  return valid;
}

bool QgsMapLayer::visible()
{
  return m_visible;
}

void QgsMapLayer::setVisible(bool vis)
{
  if (m_visible != vis)
  {
    ((QCheckListItem *) m_legendItem)->setOn(vis);
    m_visible = vis;
    emit visibilityChanged();
  }
}  /** Read property of int featureType. */

void QgsMapLayer::toggleShowInOverview()
{
  if (mShowInOverview==false)
  {
#ifdef QGISDEBUG
    std::cout << "Map layer " << ID << " requested to be added to the overview " << std::endl;
#endif
    mShowInOverview=true;
  }
  else
  {
#ifdef QGISDEBUG
    std::cout << "Map layer " << ID << " requested to be removed from the overview " << std::endl;
#endif
    mShowInOverview=false;
  }
  //update the show in overview popup menu item
  updateOverviewPopupItem();
  updateItemPixmap();
  emit showInOverview(ID,mShowInOverview);
}

void QgsMapLayer::updateItemPixmap()
{
  if (m_legendItem)
  {
      QPixmap pix=*(this->legendPixmap());
      if(mShowInOverview)
      {
	  //add overview glasses to the pixmap
	  QPainter p(&pix);
	  p.drawPixmap(0,0,mInOverviewPixmap);
      }
      if(isEditable())
      {
	  //add editing icon to the pixmap
	  QPainter p(&pix);
	  p.drawPixmap(30,0,mEditablePixmap);
      }
      ((QCheckListItem *) m_legendItem)->setPixmap(0,pix);
  }  
}

void QgsMapLayer::updateOverviewPopupItem()
{
    if (mShowInOverviewItemId != 0)
    {
	popMenu->setItemChecked(mShowInOverviewItemId,mShowInOverview);
    }
   
}

const int &QgsMapLayer::featureType()
{
  return geometryType;
}

/** Write property of int featureType. */
void QgsMapLayer::setFeatureType(const int &_newVal)
{
  geometryType = _newVal;
}

QPixmap *QgsMapLayer::legendPixmap()
{
  return &m_legendPixmap;
}

QgsLegendItem *QgsMapLayer::legendItem()
{
  return m_legendItem;
}

void QgsMapLayer::setLegendItem(QgsLegendItem * li)
{
  m_legendItem = li;
}

QPopupMenu *QgsMapLayer::contextMenu()
{
  return 0;
}


QgsFeature * QgsMapLayer::getFirstFeature(bool fetchAttributes) const
{
    return 0x0;                 // by default return NULL
} // QgsMapLayer::getFirstFeature


QgsFeature * QgsMapLayer::getNextFeature(bool fetchAttributes) const
{
    return 0x0;                 // by default return NULL
} // QgsMapLayer::getNextFeature


bool QgsMapLayer::getNextFeature(QgsFeature &feature, bool fetchAttributes) const
{
    return false;
} // QgsMapLayer::getNextFeature


long QgsMapLayer::featureCount() const
{
    return 0;
} // QgsMapLayer::featureCount

  

int QgsMapLayer::fieldCount() const
{
    return 0;
} // QgsMapLayer::fieldCount



std::vector<QgsField> const & QgsMapLayer::fields() const
{
    static std::vector<QgsField> bogus; // bogus empty container

    return bogus;
} // QgsMapLayer::fields()
