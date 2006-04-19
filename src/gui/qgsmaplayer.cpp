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
#include <limits>
#include <cmath>

#include <QDateTime>
#include <QDomNode>
#include <QFileInfo>
#include <QPainter>
#include <QAction>
#include <QKeyEvent>

#include "qgisapp.h"
#include "qgslogger.h"
#include "qgsmaptopixel.h"
#include "qgsrect.h"
#include "qgsproject.h"
#include "qgssymbol.h"
#include "qgsmaplayer.h"



QgsMapLayer::QgsMapLayer(int type,
                         QString lyrname,
                         QString source) :
        transparencyLevelInt(255), // 0 is completely transparent
        valid(true), // assume the layer is valid (data source exists and 
                     // can be used) until we learn otherwise
        dataSource(source),
        mCoordinateTransform(0),
        ID(""),
        layerType(type)

{
  QgsDebugMsg("QgsMapLayer::QgsMapLayer - lyrname is '" + lyrname);

    // Set the display name = internal name
    layerName = lyrname;
    QgsDebugMsg("QgsMapLayer::QgsMapLayer - layerName is '" + layerName);

    // Generate the unique ID of this layer
    QDateTime dt = QDateTime::currentDateTime();
    ID = lyrname + dt.toString("yyyyMMddhhmmsszzz");
    ID.replace(" ", "_");

    //set some generous  defaults for scale based visibility
    mMinScale = 0;
    mMaxScale = 100000000;
    mScaleBasedVisibility = false;
}



QgsMapLayer::~QgsMapLayer()
{
    //delete mCoordinateTransform; //crash when removing layers. Why?
}

int QgsMapLayer::type() const
{
    return layerType;
}

/** Get this layer's unique ID */
QString const & QgsMapLayer::getLayerID() const
{
    return ID;
}

/** Write property of QString layerName. */
void QgsMapLayer::setLayerName(const QString & _newVal)
{
  QgsDebugMsg("QgsMapLayer::setLayerName: new name is '" + _newVal);
  layerName = _newVal;
}

/** Read property of QString layerName. */
QString const & QgsMapLayer::name() const
{
  QgsDebugMsg("QgsMapLayer::name: returning name '" + layerName);
  return layerName;
}

QString const & QgsMapLayer::source() const
{
    return dataSource;
}

const QgsRect QgsMapLayer::extent()
{
    return layerExtent;
}

QgsRect QgsMapLayer::calculateExtent()
{
    //just to prevent any crashes
    QgsRect rect;
    
    rect.setMinimal();
    return rect;
}

void QgsMapLayer::draw(QPainter *, QgsRect * viewExtent, int yTransform)
{
    //  std::cout << "In QgsMapLayer::draw" << std::endl;
}

bool QgsMapLayer::draw(QPainter *, QgsRect *, QgsMapToPixel *)
{
    //  std::cout << "In QgsMapLayer::draw" << std::endl;
    return false;
}

void QgsMapLayer::drawLabels(QPainter *, QgsRect *, QgsMapToPixel *)
{
    //  std::cout << "In QgsMapLayer::draw" << std::endl;
}

/** Read property of QString labelField. */
const QString & QgsMapLayer::labelField()
{
    return m_labelField;
}


bool QgsMapLayer::readXML( QDomNode & layer_node )
{
    QDomElement element = layer_node.toElement();

    // XXX not needed? QString type = element.attribute("type");

    // use scale dependent visibility flag
    QString scaleBasedVisibility = element.attribute("scaleBasedVisibilityFlag");
    if ( "1" == scaleBasedVisibility )
    {
        setScaleBasedVisibility(true);
    }
    else
    {
        setScaleBasedVisibility(false);
    }
    setMinScale(element.attribute("minScale").toFloat());
    setMaxScale(element.attribute("maxScale").toFloat());

    // set data source
    QDomNode mnl = layer_node.namedItem("datasource");
    QDomElement mne = mnl.toElement();
    dataSource = mne.text();

    const char * dataSourceStr = dataSource.toLocal8Bit().data(); // debugger probe

    // the internal name is just the data source basename
    QFileInfo dataSourceFileInfo( dataSource );
    //internalName = dataSourceFileInfo.baseName();

    // set ID
    mnl = layer_node.namedItem("id");
    if ( ! mnl.isNull() ) 
    {
        mne = mnl.toElement();
        if ( ! mne.isNull() && mne.text().length() > 10 ) { // should be at least 17 (yyyyMMddhhmmsszzz)
	    ID = mne.text();
	}
    }

    // set name
    mnl = layer_node.namedItem("layername");
    mne = mnl.toElement();
    setLayerName( mne.text() );

    const char * layerNameStr = mne.text().toLocal8Bit().data(); // debugger probe



    //read srs
    QDomNode srsNode = layer_node.namedItem("coordinatetransform");
    if( ! srsNode.isNull()  )
    {
       mCoordinateTransform=new QgsCoordinateTransform();
       mCoordinateTransform->readXML(srsNode);
    }
    
    //read transparency level
    QDomNode transparencyNode = layer_node.namedItem("transparencyLevelInt");
    if ( ! transparencyNode.isNull() )
    {
      // set transparency level only if it's in project
      // (otherwise it sets the layer transparent)
      QDomElement myElement = transparencyNode.toElement();
      setTransparency(myElement.text().toInt());
    }


    // now let the children grab what they need from the DOM node.
    return readXML_( layer_node );

} // void QgsMapLayer::readXML


