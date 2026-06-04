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

    # Center crop a square based on the smallest dimension
    orig_w, orig_h = img.size
    crop_size = int(min(orig_w, orig_h) * 0.65)  # 65% of the shortest side
    left = (orig_w - crop_size) // 2
    top = (orig_h - crop_size) // 2
    right = left + crop_size
    bottom = top + crop_size
    print(f"Cropping center {crop_size}x{crop_size} from {orig_w}x{orig_h}...")
    img = img.crop((left, top, right, bottom))

    # Apply a rounded rectangle mask to remove the black corners
    print("Applying rounded square mask...")
    mask = Image.new("L", img.size, 0)
    draw = ImageDraw.Draw(mask)
    
    # We use a radius that's approximately standard for squircles
    radius = int(crop_size * 0.22)
    draw.rounded_rectangle((0, 0, crop_size, crop_size), radius=radius, fill=255)
    
    img.putalpha(mask)

    print("Resizing to 820x820...")
    img_820 = img.resize((820, 820), Image.Resampling.LANCZOS)

    print("Padding to 1024x1024...")
    canvas = Image.new("RGBA", (1024, 1024), (0, 0, 0, 0))
    offset = ((1024 - 820) // 2, (1024 - 820) // 2)
    canvas.paste(img_820, offset, img_820)

    print(f"Saving to {output_path}...")
    canvas.save(output_path, "PNG")
    print("Done.")

if __name__ == "__main__":
    src = "/Users/plokm/Downloads/focus-forge/Gemini_Generated_Image_bzg7nfbzg7nfbzg7.png"
    dst = os.path.join(os.path.dirname(__file__), "..", "assets", "images", "padded_icon.png")
    dst = os.path.abspath(dst)
    
    apply_mask(src, dst)

