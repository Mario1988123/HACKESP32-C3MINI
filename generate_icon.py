#!/usr/bin/env python3
"""
Genera iconos de app con corazón rojo en forma de señal WiFi
"""

from PIL import Image, ImageDraw
import os
import math

def create_wifi_heart_icon(size, output_path):
    """Crea un icono con corazón rojo formado por líneas WiFi"""

    # Crear imagen con fondo oscuro
    img = Image.new('RGBA', (size, size), (26, 26, 46, 255))
    draw = ImageDraw.Draw(img)

    center_x = size // 2
    center_y = size // 2

    # Escala para el corazón
    scale = size / 192.0

    # Color rojo vibrante
    heart_color = (231, 76, 60, 255)  # Rojo brillante

    # Grosor de línea basado en tamaño
    line_width = max(2, int(size / 32))

    # Dibujar el corazón usando arcos WiFi
    # El corazón se forma con múltiples arcos concéntricos

    # Función para dibujar un arco de WiFi en forma de corazón
    def draw_heart_wifi_arc(radius_factor, start_angle, end_angle):
        # Calcular el radio para este arco
        base_radius = int(size * 0.25 * radius_factor)

        # Punto central del corazón (ligeramente arriba del centro)
        heart_center_y = int(center_y + size * 0.05)

        # Dibujar arcos que forman las curvas del corazón
        # Lóbulo izquierdo
        left_center_x = int(center_x - base_radius * 0.3)
        left_bbox = [
            left_center_x - base_radius,
            heart_center_y - base_radius,
            left_center_x + base_radius,
            heart_center_y + base_radius
        ]
        draw.arc(left_bbox, start=start_angle, end=end_angle, fill=heart_color, width=line_width)

        # Lóbulo derecho
        right_center_x = int(center_x + base_radius * 0.3)
        right_bbox = [
            right_center_x - base_radius,
            heart_center_y - base_radius,
            right_center_x + base_radius,
            heart_center_y + base_radius
        ]
        draw.arc(right_bbox, start=start_angle, end=end_angle, fill=heart_color, width=line_width)

        # Punta inferior del corazón (líneas convergentes)
        if radius_factor > 0.8:  # Solo en los arcos más externos
            bottom_y = int(heart_center_y + base_radius * 1.2)
            draw.line([left_center_x, heart_center_y + base_radius, center_x, bottom_y],
                     fill=heart_color, width=line_width)
            draw.line([right_center_x, heart_center_y + base_radius, center_x, bottom_y],
                     fill=heart_color, width=line_width)

    # Dibujar múltiples arcos WiFi formando el corazón
    # Arcos concéntricos de dentro hacia fuera
    for i in range(3):
        radius_factor = 0.6 + (i * 0.3)  # 0.6, 0.9, 1.2
        # Arcos superiores (formando las curvas del corazón)
        draw_heart_wifi_arc(radius_factor, 180, 360)

    # Añadir punto central WiFi
    dot_radius = max(2, int(size / 24))
    draw.ellipse([
        center_x - dot_radius,
        int(center_y + size * 0.05) - dot_radius,
        center_x + dot_radius,
        int(center_y + size * 0.05) + dot_radius
    ], fill=heart_color)

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

    print("🎨 Generando iconos de corazón WiFi...")
    print("=" * 50)

    for folder, size in sizes.items():
        # Create both regular and round icons
        output_path = os.path.join(base_path, folder, 'ic_launcher.png')
        create_wifi_heart_icon(size, output_path)

        output_path_round = os.path.join(base_path, folder, 'ic_launcher_round.png')
        create_wifi_heart_icon(size, output_path_round)

    print("=" * 50)
    print("✅ Todos los iconos generados correctamente!")
    print("\nIconos creados en:")
    for folder in sizes.keys():
        print(f"  - {folder}/ic_launcher.png")
        print(f"  - {folder}/ic_launcher_round.png")

if __name__ == '__main__':
    main()
