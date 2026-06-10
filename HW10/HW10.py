import pgzrun
import serial
import random
from pygame import Rect

try:
    ser = serial.Serial('COM8', 115200, timeout=0.01)
except Exception as e:
    print(f"Error opening serial port: {e}")

# 2. GAME WINDOW CONFIG
WIDTH = 400
HEIGHT = 600
TITLE = "Pico Flappy Bird"

# 3. GAME VARIABLES
bird_y = 300
bird_velocity = 0
GRAVITY = 0.4
FLAP_STRENGTH = -7

pipes = []
pipe_speed = 3
pipe_gap = 150
pipe_spawn_timer = 0

score = 0
game_over = False
last_button_state = 1

def spawn_pipe():
    # Generate a random height for the gap
    gap_y = random.randint(100, 400)
    # Top pipe and bottom pipe Rects
    top_pipe = Rect((WIDTH, 0), (60, gap_y))
    bottom_pipe = Rect((WIDTH, gap_y + pipe_gap), (60, HEIGHT - (gap_y + pipe_gap)))
    pipes.append((top_pipe, bottom_pipe))

def update():
    global bird_y, bird_velocity, pipe_spawn_timer, score, game_over, last_button_state

    if game_over:
        return

    # --- PICO SERIAL READ & FLAP LOGIC ---
    if 'ser' in globals() and ser.in_waiting > 0:
        try:
            # Read the latest line from the Pico
            line = ser.readline().decode('utf-8').strip()
            if line in ('0', '1'):
                current_button_state = int(line)
                
                # Check for an "Edge Trigger": if last state was 1 (unpressed) 
                # and current state is 0 (pressed), that means a NEW click happened!
                if last_button_state == 1 and current_button_state == 0:
                    bird_velocity = FLAP_STRENGTH
                
                last_button_state = current_button_state
        except:
            pass  # Ignore brief serial glitches

    # --- GAME PHYSICS ---
    bird_velocity += GRAVITY
    bird_y += bird_velocity
    bird_rect = Rect((100, int(bird_y)), (30, 30))

    # Check boundaries
    if bird_y > HEIGHT or bird_y < 0:
        game_over = True

    # --- PIPE MANAGEMENT ---
    pipe_spawn_timer += 1
    if pipe_spawn_timer > 90:  # Spawn a pipe roughly every 1.5 seconds
        spawn_pipe = True
        # Logic inline to append
        gap_y = random.randint(100, 400)
        pipes.append([Rect(WIDTH, 0, 60, gap_y), Rect(WIDTH, gap_y + pipe_gap, 60, HEIGHT)])
        pipe_spawn_timer = 0

    for top_pipe, bottom_pipe in pipes:
        top_pipe.x -= pipe_speed
        bottom_pipe.x -= pipe_speed

        # Collision detection
        if bird_rect.colliderect(top_pipe) or bird_rect.colliderect(bottom_pipe):
            game_over = True

    # Remove offscreen pipes and add score
    if pipes and pipes[0][0].right < 0:
        pipes.pop(0)
        score += 1

def draw():
    screen.fill((135, 206, 235))  # Sky blue background

    # Draw Pipes (Green)
    for top_pipe, bottom_pipe in pipes:
        screen.draw.filled_rect(top_pipe, (34, 139, 34))
        screen.draw.filled_rect(bottom_pipe, (34, 139, 34))

    # Draw Bird (Yellow square)
    bird_rect = Rect((100, int(bird_y)), (30, 30))
    screen.draw.filled_rect(bird_rect, (255, 215, 0))

    # Draw UI text
    screen.draw.text(f"Score: {score}", (20, 20), color="white", fontsize=40, shadow=(1,1))
    
    if game_over:
        screen.draw.text("GAME OVER", (100, 250), color="red", fontsize=60, shadow=(2,2))
        screen.draw.text("Reset Pico to restart game", (75, 320), color="black", fontsize=25)

# 3. RUN VIA VSCODE PLAY BUTTON
pgzrun.go()
