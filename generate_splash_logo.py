#!/usr/bin/env python3
"""
Genera logo con corazón dibujado a lápiz (múltiples líneas rojas) y señal WiFi arriba
"""

from PIL import Image, ImageDraw
import random
import math

def create_sketchy_heart_wifi_logo(size, output_path):
    """Crea logo con corazón estilo dibujado a lápiz y WiFi arriba"""

    # Crear imagen con fondo transparente
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    center_x = size // 2
    center_y = size // 2

    # Color rojo para el corazón
    red_color = (220, 50, 50, 255)

    # Función para dibujar el contorno del corazón con variación (estilo lápiz)
    def draw_sketchy_heart(offset_x=0, offset_y=0, thickness=2):
        # Escala del corazón
        scale = size / 400.0
        heart_size = int(80 * scale)

        # Posición del corazón (más abajo para dejar espacio al WiFi)
        heart_y = int(center_y + size * 0.15)

        points = []

        # Generar puntos del corazón
        for t in range(0, 360, 2):
            angle = math.radians(t)
            # Ecuación paramétrica del corazón
            x = 16 * math.sin(angle) ** 3
            y = -(13 * math.cos(angle) - 5 * math.cos(2*angle) - 2 * math.cos(3*angle) - math.cos(4*angle))

            # Añadir variación aleatoria para efecto dibujado a mano
            x += random.uniform(-0.5, 0.5)
            y += random.uniform(-0.5, 0.5)

            # Escalar y posicionar
            px = int(center_x + x * heart_size / 16 + offset_x)
            py = int(heart_y + y * heart_size / 16 + offset_y)

            points.append((px, py))

        # Dibujar la línea del corazón
        if len(points) > 1:
            draw.line(points + [points[0]], fill=red_color, width=thickness)

    # Dibujar el corazón múltiples veces con ligeras variaciones (efecto lápiz repasado)
    for i in range(8):
        offset_x = random.uniform(-1.5, 1.5)
        offset_y = random.uniform(-1.5, 1.5)
        thickness = random.choice([2, 2, 3])
        draw_sketchy_heart(offset_x, offset_y, thickness)

    # Dibujar señal WiFi arriba del corazón
    wifi_center_y = int(center_y - size * 0.20)
    wifi_size = int(size * 0.15)

    # Color para WiFi (puede ser rojo también o blanco)
    wifi_color = (255, 255, 255, 255)

    # Dibujar 3 arcos WiFi
    for i in range(3):
        radius = int(wifi_size * (i + 1) / 2.5)
        thickness = max(2, int(size / 80))

        # Dibujar arco superior del WiFi
        bbox = [
            center_x - radius,
            wifi_center_y - radius,
            center_x + radius,
            wifi_center_y + radius
        ]
        draw.arc(bbox, start=200, end=340, fill=wifi_color, width=thickness)

    # Punto central del WiFi
    dot_radius = max(3, int(size / 50))
    draw.ellipse([
        center_x - dot_radius,
        wifi_center_y + int(wifi_size / 4) - dot_radius,
        center_x + dot_radius,
        wifi_center_y + int(wifi_size / 4) + dot_radius
    ], fill=wifi_color)

    # Guardar
    img.save(output_path, 'PNG')
    print(f"✓ Creado: {output_path}")

def main():
    """Genera el logo para splash screen"""

    output_path = "/home/user/HACKESP32-C3MINI/android/app/src/main/res/drawable/logo_elitemagic.png"

    print("🎨 Generando logo con corazón dibujado a lápiz y WiFi...")
    print("=" * 60)

    # Crear logo de tamaño grande para buena calidad
    create_sketchy_heart_wifi_logo(512, output_path)

    print("=" * 60)
    print("✅ Logo generado correctamente!")

if __name__ == '__main__':
    main()
