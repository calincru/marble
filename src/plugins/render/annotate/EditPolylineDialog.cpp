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
#include <QColorDialog>

// Marble
#include "GeoDataPlacemark.h"
#include "GeoDataStyle.h"


namespace Marble
{

class EditPolylineDialog::Private : public Ui::UiEditPolylineDialog
{
public:
    Private( GeoDataPlacemark *placemark);
    ~Private();

    // Used to tell whether the settings before showing the dialog should be restored on
    // pressing the 'Cancel' button or not.
    bool m_firstEditing;

    QColorDialog *m_linesDialog;
    GeoDataPlacemark *m_placemark;
};

EditPolylineDialog::Private::Private( GeoDataPlacemark *placemark ) :
    Ui::UiEditPolylineDialog(),
    m_firstEditing( false ),
    m_linesDialog( 0 ),
    m_placemark( placemark )
{
    // nothing to do
}

EditPolylineDialog::Private::~Private()
{
    delete m_linesDialog;
}

EditPolylineDialog::EditPolylineDialog( GeoDataPlacemark *placemark, QWidget *parent ) :
    QDialog( parent ) ,
    d ( new Private( placemark ) )
{
    d->setupUi( this );

    // If the polygon has just been drawn, assign it a default name.
    if ( d->m_placemark->name().isNull() ) {
        d->m_placemark->setName( tr("Untitled Path") );
    }

    d->m_name->setText( placemark->name() );
    d->m_description->setText( placemark->description() );
    d->m_linesWidth->setRange( 0.1, 5.0 );

    // Get the current style properties.
    const GeoDataLineStyle lineStyle = placemark->style()->lineStyle();
    d->m_linesWidth->setValue( lineStyle.width() );

    // Adjust the color button's icon to the current lines color.
    QPixmap linesPixmap( d->m_linesColorButton->iconSize().width(),
                         d->m_linesColorButton->iconSize().height() );
    linesPixmap.fill( lineStyle.color() );
    d->m_linesColorButton->setIcon( QIcon( linesPixmap ) );

    // Setup the color dialogs.
    d->m_linesDialog = new QColorDialog( this );
    d->m_linesDialog->setOption( QColorDialog::ShowAlphaChannel );
    d->m_linesDialog->setCurrentColor( lineStyle.color() );
    connect( d->m_linesColorButton, SIGNAL(clicked()), d->m_linesDialog, SLOT(exec()) );
    connect( d->m_linesDialog, SIGNAL(colorSelected(QColor)), this, SLOT(updateLinesDialog(const QColor&)) );


    // Promote "Ok" button to default button.
    d->buttonBox->button( QDialogButtonBox::Ok )->setDefault( true );

    connect( d->buttonBox->button( QDialogButtonBox::Ok ), SIGNAL(pressed()), this, SLOT(checkFields()) );
    connect( d->buttonBox, SIGNAL(accepted()), this, SLOT(updatePolyline()) );
    connect( this, SIGNAL(rejected()), SLOT(restoreInitial()) );

    // Ensure that the dialog gets deleted when closing it (either when clicking OK or
    // Close).
    connect( this, SIGNAL(finished(int)), SLOT(deleteLater()) );
}

EditPolylineDialog::~EditPolylineDialog()
{
    delete d;
}

void EditPolylineDialog::setFirstTimeEditing( bool enabled )
{
    d->m_firstEditing = enabled;
}

void EditPolylineDialog::updatePolyline()
{

    emit polylineUpdated( d->m_placemark );
}

void EditPolylineDialog::updateLinesDialog( const QColor &color )
{
    QPixmap linesPixmap( d->m_linesColorButton->iconSize().width(),
                         d->m_linesColorButton->iconSize().height() );
    linesPixmap.fill( color );
    d->m_linesColorButton->setIcon( QIcon( linesPixmap ) );
}

void EditPolylineDialog::restoreInitial()
{

    emit polylineUpdated( d->m_placemark );
}

void EditPolylineDialog::checkFields()
{

}

}

#include "EditPolylineDialog.moc"
