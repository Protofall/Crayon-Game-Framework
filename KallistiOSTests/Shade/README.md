# Colour shading/blending test

This is a basic shading example showing how to use argb/orgb parameters to get a "fade-to-colour" effect with "add" and "blend" versions. The `orgb` parameter needs a certain bit in the `pvr_poly_hdr_t` set in order to work.

This example was modified by DCemulation formum member "TapamN". You can find his post here: http://dcemulation.org/phpBB/viewtopic.php?f=29&t=104921&p=1055904#p1055904

This is basically what he had to say on the matter:

The "fade-to-colour" effect is basically alpha blending a solid color on top of the sprite. For that, oargb is what you need here.
So the formula of the effect looks like this:

`Resulting color = texsample * (1-alpha) + fadecolor * alpha`

On the Dreamcast, the argb and oargb values are mixed with the texture color like this:

`Resulting color = texsample * argb + oargb;`

This is intended to let you preform lighting equations that work like "texsample * diffuse + specular". However, by setting argb to (1-alpha) and oargb to Fadecolor * alpha, you can recreate the formula used for the fade-to-colour effect.

In this example "Blend" mode is that "fade-to-colour" mode I wanted, but he also shows "add" mode which applies the colour ontop of your texture.

