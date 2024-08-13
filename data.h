

#ifndef WEATHER_DATA_H
#define WEATHER_DATA_H

#include <iostream>
#include <string>

class WeatherData {
private:
    std::string cityName;
    std::string latitude, longitude;
    float temperature;
    float maxTemperature;
    float minTemperature;
    float humidity;
    float windSpeed;
    float pressure;
    bool favorites;
    std::string description;

public:
    WeatherData(std::string cityName, std::string latitude, std::string longitude)
        : cityName(cityName), latitude(latitude), longitude(longitude), favorites(false) {}

    // Setters for weather data
    void setWeatherData(float temp, float maxTemp, float minTemp, float hum, float windSpd, float pres, const std::string& desc) {
        temperature = temp;
        maxTemperature = maxTemp;
        minTemperature = minTemp;
        humidity = hum;
        windSpeed = windSpd;
        pressure = pres;
        description = desc;
    }

    // Mark city as favorite
    void markAsFavorite() { favorites = true; }

    // Print all data
    void printData() const {
        std::cout << "City: " << cityName << " (" << latitude << ", " << longitude << ")\n"
            << "Current Temperature: " << temperature << "°C\n"
            << "Max Temperature Today: " << maxTemperature << "°C\n"
            << "Min Temperature Today: " << minTemperature << "°C\n"
            << "Humidity: " << humidity << "%\n"
            << "Wind Speed: " << windSpeed << " km/h\n"
            << "Pressure: " << pressure << " hPa\n"
            << "Description: " << description << "\n"
            << "Favorite: " << (favorites ? "Yes" : "No") << std::endl;
    }

    const std::string& getCityName() const { return cityName; }
    bool isFavorite() const { return favorites; }
};

#endif // WEATHER_DATA_H
