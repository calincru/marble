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


namespace Marble {

class AreaAnnotation;

class MergingNodesAnimation : public QObject {

    Q_OBJECT
public:
    explicit MergingNodesAnimation( AreaAnnotation *polygon );
    ~MergingNodesAnimation();

public slots:
    void animationStarted();

signals:
    void nodesMoved();
    void startAnimation();

private:
    AreaAnnotation *m_polygon;
};

} // namespace Marble





#endif // MERGINGNODESANIMATION_H
