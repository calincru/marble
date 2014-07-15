//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2014       Calin Cruceru  <crucerucalincristian@gmail.com>
//

#ifndef MERGINGNODESANIMATION_H
#define MERGINGNODESANIMATION_H

#include <QObject>
#include <QTimer>


namespace Marble {

class AreaAnnotation;
class GeoDataCoordinates;

class MergingNodesAnimation : public QObject {

    Q_OBJECT
public:
    explicit MergingNodesAnimation( AreaAnnotation *polygon );
    ~MergingNodesAnimation();

    enum NodesBoundary {
        InnerBoundary,
        OuterBoundary
    };

public slots:
    void startAnimation();

signals:
    void nodesMoved();
    void animationFinished();

private slots:
    void updateNodes();

private:
    qreal nodesDistance();
    GeoDataCoordinates newCoords();

    AreaAnnotation *m_polygon;
    QTimer         *m_timer;
    NodesBoundary   m_boundary;

    static const int   timeOffset;
    static const qreal distanceOffset;
};

} // namespace Marble





#endif // MERGINGNODESANIMATION_H
