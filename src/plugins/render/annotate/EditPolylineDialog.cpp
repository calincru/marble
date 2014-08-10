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
#include <QMessageBox>

// Marble
#include "GeoDataPlacemark.h"
#include "GeoDataStyle.h"
#include "PolylineAnnotation.h"


namespace Marble
{

class EditPolylineDialog::Private : public Ui::UiEditPolylineDialog
{
public:
    Private( PolylineAnnotation *polyline );
    ~Private();

    // Used to tell whether the settings before showing the dialog should be restored on
    // pressing the 'Cancel' button or not.
    bool m_firstEditing;

    QColorDialog *m_linesDialog;
    PolylineAnnotation *m_polyline;

    // Used to restore if the Cancel button is pressed.
    QString m_initialName;
    QString m_initialDescription;
    GeoDataLineStyle m_initialLineStyle;
};

EditPolylineDialog::Private::Private( PolylineAnnotation *polyline ) :
    Ui::UiEditPolylineDialog(),
    m_firstEditing( false ),
    m_linesDialog( 0 ),
    m_polyline( polyline )
{
    // nothing to do
}

EditPolylineDialog::Private::~Private()
{
    delete m_linesDialog;
}

EditPolylineDialog::EditPolylineDialog( PolylineAnnotation *polyline, QWidget *parent ) :
    QDialog( parent ) ,
    d ( new Private( polyline ) )
{
    d->setupUi( this );

    // If the polygon has just been drawn, assign it a default name.
    if ( d->m_polyline->placemark()->name().isNull() ) {
        d->m_polyline->placemark()->setName( tr("Untitled Path") );
    }


    d->m_name->setText( polyline->placemark()->name() );
    d->m_initialName = d->m_name->text();
    connect( d->m_name, SIGNAL(editingFinished()), this, SLOT(updatePolyline()) );

    d->m_description->setText( polyline->placemark()->description() );
    d->m_initialDescription = d->m_description->toPlainText();

    d->m_linesWidth->setRange( 0.1, 5.0 );


    d->m_nodesCheckBox->setChecked( d->m_polyline->paintingNodes() );
    connect( d->m_nodesCheckBox, SIGNAL(toggled(bool)), this, SLOT(setPaintingNodes(bool)) );
    connect( d->m_nodesCheckBox, SIGNAL(toggled(bool)), this, SLOT(updatePolyline()) );

    // Get the current style properties.
    const GeoDataLineStyle lineStyle = polyline->placemark()->style()->lineStyle();
    d->m_initialLineStyle = lineStyle;

    d->m_linesWidth->setValue( lineStyle.width() );
    connect( d->m_linesWidth, SIGNAL(editingFinished()), this, SLOT(updatePolyline()) );

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
    connect( d->m_linesDialog, SIGNAL(colorSelected(QColor)), this, SLOT(updatePolyline()) );

    // Promote "Ok" button to default button.
    d->buttonBox->button( QDialogButtonBox::Ok )->setDefault( true );

    connect( d->buttonBox->button( QDialogButtonBox::Ok ), SIGNAL(pressed()), this, SLOT(checkFields()) );
    connect( d->buttonBox, SIGNAL(accepted()), this, SLOT(updatePolyline()) );
    connect( d->buttonBox, SIGNAL(accepted()), this, SIGNAL(editingPolylineEnded()) );
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
    d->m_polyline->placemark()->setDescription( d->m_description->toPlainText() );
    d->m_polyline->placemark()->setName( d->m_name->text() );


    GeoDataStyle *newStyle = new GeoDataStyle( *d->m_polyline->placemark()->style() );
    newStyle->lineStyle().setColor( d->m_linesDialog->currentColor() );
    newStyle->lineStyle().setWidth( d->m_linesWidth->value() );
    d->m_polyline->placemark()->setStyle( newStyle );

    emit polylineUpdated( d->m_polyline->placemark() );
}

void EditPolylineDialog::updateLinesDialog( const QColor &color )
{
    QPixmap linesPixmap( d->m_linesColorButton->iconSize().width(),
                         d->m_linesColorButton->iconSize().height() );
    linesPixmap.fill( color );
    d->m_linesColorButton->setIcon( QIcon( linesPixmap ) );
}

void EditPolylineDialog::setPaintingNodes( bool painting )
{
    d->m_polyline->setPaintingNodes( painting );
}

void EditPolylineDialog::restoreInitial()
{
    // Make sure the polyline gets removed if the 'Cancel' button is pressed immediately after
    // the 'Add Path' has been clicked.
    if ( d->m_firstEditing ) {
        emit removeRequested();
        return;
    }

    if ( d->m_polyline->placemark()->name() != d->m_initialName ) {
        d->m_polyline->placemark()->setName( d->m_initialName );
    }

    if ( d->m_polyline->placemark()->description() != d->m_initialDescription ) {
        d->m_polyline->placemark()->setDescription( d->m_initialDescription );
    }

    if ( d->m_polyline->placemark()->style()->lineStyle() != d->m_initialLineStyle ) {
        GeoDataStyle *newStyle = new GeoDataStyle( *d->m_polyline->placemark()->style() );
        newStyle->setLineStyle( d->m_initialLineStyle );
        d->m_polyline->placemark()->setStyle( newStyle );
    }

    emit polylineUpdated( d->m_polyline->placemark() );
}

void EditPolylineDialog::checkFields()
{
    if ( d->m_name->text().isEmpty() ) {
        QMessageBox::warning( this,
                              tr( "No name specified" ),
                              tr( "Please specify a name for this polyline." ) );
    }
}

}

#include "EditPolylineDialog.moc"
