/***************************************************************************
                          qgsmapcanvas.cpp  -  description
                             -------------------
    begin                : Sun Jun 30 2002
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

#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"

#include <iosfwd>
#include <cmath>
#include <cfloat>

// added double sentinals to take load off gcc 3.3.3 pre-processor, which was dying

// XXX actually that wasn't the problem, but left double sentinals in anyway as they
// XXX don't hurt anything

// #ifndef QGUARDEDPTR_H
// #include <qguardedptr.h>
// #endif

#ifndef QDOM_H
#include <qdom.h>
#endif

#ifndef QLISTVIEW_H
#include <qlistview.h>
#endif

#ifndef QMESSAGEBOX_H
#include <qmessagebox.h>
#endif

#ifndef QPAINTDEVICE_H
#include <qpaintdevice.h>
#endif

#ifndef QPAINTDEVICEMETRICS_H
#include <qpaintdevicemetrics.h>
#endif

#ifndef QPAINTER_H
#include <qpainter.h>
#endif

#ifndef QPIXMAP_H
#include <qpixmap.h>
#endif

#ifndef QRECT_H
#include <qrect.h>
#endif

#ifndef QSETTINGS_H
#include <qsettings.h>
#endif

#ifndef QSTRING_H
#include <qstring.h>
#endif

#include <qcursor.h>


#include "qgis.h"
#include "qgsrect.h"
#include "qgsacetatelines.h"
#include "qgsacetaterectangle.h"
#include "qgsattributedialog.h"
#include "qgsmaptopixel.h"
#include "qgsfeature.h"
#include "qgslegend.h"
#include "qgslegenditem.h"
#include "qgsline.h"
#include "qgslinesymbol.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerinterface.h"
#include "qgsmarkersymbol.h"
#include "qgspolygonsymbol.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsmeasure.h"
#include <qgsscalecalculator.h>



/**
 
   Implementation struct for QgsMapCanvas
 
  @note
 
  Changed to class from struct out of desperation to find workaround for g++ bug.
 
*/
class QgsMapCanvas::CanvasProperties
{
public:

  CanvasProperties( int width, int height )
      : mapWindow( 0x0 ),
      mapLegend( 0 ),
      coordXForm( 0x0 ),
      pmCanvas( 0x0 ),
      bgColor( Qt::white ),
      dragging( false ),
      panSelectorDown( false ),
      drawing( false ),
      frozen( false ),
      dirty( true ),
      scaleCalculator( 0x0 )
  {
    mapWindow = new QRect;
    coordXForm = new QgsMapToPixel;
    pmCanvas = new QPixmap(width, height);
    scaleCalculator = new QgsScaleCalculator;
    // set the initial extent - can't use a constructor since QgsRect
    // normalizes the rectangle upon construction
    fullExtent.setXmin(9999999999.0);
    fullExtent.setYmin(999999999.0);
    fullExtent.setXmax(-999999999.0);
    fullExtent.setYmax(-999999999.0);
  }

  CanvasProperties()
      : mapWindow( 0x0 ),
      mapLegend( 0 ),
      coordXForm( 0x0 ),
      pmCanvas( 0x0 ),
      bgColor( Qt::white ),
      dragging( false ),
      panSelectorDown( false ),
      drawing( false ),
      frozen( false ),
      dirty( true ),
      scaleCalculator( 0x0 )
  {
    mapWindow = new QRect;
    coordXForm = new QgsMapToPixel;
    pmCanvas = new QPixmap;
    scaleCalculator = new QgsScaleCalculator;
  }


  ~CanvasProperties()
  {
    delete coordXForm;
    delete pmCanvas;
    delete mapWindow;
    delete scaleCalculator;
  } // ~CanvasProperties


  void initMetrics(QPaintDeviceMetrics *pdm)
  {
    // set the logical dpi
    mDpi = pdm->logicalDpiX();
    scaleCalculator->setDpi(mDpi);

    // set default map units
    mMapUnits = QGis::METERS;
    scaleCalculator->setMapUnits(mMapUnits);
  }

  void setMapUnits(QGis::units u)
  {
    mMapUnits = u;
    scaleCalculator->setMapUnits(mMapUnits);
    QgsProject::instance()->mapUnits(u);
  }

  QGis::units mapUnits()
  {
    return mMapUnits;
  }

  //! map containing the layers by name
  std::map< QString, QgsMapLayer *> layers;

  //! map containing the acetate objects by key (name)
  std::map< QString, QgsAcetateObject *> acetateObjects;

  //! list containing the names of layers in zorder
  std::list< QString > zOrder;

  //! Full extent of the map canvas
  QgsRect fullExtent;

  //! Current extent
  QgsRect currentExtent;

  //! Previous view extent
  QgsRect previousExtent;

  //! Map window rectangle
  //std::auto_ptr<QRect> mapWindow;
  QRect * mapWindow;

  //! Pointer to the map legend
  //std::auto_ptr<QgsLegend> mapLegend;
  QgsLegend * mapLegend;

  /** Pointer to the coordinate transform object used to transform
    coordinates from real world to device coordinates
    */
  //std::auto_ptr<QgsMapToPixel> coordXForm;
  QgsMapToPixel * coordXForm;

  /** The output spatial reference system that was used most
      recently. Obtained from a layer on this canvas
  */
  QgsSpatialRefSys previousOutputSRS;

  /**
   * \brief Currently selected map tool.
   * @see QGis::MapTools enum for valid values
   */
  int mapTool;

  //!Flag to indicate status of mouse button
  bool mouseButtonDown;

  //! Map units per pixel
  double m_mupp;

  //! Rubber band box for dynamic zoom
  QRect zoomBox;

  //! Beginning point of a rubber band box
  QPoint boxStartPoint;

  //! Last seen point of the mouse
  QPoint mouseLastXY;

  //! Pixmap used for restoring the canvas.
  /** @note using QGuardedPtr causes sefault for some reason -- XXX trying again */
  //QGuardedPtr<QPixmap> pmCanvas;
  //std::auto_ptr<QPixmap> pmCanvas;
  QPixmap * pmCanvas;

  //! Background color for the map canvas
  QColor bgColor;

  //! Flag to indicate a map canvas drag operation is taking place
  bool dragging;

  //! Flag to indicate the pan selector key is held down by user
  bool panSelectorDown;

  //! Vector containing the inital color for a layer
  std::vector < QColor > initialColor;

  //! Flag indicating a map refresh is in progress
  bool drawing;


  //! Flag indicating if the map canvas is frozen.
  bool frozen;

  /*! \brief Flag to track the state of the Map canvas.
   *
   * The canvas is
   * flagged as dirty by any operation that changes the state of
   * the layers or the view extent. If the canvas is not dirty, paint
   * events are handled by bit-blitting the stored canvas bitmap to
   * the canvas. This improves performance by not reading the data source
   * when no real change has occurred
   */
  bool dirty;


  //! Value use to calculate the search radius when identifying features
  // TODO - Do we need this?
  double radiusValue;

  //std::auto_ptr<QgsScaleCalculator> scaleCalculator;
  QgsScaleCalculator * scaleCalculator;

  //! DPI of physical display
  int mDpi;

  //! Map units for the data on the canvas
  QGis::units mMapUnits;

  //! Map scale of the canvas at its current zool level
  double mScale;

private:

  /** not copyable
  */
  CanvasProperties( CanvasProperties const & rhs )
  {
    // XXX maybe should be NOP just like operator=() to be consistent
    std::cerr << __FILE__ << ":" << __LINE__
    << " should not be here since CanvasProperties shouldn't be copyable\n";
  } // CanvasProperties copy ctor


  /** not copyable
  */
  CanvasProperties & operator=( CanvasProperties const & rhs )
  {
    if ( this == &rhs )
    {
      return *this;
    }

    std::cerr << __FILE__ << ":" << __LINE__
    << " should not be here since CanvasProperties shouldn't be copyable\n";

    return *this;
  } // CanvasProperties assignment operator

}
; // struct QgsMapCanvas::CanvasProperties



/** note this is private and so shouldn't be accessible */
QgsMapCanvas::QgsMapCanvas()
{}


QgsMapCanvas::QgsMapCanvas(QWidget * parent, const char *name)
    : QWidget(parent, name),
    mCanvasProperties( new CanvasProperties(width(), height()) ),
    mUserInteractionAllowed(true), // by default we allow a user to interact with the canvas
      mLineEditing(false), mPolygonEditing(false)
{
  // by default, the canvas is rendered
  mRenderFlag = true;
  //by default we assume we are not an overview canvas
  mIsOverviewCanvas = false;
  setEraseColor(mCanvasProperties->bgColor);

  setMouseTracking(true);
  setFocusPolicy(QWidget::StrongFocus);

  QPaintDeviceMetrics *pdm = new QPaintDeviceMetrics(this);
  mCanvasProperties->initMetrics(pdm);
  delete pdm;
    
  mMeasure = 0;

} // QgsMapCanvas ctor


QgsMapCanvas::~QgsMapCanvas()
{
  // mCanvasProperties auto-deleted via std::auto_ptr
  // CanvasProperties struct has its own dtor for freeing resources
} // dtor



void QgsMapCanvas::setLegend(QgsLegend * legend)
{
  mCanvasProperties->mapLegend = legend;
} // setLegend



QgsLegend *QgsMapCanvas::getLegend()
{
  return mCanvasProperties->mapLegend;
} // getLegend

double QgsMapCanvas::getScale()
{
  return mCanvasProperties->mScale;
} // getScale

void QgsMapCanvas::setDirty(bool _dirty)
{
  mCanvasProperties->dirty = _dirty;
} // setDirty



bool QgsMapCanvas::isDirty() const
{
  return mCanvasProperties->dirty;
} // isDirty



bool QgsMapCanvas::isDrawing()
{
  return mCanvasProperties->drawing;
  ;
} // isDrawing



void QgsMapCanvas::addLayer(QgsMapLayerInterface * lyr)
{
  // add a maplayer interface to a layer type defined in a plugin

#ifdef QGISDEBUG
  std::cerr << __FILE__ << ":" << __LINE__
  << "  QgsMapCanvas::addLayer() invoked\n";
#endif

} // addlayer



void QgsMapCanvas::showInOverview( QgsMapLayer * maplayer, bool visible )
{
  // first, this is irrelevent if we're NOT the overview map canvas
  if ( 0 != strcmp("theOverviewCanvas", name()) )
  {
    return;
  }

  std::map < QString, QgsMapLayer * >::iterator found =
    mCanvasProperties->layers.find(maplayer->getLayerID());

  // if it's visible, and we already know about it, then do nothing;
  // otherwise, we need to add it if "visible" says so
  if ( found == mCanvasProperties->layers.end() &&
       visible )
  {
    addLayer( maplayer );
  } // if we have it and it's supposed to be removed, remove it
  else if ( found != mCanvasProperties->layers.end() &&
            ! visible )
  {
    remove
      ( maplayer->getLayerID() );
  }

} // QgsMapCanvas::showInOverview