bool QgsMapLayer::readXML_( QDomNode & layer_node )
{
    // NOP by default; children will over-ride with behavior specific to them

    return true;
} // void QgsMapLayer::readXML_



bool QgsMapLayer::writeXML( QDomNode & layer_node, QDomDocument & document )
{
    // general layer metadata
    QDomElement maplayer = document.createElement( "maplayer" );

    // use scale dependent visibility flag
    if ( scaleBasedVisibility() )
    {
        maplayer.setAttribute( "scaleBasedVisibilityFlag", 1 );
    }
    else
    {
        maplayer.setAttribute( "scaleBasedVisibilityFlag", 0 );
    }
    maplayer.setAttribute( "minScale", minScale() );
    maplayer.setAttribute( "maxScale", maxScale() );

    // ID 
    QDomElement id = document.createElement( "id" );
    QDomText idText = document.createTextNode( getLayerID() );
    id.appendChild( idText );

    maplayer.appendChild( id );

    // data source
    QDomElement dataSource = document.createElement( "datasource" );
    QDomText dataSourceText = document.createTextNode( source() );
    dataSource.appendChild( dataSourceText );

    maplayer.appendChild( dataSource );


    // layer name
    QDomElement layerName = document.createElement( "layername" );
    QDomText layerNameText = document.createTextNode( name() );
    layerName.appendChild( layerNameText );

    maplayer.appendChild( layerName );

    // zorder
    // This is no longer stored in the project file. It is superflous since the layers
    // are written and read in the proper order.

    //write the projection 
    mCoordinateTransform->writeXML(maplayer,document);

    // <transparencyLevelInt>
    QDomElement transparencyLevelIntElement = document.createElement( "transparencyLevelInt" );
    QDomText    transparencyLevelIntText    = document.createTextNode( QString::number(getTransparency()) );
    transparencyLevelIntElement.appendChild( transparencyLevelIntText );
    maplayer.appendChild( transparencyLevelIntElement );
    // now append layer node to map layer node

    layer_node.appendChild( maplayer );

    return writeXML_( maplayer, document );

} // bool QgsMapLayer::writeXML



bool QgsMapLayer::writeXML_( QDomNode & layer_node, QDomDocument & document )
{
    // NOP by default; children will over-ride with behavior specific to them

    return true;
} // void QgsMapLayer::writeXML_




/** Write property of QString labelField. */
void QgsMapLayer::setLabelField(const QString & _newVal)
{
    m_labelField = _newVal;
}

bool QgsMapLayer::isValid()
{
    return valid;
}


