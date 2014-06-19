To FIX:
- the problem regarding removing nodes when the polygon has inner boundaries: it
  is caused by the fact that GeoDataLinearRing::contains does not take into
  consideration that the polygon is Tessellated.
- the problem regarding moving the polygon around poles (and moving in general
  which changes polygon's shape).


To ADD:
- merging nodes - showing error when the polygon becomes invalid (regarding
  its inner boundaries).
- adding nodes;
- a more friendly user interface;
- redesigning the whole event filter in AnnotatePlugin: find a why to keep the
  code easier to upgrade and better documented as well as easier to be
  understood by newcomers (new contributors).
