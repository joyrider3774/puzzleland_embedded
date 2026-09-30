#!/usr/bin/env python3
"""Convert the level files in assets/levelpacks to source/puzzleland_embedded/levels.h.

Every sub folder of assets/levelpacks is a level pack, every .lev file in it a level.
A .lev file is a comma seperated list of decimal integers, Cols * Rows of them, written
column by column (X is the outer loop, Y the inner one) the way LoadLevel() reads them:

    for (X = 0; X < Cols; X++)
        for (Y = 0; Y < Rows; Y++)
            PlayField[0][X][Y] = <next value>;

The values are the block types, they are negative for the border pieces, so a level
becomes an int8_t array of exactly Cols * Rows entries: index [X * Rows + Y]. The size
is fixed, so there is no end marker.

Cols and Rows are read from defines.h, a .lev file with another number of values is an
error instead of a level that silently shifts a column.

Packs and levels are sorted naturally, so level2 comes after level1 and not after
level19, as level_data_files[pack][SelectedLevel - 1] is how the game finds a level.

Usage:
  python convert_levels.py            write levels.h
  python convert_levels.py --verify   build levels.h in memory and compare it with the
                                      current one, nothing is written
  --output FILE                       write (or verify) this file instead
"""
import argparse
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(HERE, "..")
LEVELS_DIR = os.path.join(ROOT, "assets", "levelpacks")
SOURCE_DIR = os.path.join(ROOT, "source", "puzzleland_embedded")
DEFINES = os.path.join(SOURCE_DIR, "defines.h")
OUTPUT = os.path.join(SOURCE_DIR, "levels.h")
DEFINES = os.path.join(SOURCE_DIR, "defines.h")
#the region of defines.h this tool owns, see build_switches()
BEGIN = "//>>> written by tools/convert_levels.py from assets/levelpacks, do not edit by hand"
END = "//<<<"


def playfield_size(defines=DEFINES):
    """(Cols, Rows) out of defines.h, so the tool and the game can not drift apart."""
    with open(defines, "r") as f:
        text = f.read()
    size = []
    for name in ("Cols", "Rows"):
        m = re.search(r"^#define\s+%s\s+(\d+)\s*$" % name, text, re.M)
        if not m:
            raise SystemExit("no #define %s in %s" % (name, os.path.relpath(defines, ROOT)))
        size.append(int(m.group(1)))
    return size[0], size[1]


def natural_key(name):
    """level2 < level10: digit runs compare as numbers."""
    return [int(part) if part.isdigit() else part.lower() for part in re.split(r"(\d+)", name)]


def c_name(pack, file_name):
    stem = os.path.splitext(file_name)[0]
    return "level_data_%s_%s_data" % (re.sub(r"\W", "_", pack), re.sub(r"\W", "_", stem))


def read_level(path, count):
    """The Cols * Rows block types of one .lev file, in the order LoadLevel() reads them."""
    with open(path, "r") as f:
        text = f.read()
    values = []
    for field in text.split(","):
        field = field.strip()
        if not field:
            continue  # the files end with a comma, and may end with a newline
        if not re.match(r"^-?\d+$", field):
            raise SystemExit("%s: %r is not a decimal integer" % (path, field))
        value = int(field)
        if not -128 <= value <= 127:
            raise SystemExit("%s: block type %d does not fit in an int8_t" % (path, value))
        values.append(value)
    if len(values) != count:
        raise SystemExit("%s: %d values, expected %d (Cols * Rows)" % (path, len(values), count))
    return values


def read_packs(levels_dir, count):
    """[(pack, [(file name, [values]), ...]), ...] in natural order."""
    packs = []
    for pack in sorted(os.listdir(levels_dir), key=natural_key):
        folder = os.path.join(levels_dir, pack)
        if not os.path.isdir(folder):
            continue
        levels = []
        for file_name in sorted(os.listdir(folder), key=natural_key):
            if not file_name.lower().endswith(".lev"):
                continue
            levels.append((file_name, read_level(os.path.join(folder, file_name), count)))
        if levels:
            packs.append((pack, levels))
    if not packs:
        raise SystemExit("no level packs with .lev files in %s" % os.path.relpath(levels_dir, ROOT))
    return packs