void QgsMapLayer::invalidTransformInput()
{
  QgsLogger::warning("QgsMapLayer::invalidTransformInput() called");
  // TODO: emit a signal - it will be used to update legend
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

std::vector<QgsField> const & QgsMapLayer::fields() const
{
    static std::vector<QgsField> bogus; // bogus empty container

    return bogus;
} // QgsMapLayer::fields()

QString QgsMapLayer::errorCaptionString()
{
  return QString();
}

QString QgsMapLayer::errorString()
{
  return QString();
}

void QgsMapLayer::connectNotify( const char * signal )
{
    QgsDebugMsg("QgsMapLayer connected to " + QString(signal));
} //  QgsMapLayer::connectNotify





void QgsMapLayer::keyPressed ( QKeyEvent * e )
{
  if (e->key()==Qt::Key_Escape) mDrawingCancelled = true;
// The following statment causes a crash on WIN32 and should be 
// enclosed in an #ifdef QGISDEBUG if its really necessary. Its
// commented out for now. [gsherman]
//  std::cout << e->text().toLocal8Bit().data() << " pressed in maplayer !" << std::endl;
  e->ignore();
}


    /** Accessor for the coordinate transformation object */
QgsCoordinateTransform * QgsMapLayer::coordinateTransform() 
{
#ifdef QGISDEBUG
 QgsDebugMsg("Maplayer asked for coordinateTransform which is....");
 if (!mCoordinateTransform)
 {
   QgsDebugMsg("*NOT* valid");
 }
 else
 {
   QgsDebugMsg("valid");
 }
 
#endif
  
 return mCoordinateTransform;
}

bool QgsMapLayer::projectionsEnabled() const
{
  if (QgsProject::instance()->readNumEntry("SpatialRefSys","/ProjectionsEnabled",0)!=0)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool QgsMapLayer::projectExtent(QgsRect& extent, QgsRect& r2)
{
  bool split = false;

  if (projectionsEnabled())
  {
    try
    {
#ifdef QGISDEBUG
      QgsLogger::debug<QgsRect>("Getting extent of canvas in layers CS. Canvas is ", extent, __FILE__,\
				__FUNCTION__, __LINE__);
#endif
      // Split the extent into two if the source SRS is
      // geographic and the extent crosses the split in
      // geographic coordinates (usually +/- 180 degrees,
      // and is assumed to be so here), and draw each
      // extent separately.
      static const double splitCoord = 180.0;

      if (mCoordinateTransform->sourceSRS().geographicFlag())
      {
        // Note: ll = lower left point
        //   and ur = upper right point
        QgsPoint ll = mCoordinateTransform->transform(
                          extent.xMin(), extent.yMin(), 
                          QgsCoordinateTransform::INVERSE);

        QgsPoint ur = mCoordinateTransform->transform(
                          extent.xMax(), extent.yMax(), 
                          QgsCoordinateTransform::INVERSE);

        if (ll.x() > ur.x())
        {
          extent.set(ll, QgsPoint(splitCoord, ur.y()));
          r2.set(QgsPoint(-splitCoord, ll.y()), ur);
          split = true;
        }
        else // no need to split
          extent = mCoordinateTransform->transformBoundingBox(extent, QgsCoordinateTransform::INVERSE);
      }
      else // can't cross 180
        extent = mCoordinateTransform->transformBoundingBox(extent, QgsCoordinateTransform::INVERSE);
    }
    catch (QgsCsException &cse)
      {
	QgsLogger::warning("Transform error caught in " + QString(__FILE__) + ", line " + QString::number(__LINE__));
        extent = QgsRect(-DBL_MAX, -DBL_MAX, DBL_MAX, DBL_MAX);
        r2     = QgsRect(-DBL_MAX, -DBL_MAX, DBL_MAX, DBL_MAX);
      }
  }
  return split;
}

QgsRect QgsMapLayer::calcProjectedBoundingBox(QgsRect& extent)
{
  // Calculate the bounding box of the canvas extent when
  // inverse projected to the source SRS. This is done by
  // looking at 10 points on each edge of the rectangular canvas
  // extent. This seems to be a sufficient number to get a good
  // approximation to the bounding box.
  static const double numPoints = 10.0;

  std::vector<QgsPoint> left, right, top, bottom;
  QgsPoint pt;
  // Populate the vectors

  for (int i = 0; i < numPoints; ++i)
  {
    // the left and right boundary
    pt.setX(extent.xMin());
    pt.setY(extent.yMin() + extent.height()
            / numPoints * static_cast<double>(i));
    left.push_back(mCoordinateTransform->transform(pt, 
                   QgsCoordinateTransform::INVERSE));
    pt.setX(extent.xMax());
    right.push_back(mCoordinateTransform->transform(pt, 
                    QgsCoordinateTransform::INVERSE));

    // the top and bottom boundary
    pt.setY(extent.yMin());
    pt.setX(extent.xMin() + extent.width() 
            / numPoints * static_cast<double>(i));
    bottom.push_back(mCoordinateTransform->transform(pt,
                     QgsCoordinateTransform::INVERSE));
    pt.setY(extent.yMax());
    top.push_back(mCoordinateTransform->transform(pt,
                  QgsCoordinateTransform::INVERSE));
  }

  // Calculate the bounding box and use that for the extent
        
  double xmin = std::numeric_limits<double>::max();
  double xmax = std::numeric_limits<double>::min();
  double ymin = std::numeric_limits<double>::max();
  double ymax = std::numeric_limits<double>::min();
  for (int i = 0; i < top.size(); ++i)
  {
    xmin = std::min(xmin, left[i].x());
    xmax = std::max(xmax, right[i].x());
    ymin = std::min(ymin, bottom[i].y());
    ymax = std::max(ymax, top[i].y());
  }

  QgsRect bb_extent;
  bb_extent.set(xmin, ymin, xmax, ymax);

  return bb_extent;
}

void QgsMapLayer::setScaleBasedVisibility(bool theVisibilityFlag)
{
  mScaleBasedVisibility = theVisibilityFlag;
}

bool QgsMapLayer::scaleBasedVisibility()
{
  return mScaleBasedVisibility;
}

void QgsMapLayer::setMinScale(float theMinScale)
{
  mMinScale = theMinScale;
}
    
float QgsMapLayer::minScale()
{
  return mMinScale;
}

    
void QgsMapLayer::setMaxScale(float theMaxScale)
{
  mMaxScale = theMaxScale;
}

float QgsMapLayer::maxScale()
{
  return mMaxScale;
}
