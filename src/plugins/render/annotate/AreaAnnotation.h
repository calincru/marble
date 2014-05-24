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

    virtual const char *graphicType() const;

    QList<int> &selectedNodes();

    int rightClickedNode() const;

private:
    int                m_movedNodeIndex;
    GeoDataCoordinates m_movedPointCoords;
    QList<int>         m_selectedNodes;
    int                m_rightClickedNode;

    const ViewportParams *m_viewport;

protected:
    /**
     * @brief In the implementation of these virtual functions, the following convention has  been
     * followed: if the event cannot be dealt with in this class (for example when right clicking
     * a node or polygon), the functions return false and AnnotatePlugin::eventFilter deals with it.
     */
    virtual bool mousePressEvent( QMouseEvent *event );
    virtual bool mouseMoveEvent( QMouseEvent *event );
    virtual bool mouseReleaseEvent( QMouseEvent *event );
};

}

#endif // AREAANNOTATION_H