void QgsMapCanvas::addLayer(QgsMapLayer * lyr)
{
#ifdef QGISDEBUG
  std::cout << name() << " is adding " << lyr->name().local8Bit() << std::endl;
#endif

  Q_CHECK_PTR( lyr );

  if ( ! lyr )
  {
    return;
  }

  // CRUDE HACK.  If this map canvas is the overview canvas and the given
  // layer doesn't want to be in the overview, then just skip adding it.

  // XXX Tim and I (Mark) have discussed possibly making QgsMapCanvas sub-class
  // XXX QgsMapOverviewCanvas (or just QgsMapOverview?) that behaves a bit
  // XXX differently from parent in that it properly handles overview flags

#ifdef QGISDEBUG
  const char * n = name(); // debugger sensor
#endif

  bool isThisOverviewCanvas = false;

  if ( 0 == strcmp(name(),"theOverviewCanvas") ) // canonical name set in qgisapp ctor
  {
    isThisOverviewCanvas = true;
  }

  if ( isThisOverviewCanvas )
  {
    // regardless of what happens, we need to be in communication with this
    // layer to possibly add it to overview canvas; however, we only want to
    // make this connection once, so we first check to see if we already
    // know about the layer
    if ( mCanvasProperties->layers.end() ==
         mCanvasProperties->layers.find(lyr->getLayerID() )  )
    {
      QObject::connect(lyr, SIGNAL(showInOverview(QgsMapLayer *, bool)),
                       this, SLOT(showInOverview( QgsMapLayer *, bool )));
    }

    if ( ! lyr->showInOverviewStatus() )
    {
#ifdef QGISDEBUG
      qDebug((lyr->name() + " not in overview, so skipping in addLayer()").local8Bit() );
#endif

      return;               // doesn't want to be in overview, so don't add
    }
    else
    {
#ifdef QGISDEBUG
      qDebug(( lyr->name() + " in overview, invoking addLayer()").local8Bit() );
#endif

    }
  }

  if (mCanvasProperties->layers.size() == 0)
  {
    // Adding the first layer. Set the previousOutputSRS to the output
    // SRS for the layer.

      if ( ! lyr->coordinateTransform() )
      {
          QgsDebug( "NO COORDINATE TRANSFORM FOUND FOR LAYER" );
      }
      else
      {
          mCanvasProperties->previousOutputSRS = lyr->coordinateTransform()->destSRS();
          // While we're here, set the default map units to those
          // of the destSRS().
          setMapUnits(lyr->coordinateTransform()->destSRS().mapUnits());
      }
  }

  mCanvasProperties->layers[lyr->getLayerID()] = lyr;

  // update extent if warranted
  if (mCanvasProperties->layers.size() == 1)
  {
    // if only 1 layer, set the full extent to its extent
    
    // the layer extent must be projected to the same coordinate
    // system as the map canvas prior to updating the canvas extents
    if(projectionsEnabled())
    {
      try
      {
          if ( ! lyr->coordinateTransform() )
          {
              QgsDebug( "NO COORDINATE TRANSFORM FOUND FOR LAYER" );
          }
          else
          {
              QgsRect tRect = lyr->coordinateTransform()->transformBoundingBox(lyr->extent());
              mCanvasProperties->fullExtent = tRect;
          }
      }
      catch(QgsCsException &cse)
      {
#ifdef QGISDEBUG
	std::cout << "Caught transform error in QgsMapCanvas::addLayer(). "
		  << "Setting extents to untransformed extents" << std::endl;
#endif	
	mCanvasProperties->fullExtent = lyr->extent();
	mCanvasProperties->fullExtent.scale(1.1);
      }
    }
    else
    {
      mCanvasProperties->fullExtent = lyr->extent();
      mCanvasProperties->fullExtent.scale(1.1);
    }
    // XXX why magic number of 1.1? - TO GET SOME WHITESPACE AT THE EDGES OF THE MAP
    mCanvasProperties->currentExtent = mCanvasProperties->fullExtent;
  }
  else
  {
    if(projectionsEnabled())
    {
      // project the layer extent and pass it off to update the full extent
      try 
      {
          if ( ! lyr->coordinateTransform() )
          {
              QgsDebug( "NO COORDINATE TRANSFORM FOUND FOR LAYER" );
          }
          else
          {
              updateFullExtent(lyr->coordinateTransform()->transformBoundingBox(lyr->extent()));
          }
      }
      catch(QgsCsException &cse)
      {
#ifdef QGISDEBUG
	std::cout << "Caught transform error in QgsMapCanvas::addLayer(). "
		  << "Ignoring this layer in map extents calculation." << std::endl;
#endif	
      }
    }
    else
    {
      updateFullExtent(lyr->extent());
    }
  }

  mCanvasProperties->zOrder.push_back(lyr->getLayerID());

  QObject::connect(lyr, SIGNAL(visibilityChanged()), this, SLOT(layerStateChange()));
  QObject::connect(lyr, SIGNAL(repaintRequested()), this, SLOT(refresh()));

  mCanvasProperties->dirty = true;

  emit addedLayer( lyr );

} // addLayer



void QgsMapCanvas::addAcetateObject(QString key, QgsAcetateObject *obj)
{
  // since we are adding pointers, check to see if the object
  // referenced by key already exists and if so, delete it prior
  // to adding the new object with the same key
  QgsAcetateObject *oldObj = mCanvasProperties->acetateObjects[key];
  if(oldObj)
  {
    delete oldObj;
  }

  mCanvasProperties->acetateObjects[key] = obj;
}

void QgsMapCanvas::removeAcetateObject(const QString& key)
{
  std::map< QString, QgsAcetateObject *>::iterator it=mCanvasProperties->acetateObjects.find(key);
  if(it!=mCanvasProperties->acetateObjects.end())
  {
    QgsAcetateObject* toremove=it->second;
    mCanvasProperties->acetateObjects.erase(it->first);
    delete toremove;
  }
}

void QgsMapCanvas::removeDigitizingLines(bool norepaint)
{
    bool rpaint = false;
    if(!norepaint)
    {
	rpaint = (mCaptureList.size()>0) ? true : false;
    }
    mCaptureList.clear();
    mLineEditing=false;
    mPolygonEditing=false;
    if(rpaint)
    {
	setDirty(true);
	render();
    }
}

QgsMapLayer *QgsMapCanvas::getZpos(int idx)
{
  //  iterate over the zOrder and return the layer at postion idx
  std::list < QString >::iterator zi = mCanvasProperties->zOrder.begin();
  for (int i = 0; i < idx; i++)
  {
    if (i < mCanvasProperties->zOrder.size())
    {
      zi++;
    }
  }

  QgsMapLayer *ml = mCanvasProperties->layers[*zi];
  return ml;
} // getZpos



void QgsMapCanvas::setZOrderFromLegend(QgsLegend * lv)
{
  mCanvasProperties->zOrder.clear();
  QListViewItemIterator it(lv);

  while (it.current())
  {
    QgsLegendItem *li = (QgsLegendItem *) it.current();
    QgsMapLayer *lyr = li->layer();
    mCanvasProperties->zOrder.push_front(lyr->getLayerID());
    ++it;
  }

  refresh();

} // setZOrderFromLegend



QgsMapLayer *QgsMapCanvas::layerByName(QString name)
{
  return mCanvasProperties->layers[name];
} // layerByName



void QgsMapCanvas::refresh()
{
  mCanvasProperties->dirty = true;
  render();
} // refresh

