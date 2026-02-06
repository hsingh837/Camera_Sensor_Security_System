//This program simulates data over 100 iterations and logs it, good to start out with before we introduce input from openCV laptop webcam and arduino sensors

//A hiccup with this program that I need to solve is that for it to work across platforms, the /data subfolder must exist before compiling & running.


#include <stdio.h>        // Includes functions for input/output like printf() and file operations.
#include <stdlib.h>       // Includes functions for memory allocation, random numbers, and exit control.
#include <time.h>         // Includes functions for working with time and timestamps.


int main() { // Main function where program execution begins
    FILE *f = fopen("data/sensor_log.csv", "w"); //Opens file in writing mode and creates it if doesn't exist
    if (f == NULL) {
        perror("Failed to open file"); //Checks if file opened successfully, returns error message & code if not.
        return 1;
    }


    srand(time(NULL)); //Starts random number generator using current time
    fprintf(f, "timestamp,, temperature_C,, pressure_kPa,, color_(R-G-B),,, brightness (Lumens)\n"); // writes the heading for the data(s)

    for (int i = 0; i < 99; ++i) { //loop to simulate 100 sensor readings
        int temp = 20 + rand() %10; //generates a random temperature (20 - 29 Celsius) in each iteration of loop -- This is the simulated Temperature Sensor

        int pressure = 100 + rand() % 11; //generates a random pressure (100 - 110 kPa) in each iteration of the loop -- This is the simulated Pressure Sensor

       //This is the Color Sensor below, outputs as an RGB Coordinate (R, G, B) 
        int r = rand() % 256;
        int g = rand() % 256;  //Numbers from 0 - 255
        int b = rand() % 256;

       //This is the light level - brightness sensor which generates a random brightness in lumens from 10 - 30 lumens.
        int brightnessLumens = rand() % 21; 

        time_t now = time(NULL); //gets current time at some iteration of the loop -- This is the time count

        fprintf(f, "%ld,,%d,,%d,,(%d,%d,%d),%d\n", now, temp, pressure, r, g, b, brightnessLumens); //Writes timestamp temperature, RGB coordinates, and pressure(s) to the CSV file
    }


    fclose(f); //closes and saves the edits to the file
    printf("Simulated Sensor data successfully logged into file: data/sensor_log.csv\n"); //Prints confirmation to the console
    return 0; //Exits the program

}

