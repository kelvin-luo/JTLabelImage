"""Generate toolbar icons and app.ico for JTLabelImage."""
from pathlib import Path

from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parent.parent / "resources" / "icons"
ACCENT = (74, 158, 255)
FG = (230, 235, 245)
BG = (45, 48, 56)


def new_icon(size=24):
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    return img, ImageDraw.Draw(img)


def save(name, img):
    ROOT.mkdir(parents=True, exist_ok=True)
    img.save(ROOT / name, format="PNG")


def icon_select(s=24):
    img, d = new_icon(s)
    m = s // 8
    d.rounded_rectangle([m, m, s - m, s - m], radius=3, outline=ACCENT, width=2)
    d.line([(s // 2, m + 2), (s - m - 2, s // 2)], fill=FG, width=2)
    return img


def icon_point(s=24):
    img, d = new_icon(s)
    r = s // 6
    d.ellipse([s // 2 - r, s // 2 - r, s // 2 + r, s // 2 + r], fill=ACCENT)
    return img


def icon_line(s=24):
    img, d = new_icon(s)
    m = s // 5
    d.line([(m, s - m), (s - m, m)], fill=ACCENT, width=3)
    return img


def icon_rect(s=24):
    img, d = new_icon(s)
    m = s // 5
    d.rectangle([m, m + 2, s - m, s - m], outline=ACCENT, width=2)
    return img


def icon_rotrect(s=24):
    img, d = new_icon(s)
    pts = [(s // 2, s // 6), (5 * s // 6, s // 3), (s // 2, 5 * s // 6), (s // 6, s // 3)]
    d.polygon(pts, outline=ACCENT, width=2)
    return img


def icon_polygon(s=24):
    img, d = new_icon(s)
    m = s // 5
    pts = [(s // 2, m), (s - m, s // 2), (3 * s // 4, s - m), (m, s - m)]
    d.polygon(pts, outline=ACCENT, width=2)
    return img


def icon_brush(s=24):
    img, d = new_icon(s)
    d.ellipse([s // 6, s // 3, 5 * s // 6, 5 * s // 6], fill=(*ACCENT, 120), outline=ACCENT, width=2)
    return img


def icon_open(s=24):
    img, d = new_icon(s)
    m = s // 6
    d.rectangle([m, m + 4, s - m, s - m], outline=FG, width=2)
    d.polygon([(m, m + 4), (s // 2, m), (s - m, m + 4)], fill=FG)
    return img


def icon_folder(s=24):
    img, d = new_icon(s)
    m = s // 6
    d.rectangle([m, s // 3, s - m, s - m], outline=FG, width=2)
    d.rectangle([m, s // 4, s // 2, s // 3], fill=FG)
    return img


def icon_save(s=24):
    img, d = new_icon(s)
    m = s // 5
    d.rectangle([m + 2, m, s - m - 2, s - m], outline=FG, width=2)
    d.rectangle([m + 5, m, s - m - 5, s // 3], fill=FG)
    d.rectangle([m + 4, 2 * s // 3, s - m - 4, s - m - 2], fill=ACCENT)
    return img


def icon_load(s=24):
    img, d = new_icon(s)
    m = s // 5
    d.rectangle([m, m, s - m, s - m], outline=FG, width=2)
    d.line([(m + 3, s // 2), (s - m - 3, s // 2)], fill=ACCENT, width=2)
    d.polygon([(s // 2 - 4, s // 2 - 5), (s // 2 + 4, s // 2 - 5), (s // 2, s // 2 + 3)], fill=ACCENT)
    return img


def icon_color(s=24):
    img, d = new_icon(s)
    d.ellipse([4, 4, s - 4, s - 4], fill=(255, 90, 90))
    d.pieslice([2, 2, s - 2, s - 2], 0, 120, fill=(90, 220, 120))
    d.pieslice([2, 2, s - 2, s - 2], 120, 240, fill=(90, 150, 255))
    return img


def icon_delete(s=24):
    img, d = new_icon(s)
    m = s // 5
    d.rectangle([m + 2, m + 5, s - m - 2, s - m], outline=(240, 100, 100), width=2)
    d.line([(m + 4, m + 3), (s - m - 4, m + 3)], fill=(240, 100, 100), width=2)
    d.line([(s // 2 - 2, m + 8), (s // 2 - 2, s - m - 2)], fill=FG, width=2)
    d.line([(s // 2 + 2, m + 8), (s // 2 + 2, s - m - 2)], fill=FG, width=2)
    return img


def icon_clear(s=24):
    img, d = new_icon(s)
    m = s // 5
    d.arc([m, m, s - m, s - m], 30, 330, fill=FG, width=2)
    d.polygon([(s - m - 2, m + 2), (s - m + 2, m + 6), (s - m - 6, m + 8)], fill=FG)
    return img


def icon_fit(s=24):
    img, d = new_icon(s)
    m = s // 6
    d.rectangle([m, m, s - m, s - m], outline=FG, width=2)
    d.line([(m, s // 2), (s - m, s // 2)], fill=ACCENT, width=1)
    d.line([(s // 2, m), (s // 2, s - m)], fill=ACCENT, width=1)
    return img


def icon_app(size=256):
    img = Image.new("RGBA", (size, size), BG)
    d = ImageDraw.Draw(img)
    pad = size // 8
    d.rounded_rectangle([pad, pad, size - pad, size - pad], radius=size // 6, fill=(55, 60, 72))
    # stylized "K" + tag corner
    w = size // 16
    cx, cy = size // 2 - size // 16, size // 2
    h = size // 3
    d.line([(cx - h // 2, cy - h), (cx - h // 2, cy + h)], fill=ACCENT, width=w * 2)
    d.line([(cx - h // 2, cy), (cx + h // 2, cy - h)], fill=ACCENT, width=w * 2)
    d.line([(cx - h // 2, cy), (cx + h // 2, cy + h)], fill=ACCENT, width=w * 2)
    tr = size - pad - size // 5
    d.polygon([(tr, pad + size // 10), (tr + size // 5, pad + size // 10),
               (tr + size // 5, pad + size // 10 + size // 5),
               (tr, pad + size // 10 + size // 5 // 2)], fill=(120, 200, 255))
    return img


def main():
    icons = {
        "tool_select.png": icon_select,
        "tool_point.png": icon_point,
        "tool_line.png": icon_line,
        "tool_rect.png": icon_rect,
        "tool_rotrect.png": icon_rotrect,
        "tool_polygon.png": icon_polygon,
        "tool_brush.png": icon_brush,
        "open_image.png": icon_open,
        "open_folder.png": icon_folder,
        "save.png": icon_save,
        "load.png": icon_load,
        "color.png": icon_color,
        "delete.png": icon_delete,
        "clear.png": icon_clear,
        "fit.png": icon_fit,
    }
    for name, fn in icons.items():
        save(name, fn(24))

    app = icon_app(256)
    save("app.png", app)
    ico_path = ROOT.parent / "app.ico"
    sizes = [(16, 16), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)]
    app.save(ico_path, format="ICO", sizes=[(s, s) for s, _ in sizes])
    print(f"Wrote icons to {ROOT} and {ico_path}")


if __name__ == "__main__":
    main()
