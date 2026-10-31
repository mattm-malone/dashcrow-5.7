#!/usr/bin/env python3
"""Convert a binary PBM (P4) to PNG: uv run --with pillow tools/pbm2png.py in.pbm out.png"""
import sys
from PIL import Image

Image.open(sys.argv[1]).convert("1").save(sys.argv[2])
