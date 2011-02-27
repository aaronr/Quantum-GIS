/***************************************************************************
    qgsrasterprojector.cpp - Raster projector
     --------------------------------------
    Date                 : Jan 16, 2011
    Copyright            : (C) 2005 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id: qgsrasterprojector.cpp 15005 2011-01-08 16:35:21Z rblazek $ */

#include <cassert>

#include "qgslogger.h"
#include "qgsrasterprojector.h"
#include "qgscoordinatetransform.h"

QgsRasterProjector::QgsRasterProjector ( 
  QgsCoordinateReferenceSystem theSrcCRS, 
  QgsCoordinateReferenceSystem theDestCRS, 
  QgsRectangle theDestExtent, 
  int theDestRows, int theDestCols,
  double theMaxSrcXRes, double theMaxSrcYRes )
    : mSrcCRS ( theSrcCRS )
    , mDestCRS ( theDestCRS )
    , mDestExtent ( theDestExtent )
    , mDestRows ( theDestRows ), mDestCols ( theDestCols )
    , mMaxSrcXRes ( theMaxSrcXRes), mMaxSrcYRes ( theMaxSrcYRes )
{
  QgsDebugMsg("Entered");
  QgsDebugMsg("theDestExtent = " + theDestExtent.toString() );

  // reverse transformation
  mCoordinateTransform = new QgsCoordinateTransform( theDestCRS, theSrcCRS );

  mDestXRes = mDestExtent.width() / (mDestCols);
  mDestYRes = mDestExtent.height() / (mDestRows);

  // Calculate tolerance
  // TODO: Think it over better
  // Note: we are checking on matrix each even point, that means taht the real error 
  // in that moment is approximately half size 
  double myDestRes = mDestXRes < mDestYRes ? mDestXRes : mDestYRes;
  mSqrTolerance = myDestRes * myDestRes;

  // Initialize the matrix by corners and middle points
  mCPCols = mCPRows = 3;
  for ( int i = 0; i < mCPRows; i++) {
    QList<QgsPoint> myRow;
    myRow.append( QgsPoint() );
    myRow.append( QgsPoint() );
    myRow.append( QgsPoint() );
    mCPMatrix.insert(i,  myRow );
  }
  for ( int i = 0; i < mCPRows; i++) {
    calcRow( i );
  }

  while (true) 
  {
    bool myColsOK = checkCols();
    if ( !myColsOK ) {
      insertRows();
    }
    bool myRowsOK = checkRows();
    if ( !myRowsOK ) {
      insertCols();
    }
    if ( myColsOK && myRowsOK ) {
      QgsDebugMsg("CP matrix within tolerance");
      break;
    }
    // What is the maximum reasonable size of transformatio matrix? 
    // TODO: consider better when to break - ratio
    if ( mCPRows * mCPCols > 0.25 * mDestRows * mDestCols ) 
    {
      QgsDebugMsg("Too large CP matrix");
      break;
    }
 
  }
  QgsDebugMsg( QString("CPMatrix size: mCPRows = %1 mCPCols = %2").arg(mCPRows).arg(mCPCols) );
  mDestRowsPerMatrixRow =  (float)mDestRows / (mCPRows - 1);
  mDestColsPerMatrixCol = (float)mDestCols / (mCPCols - 1);

  QgsDebugMsg( "CPMatrix:\n" + cpToString() );

  // Calculate source dimensions
  calcSrcExtent();
  calcSrcRowsCols();
  mSrcXRes = mSrcExtent.height() / mSrcRows;
  mSrcYRes = mSrcExtent.width() / mSrcCols;

  // init helper points 
  pHelperTop = new QgsPoint[mDestCols];
  pHelperBottom = new QgsPoint[mDestCols];
  calcHelper ( 0, pHelperTop );
  calcHelper ( 1, pHelperBottom );
  mHelperTopRow = 0;
}

QgsRasterProjector::~QgsRasterProjector ()
{
  delete mCoordinateTransform;
}



