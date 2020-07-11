#!/usr/bin/env python3

""" Generate a "loading" or "waiting" animated gif. """

import math

import PIL.Image
import PIL.ImageDraw

SIZE = 16
TOTAL_DOTS = 8
VISUAL_DOTS = 4  # how many dots are visible in each frame.
DIAMETER = SIZE / 8.0
SECONDS = 1.25  # how long it takes to do a complete cycle.
OUTPUT = "loading.gif"


def draw_dot(draw, i, color):
    """ Draw a dot around a circle with a color. """
    # Positions around the big circle.
    unit_x = 1 + math.sin(i/TOTAL_DOTS * 2*math.pi)
    unit_y = 1 - math.cos(i/TOTAL_DOTS * 2*math.pi)

    # Scale to the desired size and circle diameter.
    x = round(((SIZE - DIAMETER)/2 - 1) * unit_x)
    y = round(((SIZE - DIAMETER)/2 - 1) * unit_y)

    # Center the pixels for the library.  (Doesn't appear to have
    # any effect at 16x16, but might possibly matter if we add
    # some kind of filter.)
    x += 0.5
    y += 0.5

    # These are drawn as rectangles in a 16x16 image, but if we
    # increase the size then they're circles.
    draw.ellipse((x, y, x + DIAMETER, y + DIAMETER), fill=color, width=1)


def draw_frame(framenum):
    """ Draw a frame of the animation. """
    # Create new image and drawing surface.
    image = PIL.Image.new('LA', (SIZE, SIZE), (1, 255))
    draw = PIL.ImageDraw.Draw(image)

    # Draw the dots.
    for i in range(VISUAL_DOTS):
        pos = ((framenum - i) % TOTAL_DOTS)

        # The Qt background is (239,239,239) so this fades from 0
        # to 240 (but stops at 180).
        gray = round(240/4*i)

        draw_dot(draw, pos, (gray, 0))
    return image


def main(filename):
    """ Generate the animation and save it to the filename. """
    # Generate frames.
    images = []
    for framenum in range(TOTAL_DOTS):
        image = draw_frame(framenum)
        images.append(image)

    # Write gif.
    images[0].save(filename, save_all=True, append_images=images[1:],
                   duration=SECONDS / TOTAL_DOTS * 1000,
                   loop=0,
                   transparency=1,
                   disposal=2)


if __name__ == "__main__":
    main(OUTPUT)