MAX_RUN = 128


def rle(data):
    """The control byte scheme of png2rle565.py, over the block types of a level.

        c & 0x80 : a run,     (c & 0x7F) + 1 copies of the byte that follows
        else     : a literal, c + 1 bytes follow

    A run of two is left as a literal: it would cost the same and reading it is more work."""
    out = bytearray()
    literal = []

    def flush():
        while literal:
            chunk = literal[:MAX_RUN]
            del literal[:MAX_RUN]
            out.append(len(chunk) - 1)
            out.extend(chunk)

    i = 0
    while i < len(data):
        run = 1
        while i + run < len(data) and run < MAX_RUN and data[i + run] == data[i]:
            run += 1
        if run >= 3:
            flush()
            out.append(0x80 | (run - 1))
            out.append(data[i])
            i += run
        else:
            literal.append(data[i])
            i += 1
    flush()
    return bytes(out)


def unrle(data, count):
    """The first count bytes the reader in game.cpp makes of it."""
    out = bytearray()
    i = 0
    while i < len(data) and len(out) < count:
        control = data[i]
        i += 1
        if control & 0x80:
            out.extend(bytes([data[i]]) * ((control & 0x7F) + 1))
            i += 1
        else:
            n = control + 1
            out.extend(data[i:i + n])
            i += n
    return bytes(out[:count])


LEVEL_SIZE = 20 * 16


def build_header(packs, levels_dir, cols, rows):
    max_items = max(len(levels) for _, levels in packs)
    lines = [
        "// Auto-generated by tools/convert_levels.py",
        "// Source directory: %s" % os.path.relpath(levels_dir, ROOT).replace(os.sep, "/"),
        "// Access as: level_data_files[group_index][item_index]  (const uint8_t*)",
        "// level_data_counts[group_index] is how many levels that pack has, unused slots",
        "// (packs with fewer than level_data_max_items levels) are padded with NULL so the",
        "// array stays rectangular.",
        "// Every level is exactly Cols * Rows = %d * %d = %d block types, one column after" % (cols, rows, cols * rows),
        "// another, so PlayField[0][X][Y] is at index [X * Rows + Y] (one line below is one",
        "// column). The types are negative for the border pieces, so a block type is cast back",
        "// to int8_t where it is read.",
        "// Every level is run length encoded, see rle() in the tool and the reader in game.cpp:",
        "//   c & 0x80 : a run,     (c & 0x7F) + 1 copies of the byte that follows",
        "//   else     : a literal, c + 1 bytes follow",
        "",
        "#pragma once",
        "#include <stdint.h>",
        "#include <stddef.h>",
        "",
        "#define level_data_groups %d" % len(packs),
        "#define level_data_max_items %d" % max_items,
        "#define level_data_size %d" % (cols * rows),
        "",
        "// Index map:",
    ]
    for g, (pack, levels) in enumerate(packs):
        lines.append("//   [%d] %s/" % (g, pack))
        for i, (file_name, _) in enumerate(levels):
            lines.append("//       [%d] %s" % (i, file_name))
    lines.append("")

    for pack, levels in packs:
        for file_name, values in levels:
            raw = bytes(v & 0xFF for v in values)
            encoded = rle(raw)
            #never write a level that does not come back out of the reader as it went in
            assert unrle(encoded, len(raw)) == raw, file_name
            lines.append("// %s/%s (%d block types, %d columns of %d, run length encoded to %d bytes)"
                         % (pack, file_name, len(values), cols, rows, len(encoded)))
            lines.append("const uint8_t %s[] PLATFORM_PROGMEM = {" % c_name(pack, file_name))
            body = ["    " + ", ".join("0x%02X" % b for b in encoded[off:off + 16])
                    for off in range(0, len(encoded), 16)]
            lines.append(",\n".join(body))
            lines.append("};")
            lines.append("")

    lines.append("// 2D lookup table: level_data_files[group_index][item_index]")
    lines.append("const uint8_t* const level_data_files[level_data_groups][level_data_max_items] = {")
    lines.append("// A room the run leaves out is null here and its array is named nowhere, so the")
    lines.append("// compiler drops it. The slots keep their places, so a room keeps its number.")
    for pack, levels in packs:
        lines.append("    {")
        for i, (file_name, _) in enumerate(levels):
            lines.append("#if LEVELBUILT(%d)" % i)
            lines.append("        %s," % c_name(pack, file_name))
            lines.append("#else")
            lines.append("        NULL,")
            lines.append("#endif")
        for _ in range(max_items - len(levels)):
            lines.append("        NULL,")
        lines.append("    },")
    lines.append("};")
    lines.append("")
    lines.append("// How many levels every pack really has")
    lines.append("const uint8_t level_data_counts[level_data_groups] = { %s };"
                 % ", ".join(str(len(levels)) for _, levels in packs))
    lines.append("")
    return "\n".join(lines) + "\n"


