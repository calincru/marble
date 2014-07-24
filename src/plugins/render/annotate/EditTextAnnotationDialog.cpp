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
#include "EditTextAnnotationDialog.h"
#include "ui_EditTextAnnotationDialog.h"

// Qt
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QColorDialog>

// Marble
#include "GeoDataStyle.h"
#include "GeoDataPlacemark.h"
#include "PlacemarkTextAnnotation.h"

#include <QDebug>

namespace Marble {

class EditTextAnnotationDialog::Private : public Ui::UiEditTextAnnotationDialog
{
public:
    Private( PlacemarkTextAnnotation *textAnnotation );
    ~Private();

    PlacemarkTextAnnotation *m_textAnnotation;

    QColorDialog *m_iconColorDialog;
    QColorDialog *m_labelColorDialog;
};

EditTextAnnotationDialog::Private::Private( PlacemarkTextAnnotation *textAnnotation ) :
    Ui::UiEditTextAnnotationDialog(),
    m_textAnnotation( textAnnotation )
{
    // nothing to do
}

EditTextAnnotationDialog::Private::~Private()
{
    // nothing to do
}

EditTextAnnotationDialog::EditTextAnnotationDialog( PlacemarkTextAnnotation *textAnnotation, QWidget *parent ) :
    QDialog( parent ),
    d( new Private( textAnnotation ) )
{
    d->setupUi( this );

    // If the placemark has just been created, assign it a default name.
    if ( textAnnotation->placemark()->name().isNull() ) {
        textAnnotation->placemark()->setName( tr("Untitled Placemark") );
    }

    // Initialize the range for label/icon size.
    // FIXME: What should be the maximum size?
    d->m_labelScale->setRange( 1.0, 5.0 );
    d->m_iconScale->setRange( 1.0, 5.0 );
    // Initialize the range for latitude/longitude.
    d->m_latitude->setRange( -90, 90 );
    d->m_longitude->setRange( -180, 180 );


    // Setup name, icon link and latitude/longitude values.
    d->m_name->setText( textAnnotation->placemark()->name() );
    d->m_link->setText( textAnnotation->m_iconFilename );

    d->m_latitude->setValue( textAnnotation->placemark()->coordinate().latitude( GeoDataCoordinates::Degree ) );
    connect( d->m_latitude, SIGNAL(editingFinished()), this, SLOT(updateTextAnnotation()) );
    d->m_longitude->setValue( textAnnotation->placemark()->coordinate().longitude( GeoDataCoordinates::Degree ) );
    connect( d->m_longitude, SIGNAL(editingFinished()), this, SLOT(updateTextAnnotation()) );


    // Adjust icon and label scales.
    d->m_iconScale->setValue( textAnnotation->placemark()->style()->iconStyle().scale() );
    connect( d->m_iconScale, SIGNAL(editingFinished()), this, SLOT(updateTextAnnotation()) );

    d->m_labelScale->setValue( textAnnotation->placemark()->style()->labelStyle().scale() );
    connect( d->m_labelScale, SIGNAL(editingFinished()), this, SLOT(updateTextAnnotation()) );


    // Adjust the current color of the two push buttons' pixmap to resemble the label and icon colors.
    const GeoDataLabelStyle labelStyle = textAnnotation->placemark()->style()->labelStyle();
    const GeoDataIconStyle iconStyle = textAnnotation->placemark()->style()->iconStyle();

    QPixmap labelPixmap( d->m_labelButton->iconSize().width(),
                         d->m_labelButton->iconSize().height() );
    labelPixmap.fill( labelStyle.color() );
    d->m_labelButton->setIcon( QIcon( labelPixmap ) );

    QPixmap iconPixmap( d->m_iconButton->iconSize().width(),
                        d->m_iconButton->iconSize().height() );
    iconPixmap.fill( iconStyle.color() );
    d->m_iconButton->setIcon( QIcon( iconPixmap ) );

    // Setup the color dialogs.
    d->m_labelColorDialog = new QColorDialog( this );
    d->m_labelColorDialog->setOption( QColorDialog::ShowAlphaChannel );
    d->m_labelColorDialog->setCurrentColor( labelStyle.color() );
    connect( d->m_labelButton, SIGNAL(clicked()), d->m_labelColorDialog, SLOT(exec()) );
    connect( d->m_labelColorDialog, SIGNAL(colorSelected(QColor)), this, SLOT(updateLabelDialog(const QColor&)) );
    connect( d->m_labelColorDialog, SIGNAL(colorSelected(QColor)), this, SLOT(updateTextAnnotation()) );

    d->m_iconColorDialog = new QColorDialog( this );
    d->m_iconColorDialog->setOption( QColorDialog::ShowAlphaChannel );
    d->m_iconColorDialog->setCurrentColor( iconStyle.color() );
    connect( d->m_iconButton, SIGNAL(clicked()), d->m_iconColorDialog, SLOT(exec()) );
    connect( d->m_iconColorDialog, SIGNAL(colorSelected(QColor)), this, SLOT(updateIconDialog(const QColor&)) );
    connect( d->m_iconColorDialog, SIGNAL(colorSelected(QColor)), this, SLOT(updateTextAnnotation()) );


    connect( d->m_browseButton, SIGNAL(pressed()), this, SLOT(loadIconFile()) );
    connect( d->buttonBox->button( QDialogButtonBox::Ok ), SIGNAL(pressed()), this, SLOT(checkFields()) );

    // Ensure that the dialog gets deleted when closing it (either when clicking OK or
    // Close).
    connect( this, SIGNAL(finished(int)), SLOT(deleteLater()) );
}

EditTextAnnotationDialog::~EditTextAnnotationDialog()
{
    delete d;
}

void EditTextAnnotationDialog::updateTextAnnotation()
{


    emit textAnnotationUpdated( d->m_textAnnotation->placemark() );
}

void EditTextAnnotationDialog::loadIconFile()
{
    const QString filename = QFileDialog::getOpenFileName( this,
                                                           tr( "Open Annotation File" ),
                                                           QString(),
                                                           tr( "All Supported Files (*.png)" ) );
    if ( filename.isNull() ) {
        return;
    }

    d->m_link->setText( filename );
}

void EditTextAnnotationDialog::checkFields()
{
    if ( d->m_name->text().isEmpty() ) {
        QMessageBox::warning( this,
                              tr( "No name specified" ),
                              tr( "Please specify a name for this placemark." ) );
    }

    if ( d->m_link->text().isEmpty() ) {
        QMessageBox::warning( this,
                              tr( "No image specified" ),
                              tr( "Please specify an icon for this placemark." ) );
    }

    QFileInfo fileInfo( d->m_link->text() );
    if ( !fileInfo.exists() ) {
        QMessageBox::warning( this,
                              tr( "Invalid icon path" ),
                              tr( "Please specify a valid path for the icon file." ) );
    }
}

void EditTextAnnotationDialog::updateLabelDialog( const QColor &color )
{
    QPixmap labelPixmap( d->m_labelButton->iconSize().width(),
                         d->m_labelButton->iconSize().height() );
    labelPixmap.fill( color );
    d->m_labelButton->setIcon( QIcon( labelPixmap ) );
}

void EditTextAnnotationDialog::updateIconDialog( const QColor &color )
{
    QPixmap iconPixmap( d->m_iconButton->iconSize().width(),
                        d->m_iconButton->iconSize().height() );
    iconPixmap.fill( color );
    d->m_iconButton->setIcon( QIcon( iconPixmap ) );
}

}

#include "EditTextAnnotationDialog.moc"
