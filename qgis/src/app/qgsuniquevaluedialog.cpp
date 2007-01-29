/***************************************************************************
                         qgsuniquevaluedialog.cpp  -  description
                             -------------------
    begin                : July 2004
    copyright            : (C) 2004 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
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

#include "qgsuniquevaluedialog.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsfeatureattribute.h"
#include "qgssymbol.h"
#include "qgsuniquevaluerenderer.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"


QgsUniqueValueDialog::QgsUniqueValueDialog(QgsVectorLayer* vl): QDialog(), mVectorLayer(vl), sydialog(vl)
{
    setupUi(this);
    setSizeGripEnabled(true); 

    //find out the fields of mVectorLayer
    QgsVectorDataProvider *provider;
    if ((provider = dynamic_cast<QgsVectorDataProvider *>(mVectorLayer->getDataProvider())))
    {
	const QgsFieldMap & fields = provider->fields();
	QString str;
	
	for (QgsFieldMap::const_iterator it = fields.begin(); it != fields.end(); ++it)
        {
	    str = (*it).name();
	    mClassificationComboBox->insertItem(str);
        }
    } 
    else
    {
	qWarning("Warning, data provider is null in QgsUniqueValueDialog::QgsUniqueValueDialog");
	return;
    }

    const QgsUniqueValueRenderer* renderer = dynamic_cast < const QgsUniqueValueRenderer * >(mVectorLayer->renderer());
    
    if (renderer)
    {
	mClassBreakBox->clear();
	
	// XXX - mloskot - fix for Ticket #31 (bug)
	QgsAttributeList attributes = renderer->classificationAttributes();
  QgsAttributeList::iterator iter = attributes.begin();
	int classattr = *iter;
	mClassificationComboBox->setCurrentItem(classattr);

	const std::list<QgsSymbol*> list = renderer->symbols();
	//fill the items of the renderer into mValues
	for(std::list<QgsSymbol*>::const_iterator iter=list.begin();iter!=list.end();++iter)
	{
	    QgsSymbol* symbol=(*iter);
	    QString symbolvalue=symbol->lowerValue();
	    QgsSymbol* sym=new QgsSymbol(mVectorLayer->vectorType(), symbol->lowerValue(), symbol->upperValue(), symbol->label());
	    sym->setPen(symbol->pen());
	    sym->setBrush(symbol->brush());
	    sym->setNamedPointSymbol(symbol->pointSymbolName());
	    sym->setPointSize(symbol->pointSize());
	    mValues.insert(std::make_pair(symbolvalue,sym));
	    mClassBreakBox->insertItem(symbolvalue);
	}
    }

    QObject::connect(mClassifyButton, SIGNAL(clicked()), this, SLOT(changeClassificationAttribute()));
    QObject::connect(mDeletePushButton, SIGNAL(clicked()), this, SLOT(deleteCurrentClass()));
    QObject::connect(mClassBreakBox, SIGNAL(selectionChanged()), this, SLOT(changeCurrentValue()));
    QObject::connect(&sydialog, SIGNAL(settingsChanged()), this, SLOT(applySymbologyChanges()));
    mSymbolWidgetStack->addWidget(&sydialog);
    mSymbolWidgetStack->raiseWidget(&sydialog);
    
    mClassBreakBox->setCurrentItem(0);
}

QgsUniqueValueDialog::~QgsUniqueValueDialog()
{
    std::map<QString, QgsSymbol *>::iterator myValueIterator = mValues.begin();
    while ( myValueIterator != mValues.end() )
    {
        delete myValueIterator->second;
        
        mValues.erase( myValueIterator );
        
        myValueIterator = mValues.begin(); // since iterator invalidated due to
                                        // erase(), reset to new first element
    }
    mClassBreakBox->setCurrentItem(0);
}

void QgsUniqueValueDialog::apply()
{
    QgsUniqueValueRenderer *renderer = new QgsUniqueValueRenderer(mVectorLayer->vectorType());

    //go through mValues and add the entries to the renderer
    for(std::map<QString,QgsSymbol*>::iterator it=mValues.begin();it!=mValues.end();++it)
    {
	QgsSymbol* symbol=it->second;
	QgsSymbol* newsymbol=new QgsSymbol(mVectorLayer->vectorType(), symbol->lowerValue(), symbol->upperValue(), symbol->label());
	newsymbol->setPen(symbol->pen());
	newsymbol->setBrush(symbol->brush());
	newsymbol->setNamedPointSymbol(symbol->pointSymbolName());
	newsymbol->setPointSize(symbol->pointSize());
	renderer->insertValue(it->first,newsymbol);
    }

    renderer->setClassificationField(mClassificationComboBox->currentItem());
    mVectorLayer->setRenderer(renderer);
}

void QgsUniqueValueDialog::changeClassificationAttribute()
{
  int nr = mClassificationComboBox->currentIndex();
    //delete old entries
    for(std::map<QString,QgsSymbol*>::iterator it=mValues.begin();it!=mValues.end();++it)
    {
	delete it->second;
    }
    mValues.clear();
    
    QgsVectorDataProvider *provider = dynamic_cast<QgsVectorDataProvider *>(mVectorLayer->getDataProvider());
    if (provider)
    {
	QString value;
	QgsAttributeList attlist;
	attlist.append(nr);
	QgsSymbol* symbol;

	provider->reset();
	QgsFeature feat;

	//go through all the features and insert their value into the map and into mClassBreakBox
	mClassBreakBox->clear();
	while(provider->getNextFeature(feat, false, attlist))
	{
      const QgsAttributeMap& vec = feat.attributeMap();
	    value=vec[nr].fieldValue();
	   
	    if(mValues.find(value)==mValues.end())
	    {
		symbol=new QgsSymbol(mVectorLayer->vectorType(), value);
		mValues.insert(std::make_pair(value,symbol));
	    }
	}

	//set symbology for all QgsSiSyDialogs
	QColor thecolor;

	for(std::map<QString,QgsSymbol*>::iterator it=mValues.begin();it!=mValues.end();++it)
	{
	    //insert a random color
	    int red = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
	    int green = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
	    int blue = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
	    thecolor.setRgb(red, green, blue);
	    mClassBreakBox->insertItem(it->first);
	    QgsSymbol* sym=it->second;
	    QPen pen;
	    QBrush brush;
	    if(mVectorLayer->vectorType() == QGis::Line)
	    {
		pen.setColor(thecolor);
		pen.setStyle(Qt::SolidLine);
		pen.setWidth(1);
	    }
	    else
	    {
		brush.setColor(thecolor);
		brush.setStyle(Qt::SolidPattern);
		pen.setColor(Qt::black);
		pen.setStyle(Qt::SolidLine);
		pen.setWidth(1);
	    }
	    sym->setPen(pen);
	    sym->setBrush(brush);
	}
    }
    mClassBreakBox->setCurrentItem(0);
}

void QgsUniqueValueDialog::changeCurrentValue()
{
    sydialog.blockSignals(true);//block signal to prevent sydialog from changing the current QgsRenderItem
    Q3ListBoxItem* item=mClassBreakBox->selectedItem();
    if(item)
      {
	QString value=item->text();
	std::map<QString,QgsSymbol*>::iterator it=mValues.find(value);
	if(it!=mValues.end())
	  {
	    sydialog.set( it->second);
	    sydialog.setLabel(it->second->label());
	  }
	else
	  {
	    //no entry found
	  }
      }
    sydialog.blockSignals(false);
}

void QgsUniqueValueDialog::deleteCurrentClass()
{
  QString classValue = mClassBreakBox->currentText();
  int currentIndex = mClassBreakBox->currentItem();
  mValues.erase(classValue);
  mClassBreakBox->removeItem(currentIndex);
  qWarning("numRows: ");
  qWarning(QString::number(mClassBreakBox->numRows()));
  //
  if(mClassBreakBox->numRows() < (currentIndex + 1))
    {
      qWarning("selecting numRows - 1");
      mClassBreakBox->setSelected(mClassBreakBox->numRows() - 1, true);
    }
  else
    {
      qWarning("selecting currentIndex");
      mClassBreakBox->setSelected(currentIndex, true);
    }
}

void QgsUniqueValueDialog::applySymbologyChanges()
{
  Q3ListBoxItem* item=mClassBreakBox->selectedItem();
  QString value=item->text();
  std::map<QString,QgsSymbol*>::iterator it=mValues.find(value);
  if(it!=mValues.end())
  {
      it->second->setLabel(sydialog.label());
      it->second->setLowerValue(value);
      sydialog.apply( it->second );
  }
}