// The painter device parameter is optional - if ommitted it will default
// to the pmCanvas (ie the gui map display). The idea is that you can pass
// an alternative device such as one that will be used for printing or
// saving a map view as an image file.
void QgsMapCanvas::render(QPaintDevice * theQPaintDevice)
{
  // If this is the first time that we are rendering since the output
  // projection has changed, transform the current extent from the old
  // output projection to the new output projection.

  if (layerCount() > 0)
  {
    // Get a map layer. Any layer should do, so just grab the first one.
    std::map<QString, QgsMapLayer*>::const_iterator 
     i = mCanvasProperties->layers.begin();

    if ( ! i->second->coordinateTransform() )
    {
        QgsDebug( "NO COORDINATE TRANSFORM FOUND FOR LAYER" );

        return;                 // XXX KLUDGY HACK, but necessary to underscore problem
                                // XXX with layers NOT HAVING DEFAULT COORDINATE TRANSFORMS
    }


    const QgsSpatialRefSys& currentOutputSRS = 
      i->second->coordinateTransform()->destSRS();

    if (!(mCanvasProperties->previousOutputSRS == currentOutputSRS))
    {
#ifdef QGISDEBUG
      std::cerr << "The previous output projection is different to the "
                << "current output projection, so the map extents are "
                << "begin reprojected.\n";
#endif

      QgsCoordinateTransform transform(mCanvasProperties->previousOutputSRS, currentOutputSRS);
      
      try
      {
        mCanvasProperties->currentExtent = 
          transform.transformBoundingBox(mCanvasProperties->currentExtent);
      }
      catch(QgsCsException &cse)
      {
        qWarning(tr("Error when projecting the view extent, you may need to manually zoom to the region of interest.").local8Bit());
#ifdef QGISDEBUG
        std::cerr << "Attempted to transform the view extent from\n"
                  << mCanvasProperties->previousOutputSRS 
                  << "\nto\n" << currentOutputSRS;
        std::cerr << "The view extent was: " 
                  << mCanvasProperties->currentExtent
                  << '\n';
#endif
      }

      mCanvasProperties->previousOutputSRS = currentOutputSRS;
    }
#ifdef QGISDEBUG
    else
      std::cerr << "The previous output projection is the same as the "
                << "current output projection, so the map extents are "
                << "not begin reprojected.\n";
#endif
  }

  // Don't allow zooms where the current extent is so small that it
  // can't be accurately represented using a double (which is what
  // currentExtent uses). Excluding 0 avoids a divide by zero and an
  // infinite loop when rendering to a new canvas. Excluding extents
  // greater than 1 avoids doing unnecessary calculations.

  // The scheme is to compare the width against the mean x coordinate
  // (and height against mean y coordinate) and only allow zooms where
  // the ratio indicates that there is more than about 12 significant
  // figures (there are about 16 significant figures in a double).

  if (mCanvasProperties->currentExtent.width()  > 0 &&
      mCanvasProperties->currentExtent.height() > 0 && 
      mCanvasProperties->currentExtent.width()  < 1 &&
      mCanvasProperties->currentExtent.height() < 1)
  {
    QgsRect extent = mCanvasProperties->currentExtent;

    // Use abs() on the extent to avoid the case where the extent is
    // symmetrical about 0.
    double xMean = (std::abs(extent.xMin()) + std::abs(extent.xMax())) * 0.5;
    double yMean = (std::abs(extent.yMin()) + std::abs(extent.yMax())) * 0.5;

    double xRange = extent.width() / xMean;
    double yRange = extent.height() / yMean;

    static const double minProportion = 1e-12;
    if (xRange < minProportion || yRange < minProportion)
    {
      // Go back to the previous extent
      mCanvasProperties->currentExtent = 
  mCanvasProperties->previousExtent; 
      repaint();
      return;
    }
  }

#ifdef QGISDEBUG
  QString msg = mCanvasProperties->frozen ? "frozen" : "thawed";
  std::cout << ".............................." << std::endl;
  std::cout << "...........Rendering.........." << std::endl;
  std::cout << ".............................." << std::endl;
  std::cout << name() << " canvas is " << msg.local8Bit() << std::endl;
#endif

  int myHeight=0;
  int myWidth=0;
  if ((!mCanvasProperties->frozen && mCanvasProperties->dirty) ||
      theQPaintDevice)
  {
    if (!mCanvasProperties->drawing)
    {
      mCanvasProperties->drawing = true;
      QPainter *paint = new QPainter();

      //default to pmCanvas if no paintdevice is supplied
      if ( ! theQPaintDevice )  //painting to mapCanvas->pixmap
      {
        mCanvasProperties->pmCanvas->fill(mCanvasProperties->bgColor);
        paint->begin(mCanvasProperties->pmCanvas);
        myHeight=height();
        myWidth=width();
      }
      else  //painting to an arbitary pixmap passed to render()
      {
        // need width/height of paint device
        QPaintDeviceMetrics myMetrics( theQPaintDevice );
        myHeight = myMetrics.height();
        myWidth = myMetrics.width();
        //fall back to widget height & width if retrieving them from passed in canvas fails
        if (myHeight==0)
        {
          myHeight=height();
        }
        if (myWidth==0)
        {
          myWidth=width();
        }
        //initialise the painter
        paint->begin(theQPaintDevice);
      }
      // hardwire the current extent for projection testing
      //     mCanvasProperties->currentExtent.setXmin(-156.00);
      //     mCanvasProperties->currentExtent.setXmax(-152.00);
      //     mCanvasProperties->currentExtent.setYmin(55.00);
      //     mCanvasProperties->currentExtent.setYmax(60.00);

      // calculate the translation and scaling parameters
      // mupp = map units per pixel
      double muppX, muppY;
      muppY = mCanvasProperties->currentExtent.height() / myHeight;
      muppX = mCanvasProperties->currentExtent.width() / myWidth;
      mCanvasProperties->m_mupp = muppY > muppX ? muppY : muppX;

      // calculate the actual extent of the mapCanvas
      double dxmin, dxmax, dymin, dymax, whitespace;

      if (muppY > muppX)
      {
        dymin = mCanvasProperties->currentExtent.yMin();
        dymax = mCanvasProperties->currentExtent.yMax();
        whitespace = ((myWidth * mCanvasProperties->m_mupp) - 
            mCanvasProperties->currentExtent.width()) / 2;
        dxmin = mCanvasProperties->currentExtent.xMin() - whitespace;
        dxmax = mCanvasProperties->currentExtent.xMax() + whitespace;
      }
      else
      {
        dxmin = mCanvasProperties->currentExtent.xMin();
        dxmax = mCanvasProperties->currentExtent.xMax();
        whitespace = ((myHeight * mCanvasProperties->m_mupp) - 
            mCanvasProperties->currentExtent.height()) / 2;
        dymin = mCanvasProperties->currentExtent.yMin() - whitespace;
        dymax = mCanvasProperties->currentExtent.yMax() + whitespace;
      }

      //update the scale shown on the statusbar
      // XXX This function seems to change the mCanvasProperties->m_mupp -
      // is that really intended? We've just gone through a lot of trouble
      // to calculate it in _this_ function. Anyway, we can't let it change
      // the mupp if we're rendering to an alternative device (it uses the
      // widget size in its calculations).
      if (!theQPaintDevice)
        currentScale(0);

#ifdef QGISDEBUG

      std::cout
        << "Paint device width : " << myWidth << std::endl
        << "Paint device height : " << myHeight << std::endl
        << "Canvas current extent height : " << mCanvasProperties->currentExtent.height()  << std::endl
        << "Canvas current extent width : " << mCanvasProperties->currentExtent.width()  << std::endl
        << "muppY: " << muppY << std::endl
        << "muppX: " << muppX << std::endl
        << "dxmin: " << dxmin << std::endl
        << "dxmax: " << dxmax << std::endl
        << "dymin: " << dymin << std::endl
        << "dymax: " << dymax << std::endl
        << "whitespace: " << whitespace << std::endl;
#endif

      mCanvasProperties->coordXForm->setParameters(mCanvasProperties->m_mupp, dxmin, dymin, myHeight);
      //currentExtent.xMin(),      currentExtent.yMin(), currentExtent.yMax());
      // update the currentExtent to match the device coordinates
      //GS - removed the current extent update to fix bug --
      //TODO remove the next 4 lines after we're sure this works ok

      // TS - Update : We want these 4 lines because the device coordinates may be
      // the printing device or something unrelated to the actual mapcanvas pixmap.
      // However commenting it it out causes map not to scale when QGIS  window is resized

      mCanvasProperties->currentExtent.setXmin(dxmin);
      mCanvasProperties->currentExtent.setXmax(dxmax);
      mCanvasProperties->currentExtent.setYmin(dymin);
      mCanvasProperties->currentExtent.setYmax(dymax);
      int myRenderCounter=1;
      //bail out if user has requested rendering to be suppressed (usually in statusbar checkbox in gui
      if (!mRenderFlag)
      {
        // do nothing but continue processing after main render loop
        // so all render complete etc events will
        // still get fired
      }
      else
      {
        // render all layers in the stack, starting at the base
        std::list < QString >::iterator li = mCanvasProperties->zOrder.begin();
        // std::cout << "MAP LAYER COUNT: " << layers.size() << std::endl;
        while (li != mCanvasProperties->zOrder.end())
        {
          emit setProgress(myRenderCounter++,mCanvasProperties->zOrder.size());
          QgsMapLayer *ml = mCanvasProperties->layers[*li];

          if (ml)
          {
            //    QgsDatabaseLayer *dbl = (QgsDatabaseLayer *)&ml;
#ifdef QGISDEBUG
            std::cout << "QgsMapCanvas::render: Rendering layer " << ml->name().local8Bit() << '\n'
              << "Layer minscale " << ml->minScale() 
              << ", maxscale " << ml->maxScale() << '\n' 
              << ". Scale dep. visibility enabled? " 
              << ml->scaleBasedVisibility() << '\n'
              << "Input extent: " << ml->extent().stringRep().local8Bit() 
              << std::endl;
            try
            {
              std::cout << "Transformed extent" 
                << ml->coordinateTransform()->transformBoundingBox(ml->extent()).stringRep().local8Bit() 
                << std::endl;
            }
            catch (QgsCsException &e)
            {
              qDebug( "Transform error caught in %s line %d:\n%s", 
                  __FILE__, __LINE__, e.what());
            }
#endif

            if (ml->visible())
            {
              if ((ml->scaleBasedVisibility() && 
                    ml->minScale() < mCanvasProperties->mScale 
                    && ml->maxScale() > mCanvasProperties->mScale)
                  || (!ml->scaleBasedVisibility()))
              {
                QgsRect r1 = mCanvasProperties->currentExtent, r2;
                bool split = ml->projectExtent(r1, r2);
                //
                // Now do the call to the layer that actually does
                // the rendering work!
                //
                ml->draw(paint, &r1, mCanvasProperties->coordXForm, this);
                if (split)
                  ml->draw(paint, &r2,mCanvasProperties->coordXForm, this);
              }
#ifdef QGISDEBUG
              else
              {
                std::cout << "Layer not rendered because it is not within "
                  << "the defined visibility scale range" << std::endl;
              }
#endif

            }
            li++;
          }
        }
#ifdef QGISDEBUG
        std::cout << "Done rendering map layers...emitting renderComplete(paint)\n";
#endif

        // render all labels for vector layers in the stack, starting at the base
        //first check that this is not an overview canvas ( and suppress labels if it is)
        if (!mIsOverviewCanvas)
        {
          li = mCanvasProperties->zOrder.begin();
          // std::cout << "MAP LAYER COUNT: " << layers.size() << std::endl;
          while (li != mCanvasProperties->zOrder.end())
          {
            emit setProgress((myRenderCounter++),mCanvasProperties->zOrder.size());
            QgsMapLayer *ml = mCanvasProperties->layers[*li];

            if (ml)
            {
#ifdef QGISDEBUG
#endif

              if (ml->visible() && (ml->type() != QgsMapLayer::RASTER))
              {
                //only make labels if the layer is visible
                //after scale dep viewing settings are checked
                if ((ml->scaleBasedVisibility() && 
                      ml->minScale() < mCanvasProperties->mScale 
                      && ml->maxScale() > mCanvasProperties->mScale)
                    || (!ml->scaleBasedVisibility()))
                {
                  QgsRect r1 = mCanvasProperties->currentExtent, r2;
                  bool split = ml->projectExtent(r1, r2);

                  ml->drawLabels(paint, &r1, mCanvasProperties->coordXForm, 
                      this);
                  if (split)
                    ml->drawLabels(paint, &r2, mCanvasProperties->coordXForm, 
                        this);
                }
              }
              li++;
            }
          }
        }
      }
      //make verys sure progress bar arrives at 100%!
      emit setProgress(1,1);
#ifdef QGISDEBUG

      std::cout << "Done rendering map labels...emitting renderComplete(paint)\n";
#endif

      // draw the acetate layer
      std::map <QString, QgsAcetateObject *>::iterator 
        ai = mCanvasProperties->acetateObjects.begin();
      while(ai != mCanvasProperties->acetateObjects.end())
      {
        QgsAcetateObject *acObj = ai->second;
        if(acObj)
        {
          try
          {

          acObj->draw(paint, mCanvasProperties->coordXForm);
          }
          catch(QgsException &e)
          {
            std::cout << "Throwing exception " << __FILE__ << __LINE__ << std::endl; 
          }
        }
        ai++;
      }

      //draw mCaptureList with color and width stored in QgsProject
      QColor digitcolor(QgsProject::instance()->readNumEntry("Digitizing","/LineColorRedPart",255),
          QgsProject::instance()->readNumEntry("Digitizing","/LineColorGreenPart",0),
          QgsProject::instance()->readNumEntry("Digitizing","/LineColorBluePart",0));
      paint->setPen(QPen(digitcolor,QgsProject::instance()->readNumEntry("Digitizing","/LineWidth",1),Qt::SolidLine));
      if(mCaptureList.size() > 0)
      {

        std::list<QgsPoint>::iterator it=mCaptureList.begin();
        QgsPoint current;
        try
        {
          QgsPoint previous=mCanvasProperties->coordXForm->transform(it->x(), it->y());
          for(std::list<QgsPoint>::iterator it=++mCaptureList.begin();it!=mCaptureList.end();++it)
          {
            current=mCanvasProperties->coordXForm->transform(it->x(), it->y());
            paint->drawLine(static_cast<int>(previous.x()), static_cast<int>(previous.y()), static_cast<int>(current.x()),\
                static_cast<int>(current.y()));
            previous=mCanvasProperties->coordXForm->transform(it->x(), it->y());
          }
        }
        catch(QgsException &e)
        {
          // ignore this exception at present
          std::cout << "QgsException: " << __FILE__ << __LINE__ << std::endl; 
        }


        //and also the connection to mDigitMovePoint
        if((mLineEditing || mPolygonEditing) && mCaptureList.size()>0)
        {
          paint->setRasterOp(Qt::XorROP);
          try
          {
            QgsPoint digitpoint=mCanvasProperties->coordXForm->transform(mDigitMovePoint.x(), mDigitMovePoint.y());
            paint->drawLine(static_cast<int>(current.x()), static_cast<int>(current.y()), static_cast<int>(digitpoint.x()),\
                static_cast<int>(digitpoint.y()));
            if(mPolygonEditing && mCaptureList.size()>1)
            {
              QgsPoint first=mCanvasProperties->coordXForm->transform(*(mCaptureList.begin()));
              paint->drawLine(static_cast<int>(first.x()), static_cast<int>(first.y()), static_cast<int>(digitpoint.x()),\
                  static_cast<int>(digitpoint.y()));
            }
          }


          catch(QgsException &e)
          {
            // ignore this for now
            // we need this to keep windows from wanted to send
            // a bug report to Bill
          }
        }
      }
				      

      // notify any listeners that rendering is complete
      //note that pmCanvas is not draw to gui yet
      emit renderComplete(paint);

      paint->end();
      mCanvasProperties->drawing = false;
      delete paint;
    }
    mCanvasProperties->dirty = false;
    repaint();
  }

} // render

