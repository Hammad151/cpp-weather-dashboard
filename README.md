# cpp-weather-dashboard
# C++ Weather Dashboard

## 1. Project Overview

The C++ Weather Dashboard is a command-line application that retrieves current weather information from a public weather API, displays the results, stores timestamped readings in an SQLite database, and generates SVG charts to visualize saved weather data.

The project demonstrates how an application communicates with an API, processes JSON responses, stores data, handles errors, and visualizes historical readings.

## 2. Features

- Search for weather by city name.
- Retrieve current temperature, relative humidity, wind speed, wind direction, and weather conditions.
- Display measurement units clearly.
- Save successful weather readings with timestamps in SQLite.
- View previously saved readings.
- Generate temperature, humidity, and wind speed trend charts.
- Handle invalid city names and failed API requests.
- Use a public API without requiring an API key for ordinary non-commercial use.

## 3. Technologies Used

- **C++17:** Application logic.
- **Open-Meteo API:** Current weather and geographic coordinates.
- **libcurl:** HTTP requests.
- **nlohmann/json:** JSON response parsing.
- **SQLite:** Persistent storage of weather readings.
- **SVG:** Chart generation.

## 4. Weather API Documentation

The application uses Open-Meteo, a public weather service.

Official documentation:

- Weather API: https://open-meteo.com/en/docs
- Geocoding API: https://open-meteo.com/en/docs/geocoding-api

### Geocoding endpoint

`https://geocoding-api.open-meteo.com/v1/search`

The application sends a city name and requests a matching location. The JSON response provides geographic coordinates, including latitude and longitude.

Example parameters:

- `name`: City name
- `count`: Maximum number of results
- `language`: Preferred result language
- `format`: Response format

### Current weather endpoint

`https://api.open-meteo.com/v1/forecast`

The application supplies the location's latitude and longitude and requests current weather measurements.

Example parameters:

- `latitude` and `longitude`: Geographic coordinates
- `current`: Requested weather variables
- `temperature_unit=celsius`: Temperature in degrees Celsius
- `wind_speed_unit=kmh`: Wind speed in kilometres per hour
- `timezone=auto`: Local time for the selected location

The requested measurements include:

- `temperature_2m`: Temperature in °C
- `relative_humidity_2m`: Relative humidity in %
- `wind_speed_10m`: Wind speed in km/h
- `wind_direction_10m`: Wind direction in degrees
- `weather_code`: Numeric code representing weather conditions

### Authentication and response format

An API key is not required for the endpoints used by this project under Open-Meteo's applicable free-use terms.

The service returns JSON data. The application parses the response, extracts the required fields, and stores the readings in the database.

## 5. Database Design

The application stores successful weather snapshots in the SQLite database file `weather_history.db`.

The `snapshots` table stores the weather information returned by the API along with timestamps. Fields include the city, temperature, humidity, wind speed, wind direction, weather code or description, and saved time, depending on the table definition in the source code.

The database allows previous readings to be retrieved for display and visualization. If a request fails, the application should report the problem without replacing a previous successful reading.

## 6. Visualizations

The chart generator reads saved readings from SQLite and creates SVG files:

- `temperature_chart.svg`: Temperature trend in °C.
- `humidity_chart.svg`: Humidity trend in %.
- `wind_chart.svg`: Wind speed trend in km/h.

These charts use previously saved readings rather than manually entered weather values. Multiple snapshots help demonstrate changes over time.

## 7. Running the Application

Compile the main application in an environment with a C++ compiler, libcurl development files, SQLite development files, and the nlohmann/json header installed.

Example compilation commands for a Linux-based environment:

```bash
g++ -std=c++17 main.cpp -o weather_dashboard $(pkg-config --cflags --libs libcurl) -lsqlite3
./weather_dashboard
```

Use the application's menu to retrieve weather for different cities and inspect saved history.

To generate the charts:

```bash
g++ -std=c++17 charts.cpp -o charts -lsqlite3
./charts
```

The chart generator requires the existing `weather_history.db` database and saved readings.

## 8. Error Handling

The application should handle invalid city names, unsuccessful HTTP requests, malformed or incomplete API responses, and database errors. Error messages help users understand why a request or chart generation may have failed.

The latest successful reading time can help distinguish fresh information from older saved data.

## 9. Limitations

- Weather information depends on the availability and accuracy of the external API.
- The charts show saved snapshots; they do not automatically create a continuous history.
- A meaningful trend requires multiple saved readings.
- Wind direction is reported in degrees.
- The free service is subject to Open-Meteo's current usage terms.

## 10. Conclusion

This project demonstrates a practical API workflow in C++: requesting external data, parsing JSON, persisting records in a database, and visualizing stored measurements. It combines networking, data processing, database management, error handling, and basic data visualization in one application.