void QgsRasterProjector::calcSrcExtent ()
{
  /* Run around the mCPMatrix and find source extent */
  QgsPoint myPoint = mCPMatrix[0][0];
  mSrcExtent = QgsRectangle ( myPoint.x(), myPoint.y(), myPoint.x(), myPoint.y() );
  for ( int i = 0; i < mCPRows; i++) {
    myPoint = mCPMatrix[i][0];
    mSrcExtent.combineExtentWith ( myPoint.x(), myPoint.y() );
    myPoint = mCPMatrix[i][mCPCols-1];
    mSrcExtent.combineExtentWith ( myPoint.x(), myPoint.y() );
  }
  for ( int i = 1; i < mCPCols-1; i++) {
    myPoint = mCPMatrix[0][i];
    mSrcExtent.combineExtentWith ( myPoint.x(), myPoint.y() );
    myPoint = mCPMatrix[mCPRows-1][i];
    mSrcExtent.combineExtentWith ( myPoint.x(), myPoint.y() );
  }
  // Expand a bit to avoid possible approx coords falling out because of representation error
  //mSrcExtent.setXMinimum();

  QgsDebugMsg(  "mSrcExtent = " + mSrcExtent.toString() );
}

QString QgsRasterProjector::cpToString()
{
  QString myString;
  for ( int i = 0; i < mCPRows; i++) {
    if ( i > 0 ) myString += "\n";
    for ( int j = 0; j < mCPCols; j++) {
      if ( j > 0 ) myString += "  ";
      QgsPoint myPoint = mCPMatrix[i][j];
      myString += myPoint.toString();
    }
  }
  return myString;
}

void QgsRasterProjector::calcSrcRowsCols ()
{
  // Wee need to calculate minimum cell size in the source 
  // TODO: Think it over better, what is the right source resolution?
  //       Taking distances between cell centers projected to source along source 
  //       axis would result in very high resolution
  // TODO: different resolution for rows and cols ?

  // For now, we take cell sizes projected to source but not to source axes
  double myDestColsPerMatrixCell = mDestCols / mCPCols;
  double myDestRowsPerMatrixCell = mDestRows / mCPRows;

  double myMinSize = DBL_MAX;
  
  for ( int i = 0; i < mCPRows-1; i++) {
    for ( int j = 0; j < mCPCols-1; j++) {
      QgsPoint myPointA = mCPMatrix[i][j];
      QgsPoint myPointB = mCPMatrix[i][j+1];
      QgsPoint myPointC = mCPMatrix[i+1][j];
      double mySize = sqrt(myPointA.sqrDist( myPointB )) / myDestColsPerMatrixCell; 
      if ( mySize < myMinSize ) { myMinSize = mySize; }

      mySize = sqrt(myPointA.sqrDist( myPointC )) / myDestRowsPerMatrixCell;
      if ( mySize < myMinSize ) { myMinSize = mySize; }
    }
  }
  
  // Make it a bit higher resolution
  // TODO: find the best coefficient 
  myMinSize *= 0.5; 

  double myMinXSize = mMaxSrcXRes > myMinSize ? mMaxSrcXRes : myMinSize;
  double myMinYSize = mMaxSrcYRes > myMinSize ? mMaxSrcYRes : myMinSize;
  QgsDebugMsg( QString("mSrcExtent.width = %1 mSrcExtent.height = %2" ).arg ( mSrcExtent.width() ).arg( mSrcExtent.height() ) );
  mSrcRows = (int) ceil ( mSrcExtent.height() / myMinYSize );
  mSrcCols = (int) ceil ( mSrcExtent.width() / myMinXSize );

  QgsDebugMsg( QString("mSrcRows = %1 theSrcCols = %2").arg(mSrcRows).arg(mSrcCols) );
}


inline void QgsRasterProjector::destPointOnCPMatrix ( int theRow, int theCol, double *theX, double *theY )
{
  //QgsDebugMsg( QString("theRow = %1 theCol = %2").arg(theRow).arg(theCol) );
  *theX = mDestExtent.xMinimum() + theCol * mDestExtent.width() / (mCPCols-1);
  *theY = mDestExtent.yMaximum() - theRow * mDestExtent.height() / (mCPRows-1);
  //QgsDebugMsg( QString("x = %1 y = %2").arg(x).arg(y) );
}

