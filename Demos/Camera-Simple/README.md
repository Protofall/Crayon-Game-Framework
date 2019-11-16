# Camera-Simple

This is based on the Basic-Scene demo, but using cameras/viewports to show how it works. There are 4 cameras demo-ed here:
- Full screen, no scale: Effectively the same as not even using a camera at all
- Part of the screen, no scale: Showcases the vertex/UV cropping. The camera region is displayed with the light grey poly
- Zoomed-out (x2): Shows what stuff likes like with a zoomed out camera. Note the faces and green men shimmer because their base scale is 3 and 3/2 = 1.5 which isn't a nice integer so you get the effect
- Zoomed-in (x2): Stuff appears twice as large as before