def build_switches(packs):
    """The run of rooms a build keeps.

    A room that is left out is named nowhere, so the compiler drops its array; its slot in the
    lookup table stays null, which is what tells the game it has not got that room."""
    total = sum(len(levels) for _, levels in packs)
    return "\n".join([
        "//FIRSTLEVEL and MAXLEVELS: the run of the %d rooms a build keeps, for a device with not" % total,
        "//the flash for all of them. Several builds whose runs follow one another hold the lot",
        "//between them, and a room keeps its number whichever build it is in, so a password names",
        "//the same room everywhere. 0 rooms means all of them from the first on",
        "#ifndef FIRSTLEVEL",
        "#define FIRSTLEVEL 0",
        "#endif",
        "#ifndef MAXLEVELS",
        "#define MAXLEVELS 0",
        "#endif",
        "//how many rooms the game has in all, whichever of them this build holds",
        "#define LEVELCOUNT %d" % total,
        "//1 while room n is in the build, counted from 0",
        "#define LEVELBUILT(n) (((n) >= FIRSTLEVEL) && ((MAXLEVELS == 0) || \\",
        "                       ((n) < FIRSTLEVEL + MAXLEVELS)))",
        "//how many rooms that leaves",
        "#define LEVELSKEPT ((%d <= FIRSTLEVEL) ? 0 : \\" % total,
        "                    (((MAXLEVELS == 0) || (%d - FIRSTLEVEL <= MAXLEVELS)) \\" % total,
        "                     ? %d - FIRSTLEVEL : MAXLEVELS))" % total,
        "#if LEVELSKEPT == 0",
        '#error "the run leaves no rooms at all, see FIRSTLEVEL and MAXLEVELS"',
        "#endif",
    ])


def splice(text, block):
    """Puts block between the markers, which is the part of defines.h this tool writes."""
    start = text.find(BEGIN)
    end = text.find(END, start + 1) if start >= 0 else -1
    if start < 0 or end < 0:
        raise SystemExit("the markers are gone from %s, put them back" % DEFINES)
    return text[:start] + BEGIN + "\n" + block + "\n" + text[end:]


def parse_header(text):
    """What the game gets out of a levels.h: the arrays (text and values) and the lookup table."""
    arrays = {}
    for m in re.finditer(r"(// [^\n]+\n)const uint8_t (\w+)\[\] PLATFORM_PROGMEM = \{\n(.*?)\n\};", text, re.S):
        stored = bytes(int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]{2})", m.group(3)))
        #what the game reads is the decoded level, so that is what is compared
        decoded = unrle(stored, LEVEL_SIZE)
        arrays[m.group(2)] = {
            "text": m.group(0),
            "values": [b - 256 if b > 127 else b for b in decoded],
        }
    m = re.search(r"level_data_files\[[^\]]*\]\[[^\]]*\] = \{\n(.*?)\n\};", text, re.S)
    table = None
    if m:
        #a row is what lies between the brace that opens it and the one that closes it, and names
        #every room of the pack whatever the run would leave out
        table = {"rows": [#only the room named in each slot, not the null the run would put there instead
                          re.findall(r"level_data_\w+", block)
                          for block in re.findall(r"\{\n(.*?)\n    \},", m.group(1), re.S)]}
    return arrays, table