// return the current coordinate transform based on the extents and
// device size
QgsMapToPixel * QgsMapCanvas::getCoordinateTransform()
{
  return mCanvasProperties->coordXForm;
}

void QgsMapCanvas::currentScale(int thePrecision)
{
  // calculate the translation and scaling parameters
  double muppX, muppY;
  muppY = mCanvasProperties->currentExtent.height() / height();
  muppX = mCanvasProperties->currentExtent.width() / width();
  mCanvasProperties->m_mupp = muppY > muppX ? muppY : muppX;
#ifdef QGISDEBUG

  std::cout << "------------------------------------------ " << std::endl;
  std::cout << "----------   Current Scale --------------- " << std::endl;
  std::cout << "------------------------------------------ " << std::endl;

  std::cout << "Current extent is " 
      << mCanvasProperties->currentExtent.stringRep().local8Bit() << std::endl;
  std::cout << "MuppX is: " << muppX << "\n"
      << "MuppY is: " << muppY << std::endl;
  std::cout << "Canvas width: " << width() 
      << ", height: " << height() << std::endl;
  std::cout << "Extent width: " 
      << mCanvasProperties->currentExtent.width() << ", height: "
      << mCanvasProperties->currentExtent.height() << std::endl;

  QPaintDeviceMetrics pdm(this);
  std::cout << "dpiX " << pdm.logicalDpiX() 
      << ", dpiY " << pdm.logicalDpiY() << std::endl;
  std::cout << "widthMM " << pdm.widthMM() 
      << ", heightMM " << pdm.heightMM() << std::endl;

#endif
  // calculate the actual extent of the mapCanvas
  double dxmin, dxmax, dymin, dymax, whitespace;
  if (muppY > muppX)
  {
    dymin = mCanvasProperties->currentExtent.yMin();
    dymax = mCanvasProperties->currentExtent.yMax();
    whitespace = ((width() * mCanvasProperties->m_mupp) - mCanvasProperties->currentExtent.width()) / 2;
    dxmin = mCanvasProperties->currentExtent.xMin() - whitespace;
    dxmax = mCanvasProperties->currentExtent.xMax() + whitespace;
  }
  else
  {
    dxmin = mCanvasProperties->currentExtent.xMin();
    dxmax = mCanvasProperties->currentExtent.xMax();
    whitespace = ((height() * mCanvasProperties->m_mupp) - mCanvasProperties->currentExtent.height()) / 2;
    dymin = mCanvasProperties->currentExtent.yMin() - whitespace;
    dymax = mCanvasProperties->currentExtent.yMax() + whitespace;

  }
  QgsRect paddedExtent(dxmin, dymin, dxmax, dymax);
  mCanvasProperties->mScale = mCanvasProperties->scaleCalculator->calculate(paddedExtent, width());
#ifdef QGISDEBUG

  std::cout << "Scale (assuming meters as map units) = 1:"
      << mCanvasProperties->mScale << std::endl;
#endif
  // return scale based on geographic
  //

  //@todo return a proper value
  QString myScaleString("Scale ");
  
  if (mCanvasProperties->mScale == 0)
    myScaleString = "";
  else if (mCanvasProperties->mScale >= 1)
    myScaleString += QString("1: ")
      + QString::number(mCanvasProperties->mScale,'f',thePrecision);
  else
    myScaleString += QString::number(1.0/mCanvasProperties->mScale, 'f', thePrecision)
      + QString(": 1");

  emit scaleChanged(myScaleString) ;
#ifdef QGISDEBUG

  std::cout << "------------------------------------------ " << std::endl;
#endif
}



//the format defaults to "PNG" if not specified
void QgsMapCanvas::saveAsImage(QString theFileName, QPixmap * theQPixmap, QString theFormat)
{
  //
  //check if the optional QPaintDevice was supplied
  //
  if (theQPixmap != NULL)
  {
    render(theQPixmap);
    theQPixmap->save(theFileName,theFormat.local8Bit());
  }
  else //use the map view
  {
    mCanvasProperties->pmCanvas->save(theFileName,theFormat.local8Bit());
  }
} // saveAsImage



void QgsMapCanvas::paintEvent(QPaintEvent * ev)
{
  if (!mCanvasProperties->dirty)
  {
    // just bit blit the image to the canvas
    bitBlt(this, ev->rect().topLeft(), mCanvasProperties->pmCanvas, ev->rect());
  }
  else
  {
    if (!mCanvasProperties->drawing)
    {
      render();
    }
  }
} // paintEvent



QgsRect const & QgsMapCanvas::extent() const
{
  return mCanvasProperties->currentExtent;
} // extent

QgsRect const & QgsMapCanvas::fullExtent() const
{
  return mCanvasProperties->fullExtent;
} // extent

void QgsMapCanvas::setExtent(QgsRect const & r)
{
  mCanvasProperties->currentExtent = r;
  emit extentsChanged(r);
} // setExtent


void QgsMapCanvas::clear()
{
  mCanvasProperties->dirty = true;
  erase();
} // clear



void QgsMapCanvas::zoomFullExtent()
{
  mCanvasProperties->previousExtent = mCanvasProperties->currentExtent;
  mCanvasProperties->currentExtent = mCanvasProperties->fullExtent;

  clear();
  render();
  emit extentsChanged(mCanvasProperties->currentExtent);
} // zoomFullExtent



void QgsMapCanvas::zoomPreviousExtent()
{
  if (mCanvasProperties->previousExtent.width() > 0)
  {
    QgsRect tempRect = mCanvasProperties->currentExtent;
    mCanvasProperties->currentExtent = mCanvasProperties->previousExtent;
    mCanvasProperties->previousExtent = tempRect;
    clear();
    render();
    emit extentsChanged(mCanvasProperties->currentExtent);
  }
} // zoomPreviousExtent

bool QgsMapCanvas::projectionsEnabled()
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

void QgsMapCanvas::zoomToSelected()
{
  QgsVectorLayer *lyr =
    dynamic_cast < QgsVectorLayer * >(mCanvasProperties->mapLegend->currentLayer());

  if (lyr)
  {


    QgsRect rect ;
    if (projectionsEnabled())
    {
      try
      {      
        if ( ! lyr->coordinateTransform() )
        {
#ifdef QGISDEBUG 
            std::cout << "Throwing exception "<< __FILE__ << __LINE__ << std::endl; 
#endif
            throw QgsCsException( string("NO COORDINATE TRANSFORM FOUND FOR LAYER") );
        }

        rect = lyr->coordinateTransform()->transformBoundingBox(lyr->bBoxOfSelected());
      }
      catch (QgsCsException &e)
      {
        qDebug( "Transform error caught in %s line %d:\n%s", __FILE__, __LINE__, e.what());
      }
    }
    else
    {
      rect = lyr->bBoxOfSelected();
    }

    // no selected features, only one selected point feature 
    //or two point features with the same x- or y-coordinates
    if(rect.isEmpty())
    {
  return;
    }
    //zoom to an area
    else
    {
      mCanvasProperties->previousExtent = mCanvasProperties->currentExtent;
      mCanvasProperties->currentExtent.setXmin(rect.xMin());
      mCanvasProperties->currentExtent.setYmin(rect.yMin());
      mCanvasProperties->currentExtent.setXmax(rect.xMax());
      mCanvasProperties->currentExtent.setYmax(rect.yMax());
      emit extentsChanged(mCanvasProperties->currentExtent);
      clear();
      render();
      return;
    }
  }
} // zoomToSelected

