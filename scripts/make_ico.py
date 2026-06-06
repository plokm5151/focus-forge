from PIL import Image
import sys

def main():
    if len(sys.argv) != 3:
        print("Usage: python make_ico.py <input.png> <output.ico>")
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = sys.argv[2]

    try:
        img = Image.open(input_path).convert("RGBA")
    except Exception as e:
        print(f"Failed to load image: {e}")
        sys.exit(1)

    # Crop to bounding box
    bbox = img.getbbox()
    if bbox:
        img = img.crop(bbox)

    # Windows icons usually look best when they completely fill the bounding box
    # so we just save the cropped squircle directly as an ICO.
    # PIL will automatically scale to the requested sizes.
    sizes = [(16, 16), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)]
    
    print(f"Saving to {output_path}...")
    img.save(output_path, format="ICO", sizes=sizes)
    print("Done.")

if __name__ == "__main__":
    main()
