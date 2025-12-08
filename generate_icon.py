#!/usr/bin/env python3
"""
Genera iconos de app con símbolo WiFi y carta de poker
"""

from PIL import Image, ImageDraw, ImageFont
import os

def create_icon(size, output_path):
    """Crea un icono con WiFi + carta de poker"""

    # Crear imagen con fondo degradado
    img = Image.new('RGB', (size, size), '#1a1a2e')
    draw = ImageDraw.Draw(img)

    # Fondo con gradiente simulado
    for y in range(size):
        brightness = int(26 + (y / size) * 40)
        color = (brightness, brightness, brightness + 30)
        draw.rectangle([(0, y), (size, y+1)], fill=color)

    # Dibujar carta de poker (rectángulo redondeado blanco)
    card_margin = size // 6
    card_x1 = card_margin
    card_y1 = size // 3
    card_x2 = size - card_margin
    card_y2 = size - card_margin
    card_radius = size // 12

    # Carta blanca con borde redondeado
    draw.rounded_rectangle(
        [(card_x1, card_y1), (card_x2, card_y2)],
        radius=card_radius,
        fill='white',
        outline='#cccccc',
        width=max(1, size//48)
    )

    # Símbolo de corazón rojo en la carta
    try:
        font_size = size // 3
        # Intentar usar una fuente del sistema
        try:
            font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", font_size)
        except:
            try:
                font = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", font_size)
            except:
                font = ImageFont.load_default()

        heart = "♥"
        # Calcular posición centrada del corazón
        bbox = draw.textbbox((0, 0), heart, font=font)
        text_width = bbox[2] - bbox[0]
        text_height = bbox[3] - bbox[1]

        heart_x = (size - text_width) // 2
        heart_y = card_y1 + (card_y2 - card_y1 - text_height) // 2 - bbox[1]

        draw.text((heart_x, heart_y), heart, fill='#e74c3c', font=font)

    except Exception as e:
        print(f"No se pudo cargar fuente TrueType: {e}")
        # Fallback: dibujar un círculo rojo
        center_x = size // 2
        center_y = (card_y1 + card_y2) // 2
        radius = size // 8
        draw.ellipse(
            [(center_x - radius, center_y - radius),
             (center_x + radius, center_y + radius)],
            fill='#e74c3c'
        )

    # Dibujar símbolo WiFi en la parte superior
    wifi_center_x = size // 2
    wifi_center_y = size // 5
    wifi_size = size // 4

    # Tres arcos WiFi
    for i in range(3):
        radius = wifi_size * (i + 1) // 3
        thickness = max(2, size // 24)

        # Dibujar arco superior
        draw.arc(
            [(wifi_center_x - radius, wifi_center_y - radius),
             (wifi_center_x + radius, wifi_center_y + radius)],
            start=200,
            end=340,
            fill='#3498db',
            width=thickness
        )

    # Punto central del WiFi
    dot_radius = max(2, size // 20)
    draw.ellipse(
        [(wifi_center_x - dot_radius, wifi_center_y + wifi_size//6 - dot_radius),
         (wifi_center_x + dot_radius, wifi_center_y + wifi_size//6 + dot_radius)],
        fill='#3498db'
    )

    # Guardar
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    img.save(output_path, 'PNG')
    print(f"✓ Creado: {output_path}")

def main():
    """Genera todos los tamaños de iconos"""

    base_path = "/home/user/HACKESP32-C3MINI/android/app/src/main/res"

    sizes = {
        'mipmap-mdpi': 48,
        'mipmap-hdpi': 72,
        'mipmap-xhdpi': 96,
        'mipmap-xxhdpi': 144,
        'mipmap-xxxhdpi': 192
    }

    print("🎨 Generando iconos de app...")
    print("=" * 50)

    for folder, size in sizes.items():
        output_path = os.path.join(base_path, folder, 'ic_launcher.png')
        create_icon(size, output_path)

    print("=" * 50)
    print("✅ Todos los iconos generados correctamente!")
    print("\nIconos creados en:")
    for folder in sizes.keys():
        print(f"  - {folder}/ic_launcher.png")

if __name__ == '__main__':
    main()
