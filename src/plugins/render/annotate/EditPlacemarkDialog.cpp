//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2014      Calin Cruceru <crucerucalincristian@gmail.com>
//

// self
#include "EditPlacemarkDialog.h"
#include "ui_EditPlacemarkDialog.h"

// Qt


// Marble
#include "GeoDataPlacemark.h"


namespace Marble {

class EditPlacemarkDialog::Private : public Ui::UiEditPlacemarkDialog
{
public:
    Private( GeoDataPlacemark *placemark );
    ~Private();

    GeoDataPlacemark *m_placemark;
};

EditPlacemarkDialog::Private::Private( GeoDataPlacemark *placemark ) :
    Ui::UiEditPlacemarkDialog(),
    m_placemark( placemark )
{
    // nothing to do
}

EditPlacemarkDialog::Private::~Private()
{
    // nothing to do
}

EditPlacemarkDialog::EditPlacemarkDialog( GeoDataPlacemark *placemark, QWidget *parent ) :
    QDialog( parent ),
    d( new Private( placemark ) )
{
    d->setupUi( this );

    // to be continued
}

EditPlacemarkDialog::~EditPlacemarkDialog()
{
    delete d;
}

}

#include "EditPlacemarkDialog.moc"