void QgsMapCanvas::keyPressEvent(QKeyEvent * e)
{

#ifdef QGISDEBUG
  qDebug("keyPress event at line %d in %s",  __LINE__, __FILE__);
#endif
  
  if (!mUserInteractionAllowed || mCanvasProperties->mouseButtonDown
      || mCanvasProperties->panSelectorDown)
    return;

  QPainter paint;
  QPen     pen(Qt::gray);
  QgsPoint ll, ur;

  if (! mCanvasProperties->mouseButtonDown )
  {
    // Don't want to interfer with mouse events

    double dx = fabs((mCanvasProperties->currentExtent.xMax()- mCanvasProperties->currentExtent.xMin()) / 4);
    double dy = fabs((mCanvasProperties->currentExtent.yMax()- mCanvasProperties->currentExtent.yMin()) / 4);

    switch ( e->key() ) 
    {
    case Qt::Key_Left:
#ifdef QGISDEBUG
	    std::cout << "Pan left" << std::endl;
#endif

	    mCanvasProperties->previousExtent = mCanvasProperties->currentExtent;

	    mCanvasProperties->currentExtent.setXmin(mCanvasProperties->currentExtent.xMin() - dx);
	    mCanvasProperties->currentExtent.setXmax(mCanvasProperties->currentExtent.xMax() - dx);
	    
	    clear();
	    render();
	    emit extentsChanged(mCanvasProperties->currentExtent);
	    break;

    case Qt::Key_Right:
#ifdef QGISDEBUG
	    std::cout << "Pan right" << std::endl;
#endif

	    mCanvasProperties->previousExtent = mCanvasProperties->currentExtent;

	    mCanvasProperties->currentExtent.setXmin(mCanvasProperties->currentExtent.xMin() + dx);
	    mCanvasProperties->currentExtent.setXmax(mCanvasProperties->currentExtent.xMax() + dx);

	    clear();
	    render();
	    emit extentsChanged(mCanvasProperties->currentExtent);
	    break;

    case Qt::Key_Up:
#ifdef QGISDEBUG
	    std::cout << "Pan up" << std::endl;
#endif

	    mCanvasProperties->previousExtent = mCanvasProperties->currentExtent;

	    mCanvasProperties->currentExtent.setYmax(mCanvasProperties->currentExtent.yMax() + dy);
	    mCanvasProperties->currentExtent.setYmin(mCanvasProperties->currentExtent.yMin() + dy);

	    clear();
	    render();
	    emit extentsChanged(mCanvasProperties->currentExtent);
	    break;

    case Qt::Key_Down:
#ifdef QGISDEBUG
	    std::cout << "Pan down" << std::endl;
#endif
	    mCanvasProperties->previousExtent = mCanvasProperties->currentExtent;

	    mCanvasProperties->currentExtent.setYmax(mCanvasProperties->currentExtent.yMax() - dy);
	    mCanvasProperties->currentExtent.setYmin(mCanvasProperties->currentExtent.yMin() - dy);

	    clear();
	    render();
	    emit extentsChanged(mCanvasProperties->currentExtent);
	    break;

    case Qt::Key_Space:
#ifdef QGISDEBUG
	    std::cout << "Pressing pan selector" << std::endl;
#endif
	    //mCanvasProperties->dragging = true;
	    if ( ! e->isAutoRepeat() )
	    {
	      mCanvasProperties->panSelectorDown = true;
	      mCanvasProperties->boxStartPoint = mCanvasProperties->mouseLastXY;
	    }
	    break;

    default:
	    // Pass it on
	    e->ignore();
#ifdef QGISDEBUG
	    qDebug("Ignoring key (%d)", e->key());
#endif
	      

    }
  }
} //keyPressEvent()

void QgsMapCanvas::keyReleaseEvent(QKeyEvent * e)
{

#ifdef QGISDEBUG
  qDebug("keyRelease event at line %d in %s",  __LINE__, __FILE__);
#endif
  
  if (!mUserInteractionAllowed)
    return;
  switch( e->key() )
  {
    case Qt::Key_Space:
      if ( !e->isAutoRepeat() && mCanvasProperties->panSelectorDown)
      {
#ifdef QGISDEBUG
	std::cout << "Releaseing pan selector" << std::endl;
#endif
      // mCanvasProperties->dragging = false;
	mCanvasProperties->panSelectorDown = false;
	panActionEnd(mCanvasProperties->mouseLastXY);
      }
      break;

    default:
      // Pass it on
      e->ignore();
#ifdef QGISDEBUG
      qDebug("Ignoring key release (%d)", e->key());
#endif
  }
} //keyReleaseEvent()

void QgsMapCanvas::mousePressEvent(QMouseEvent * e)
{
  if (!mUserInteractionAllowed || mCanvasProperties->panSelectorDown)
    return;

  // right button was pressed in zoom tool, return to previous non zoom tool
  if ( e->button() == Qt::RightButton &&
       ( mCanvasProperties->mapTool == QGis::ZoomIn || mCanvasProperties->mapTool == QGis::ZoomOut
   || mCanvasProperties->mapTool == QGis::Pan ) )
  {
      //emit stopZoom();
      return;
  }
  
  mCanvasProperties->mouseButtonDown = true;
  mCanvasProperties->boxStartPoint = e->pos();

  switch (mCanvasProperties->mapTool)
  {
  case QGis::Select:
  case QGis::ZoomIn:
  case QGis::ZoomOut:
    mCanvasProperties->zoomBox.setRect(0, 0, 0, 0);
    break;
  case QGis::Distance:
    //              distanceEndPoint = e->pos();
    break;
  case QGis::EmitPoint: 
    {
    QgsPoint  idPoint = mCanvasProperties->coordXForm->
      toMapCoordinates(e->x(), e->y());
    emit xyClickCoordinates(idPoint);
    emit xyClickCoordinates(idPoint,e->button());
    break;
    }
  }
} // mousePressEvent