inline int QgsRasterProjector::matrixRow ( int theDestRow )
{
  //QgsDebugMsg( QString("theDestRow = %1 mDestRows = %2 mCPRows = %3").arg(theDestRow).arg( mDestRows).arg(mCPRows) );
  //int myRow = (int) ( floor ( (theDestRow + 0.5) / ( (float)mDestRows / (mCPRows - 1) )  ) );
  return (int) ( floor ( (theDestRow + 0.5) / mDestRowsPerMatrixRow  ) );
}
inline int QgsRasterProjector::matrixCol ( int theDestCol )
{
  //QgsDebugMsg( QString("theDestCol = %1 mDestCols = %2 mCPCols = %3").arg(theDestCol).arg( mDestCols).arg(mCPCols) );
  //return (int) ( floor ( (theDestCol + 0.5) / ( (float)mDestCols / (mCPCols - 1) )  ) );
  return (int) ( floor ( (theDestCol + 0.5) / mDestColsPerMatrixCol  ) );
}

QgsPoint QgsRasterProjector::srcPoint ( int theDestRow, int theCol )
{
  return QgsPoint();
}


//void QgsRasterProjector::calcHelper ( int theMatrixRow, QList<QgsPoint> *thePoints )
void QgsRasterProjector::calcHelper ( int theMatrixRow, QgsPoint *thePoints )
{
  // TODO?: should we also precalc dest cell center coordinates for x and y?
  for ( int myDestCol = 0; myDestCol < mDestCols; myDestCol++) {
    double myDestX = mDestExtent.xMinimum() + ( myDestCol + 0.5 ) * mDestXRes;
    //QgsDebugMsg( QString("myDestCol = %1 mDestCols = %2").arg( myDestCol ).arg (mDestCols) );
    //QgsDebugMsg( QString("myDestX = %1").arg( myDestX ) );

    int myMatrixCol = matrixCol ( myDestCol );
    //QgsDebugMsg( QString("myMatrixCol = %1 mCPCols = %2").arg( myMatrixCol ).arg(mCPCols) );

    double myDestXMin, myDestYMin, myDestXMax, myDestYMax;

    destPointOnCPMatrix ( theMatrixRow, myMatrixCol, &myDestXMin, &myDestYMin );
    destPointOnCPMatrix ( theMatrixRow, myMatrixCol + 1, &myDestXMax, &myDestYMax );
    
    //QgsDebugMsg( QString("myDestXMin = %1 myDestXMax= %2").arg( myDestXMin ).arg( myDestXMax) );

    double xfrac = ( myDestX - myDestXMin ) / ( myDestXMax - myDestXMin );

    QgsPoint &mySrcPoint0 = mCPMatrix[theMatrixRow][myMatrixCol];
    QgsPoint &mySrcPoint1 = mCPMatrix[theMatrixRow][myMatrixCol+1];
    double s = mySrcPoint0.x() + ( mySrcPoint1.x() - mySrcPoint0.x() ) * xfrac;
    double t = mySrcPoint0.y() + ( mySrcPoint1.y() - mySrcPoint0.y() ) * xfrac; 

    //QgsDebugMsg( QString("s = %1 t = %2").arg(s).arg(t) );

    //QgsDebugMsg( QString("thePoints = %1").arg ( (long)thePoints ) );

    thePoints[myDestCol].setX ( s );
    thePoints[myDestCol].setY ( t );

    //QgsDebugMsg( QString("thePoints[myDestCol] = %1").arg( (*thePoints)[myDestCol].toString() ) );
  }
}
void QgsRasterProjector::nextHelper ()
{
  //QgsDebugMsg( QString("mHelperTopRow = %1").arg(mHelperTopRow) );
  /*
  QList <QgsPoint> *tmp;
  tmp = pHelperTop;
  pHelperTop = pHelperBottom;
  pHelperBottom = tmp;
  calcHelper ( mHelperTopRow+2, pHelperBottom );
  */
  QgsPoint *tmp;
  tmp = pHelperTop;
  pHelperTop = pHelperBottom;
  pHelperBottom = tmp;
  calcHelper ( mHelperTopRow+2, pHelperBottom );
  mHelperTopRow++;
}

