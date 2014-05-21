//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Andrew Manson            <g.real.ate@gmail.com>
// Copyright 2013      Thibaut Gridel           <tgridel@free.fr>
// Copyright 2014      Calin-Cristian Cruceru   <crucerucalincristian@gmail.com
//

#ifndef AREAANNOTATION_H
#define AREAANNOTATION_H

#include "SceneGraphicsItem.h"
#include "GeoDataCoordinates.h"

namespace Marble
{

class AreaAnnotation : public SceneGraphicsItem
{
public:
    explicit AreaAnnotation( GeoDataPlacemark *placemark );

    virtual void paint( GeoPainter *painter, const ViewportParams *viewport );
    virtual bool mousePressEvent( QMouseEvent *event );
    virtual bool mouseMoveEvent( QMouseEvent *event );
    virtual bool mouseReleaseEvent( QMouseEvent *event );

    QList<int> selectedNodes() const;

private:
    int m_movedNode;
    int m_rmbNode;
    bool m_movingPolygon;
    QList<int> m_selectedNodes;
    GeoDataCoordinates m_movedNodeCoords;
    const ViewportParams *m_viewport;
};

}

#endif // AREAANNOTATION_H
