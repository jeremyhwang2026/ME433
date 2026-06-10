import time
import board
import analogio

potentiometer = analogio.AnalogIn(board.GP26)

while True:
    raw_value = potentiometer.value
    scaled_angle = raw_value // 16
    print(f"ANGLE:{raw_value}")
    
    time.sleep(0.01)