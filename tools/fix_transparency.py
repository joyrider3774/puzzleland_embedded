#!/usr/bin/env python3
"""Put the transparent pixels of the skin images back to exactly magenta (255,0,255).

The game skips the pixels that are COLOR_TRANSPARENT, RGB565 0xF81F, which is magenta
(255,0,255) (see defines.h). Some of the images have been through a lossy format at some
point, which smeared the key colour along the edges of the sprites: (249,1,249),
(233,3,233), (204,41,204) and so on. Those pixels are not the key any more, so the game
draws them, and the sprite ends up with a pink fringe around it.

A pixel is put back to the key when it is clearly a smeared one: red and blue both high
and close together and green low. Nothing in these skins is painted in such a colour, the
purples of the player's hair are far away from it.

Usage:
  python fix_transparency.py            write the images back
  python fix_transparency.py --check    only say what would change, write nothing
"""
import argparse
import os
import sys

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
SKINS_DIR = os.path.join(HERE, "..", "assets", "skins")
KEY = (255, 0, 255)
SKINS = ("default", "black_white")


def is_smeared_key(pixel):
    """True for a pixel that was the magenta key before a lossy format got at it."""
    r, g, b = pixel
    return (pixel != KEY) and (r > 150) and (b > 150) and (g < 100) and (abs(r - b) < 30)


def fix(path, write):
    img = Image.open(path).convert("RGB")
    pixels = img.load()
    width, height = img.size
    changed = 0
    for y in range(height):
        for x in range(width):
            if is_smeared_key(pixels[x, y]):
                pixels[x, y] = KEY
                changed += 1
    if changed and write:
        img.save(path)
    return changed


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="say what would change, write nothing")
    parser.add_argument("skins", nargs="?", default=SKINS_DIR, help="the assets/skins folder")
    args = parser.parse_args()

    total = 0
    for skin in SKINS:
        folder = os.path.join(args.skins, skin)
        for png in sorted(os.listdir(folder)):
            if not png.endswith(".png"):
                continue
            changed = fix(os.path.join(folder, png), not args.check)
            if changed:
                total += changed
                print("%-12s %-24s %d pixel%s back to magenta" % (skin, png, changed, "" if changed == 1 else "s"))
    if not total:
        print("every transparent pixel is already magenta")
    elif args.check:
        print("%d pixels would change, nothing was written" % total)
    else:
        print("%d pixels changed, run convert_skins.py to rebuild the headers" % total)
    return 0


if __name__ == "__main__":
    sys.exit(main())
