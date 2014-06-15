//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2014      Calin-Cristian Cruceru <crucerucalincristian@gmail.com>
//

// Self
#include "EditPolygonDialog.h"
#include "ui_EditPolygonDialog.h"

// Marble
#include "GeoDataStyle.h"

// Qt
#include <QColorDialog>

namespace Marble {

class EditPolygonDialog::Private : public Ui::UiEditPolygonDialog
{
public:
    Private( GeoDataPlacemark *placemark );
    ~Private();

    GeoDataPlacemark *m_placemark;

    QColorDialog *m_linesDialog;
    QColorDialog *m_polyDialog;
};

EditPolygonDialog::Private::Private( GeoDataPlacemark *placemark ) :
    Ui::UiEditPolygonDialog(),
    m_placemark( placemark ),
    m_linesDialog( 0 ),
    m_polyDialog( 0 )
{
    // nothing to do
}

EditPolygonDialog::Private::~Private()
{
    // nothig to do
}

EditPolygonDialog::EditPolygonDialog( GeoDataPlacemark *placemark, QWidget *parent ) :
    QDialog( parent ),
    d( new Private( placemark ) )
{
    d->setupUi( this );

    d->m_name->setText( placemark->name() );
    d->m_description->setText( placemark->description() );

    d->m_linesWidth->setRange( 0.1, 5.0 );
    d->m_linesOpacity->setRange( 0, 100 );
    d->m_polyOpacity->setRange( 0, 100 );

    // Get the current style properties.
    const GeoDataLineStyle lineStyle = placemark->style()->lineStyle();
    const GeoDataPolyStyle polyStyle = placemark->style()->polyStyle();

    // Adjust the "Filled"/"Not Filled" option according to its fill.
    d->m_linesWidth->setValue( lineStyle.width() );
    if ( polyStyle.fill() ) {
        d->m_filledColor->setCurrentIndex( 0 );
    } else {
        d->m_filledColor->setCurrentIndex( 1 );
    }

    // Set the current opacity by using its QColor's alpha component.
    d->m_linesOpacity->setValue( qRound( lineStyle.color().alpha() / 2.55) );
    d->m_polyOpacity->setValue( qRound( polyStyle.color().alpha() / 2.55) );

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
    d->m_linesDialog = new QColorDialog( lineStyle.color(), this );
    d->m_polyDialog = new QColorDialog( polyStyle.color(), this );

    connect( d->m_linesColorButton, SIGNAL(clicked()), d->m_linesDialog, SLOT(exec()) );
    connect( d->m_linesDialog, SIGNAL(colorSelected(QColor)), this, SLOT(updateLinesDialog(const QColor&)) );
    connect( d->m_polyColorButton, SIGNAL(clicked()), d->m_polyDialog, SLOT(exec()) );
    connect( d->m_polyDialog, SIGNAL(colorSelected(QColor)), this, SLOT(updatePolyDialog(const QColor&)) );
    connect( d->buttonBox, SIGNAL(accepted()), this, SLOT(updatePolygon()) );
}

EditPolygonDialog::~EditPolygonDialog()
{
    // nothing to do
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
    QColor lineColor = d->m_linesDialog->currentColor();
    QColor polyColor = d->m_polyDialog->currentColor();

    lineColor.setAlpha( qRound( d->m_linesOpacity->value() * 2.55 ) );
    polyColor.setAlpha( qRound( d->m_polyOpacity->value() * 2.55 ) );

    style->lineStyle().setColor( lineColor );
    style->polyStyle().setColor( polyColor );


    d->m_placemark->setStyle( style );
    emit polygonUpdated();
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

}

#include "EditPolygonDialog.moc"
