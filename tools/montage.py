#!/usr/bin/env python3
"""Contact sheet of preview PBMs: uv run --with pillow tools/montage.py out.png in1.pbm in2.pbm ...
Two columns, each frame at half size with a 1 px border."""
import sys
from PIL import Image, ImageDraw

out, files = sys.argv[1], sys.argv[2:]
frames = [Image.open(f).convert("1").convert("L") for f in files]
w, h = frames[0].size
tw, th = w // 2, h // 2
cols = 2
rows = (len(frames) + cols - 1) // cols
sheet = Image.new("L", (cols * (tw + 6) + 6, rows * (th + 6) + 6), 160)
for i, fr in enumerate(frames):
    x, y = 6 + (i % cols) * (tw + 6), 6 + (i // cols) * (th + 6)
    sheet.paste(fr.resize((tw, th), Image.LANCZOS), (x, y))
sheet.save(out)
