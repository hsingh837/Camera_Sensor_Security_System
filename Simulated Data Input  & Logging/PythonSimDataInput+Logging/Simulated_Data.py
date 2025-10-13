#same idea as SimulatedData.C

import os, random, time


script_dir = os.path.dirname(os.path.abspath(__file__))
csv_path = os.path.join(script_dir, "sensor_log_python.csv")
#os.makedirs("data", exist_ok = True)


with open (csv_path, "w", newline = "") as f:
    random.seed(time.time())
    f.write("timestamp,, temperature_C,, pressure_kPa,, color_(R-G-B),,, brightness (Lumens)\n")
    for i in range(99):
        temp = 20 + random.randint(0,9)
        pressure = 100 + random.randint(0, 10)

        r = random.randint(0,255)
        g = random.randint(0,255)  #r, g, and b, are all simulating different color values on color spectrum as would be picked up by a color sensor.
        b = random.randint(0,255)

        brightness = random.randint(0, 20)
        now = int(time.time())
        f.write(f"{now},, {temp},, {pressure},, ({r},{g},{b}), {brightness}\n")


print("Simulated Sensor Data succesfully logged into file:\n{csv_path}")

# print(os.getcwd())
# ^ I used this line to understand where my working directory was because in my first run I was unable to find where the folder and file were created.
