from PIL import Image
import os

def remove_white_pixels(input_path, output_path=None):
    if output_path is None:
        output_path = input_path
        
    print(f"Processing {input_path}...")
    try:
        img = Image.open(input_path)
        img = img.convert("RGBA")
        datas = img.getdata()

        new_data = []
        for item in datas:
            # Check if pixel is near-white (distance from white <= 20)
            dist_from_white = ((255 - item[0])**2 + (255 - item[1])**2 + (255 - item[2])**2) ** 0.5
            if dist_from_white <= 20:
                # Replace with transparent pixel
                new_data.append((255, 255, 255, 0))
            else:
                new_data.append(item)

        img.putdata(new_data)
        img.save(output_path, "PNG")
        print(f"Successfully saved to {output_path}")
    except Exception as e:
        print(f"Error processing image: {e}")

if __name__ == "__main__":
    # Path is relative to the directory from where the script is executed
    target_image = os.path.join("assets", "textures", "serin.png")
    
    if os.path.exists(target_image):
        remove_white_pixels(target_image)
    else:
        print(f"File not found: {target_image}")
