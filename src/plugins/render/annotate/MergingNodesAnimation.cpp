//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2014       Calin Cruceru  <crucerucalincristian@gmail.com>
//

// Self
#include "MergingNodesAnimation.h"

// Marble
#include "AreaAnnotation.h"

namespace Marble {

MergingNodesAnimation::MergingNodesAnimation( AreaAnnotation *polygon ) :
    m_polygon( polygon )
{
    connect( this, SIGNAL(startAnimation()), this, SLOT(animationStarted()) );
}

MergingNodesAnimation::~MergingNodesAnimation()
{
    // nothing to do
}

void MergingNodesAnimation::animationStarted()
{

}

} // namespace Marble