void QgsMapCanvas::mouseReleaseEvent(QMouseEvent * e)
{

  mCanvasProperties->mouseButtonDown = false;

  if (!mUserInteractionAllowed || mCanvasProperties->panSelectorDown)
    return;

  // right button was pressed in zoom tool, return to previous non zoom tool
  if ( e->button() == Qt::RightButton &&
       ( mCanvasProperties->mapTool == QGis::ZoomIn || mCanvasProperties->mapTool == QGis::ZoomOut
   || mCanvasProperties->mapTool == QGis::Pan ) )
  {
      emit stopZoom();
      return;
  }

  QPainter paint;
  QPen     pen(Qt::gray);
  QgsPoint ll, ur;

  if (mCanvasProperties->dragging)
  {
    mCanvasProperties->dragging = false;

    switch (mCanvasProperties->mapTool)
    {
    case QGis::ZoomIn:
      // erase the rubber band box
      paint.begin(this);
      paint.setPen(pen);
      paint.setRasterOp(Qt::XorROP);
      paint.drawRect(mCanvasProperties->zoomBox);
      paint.end();
      // store the rectangle
      mCanvasProperties->zoomBox.setRight(e->pos().x());
      mCanvasProperties->zoomBox.setBottom(e->pos().y());
      // set the extent to the zoomBox

      ll = mCanvasProperties->coordXForm->toMapCoordinates(mCanvasProperties->zoomBox.left(), mCanvasProperties->zoomBox.bottom());
      ur = mCanvasProperties->coordXForm->toMapCoordinates(mCanvasProperties->zoomBox.right(), mCanvasProperties->zoomBox.top());
      mCanvasProperties->previousExtent = mCanvasProperties->currentExtent;
      //QgsRect newExtent(ll.x(), ll.y(), ur.x(), ur.y());
      mCanvasProperties->currentExtent.setXmin(ll.x());
      mCanvasProperties->currentExtent.setYmin(ll.y());
      mCanvasProperties->currentExtent.setXmax(ur.x());
      mCanvasProperties->currentExtent.setYmax(ur.y());
      mCanvasProperties->currentExtent.normalize();
      clear();
      render();
      emit extentsChanged(mCanvasProperties->currentExtent);

      break;
    case QGis::ZoomOut:
      {
        // erase the rubber band box
        paint.begin(this);
        paint.setPen(pen);
        paint.setRasterOp(Qt::XorROP);
        paint.drawRect(mCanvasProperties->zoomBox);
        paint.end();
        // store the rectangle
        mCanvasProperties->zoomBox.setRight(e->pos().x());
        mCanvasProperties->zoomBox.setBottom(e->pos().y());
        // scale the extent so the current view fits inside the zoomBox
        ll = mCanvasProperties->coordXForm->toMapCoordinates(mCanvasProperties->zoomBox.left(), mCanvasProperties->zoomBox.bottom());
        ur = mCanvasProperties->coordXForm->toMapCoordinates(mCanvasProperties->zoomBox.right(), mCanvasProperties->zoomBox.top());
        mCanvasProperties->previousExtent = mCanvasProperties->currentExtent;
        QgsRect tempRect = mCanvasProperties->currentExtent;
        mCanvasProperties->currentExtent.setXmin(ll.x());
        mCanvasProperties->currentExtent.setYmin(ll.y());
        mCanvasProperties->currentExtent.setXmax(ur.x());
        mCanvasProperties->currentExtent.setYmax(ur.y());
        mCanvasProperties->currentExtent.normalize();

        QgsPoint cer = mCanvasProperties->currentExtent.center();

        /* std::cout << "Current extent rectangle is " << tempRect << std::endl;
           std::cout << "Center of zoom out rectangle is " << cer << std::endl;
           std::cout << "Zoom out rectangle should have ll of " << ll << " and ur of " << ur << std::endl;
           std::cout << "Zoom out rectangle is " << mCanvasProperties->currentExtent << std::endl;
           */
        double sf;
        if (mCanvasProperties->zoomBox.width() > mCanvasProperties->zoomBox.height())
        {
          sf = tempRect.width() / mCanvasProperties->currentExtent.width();
        }
        else
        {
          sf = tempRect.height() / mCanvasProperties->currentExtent.height();
        }
        //center = new QgsPoint(zoomRect->center());
        //  delete zoomRect;
        mCanvasProperties->currentExtent.expand(sf);
#ifdef QGISDEBUG

        std::cout << "Extent scaled by " << sf << " to " << mCanvasProperties->currentExtent << std::endl;
        std::cout << "Center of currentExtent after scaling is " << mCanvasProperties->currentExtent.center() << std::endl;
#endif

        clear();
        render();
        emit extentsChanged(mCanvasProperties->currentExtent);
      }
      break;

    case QGis::Pan:
      panActionEnd(e->pos());
      break;
      
    case QGis::Select:
      {
      // erase the rubber band box
      paint.begin(this);
      paint.setPen(pen);
      paint.setRasterOp(Qt::XorROP);
      paint.drawRect(mCanvasProperties->zoomBox);
      paint.end();

      QgsMapLayer *lyr = mCanvasProperties->mapLegend->currentLayer();

      if (lyr)
      {
        QgsPoint ll, ur;

        // store the rectangle
        mCanvasProperties->zoomBox.setRight(e->pos().x());
        mCanvasProperties->zoomBox.setBottom(e->pos().y());

        ll = mCanvasProperties->coordXForm->toMapCoordinates(mCanvasProperties->zoomBox.left(), mCanvasProperties->zoomBox.bottom());
        ur = mCanvasProperties->coordXForm->toMapCoordinates(mCanvasProperties->zoomBox.right(), mCanvasProperties->zoomBox.top());

        QgsRect *search = new QgsRect(ll.x(), ll.y(), ur.x(), ur.y());

        if (e->state() == 513) // XXX 513? Magic numbers bad!
        {
          lyr->select(search, true);
        } else
        {
          lyr->select(search, false);
        }
        delete search;
      }
      else
      {
        QMessageBox::warning(this,
                             tr("No active layer"),
                             tr("To select features, you must choose an layer active by clicking on its name in the legend"));
      }
      }
      break;
    }
  }
  else
  {
    // map tools that rely on a click not a drag
    switch (mCanvasProperties->mapTool)
    {
    case QGis::Identify:
      {
        // call identify method for selected layer
        QgsMapLayer * lyr = mCanvasProperties->mapLegend->currentLayer();

        if (lyr)
        {

          // create the search rectangle
          double searchRadius = extent().width() * calculateSearchRadiusValue();
          QgsRect * search = new QgsRect();
          // convert screen coordinates to map coordinates
          QgsPoint idPoint = mCanvasProperties->coordXForm->toMapCoordinates(e->x(), e->y());
          search->setXmin(idPoint.x() - searchRadius);
          search->setXmax(idPoint.x() + searchRadius);
          search->setYmin(idPoint.y() - searchRadius);
          search->setYmax(idPoint.y() + searchRadius);

          lyr->identify(search);

          delete search;
        }
        else
        {
          QMessageBox::warning(this,
                               tr("No active layer"),
                               tr("To identify features, you must choose an layer active by clicking on its name in the legend"));
        }
      }
      break;

    case QGis::CapturePoint:
      {
        QgsVectorLayer* vlayer=
          dynamic_cast<QgsVectorLayer*>(mCanvasProperties->mapLegend->currentLayer());

        if(vlayer)
        {

      QgsPoint  idPoint = mCanvasProperties->coordXForm->toMapCoordinates(e->x(), e->y());
      emit xyClickCoordinates(idPoint);
      
      //only do the rest for provider with feature addition support
      //note that for the grass provider, this will return false since
      //grass provider has its own mechanism of feature addition
      if(vlayer->getDataProvider()->capabilities()&QgsVectorDataProvider::AddFeatures)
      {
    if(!vlayer->isEditable() )
    {
        QMessageBox::information(0,"Layer not editable","Cannot edit the vector layer. Use 'Start editing' in the legend item menu",QMessageBox::Ok);
        break;
    }

    //snap point to points within the vector layer snapping tolerance
    vlayer->snapPoint(idPoint,QgsProject::instance()->readDoubleEntry("Digitizing","/Tolerance",0));

    QgsFeature* f = new QgsFeature(0,"WKBPoint");
    int size=5+2*sizeof(double);
    unsigned char *wkb = new unsigned char[size];
    int wkbtype=QGis::WKBPoint;
          QgsPoint savePoint = maybeInversePoint(idPoint, "adding point");
          double x = savePoint.x();
          double y = savePoint.y();

    memcpy(&wkb[1],&wkbtype, sizeof(int));
    memcpy(&wkb[5], &x, sizeof(double));
    memcpy(&wkb[5]+sizeof(double), &y, sizeof(double));
    f->setGeometry(&wkb[0],size);

    //add the fields to the QgsFeature
    std::vector<QgsField> fields=vlayer->fields();
    for(std::vector<QgsField>::iterator it=fields.begin();it!=fields.end();++it)
    {
        f->addAttribute((*it).name(), vlayer->getDefaultValue(it->name(),f));
    }

    //show the dialog to enter attribute values
    if(f->attributeDialog())
    {
        vlayer->addFeature(f);
    }
    refresh();
      }
  }
  else
  {
      QMessageBox::information(0,"Not a vector layer","The current layer is not a vector layer",QMessageBox::Ok);
  }

      break;
      }

    case QGis::CaptureLine:
    case QGis::CapturePolygon:
      {
  QgsVectorLayer* vlayer=dynamic_cast<QgsVectorLayer*>(mCanvasProperties->mapLegend->currentLayer());
  
  if(vlayer)
    {
      if(!vlayer->isEditable())// && (vlayer->providerType().lower() != "grass"))
        {
    QMessageBox::information(0,"Layer not editable","Cannot edit the vector layer. Use 'Start editing' in the legend item menu",QMessageBox::Ok);
    break;
        }
    }
  else
    {
      QMessageBox::information(0,"Not a vector layer","The current layer is not a vector layer",QMessageBox::Ok);
      return;
    }
  
  //clear the line to the location of the digitising cursor
  if(mCaptureList.size()>0)
  {
      QPainter paint(mCanvasProperties->pmCanvas);
      drawLineToDigitisingCursor(&paint);
  }
  mDigitMovePoint.setX(e->x());
  mDigitMovePoint.setY(e->y());
  mDigitMovePoint=mCanvasProperties->coordXForm->toMapCoordinates(e->x(), e->y());

  QgsPoint digitisedpoint=mCanvasProperties->coordXForm->toMapCoordinates(e->x(), e->y());
  vlayer->snapPoint(digitisedpoint,QgsProject::instance()->readDoubleEntry("Digitizing","/Tolerance",0));
  mCaptureList.push_back(digitisedpoint);
  if(mCaptureList.size()>1)
    {
      QPainter paint(mCanvasProperties->pmCanvas);
      QColor digitcolor(QgsProject::instance()->readNumEntry("Digitizing","/LineColorRedPart",255),
            QgsProject::instance()->readNumEntry("Digitizing","/LineColorGreenPart",0),
            QgsProject::instance()->readNumEntry("Digitizing","/LineColorBluePart",0));
      paint.setPen(QPen(digitcolor,QgsProject::instance()->readNumEntry("Digitizing","/LineWidth",1),Qt::SolidLine));
      std::list<QgsPoint>::iterator it=mCaptureList.end();
      --it;
      --it;
    try
	{
      QgsPoint lastpoint = mCanvasProperties->coordXForm->transform(it->x(),it->y());
      QgsPoint endpoint = mCanvasProperties->coordXForm->transform(digitisedpoint.x(),digitisedpoint.y());
	
	
	paint.drawLine(static_cast<int>(lastpoint.x()),static_cast<int>(lastpoint.y()),
      static_cast<int>(endpoint.x()),static_cast<int>(endpoint.y()));
	}
	catch(QgsException &e)
	{
		// ignore this 
		// we need it to keep windows quiet
	}
      repaint();
    }
  if(e->button()==Qt::RightButton)
    {
      //create QgsFeature with wkb representation
      QgsFeature* f=new QgsFeature(0,"WKBLineString");
      unsigned char* wkb;
      int size;
      if(mCanvasProperties->mapTool==QGis::CaptureLine)
        {
    size=1+2*sizeof(int)+2*mCaptureList.size()*sizeof(double);
    wkb= new unsigned char[size];
    int wkbtype=QGis::WKBLineString;
    int length=mCaptureList.size();
    memcpy(&wkb[1],&wkbtype, sizeof(int));
    memcpy(&wkb[5],&length, sizeof(int));
    int position=1+2*sizeof(int);
    double x,y;
    for(std::list<QgsPoint>::iterator it=mCaptureList.begin();it!=mCaptureList.end();++it)
      {
            QgsPoint savePoint = maybeInversePoint(*it, "adding line");
            x = savePoint.x();
            y = savePoint.y();

        memcpy(&wkb[position],&x,sizeof(double));
        position+=sizeof(double);

        memcpy(&wkb[position],&y,sizeof(double));
        position+=sizeof(double);
      }
        }
      else//polygon
        {
    size=1+3*sizeof(int)+2*(mCaptureList.size()+1)*sizeof(double);
    wkb= new unsigned char[size];
    int wkbtype=QGis::WKBPolygon;
    int length=mCaptureList.size()+1;//+1 because the first point is needed twice
    int numrings=1;
    memcpy(&wkb[1],&wkbtype, sizeof(int));
    memcpy(&wkb[5],&numrings,sizeof(int));
    memcpy(&wkb[9],&length, sizeof(int));
    int position=1+3*sizeof(int);
    double x,y;
    std::list<QgsPoint>::iterator it;
    for(it=mCaptureList.begin();it!=mCaptureList.end();++it)
      {
            QgsPoint savePoint = maybeInversePoint(*it, "adding poylgon");
            x = savePoint.x();
            y = savePoint.y();

        memcpy(&wkb[position],&x,sizeof(double));
        position+=sizeof(double);

        memcpy(&wkb[position],&y,sizeof(double));
        position+=sizeof(double);
      }
    //close the polygon
    it=mCaptureList.begin();
          QgsPoint savePoint = maybeInversePoint(*it, "closing polygon");
          x = savePoint.x();
          y = savePoint.y();

    memcpy(&wkb[position],&x,sizeof(double));
    position+=sizeof(double);

    memcpy(&wkb[position],&y,sizeof(double));
        }
      f->setGeometry(&wkb[0],size);
      
      //add the fields to the QgsFeature
      std::vector<QgsField> fields=vlayer->fields();
      for(std::vector<QgsField>::iterator it=fields.begin();it!=fields.end();++it)
        {
    f->addAttribute((*it).name(),vlayer->getDefaultValue(it->name(), f));
        }
      
      if(f->attributeDialog())
      {
    vlayer->addFeature(f);
      }
      
      //delete the acetate objects and the elements of mCaptureList
      mCaptureList.clear();
      refresh();
      
    }
  break;
      }
      

    case QGis::Measure:
      {
        QgsPoint point = mCanvasProperties->coordXForm->toMapCoordinates(e->x(), e->y());

        if(e->button()==Qt::RightButton) // restart
	{
	     if ( mMeasure ) {   
		 mMeasure->restart();
	     }
	} 
	else 
	{
	  if ( !mMeasure ) {
	      mMeasure = new QgsMeasure(this, topLevelWidget() );
	  }
	  mMeasure->addPoint(point);
	  mMeasure->show();
	}
        break;
      }
    }
  }
} // mouseReleaseEvent

void QgsMapCanvas::resizeEvent(QResizeEvent * e)
{
  mCanvasProperties->dirty = true;
  mCanvasProperties->pmCanvas->resize(e->size());
  emit extentsChanged(mCanvasProperties->currentExtent);
} // resizeEvent

void QgsMapCanvas::wheelEvent(QWheelEvent *e)
{
  // Zoom the map canvas in response to a mouse wheel event. Moving the
  // wheel forward (away) from the user zooms in by a factor of 2.
  // TODO The scale factor needs to be customizable by the user.
#ifdef QGISDEBUG
  std::cout << "Wheel event delta " << e->delta() << std::endl;
#endif
  // change extent
  double scaleFactor;
  if(e->delta() > 0)
  {
    scaleFactor = .5;
  }
  else
  {
    scaleFactor = 2;
  }
  // transform the mouse pos to map coordinates
  QgsPoint center  = mCanvasProperties->coordXForm->toMapPoint(e->x(), e->y());
  mCanvasProperties->currentExtent.scale(scaleFactor,&center );
  clear();
  render();
  emit extentsChanged(mCanvasProperties->currentExtent);


}

