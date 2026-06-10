import sys
import math
import serial
import pygame

# --- 1. SERIAL CONFIGURATION ---
SERIAL_PORT = 'COM8'  # Matches your successful connection port
BAUD_RATE = 115200

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.01)
    print(f"Connected successfully to {SERIAL_PORT}")
except Exception as e:
    print(f"Serial Error: {e}")
    sys.exit()

# --- 2. STANDARD PYGAME INITIALIZATION ---
pygame.init()
WIDTH, HEIGHT = 500, 500
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("HW17: Haptic Paddle Visualizer")
clock = pygame.time.Clock()

# --- 3. RENDERING VARIABLES ---
CENTER = (WIDTH // 2, HEIGHT // 2)
LINE_LENGTH = 160
current_angle_raw = 0
font = pygame.font.SysFont(None, 24)

# --- 4. EXPLICIT CORE LOOP ---
running = True
while running:
    # A. Handle Window Close Events (Forces window to stay alive)
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    # B. Read the incoming Serial Data stream
    if ser.in_waiting > 0:
        try:
            raw_line = ser.readline().decode('utf-8').strip()
            if raw_line.startswith("ANGLE:"):
                parts = raw_line.split(":", 1)
                current_angle_raw = int(parts[1])
        except (ValueError, IndexError, UnicodeDecodeError):
            pass  # Drops messy data packets safely

    # C. Graphical Calculations
    # Map 0-4095 cleanly to radians (0 to 2*pi)
    angle_rad = (current_angle_raw / 4095.0) * 2 * math.pi
    
    # Calculate geometric vector endpoints
    end_x = CENTER[0] + LINE_LENGTH * math.sin(angle_rad)
    end_y = CENTER[1] - LINE_LENGTH * math.cos(angle_rad)

    # D. Render Visuals
    screen.fill((30, 30, 30))  # Clear background
    
    # Draw track ring, paddle line, and pivot center
    pygame.draw.circle(screen, (60, 60, 60), CENTER, LINE_LENGTH, 2)
    pygame.draw.line(screen, (0, 255, 150), CENTER, (end_x, end_y), 4)
    pygame.draw.circle(screen, (255, 255, 255), CENTER, 8)
    
    # Text Overlay
    text_surface = font.render(f"Raw Value: {current_angle_raw}", True, (200, 200, 200))
    screen.blit(text_surface, (20, 20))

    # E. Flip Display and Cap Frame Rate
    pygame.display.flip()
    clock.tick(60)  # Runs steady at 60 FPS

# Clean exit when loop breaks
ser.close()
pygame.quit()
sys.exit()