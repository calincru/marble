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
#include "EditPolygonDialog.h"
#include "ui_EditPolygonDialog.h"

// Qt
#include <QColorDialog>
#include <QMessageBox>

// Marble
#include "GeoDataStyle.h"


namespace Marble {

class EditPolygonDialog::Private : public Ui::UiEditPolygonDialog
{
public:
    Private( GeoDataPlacemark *placemark );
    ~Private();

    GeoDataPlacemark *m_placemark;
    bool m_firstEditing;

    QColorDialog *m_linesDialog;
    QColorDialog *m_polyDialog;

    QString m_initialDescription;
    QString m_initialName;
    GeoDataStyle m_initialStyle;
};

EditPolygonDialog::Private::Private( GeoDataPlacemark *placemark ) :
    Ui::UiEditPolygonDialog(),
    m_placemark( placemark ),
    m_firstEditing( false ),
    m_linesDialog( 0 ),
    m_polyDialog( 0 )
{
    // nothing to do
}

EditPolygonDialog::Private::~Private()
{
    delete m_linesDialog;
    delete m_polyDialog;
}

EditPolygonDialog::EditPolygonDialog( GeoDataPlacemark *placemark, QWidget *parent ) :
    QDialog( parent ),
    d( new Private( placemark ) )
{
    d->setupUi( this );

    d->m_initialStyle = *placemark->style();

    // If the polygon has just been drawn, assign it a default name.
    if ( d->m_placemark->name().isNull() ) {
        d->m_placemark->setName( tr("Untitled Polygon") );
    }
    d->m_name->setText( placemark->name() );
    d->m_initialName = placemark->name();
    connect( d->m_name, SIGNAL(editingFinished()), this, SLOT(updatePolygon()) );

    d->m_description->setText( placemark->description() );
    d->m_initialDescription = placemark->description();

    // Get the current style properties.
    const GeoDataLineStyle lineStyle = placemark->style()->lineStyle();
    const GeoDataPolyStyle polyStyle = placemark->style()->polyStyle();

    d->m_linesWidth->setRange( 0.1, 5.0 );
    d->m_linesWidth->setValue( lineStyle.width() );
    connect( d->m_linesWidth, SIGNAL(editingFinished()), this, SLOT(updatePolygon()) );

    // Adjust the "Filled"/"Not Filled" option according to its current fill.
    if ( polyStyle.fill() ) {
        d->m_filledColor->setCurrentIndex( 0 );
    } else {
        d->m_filledColor->setCurrentIndex( 1 );
    }
    connect( d->m_filledColor, SIGNAL(currentIndexChanged(int)), this, SLOT(updatePolygon()) );

    // Adjust the color buttons' icons to the current lines and polygon colors.
    QPixmap linesPixmap( d->m_linesColorButton->iconSize().width(),
                         d->m_linesColorButton->iconSize().height() );
    linesPixmap.fill( lineStyle.color() );
    d->m_linesColorButton->setIcon( QIcon( linesPixmap ) );

    QPixmap polyPixmap( d->m_polyColorButton->iconSize().width(),
                        d->m_polyColorButton->iconSize().height() );
    polyPixmap.fill( polyStyle.color() );
    d->m_polyColorButton->setIcon( QIcon( polyPixmap ) );

    // Setup the color dialogs.
    d->m_linesDialog = new QColorDialog( this );
    d->m_linesDialog->setOption( QColorDialog::ShowAlphaChannel );
    d->m_linesDialog->setCurrentColor( lineStyle.color() );
    connect( d->m_linesColorButton, SIGNAL(clicked()), d->m_linesDialog, SLOT(exec()) );
    connect( d->m_linesDialog, SIGNAL(colorSelected(QColor)), this, SLOT(updateLinesDialog(const QColor&)) );
    connect( d->m_linesDialog, SIGNAL(colorSelected(QColor)), this, SLOT(updatePolygon()) );

    d->m_polyDialog = new QColorDialog( this );
    d->m_polyDialog->setOption( QColorDialog::ShowAlphaChannel );
    d->m_polyDialog->setCurrentColor( polyStyle.color() );
    connect( d->m_polyColorButton, SIGNAL(clicked()), d->m_polyDialog, SLOT(exec()) );
    connect( d->m_polyDialog, SIGNAL(colorSelected(QColor)), this, SLOT(updatePolyDialog(const QColor&)) );
    connect( d->m_polyDialog, SIGNAL(colorSelected(QColor)), this, SLOT(updatePolygon()) );

    // Promote "Ok" button to default button.
    d->buttonBox->button( QDialogButtonBox::Ok )->setDefault( true );

    connect( d->buttonBox->button( QDialogButtonBox::Ok ), SIGNAL(pressed()), this, SLOT(checkFields()) );
    connect( d->buttonBox, SIGNAL(accepted()), this, SLOT(updatePolygon()) );
    connect( d->buttonBox, SIGNAL(rejected()), this, SLOT(restoreInitial()) );

    // Ensure that the dialog gets deleted when closing it (either when clicking OK or
    // Close).
    connect( this, SIGNAL(finished(int)), SLOT(deleteLater()) );
}

EditPolygonDialog::~EditPolygonDialog()
{
    delete d;
}

void EditPolygonDialog::setFirstTimeEditing( bool enabled )
{
    d->m_firstEditing = enabled;
}

void EditPolygonDialog::updatePolygon()
{
    GeoDataStyle *style = new GeoDataStyle( *d->m_placemark->style() );

    d->m_placemark->setName( d->m_name->text() );
    d->m_placemark->setDescription( d->m_description->toPlainText() );

    style->lineStyle().setWidth( d->m_linesWidth->value() );
    // 0 corresponds to "Filled" and 1 corresponds to "Not Filled".
    style->polyStyle().setFill( !d->m_filledColor->currentIndex() );


    // Adjust the lines/polygon colors.
    // QColorDialog::currentColor() also works even if the color dialog
    // has not been exec'ed, while QColorDialog::selectedColor() does not.
    style->lineStyle().setColor( d->m_linesDialog->currentColor() );
    style->polyStyle().setColor( d->m_polyDialog->currentColor() );


    d->m_placemark->setStyle( style );
    emit polygonUpdated( d->m_placemark );
}

void EditPolygonDialog::updateLinesDialog( const QColor &color )
{
    QPixmap linesPixmap( d->m_linesColorButton->iconSize().width(),
                         d->m_linesColorButton->iconSize().height() );
    linesPixmap.fill( color );
    d->m_linesColorButton->setIcon( QIcon( linesPixmap ) );
}

void EditPolygonDialog::updatePolyDialog( const QColor &color )
{
    QPixmap polyPixmap( d->m_polyColorButton->iconSize().width(),
                        d->m_polyColorButton->iconSize().height() );
    polyPixmap.fill( color );
    d->m_polyColorButton->setIcon( QIcon( polyPixmap ) );
}

void EditPolygonDialog::checkFields()
{
    if ( d->m_name->text().isEmpty() ) {
        QMessageBox::warning( this,
                              tr( "No name specified" ),
                              tr( "Please specify a name for this polygon." ) );
    }
}

void EditPolygonDialog::restoreInitial()
{
    // Make sure the polygon gets removed if the 'Cancel' button is pressed immediately after
    // the 'Add Placemark' has been clicked.
    if ( d->m_firstEditing ) {
        emit removeRequested();
        return;
    }

    if ( d->m_placemark->name() != d->m_initialName ) {
        d->m_placemark->setName( d->m_initialName );
    }

    if ( d->m_placemark->description() != d->m_initialDescription ) {
        d->m_placemark->setDescription( d->m_initialDescription );
    }

    if ( *d->m_placemark->style() != d->m_initialStyle ) {
        d->m_placemark->setStyle( new GeoDataStyle( d->m_initialStyle ) );
    }

    emit polygonUpdated( d->m_placemark );
}

}

#include "EditPolygonDialog.moc"
