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

// Marble
#include "GeoDataPlacemark.h"
#include "PlacemarkTextAnnotation.h"


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

    d->m_name->setText( d->m_textAnnotation->placemark()->name() );

    // to be continued
    connect( d->m_browseButton, SIGNAL(pressed()), this, SLOT(loadIconFile()) );
}

EditTextAnnotationDialog::~EditTextAnnotationDialog()
{
    delete d;
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

}

#include "EditTextAnnotationDialog.moc"
