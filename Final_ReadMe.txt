Owen Gebhardt
12/15/2024

For my final graphical effect, I decided to replicate the dithering effect with camera rotation tracking seen in Return of the Obra Dinn (2018).
The process is described in this TIGSource post by the game's developer, Lucas Pope: https://forums.tigsource.com/index.php?topic=40832.msg1363742#msg1363742

The effect is accomplished with a standard comparison to a dither map, but offset to combat visual "swimming" of the dither pattern caused by camera rotation.

Dither is applied after the box blur, and both can be activated/deactivated independently of one another.

To see dither settings, open Post-Processing > Dither in the ImGui Inspector.



DITHER SETTINGS

- Run Dither?:			Applies the dither post-process when checked. Skips the dither post-process when unchecked.
- Dither Pixel Size:	Downscaling factor; describes the number of display pixels that each pixel of the dithered image should occupy.
- Bias:					Biases the base render vs. dither map lightness check. Makes the overall image lighter or darker.
- Light Color:			The color to fill in pixels lighter than the dither map.
- Dark Color:			The color to fill in pixels darker than the dither map.
- Dither Map:			The texture to use for dithering.
	- Bayer Matrix (2-8px):		Bayer matrix with varying levels of detail. Higher pixel counts can detect more gradations in lightness. Camera rotations may cause significant flickering.
	- [RECOMMENDED] Blue Noise:	Blue noise texture. Offers more natural randomness than Bayer matrices and causes less flickering.
	- Test Bayer Matrix (8px):	8px Bayer matrix with reference markings to track dither map movement with camera rotation.
