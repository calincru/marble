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

// Marble
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

    if ( d->m_textAnnotation->placemark()->name().isNull() ) {
        d->m_textAnnotation->placemark()->setName( tr("Untitled Placemark") );
    }

    // What should be the maximum size?
    d->m_labelSize->setRange( 0.1, 5 );
    d->m_iconSize->setRange( 0.1, 5 );
    d->m_latitude->setRange( -90, 90 );
    d->m_longitude->setRange( -180, 180 );

    d->m_name->setText( d->m_textAnnotation->placemark()->name() );
    d->m_link->setText( d->m_textAnnotation->m_iconFilename );
    d->m_latitude->setValue( d->m_textAnnotation->placemark()->coordinate().latitude( GeoDataCoordinates::Degree ) );
    d->m_longitude->setValue( d->m_textAnnotation->placemark()->coordinate().longitude( GeoDataCoordinates::Degree ) );

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

void EditTextAnnotationDialog::modifyTextAnnotation()
{

    emit textAnnotationModified( d->m_textAnnotation->placemark() );
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

}

#include "EditTextAnnotationDialog.moc"
