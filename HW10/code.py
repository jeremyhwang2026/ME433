import time
import board
import digitalio

button = digitalio.DigitalInOut(board.GP16)
button.direction = digitalio.Direction.INPUT
button.pull = digitalio.Pull.UP

while True:
    button_state = int(button.value)
    print(button_state)
    time.sleep(0.02)
