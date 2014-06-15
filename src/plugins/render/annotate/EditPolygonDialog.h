//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2014      Calin-Cristian Cruceru <crucerucalincristian@gmail.com>
//

#ifndef MARBLE_EDITPOLYGONDIALOG_H
#define MARBLE_EDITPOLYGONDIALOG_H


#include <QDialog>
#include <QColor>

#include "MarbleGlobal.h"
#include "GeoDataPlacemark.h"

namespace Marble {

class MARBLE_EXPORT EditPolygonDialog : public QDialog
{
    Q_OBJECT

public:
    EditPolygonDialog( GeoDataPlacemark *placemark, QWidget *parent = 0 );
    ~EditPolygonDialog();

private slots:
    void updatePolygon();
    void updateDialog( QColor color );

signals:
    void polygonUpdated();

private:
    class Private;
    Private *d;
};

}

#endif
