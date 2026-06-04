#!/usr/bin/env python3
import sys
import os

try:
    from PIL import Image
except ImportError:
    print("Pillow not installed. Please run: pip install pillow")
    sys.exit(1)

from collections import deque

def flood_fill_transparent(image_path, output_path):
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

    width, height = img.size
    pixels = img.load()

    # We will do a BFS flood fill from the 4 corners
    corners = [(0, 0), (width - 1, 0), (0, height - 1), (width - 1, height - 1)]
    visited = set()
    queue = deque()

    # Tolerance for "near black"
    def is_near_black(r, g, b):
        return r < 15 and g < 15 and b < 15

    for cx, cy in corners:
        r, g, b, a = pixels[cx, cy]
        if is_near_black(r, g, b):
            queue.append((cx, cy))
            visited.add((cx, cy))

    print("Running flood fill algorithm...")
    # BFS
    while queue:
        x, y = queue.popleft()
        # Set pixel to completely transparent
        pixels[x, y] = (0, 0, 0, 0)
        
        # Check neighbors
        for dx, dy in [(-1, 0), (1, 0), (0, -1), (0, 1)]:
            nx, ny = x + dx, y + dy
            if 0 <= nx < width and 0 <= ny < height:
                if (nx, ny) not in visited:
                    visited.add((nx, ny))
                    nr, ng, nb, na = pixels[nx, ny]
                    if is_near_black(nr, ng, nb):
                        queue.append((nx, ny))

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
    
    flood_fill_transparent(src, dst)