void QgsMapCanvas::mouseMoveEvent(QMouseEvent * e)
{
  if (!mUserInteractionAllowed)
    return;

  mCanvasProperties->mouseLastXY = e->pos();

  if (mCanvasProperties->panSelectorDown)
  {
    panAction(e);
  }
  else if (e->state() == Qt::LeftButton || e->state() == 513)
  // XXX magic numbers BAD -- 513?
  {
    int dx, dy;
    QPainter paint;
    QPen pen(Qt::gray);

    // this is a drag-type operation (zoom, pan or other maptool)

    switch (mCanvasProperties->mapTool)
    {
    case QGis::Select:
    case QGis::ZoomIn:
    case QGis::ZoomOut:
      // draw the rubber band box as the user drags the mouse
      mCanvasProperties->dragging = true;

      paint.begin(this);
      paint.setPen(pen);
      paint.setRasterOp(Qt::XorROP);
      paint.drawRect(mCanvasProperties->zoomBox);

      mCanvasProperties->zoomBox.setLeft(mCanvasProperties->boxStartPoint.x());
      mCanvasProperties->zoomBox.setTop(mCanvasProperties->boxStartPoint.y());
      mCanvasProperties->zoomBox.setRight(e->pos().x());
      mCanvasProperties->zoomBox.setBottom(e->pos().y());

      paint.drawRect(mCanvasProperties->zoomBox);
      paint.end();
      break;
    case QGis::Pan:
      // show the pmCanvas as the user drags the mouse
      mCanvasProperties->dragging = true;
      panAction(e);

      break;

    }
  }
  else if ( mCanvasProperties->mapTool == QGis::Measure ) 
  {
      if ( mMeasure ) {
        QgsPoint point = mCanvasProperties->coordXForm->toMapCoordinates(e->pos().x(), e->pos().y());
	mMeasure->mouseMove(point);
      }
  }

  //draw a line to the cursor position in line/polygon editing mode
  if ( mCanvasProperties->mapTool == QGis::CaptureLine || mCanvasProperties->mapTool == QGis::CapturePolygon )
  {
	      if(mCaptureList.size()>0)
	      {
		  QPainter paint(mCanvasProperties->pmCanvas);
		  QPainter paint2(this);

		  drawLineToDigitisingCursor(&paint);
		  drawLineToDigitisingCursor(&paint2);
		  if(mCanvasProperties->mapTool == QGis::CapturePolygon && mCaptureList.size()>1)
		  {
		      drawLineToDigitisingCursor(&paint, false);
		      drawLineToDigitisingCursor(&paint2, false);
		  }
		  QgsPoint digitmovepoint(e->pos().x(), e->pos().y());
		  mDigitMovePoint=mCanvasProperties->coordXForm->toMapCoordinates(e->pos().x(), e->pos().y());

		  drawLineToDigitisingCursor(&paint);
		  drawLineToDigitisingCursor(&paint2);
		  if(mCanvasProperties->mapTool == QGis::CapturePolygon && mCaptureList.size()>1)
		  {
		      drawLineToDigitisingCursor(&paint, false);
		      drawLineToDigitisingCursor(&paint2, false);
		  }
	      }
  }

  // show x y on status bar
  QPoint xy = e->pos();
  QgsPoint coord = mCanvasProperties->coordXForm->toMapCoordinates(xy);
  emit xyCoordinates(coord);
} // mouseMoveEvent

void QgsMapCanvas::drawLineToDigitisingCursor(QPainter* paint, bool last)
{
   QColor digitcolor(QgsProject::instance()->readNumEntry("Digitizing","/LineColorRedPart",255),\
			    QgsProject::instance()->readNumEntry("Digitizing","/LineColorGreenPart",0),\
			    QgsProject::instance()->readNumEntry("Digitizing","/LineColorBluePart",0));
   paint->setPen(QPen(digitcolor,QgsProject::instance()->readNumEntry("Digitizing","/LineWidth",1),Qt::SolidLine));
   paint->setRasterOp(Qt::XorROP);
   std::list<QgsPoint>::iterator it;
   if(last)
   {
       it=mCaptureList.end();
       --it;
   }
   else
   {
       it=mCaptureList.begin();
   }
   QgsPoint lastpoint = mCanvasProperties->coordXForm->transform(it->x(),it->y());
   QgsPoint digitpoint = mCanvasProperties->coordXForm->transform(mDigitMovePoint.x(), mDigitMovePoint.y());
   paint->drawLine(static_cast<int>(lastpoint.x()),static_cast<int>(lastpoint.y()),\
		  static_cast<int>(digitpoint.x()), static_cast<int>(digitpoint.y()));
}

/** Sets the map tool currently being used on the canvas */
void QgsMapCanvas::setMapTool(int tool)
{
    mCanvasProperties->mapTool = tool;
    if ( tool == QGis::EmitPoint ) {
  setCursor ( Qt::CrossCursor );
    }
    if(tool == QGis::CapturePoint)
    {
	mLineEditing=false;
	mPolygonEditing=false;
    }
    else if (tool == QGis::CaptureLine)
    {
	mLineEditing=true;
	mPolygonEditing=false;
    }
    else if (tool == QGis::CapturePolygon)
    {
	mLineEditing=false;
	mPolygonEditing=true;
    }
} // setMapTool


/** Write property of QColor bgColor. */
void QgsMapCanvas::setbgColor(const QColor & _newVal)
{
  mCanvasProperties->bgColor = _newVal;
  setEraseColor(_newVal);
} // setbgColor


/** Updates the full extent to include the mbr of the rectangle r */
void QgsMapCanvas::updateFullExtent(QgsRect const & r)
{
  if (r.xMin() < mCanvasProperties->fullExtent.xMin())
  {
    mCanvasProperties->fullExtent.setXmin(r.xMin());
  }

  if (r.xMax() > mCanvasProperties->fullExtent.xMax())
  {
    mCanvasProperties->fullExtent.setXmax(r.xMax());
  }

  if (r.yMin() < mCanvasProperties->fullExtent.yMin())
  {
    mCanvasProperties->fullExtent.setYmin(r.yMin());
  }

  if (r.yMax() > mCanvasProperties->fullExtent.yMax())
  {
    mCanvasProperties->fullExtent.setYmax(r.yMax());
  }

  emit extentsChanged(mCanvasProperties->currentExtent);
} // updateFullExtent



/*const std::map<QString,QgsMapLayer *> * QgsMapCanvas::mapLayers(){
  return &layers;
  }
  */
int QgsMapCanvas::layerCount() const
{
  return mCanvasProperties->layers.size();
} // layerCount



void QgsMapCanvas::layerStateChange()
{
  if (!mCanvasProperties->frozen)
  {
    clear();
    render();
  }
} // layerStateChange



void QgsMapCanvas::freeze(bool frz)
{
  mCanvasProperties->frozen = frz;
} // freeze

bool QgsMapCanvas::isFrozen()
{
  return mCanvasProperties->frozen ;
} // freeze


void QgsMapCanvas::remove
  (QString key)
{
  // XXX As a safety precaution, shouldn't we check to see if the 'key' is
  // XXX in 'layers'?  Theoretically it should be ok to skip this check since
  // XXX this should always be called with correct keys.

  // We no longer delete the layer here - deleting of layers is now managed
  // by the MapLayerRegistry. All we do now is remove any local reference to this layer.
  // delete mCanvasProperties->layers[key];

  // first delete the map layer itself

  // convenience variable
  QgsMapLayer * layer = mCanvasProperties->layers[key];

  Q_ASSERT( layer );

  // disconnect layer signals
  QObject::disconnect(layer, SIGNAL(visibilityChanged()), this, SLOT(layerStateChange()));
  QObject::disconnect(layer, SIGNAL(repaintRequested()), this, SLOT(refresh()));

  // we DO NOT disconnect this as this is currently the only means for overview
  // canvases to add layers; natch this is irrelevent if this is NOT the overview canvas

  // QObject::disconnect(lyr, SIGNAL(showInOverView(QgsMapLayer *, bool)),
  //                     this, SLOT(showInOverView(QgsMapLayer *, bool )));

  mCanvasProperties->layers[key] = 0;
  mCanvasProperties->layers.erase( key );  // then erase its entry from layer table

  // XXX after removing this layer, we should probably compact the
  // XXX remaining Z values
  mCanvasProperties->zOrder.remove(key);   // ... and it's Z order entry, too

  // Since we removed a layer, we may have to adjust the map canvas'
  // over-all extent; so we update to the largest extent found in the
  // remaining layers.

  if ( mCanvasProperties->layers.empty() )
  {
    // XXX do we want to reset the extents if the last layer is deleted?
  }
  else
  {
    recalculateExtents();
  }

  mCanvasProperties->dirty = true;

  // signal that we've erased this layer
  emit removedLayer( key );

} // QgsMapCanvas::remove()



void QgsMapCanvas::removeAll()
{

  // Welllllll, yeah, this works, but now we have to ensure that the
  // removedLayer() signal is emitted.
  //   mCanvasProperties->layers.clear();
  //   mCanvasProperties->zOrder.clear();

  // So:
  std::map < QString, QgsMapLayer * >::iterator mi =
    mCanvasProperties->layers.begin();

  QString current_key;

  // first disconnnect all layer signals from this canvas
  while ( mi != mCanvasProperties->layers.end() )
  {
    // save the current key
    current_key = mi->first;

    QgsMapLayer * layer = mCanvasProperties->layers[current_key];

    // disconnect layer signals
    QObject::disconnect(layer, SIGNAL(visibilityChanged()), this, SLOT(layerStateChange()));
    QObject::disconnect(layer, SIGNAL(repaintRequested()), this, SLOT(refresh()));

    ++mi;
  }

  // then empty all the other state containers

  mCanvasProperties->layers.clear();

  mCanvasProperties->acetateObjects.clear(); // XXX are these managed elsewhere?

  mCanvasProperties->zOrder.clear();

  mCanvasProperties->dirty = true;

  emit removedAll();              // let observers know we're now empty

} // QgsMapCanvas::removeAll



/* Calculates the search radius for identifying features
 * using the radius value stored in the users settings
 */
double QgsMapCanvas::calculateSearchRadiusValue()
{
  QSettings settings;

  int identifyValue = settings.readNumEntry("/qgis/map/identifyRadius", QGis::DEFAULT_IDENTIFY_RADIUS);

  return(identifyValue/1000.0);

} // calculateSearchRadiusValue


