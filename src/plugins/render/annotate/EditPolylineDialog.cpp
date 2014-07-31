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
    Private();
    ~Private();

};

EditPolylineDialog::Private::Private() :
    Ui::UiEditPolylineDialog()
{

}

EditPolylineDialog::Private::~Private()
{

}

EditPolylineDialog::EditPolylineDialog() :
    d (0)
{

}

EditPolylineDialog::~EditPolylineDialog()
{

}

}

#include "EditPolylineDialog.moc"
