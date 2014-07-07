To FIX:
- the problem regarding removing nodes when the polygon has inner boundaries: it
  is caused by the fact that GeoDataLinearRing::contains does not take into
  consideration that the polygon is Tessellated.
- the problem regarding moving the polygon around poles (and moving in general
  which changes polygon's shape).

To ADD:
- adding nodes (the structure is done, there are still a few problems):
    -> in the end, maybe take into consideration showing the virtual node
    everywhere on a polygon's side.
- a more friendly user interface;