void QgsRasterProjector::srcRowCol ( int theDestRow, int theDestCol, int *theSrcRow, int *theSrcCol )
{
  //QgsDebugMsg( QString("theDestRow = %1 theDestCol = %2").arg( theDestRow).arg(theDestCol) );
  int myMatrixRow = matrixRow ( theDestRow );
  int myMatrixCol = matrixCol ( theDestCol );

  //int myMatrixRow = 0;
  //int myMatrixCol = 0;
  *theSrcRow = 0; // debug
  *theSrcCol = 0;

  if ( myMatrixRow > mHelperTopRow ) {
    // TODO: make it more robust (for random, not sequential reading)
    nextHelper ();
  }

  double myDestY = mDestExtent.yMaximum() - ( theDestRow + 0.5 ) * mDestYRes;

  //QgsDebugMsg( QString("myDestX = %1 myDestY = %2").arg( myDestX ).arg( myDestY) );

  //QgsDebugMsg( QString("myMatrixRow = %1 myMatrixCol = %2 mCPRows = %3 mCPCols = %4").arg( myMatrixRow ).arg( myMatrixCol).arg(mCPRows).arg(mCPCols) );

  // See the schema in javax.media.jai.WarpGrid doc (but up side down)
  // TODO: use some kind of cache of values which can be reused
  double myDestXMin, myDestYMin, myDestXMax, myDestYMax;

  destPointOnCPMatrix ( myMatrixRow + 1, myMatrixCol, &myDestXMin, &myDestYMin );
  destPointOnCPMatrix ( myMatrixRow, myMatrixCol + 1, &myDestXMax, &myDestYMax );

  //QgsDebugMsg( "mySrcPoint : " + mySrcPoint0.toString() + " " + mySrcPoint1.toString() + " " +mySrcPoint2.toString() + " " + mySrcPoint3.toString() + " " );

  double yfrac = ( myDestY - myDestYMin ) / ( myDestYMax - myDestYMin );
  //QgsDebugMsg( QString("yfrac = %2").arg(yfrac) );
  //assert ( yfrac >= 0 );
  
  QgsPoint &myTop = pHelperTop[theDestCol];
  QgsPoint &myBot = pHelperBottom[theDestCol];

  // Warning: this is very SLOW compared to the following code!:
  //double mySrcX = myBot.x() + (myTop.x() - myBot.x()) * yfrac;
  //double mySrcY = myBot.y() + (myTop.y() - myBot.y()) * yfrac;

  double tx = myTop.x();
  double ty = myTop.y();
  double bx = myBot.x();
  double by = myBot.y();
  double mySrcX = bx + (tx - bx) * yfrac;
  double mySrcY = by + (ty - by) * yfrac;

  //QgsDebugMsg( QString("mySrcX = %1 mySrcY = %2").arg(mySrcX).arg(mySrcY) );

  // TODO: check again cell selection (coor is in the middle)

  //QgsDebugMsg( QString("mSrcExtent.yMaximum() = %1 mySrcY = %2").arg(mSrcExtent.yMaximum()).arg(mySrcY)) ;

  *theSrcRow = (int) floor ( ( mSrcExtent.yMaximum() - mySrcY ) / mSrcXRes );
  *theSrcCol =  (int) floor ( ( mySrcX - mSrcExtent.xMinimum() ) / mSrcYRes );

  //*theSrcRow = 0; // debug
  //*theSrcCol = 0;

  //QgsDebugMsg( QString("( mSrcExtent.yMaximum() - mySrcY ) / ( mSrcExtent.height() / mSrcRows ) = %1") .arg( ( mSrcExtent.yMaximum() - mySrcY ) / ( mSrcExtent.height() / mSrcRows )  ) );
  //QgsDebugMsg( QString("mySrcY = %1 yMaximum = %2").arg(mySrcY).arg(mSrcExtent.yMaximum()) );
  
  //QgsDebugMsg( QString("theSrcRow = %1 theSrcCol = %2 mSrcRows = %3 mSrcCols = %4").arg( *theSrcRow ).arg( *theSrcCol ).arg(mSrcRows).arg(mSrcCols) );

  //assert ( *theSrcRow < mSrcRows );
  //assert ( *theSrcCol < mSrcCols );
}

void QgsRasterProjector::insertRows()
{
  for ( int r = 0; r < mCPRows-1; r++) {
    QList<QgsPoint> myRow;
    for ( int c = 0; c < mCPCols; c++) {
      myRow.append( QgsPoint() );
    }
    QgsDebugMsg( QString("insert new row at %1" ).arg(1+r*2) );
    mCPMatrix.insert( 1+r*2,  myRow );
  }
  mCPRows += mCPRows-1;
  for ( int r = 1; r < mCPRows-1; r += 2) {
    calcRow( r );
  }
}

void QgsRasterProjector::insertCols()
{
  for ( int r = 0; r < mCPRows; r++) {
    QList<QgsPoint> myRow;
    for ( int c = 0; c < mCPCols-1; c++) {
      mCPMatrix[r].insert( 1+c*2,  QgsPoint() );
    }
  }
  mCPCols += mCPCols-1;
  for ( int c = 1; c < mCPCols-1; c += 2) {
    calcCol( c );
  }

}

