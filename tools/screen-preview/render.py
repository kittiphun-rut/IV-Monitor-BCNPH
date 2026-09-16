#!/usr/bin/env python3
"""เรนเดอร์คำสั่งวาดจากเฟิร์มแวร์ (ops.txt) ออกเป็นภาพพรีวิวหน้าจอ TFT 172x320"""
import sys, os
from PIL import Image, ImageDraw, ImageFont

W, H, S = 172, 320, 3
FONT_PATH = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"
_fonts = {}

def font_for(size):
    if size not in _fonts:
        _fonts[size] = ImageFont.truetype(FONT_PATH, int(round(7 * size * S * 1.18)))
    return _fonts[size]

def rgb(c):
    c = int(c)
    r = (c >> 11) & 0x1F; g = (c >> 5) & 0x3F; b = c & 0x1F
    return ((r * 255) // 31, (g * 255) // 63, (b * 255) // 31)

def main(ops_path, outdir):
    os.makedirs(outdir, exist_ok=True)
    img = Image.new("RGB", (W * S, H * S), (0, 0, 0))
    d = ImageDraw.Draw(img)
    saved = []
    for line in open(ops_path, encoding="utf-8"):
        p = line.rstrip("\n").split(" ")
        op = p[0]
        if op == "FILLSCREEN":
            d.rectangle([0, 0, W * S, H * S], fill=rgb(p[1]))
        elif op == "RECT":
            x, y, w, h, c, fill = int(p[1]), int(p[2]), int(p[3]), int(p[4]), p[5], p[6] == "1"
            box = [x * S, y * S, (x + w) * S - 1, (y + h) * S - 1]
            if fill: d.rectangle(box, fill=rgb(c))
            else:    d.rectangle(box, outline=rgb(c), width=S)
        elif op == "RRECT":
            x, y, w, h, r, c, fill = int(p[1]), int(p[2]), int(p[3]), int(p[4]), int(p[5]), p[6], p[7] == "1"
            box = [x * S, y * S, (x + w) * S - 1, (y + h) * S - 1]
            if fill: d.rounded_rectangle(box, radius=r * S, fill=rgb(c))
            else:    d.rounded_rectangle(box, radius=r * S, outline=rgb(c), width=S)
        elif op == "LINE":
            d.line([int(p[1]) * S, int(p[2]) * S, int(p[3]) * S, int(p[4]) * S], fill=rgb(p[5]), width=S)
        elif op == "CIRC":
            x, y, r = int(p[1]), int(p[2]), int(p[3])
            d.ellipse([(x - r) * S, (y - r) * S, (x + r) * S, (y + r) * S], fill=rgb(p[4]))
        elif op == "TEXT":
            x, y, size, col, bg = int(p[1]), int(p[2]), int(p[3]), p[4], p[5]
            text = " ".join(p[6:]) if len(p) > 6 else ""
            cw, ch = 6 * size * S, 8 * size * S
            f = font_for(size)
            for i, chpr in enumerate(text):
                cx = x * S + i * cw
                d.rectangle([cx, y * S, cx + cw - 1, y * S + ch - 1], fill=rgb(bg))
                if chpr != " ":
                    d.text((cx + cw // 2, y * S + ch // 2), chpr, font=f, fill=rgb(col), anchor="mm")
        elif op == "MARK":
            name = p[1]
            out = os.path.join(outdir, f"{name}.png")
            img.save(out)
            saved.append(out)
    print("\n".join(saved))

if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
