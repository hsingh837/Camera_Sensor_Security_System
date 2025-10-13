#same idea as SimulatedData.C

import os, random, time


os.makedirs("data", exist_ok = True)


with open ("data/sensor_log.csv", "w", newline = "") as f:
    random.seed(time.time())
    f.write("timestamp,, temperature_C,, pressure_kPa,, color_(R-G-B),,, brightness (Lumens)\n")
    for i in range(99):
        temp = 20 + random.randint(0,9)
        pressure = 100 + random.randint(0, 10)
        r = random.randint(0,255)
        g = random.randint(0,255)
        b = random.randint(0,255)
        brightness = random.randint(0, 20)
        now = int(time.time())
        f.write(f"{now},, {temp},, {pressure},, ({r},{g},{b}), {brightness}\n")


print("Simulated Sensor Data succesfully logged into file: data/sensor_log.csv")