QPixmap * QgsMapCanvas::canvasPixmap()
{
  return mCanvasProperties->pmCanvas;
} // canvasPixmap



void QgsMapCanvas::setCanvasPixmap(QPixmap * theQPixmap)
{
  mCanvasProperties->pmCanvas = theQPixmap;
} // setCanvasPixmap


void QgsMapCanvas::setZOrder(std::list <QString> theZOrder)
{
  //
  // We need to evaluate each layer in the zOrder and see
  // if it is actually a member of this mapCanvas
  //
  std::list < QString >::iterator li = theZOrder.begin();
  mCanvasProperties->zOrder.clear();
  while (li != theZOrder.end())
  {
    QgsMapLayer *ml = mCanvasProperties->layers[*li];

    if (ml)
    {
#ifdef QGISDEBUG
      std::cout << "Adding  " << ml->name().local8Bit() << " to zOrder" << std::endl;
#endif

      mCanvasProperties->zOrder.push_back(ml->getLayerID());
    }
    else
    {
#ifdef QGISDEBUG
      std::cout << "Cant add  " << ml->name().local8Bit() << " to zOrder (it isnt in layers array)" << std::endl;
#endif

    }
    li++;
  }
}

std::list < QString > const & QgsMapCanvas::zOrders() const
{
  return mCanvasProperties->zOrder;
} // zOrders


std::list < QString >       & QgsMapCanvas::zOrders()
{
  return mCanvasProperties->zOrder;
} // zOrders

//! determines whether the user can interact with the overview canvas.
void QgsMapCanvas::userInteractionAllowed(bool theFlag)
{
  mUserInteractionAllowed = theFlag;
}
//! determines whether the user can interact with the overview canvas.
bool QgsMapCanvas::isUserInteractionAllowed()
{
  return mUserInteractionAllowed;
}


double QgsMapCanvas::mupp() const
{
  return mCanvasProperties->m_mupp;
} // mupp


void QgsMapCanvas::setMapUnits(QGis::units u)
{
#ifdef QGISDEBUG
  std::cerr << "Setting map units to " << static_cast<int>(u) << std::endl;
#endif

  mCanvasProperties->setMapUnits(u);
}


QGis::units QgsMapCanvas::mapUnits() const
{
  return mCanvasProperties->mapUnits();
}


void QgsMapCanvas::setRenderFlag(bool theFlag)
{
  mRenderFlag = theFlag;
  // render the map
  if(mRenderFlag)
  {
    refresh();
  }
}

void QgsMapCanvas::connectNotify( const char * signal )
{
#ifdef QGISDEBUG
  std::cerr << "QgsMapCanvas connected to " << signal << "\n";
#endif
} //  QgsMapCanvas::connectNotify( const char * signal )


bool QgsMapCanvas::writeXML(QDomNode & layerNode, QDomDocument & doc)
{
  // Write current view extents
  QDomElement extentNode = doc.createElement("extent");
  layerNode.appendChild(extentNode);

  QDomElement xMin = doc.createElement("xmin");
  QDomElement yMin = doc.createElement("ymin");
  QDomElement xMax = doc.createElement("xmax");
  QDomElement yMax = doc.createElement("ymax");

  QDomText xMinText = doc.createTextNode(QString::number(mCanvasProperties->currentExtent.xMin(), 'f'));
  QDomText yMinText = doc.createTextNode(QString::number(mCanvasProperties->currentExtent.yMin(), 'f'));
  QDomText xMaxText = doc.createTextNode(QString::number(mCanvasProperties->currentExtent.xMax(), 'f'));
  QDomText yMaxText = doc.createTextNode(QString::number(mCanvasProperties->currentExtent.yMax(), 'f'));

  xMin.appendChild(xMinText);
  yMin.appendChild(yMinText);
  xMax.appendChild(xMaxText);
  yMax.appendChild(yMaxText);

  extentNode.appendChild(xMin);
  extentNode.appendChild(yMin);
  extentNode.appendChild(xMax);
  extentNode.appendChild(yMax);

  // Iterate over layers in zOrder
  // Call writeXML() on each
  QDomElement projectLayersNode = doc.createElement("projectlayers");
  projectLayersNode.setAttribute("layercount", mCanvasProperties->layers.size());

  std::list < QString >::iterator li = mCanvasProperties->zOrder.begin();
  while (li != mCanvasProperties->zOrder.end())
  {
    QgsMapLayer *ml = mCanvasProperties->layers[*li];

    if (ml)
    {
      ml->writeXML(projectLayersNode, doc);
    }
    li++;
  }

  layerNode.appendChild(projectLayersNode);

  return true;
}

void QgsMapCanvas::recalculateExtents()
{
#ifdef QGISDEBUG
  std::cout << "QgsMapCanvas::recalculateExtents() called !" << std::endl;
#endif

  // reset the map canvas extent since the extent may now be smaller
  // We can't use a constructor since QgsRect normalizes the rectangle upon construction
  mCanvasProperties->fullExtent.setMinimal();;
  // get the map layer register collection
  QgsMapLayerRegistry *reg = QgsMapLayerRegistry::instance();
  std::map<QString, QgsMapLayer*>layers = reg->mapLayers();
  // iterate through the map layers and test each layers extent
  // against the current min and max values
  std::map<QString, QgsMapLayer*>::iterator mit = layers.begin();
  while(mit != layers.end())
  {
    QgsMapLayer * lyr = dynamic_cast<QgsMapLayer *>(mit->second);
#ifdef QGISDEBUG
    std::cout << "Updating extent using " << lyr->name().local8Bit() << std::endl;
    std::cout << "Input extent: " << lyr->extent().stringRep().local8Bit() << std::endl;
    try
    {
      if ( ! lyr->coordinateTransform() )
      {
#ifdef QGISDEBUG 
        std::cout << "Throwing exception "<< __FILE__ << __LINE__ << std::endl; 
#endif 
        throw QgsCsException( string("NO COORDINATE TRANSFORM FOUND FOR LAYER") );
      }

      std::cout << "Transformed extent" << 
        lyr->coordinateTransform()->transformBoundingBox(lyr->extent(), 
            QgsCoordinateTransform::FORWARD) << std::endl;
    }
    catch (QgsCsException &e)
    {
      qDebug( "Transform error caught in %s line %d:\n%s", __FILE__, __LINE__, e.what());
    }
#endif
    // Layer extents are stored in the coordinate system (CS) of the
    // layer. The extent must be projected to the canvas CS prior to passing
    // on to the updateFullExtent function
    if (projectionsEnabled())
    {
      try
      {
        if ( ! lyr->coordinateTransform() )
        {
#ifdef QGISDEBUG 
          std::cout << "Throwing exception "<< __FILE__ << __LINE__ << std::endl; 
#endif 
          throw QgsCsException( string("NO COORDINATE TRANSFORM FOUND FOR LAYER") );
        }

        updateFullExtent(lyr->coordinateTransform()->transformBoundingBox(lyr->extent()));
      }
      catch (QgsCsException &e)
      {
        qDebug( "Transform error caught in %s line %d:\n%s", __FILE__, __LINE__, e.what());
      }
    }
    else
    {
      updateFullExtent(lyr->extent());
    }
    mit++;
  }
}

int QgsMapCanvas::mapTool()
{
  return mCanvasProperties->mapTool;
}

void QgsMapCanvas::panActionEnd(QPoint releasePoint)
{
  // use start and end box points to calculate the extent
  QgsPoint start = mCanvasProperties->coordXForm->toMapCoordinates(mCanvasProperties->boxStartPoint);
  QgsPoint end = mCanvasProperties->coordXForm->toMapCoordinates(releasePoint);
  
  double dx = fabs(end.x() - start.x());
  double dy = fabs(end.y() - start.y());
  
  // modify the extent
  mCanvasProperties->previousExtent = mCanvasProperties->currentExtent;
  
  if (end.x() < start.x())
  {
    mCanvasProperties->currentExtent.setXmin(mCanvasProperties->currentExtent.xMin() + dx);
    mCanvasProperties->currentExtent.setXmax(mCanvasProperties->currentExtent.xMax() + dx);
  }
  else
  {
    mCanvasProperties->currentExtent.setXmin(mCanvasProperties->currentExtent.xMin() - dx);
    mCanvasProperties->currentExtent.setXmax(mCanvasProperties->currentExtent.xMax() - dx);
  }
  
  if (end.y() < start.y())
  {
    mCanvasProperties->currentExtent.setYmax(mCanvasProperties->currentExtent.yMax() + dy);
    mCanvasProperties->currentExtent.setYmin(mCanvasProperties->currentExtent.yMin() + dy);
    
  }
  else
  {
    mCanvasProperties->currentExtent.setYmax(mCanvasProperties->currentExtent.yMax() - dy);
    mCanvasProperties->currentExtent.setYmin(mCanvasProperties->currentExtent.yMin() - dy);
    
  }
  clear();
  render();
  emit extentsChanged(mCanvasProperties->currentExtent);
}

void QgsMapCanvas::panAction(QMouseEvent * e)
{

  // bitBlt the pixmap on the screen, offset by the
  // change in mouse coordinates
  double dx = e->pos().x() - mCanvasProperties->boxStartPoint.x();
  double dy = e->pos().y() - mCanvasProperties->boxStartPoint.y();
  
  //erase only the necessary parts to avoid flickering
  if (dx > 0)
  {
    erase(0, 0, (int)dx, height());
  }
  else
  {
    erase(width() + (int)dx, 0, -(int)dx, height());
  }
  if (dy > 0)
  {
    erase(0, 0, width(), (int)dy);
  }
  else
  {
    erase(0, height() + (int)dy, width(), -(int)dy);
  }
  
  bitBlt(this, (int)dx, (int)dy, mCanvasProperties->pmCanvas);
}    

QgsPoint QgsMapCanvas::maybeInversePoint(QgsPoint point, const char whenmsg[])
{
  QgsVectorLayer* vlayer=
    dynamic_cast<QgsVectorLayer*>(mCanvasProperties->mapLegend->currentLayer());
  QgsPoint transformedPoint;

  if( projectionsEnabled() )
  {
    // Do reverse transformation before saving. If possible!
    try
    {
      transformedPoint = vlayer->coordinateTransform()->transform(point, QgsCoordinateTransform::INVERSE);
    }
    catch(QgsCsException &cse)
    {
      //#ifdef QGISDEBUG
      std::cout << "Caught transform error when " << whenmsg <<"." 
                << "Setting untransformed values." << std::endl;
      //#endif	
      // Transformation failed,. Bail out with original rectangle.
      return point;
    }
    return transformedPoint;
  }
  return point;
}
