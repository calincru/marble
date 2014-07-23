//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2014      Calin Cruceru <crucerucalincristian@gmail.com>
//

#ifndef EDITPLACEMARKDIALOG_H
#define EDITPLACEMARKDIALOG_H

#include <QDialog>


namespace Marble {

class GeoDataPlacemark;
class PlacemarkTextAnnotation;

class EditTextAnnotationDialog : public QDialog
{
    Q_OBJECT

public:
    EditTextAnnotationDialog( PlacemarkTextAnnotation *textAnnotation, QWidget *parent = 0 );
    ~EditTextAnnotationDialog();

private slots:
    void loadIconFile();

private:
    class Private;
    Private * const d;
};

}

#endif
