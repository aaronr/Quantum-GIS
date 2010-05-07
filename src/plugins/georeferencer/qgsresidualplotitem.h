#ifndef QGSRESIDUALPLOTITEM_H
#define QGSRESIDUALPLOTITEM_H

#include "qgscomposeritem.h"
#include "qgsgcplist.h"
#include "qgsrectangle.h"

/**A composer item to visualise the distribution of georeference residuals. For the visualisation, \
the length of the residual arrows are scaled*/
class QgsResidualPlotItem: public QgsComposerItem
{
  public:
    QgsResidualPlotItem( QgsComposition* c );
    ~QgsResidualPlotItem();

    /** \brief Reimplementation of QCanvasItem::paint*/
    virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    void setGCPList( const QgsGCPList& list ) { mGCPList = list; }
    QgsGCPList GCPList() const { return mGCPList; }

    void setExtent( const QgsRectangle& rect ) { mExtent = rect;}
    QgsRectangle extent() const { return mExtent; }

    void setPixelToMapUnits( double d ) { mPixelToMapUnits = d; }
    double pixelToMapUnits() const { return mPixelToMapUnits; }

    void setConvertScaleToMapUnits( bool convert ) { mConvertScaleToMapUnits = convert; }
    bool convertScaleToMapUnits() const { return mConvertScaleToMapUnits; }

    virtual bool writeXML( QDomElement& elem, QDomDocument & doc ) const;
    virtual bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

  private:
    //gcp list
    QgsGCPList mGCPList;

    QgsRectangle mExtent;
    /**True if the scale bar units should be converted to map units. This can be done for transformation where the scaling in all directions is the same (helmert)*/
    bool mConvertScaleToMapUnits;
    /**Scale factor from pixels to map units*/
    double mPixelToMapUnits;

    /**Calculates maximal possible mm to pixel ratio such that the residual arrow is still inside the frame*/
    double maxMMToPixelRatioForGCP( const QgsGeorefDataPoint* p, double pixelXMM, double pixelYMM );

    /**Returns distance between two points*/
    double dist( const QPointF& p1, const QPointF& p2 ) const;
};

#endif // QGSRESIDUALPLOTITEM_H