def verify(new_text, path):
    with open(path, "r", newline="") as f:
        old_text = f.read()
    new_arrays, new_table = parse_header(new_text)
    old_arrays, old_table = parse_header(old_text)
    ok = True

    if old_table is None:
        print("  no level_data_files table found in %s" % path)
        return False
    new_dims = (len(new_table["rows"]), max(len(r) for r in new_table["rows"]))
    old_dims = (len(old_table["rows"]), max(len(r) for r in old_table["rows"]))
    if new_dims != old_dims:
        ok = False
        print("  DIFFERS  table size %s, current file has %s" % (new_dims, old_dims))
    for g, (new_row, old_row) in enumerate(zip(new_table["rows"], old_table["rows"])):
        for i, (new_name, old_name) in enumerate(zip(new_row, old_row)):
            if new_name != old_name:
                ok = False
                print("  DIFFERS  level_data_files[%d][%d] = %s, current file has %s" % (g, i, new_name, old_name))
    print("  lookup table %dx%d: %s" % (new_dims + ("identical" if ok else "differs",)))

    same = 0
    for name in sorted(set(new_arrays) | set(old_arrays), key=natural_key):
        if name not in old_arrays:
            ok = False
            print("  MISSING  %s is not in the current file" % name)
        elif name not in new_arrays:
            ok = False
            print("  EXTRA    %s is in the current file but made from no .lev file" % name)
        elif new_arrays[name]["values"] != old_arrays[name]["values"]:
            ok = False
            print("  DIFFERS  %s has other level data" % name)
        elif new_arrays[name]["text"] != old_arrays[name]["text"]:
            ok = False
            print("  DIFFERS  %s has the same block types but is formatted differently" % name)
        else:
            same += 1
    print("  level arrays: %d of %d identical (data and text)" % (same, len(new_arrays)))

    if ok and new_text != old_text:
        print("  note: the file text differs outside the arrays and the table (generator line, index map")
        print("        comment or the order the arrays are defined in), this has no effect on the game")
    return ok


def main():
    parser = argparse.ArgumentParser(description="Convert assets/levelpacks to levels.h")
    parser.add_argument("--verify", action="store_true", help="compare with the current levels.h, write nothing")
    parser.add_argument("--output", default=OUTPUT, help="levels.h to write or verify")
    args = parser.parse_args()

    cols, rows = playfield_size()
    packs = read_packs(LEVELS_DIR, cols * rows)
    text = build_header(packs, LEVELS_DIR, cols, rows)
    summary = ", ".join("%s %d" % (pack, len(levels)) for pack, levels in packs)

    switches = build_switches(packs)
    with open(DEFINES, "r", newline="") as f:
        defines_raw = f.read()
    defines_old = defines_raw.replace("\r\n", "\n")
    defines_new = splice(defines_old, switches)

    if args.verify:
        print("verifying %s against assets/levelpacks (%s)" % (os.path.relpath(args.output, ROOT), summary))
        ok = verify(text, args.output)
        print("everything matches" if ok else "there are differences")
        return 0 if ok else 1

    with open(args.output, "w", newline="\n") as f:
        f.write(text)
    with open(DEFINES, "w", newline="\r\n") as f:
        f.write(defines_new)
    print("wrote %s (%s)" % (os.path.relpath(args.output, ROOT), summary))
    return 0


if __name__ == "__main__":
    sys.exit(main())
