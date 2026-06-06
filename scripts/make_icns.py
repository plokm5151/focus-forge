#!/usr/bin/env python3
import sys
import os

try:
    from PIL import Image, ImageDraw
except ImportError:
    print("Pillow not installed. Please run: pip install pillow")
    sys.exit(1)

def apply_mask(image_path, output_path):
    print(f"Loading {image_path}...")
    try:
        img = Image.open(image_path).convert("RGBA")
    except Exception as e:
        print(f"Failed to load image: {e}")
        sys.exit(1)

    print("Cropping to non-transparent bounding box...")
    bbox = img.getbbox()
    if bbox:
        img = img.crop(bbox)

    print("Resizing to 920x920...")
    img_920 = img.resize((920, 920), Image.Resampling.LANCZOS)

    print("Padding to 1024x1024 with transparent background...")
    # Create transparent canvas
    canvas = Image.new("RGBA", (1024, 1024), (0, 0, 0, 0))
    offset = ((1024 - 920) // 2, (1024 - 920) // 2)
    canvas.paste(img_920, offset, img_920)

    print(f"Saving to {output_path}...")
    canvas.save(output_path, "PNG")
    print("Done.")

if __name__ == "__main__":
    if len(sys.argv) >= 3:
        src = sys.argv[1]
        dst = sys.argv[2]
    else:
        src = os.path.join(os.path.dirname(__file__), "..", "assets", "images", "app_icon.png")
        dst = os.path.join(os.path.dirname(__file__), "..", "assets", "images", "padded_icon.png")
    
    src = os.path.abspath(src)
    dst = os.path.abspath(dst)
    
    apply_mask(src, dst)

