//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2014      Calin Cruceru <crucerucalincristian@gmail.com>
//

// Self
#include "EditPolylineDialog.h"
#include "ui_EditPolylineDialog.h"

// Qt

// Marble
#include "GeoDataPlacemark.h"


namespace Marble
{

class EditPolylineDialog::Private : public Ui::UiEditPolylineDialog
{
public:
    Private( GeoDataPlacemark *placemark);
    ~Private();

    GeoDataPlacemark *m_placemark;

    // Used to tell whether the settings before showing the dialog should be restored on
    // pressing the 'Cancel' button or not.
    bool m_firstEditing;

};

EditPolylineDialog::Private::Private( GeoDataPlacemark *placemark ) :
    Ui::UiEditPolylineDialog(),
    m_placemark( placemark ),
    m_firstEditing( false )
{
    // nothing to do
}

EditPolylineDialog::Private::~Private()
{
    // nothing to do
}

EditPolylineDialog::EditPolylineDialog( GeoDataPlacemark *placemark, QWidget *parent ) :
    QDialog( parent ) ,
    d ( new Private( placemark ) )
{

}

EditPolylineDialog::~EditPolylineDialog()
{
    delete d;
}

void EditPolylineDialog::setFirstTimeEditing( bool enabled )
{
    d->m_firstEditing = enabled;
}

}

#include "EditPolylineDialog.moc"