void QgsRasterProjector::calcCP ( int theRow, int theCol ) 
{
  //QgsDebugMsg( QString("theRow = %1 theCol = %2").arg(theRow).arg(theCol) );
  //QgsPoint myDestPoint = destPointOnCPMatrix ( theRow, theCol );
  double myDestX, myDestY;
  destPointOnCPMatrix ( theRow, theCol, &myDestX, &myDestY  );
  QgsPoint myDestPoint ( myDestX, myDestY ); 
  //QgsDebugMsg( "myDestPoint : " + myDestPoint.toString() );

  mCPMatrix[theRow][theCol] = mCoordinateTransform->transform( myDestPoint ); 
  //QgsDebugMsg( QString("x = %1 y = %2").arg( mCPMatrix[theRow][theCol].x() ).arg( mCPMatrix[theRow][theCol].y() ) );
}

bool QgsRasterProjector::calcRow( int theRow )
{
  QgsDebugMsg( QString("theRow = %1").arg(theRow) );
  for ( int i = 0; i < mCPCols; i++) {
    calcCP ( theRow, i );
  }
}

bool QgsRasterProjector::calcCol( int theCol )
{
  QgsDebugMsg( QString("theCol = %1").arg(theCol) );
  for ( int i = 0; i < mCPRows; i++) {
    calcCP ( i, theCol );
  }
}

bool QgsRasterProjector::checkCols()
{
  //QgsDebugMsg( "Entered" );
  for ( int c = 0; c < mCPCols; c++) {
    for ( int r = 1; r < mCPRows-1; r += 2 ) {
      //QgsPoint myDestPoint = destPointOnCPMatrix ( r, c );
      double myDestX, myDestY;
      destPointOnCPMatrix ( r, c, &myDestX, &myDestY  );
      QgsPoint myDestPoint ( myDestX, myDestY ); 

      QgsPoint mySrcPoint1 = mCPMatrix[r-1][c];
      QgsPoint mySrcPoint2 = mCPMatrix[r][c];
      QgsPoint mySrcPoint3 = mCPMatrix[r+1][c];

      QgsPoint mySrcApprox( ( mySrcPoint1.x() + mySrcPoint3.x() ) / 2,  (mySrcPoint1.y() + mySrcPoint3.y() ) / 2 );
      QgsPoint myDestApprox = mCoordinateTransform->transform( mySrcApprox, QgsCoordinateTransform::ReverseTransform );
      double mySqrDist = myDestApprox.sqrDist( myDestPoint );
      QgsDebugMsg( QString("mySqrDist = %1 mSqrTolerance = %2").arg(mySqrDist).arg(mSqrTolerance) );
      if ( mySqrDist > mSqrTolerance ) { return false; }
    }
  }
  return true;
}

bool QgsRasterProjector::checkRows()
{
  //QgsDebugMsg( "Entered" );
  for ( int r = 0; r < mCPRows; r++) {
    for ( int c = 1; c < mCPCols-1; c += 2 ) {
      //QgsPoint myDestPoint = destPointOnCPMatrix ( r, c );
      double myDestX, myDestY;
      destPointOnCPMatrix ( r, c, &myDestX, &myDestY  );

      QgsPoint myDestPoint ( myDestX, myDestY ); 
      QgsPoint mySrcPoint1 = mCPMatrix[r][c-1];
      QgsPoint mySrcPoint2 = mCPMatrix[r][c];
      QgsPoint mySrcPoint3 = mCPMatrix[r][c+1];
      
      QgsPoint mySrcApprox( ( mySrcPoint1.x() + mySrcPoint3.x() ) / 2,  (mySrcPoint1.y() + mySrcPoint3.y() ) / 2 );
      QgsPoint myDestApprox = mCoordinateTransform->transform( mySrcApprox, QgsCoordinateTransform::ReverseTransform );
      double mySqrDist = myDestApprox.sqrDist( myDestPoint );
      QgsDebugMsg( QString("mySqrDist = %1 mSqrTolerance = %2").arg(mySqrDist).arg(mSqrTolerance) );
      if ( mySqrDist > mSqrTolerance ) { return false; }
    }
  }
  return true;
}
