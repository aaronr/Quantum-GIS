/***************************************************************************
    qgspythondialog.h - dialog with embedded python console
    ---------------------
    begin                : October 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgspythondialog.h"
#include "qgspythonutils.h"


QgsPythonDialog::QgsPythonDialog(QgisInterface* pIface, QWidget *parent)
  : QDialog(parent)
{
  setupUi(this);
  mIface = pIface;
  
  QgsPythonUtils::installConsoleHooks();
}

QgsPythonDialog::~QgsPythonDialog()
{
  QgsPythonUtils::uninstallConsoleHooks();
}

void QgsPythonDialog::on_edtCmdLine_returnPressed()
{
  QString command = edtCmdLine->text();
  QString output;
  
  // when using Py_single_input the return value will be always null
  // we're using custom hooks for output and exceptions to show output in console
  if (QgsPythonUtils::runString(command))
  {
    QgsPythonUtils::evalString("sys.stdout.data", output);
    QgsPythonUtils::runString("sys.stdout.data = ''");
    
    output += QgsPythonUtils::getResult();
    
    if (!output.isEmpty())
      output += "<br>";
  }
  else
  {
    QString className, errorText;
    QgsPythonUtils::getError(className, errorText);
    
    output = "<font color=\"red\">" + className + ": " + errorText + "</font><br>";
  }
  
  QString str = "<b><font color=\"green\">>>></font> " + command + "</b><br>" + output;
  txtHistory->setText(txtHistory->text() + str);
  edtCmdLine->setText("");
  
  txtHistory->moveCursor(QTextCursor::End);
  txtHistory->ensureCursorVisible();
}
