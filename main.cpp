
// #include <curl/curl.h>
// #include <nlohmann/json.hpp>

// #include <iostream>
// #include <string>
// #include <stdexcept>

// using json = nlohmann::json;

// size_t write_callback(void* contents, size_t size,
//                       size_t nmemb, void* user_data) {
//     auto* response = static_cast<std::string*>(user_data);
//     response->append(static_cast<char*>(contents), size * nmemb);
//     return size * nmemb;
// }

// std::string get_url(const std::string& url) {
//     CURL* curl = curl_easy_init();

//     if (!curl) {
//         throw std::runtime_error("Could not initialize HTTP client.");
//     }

//     std::string response;
//     curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
//     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
//     curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
//     curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
//     curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);

//     CURLcode result = curl_easy_perform(curl);
//     long status = 0;
//     curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
//     curl_easy_cleanup(curl);

//     if (result != CURLE_OK) {
//         throw std::runtime_error("Network request failed.");
//     }

//     if (status < 200 || status >= 300) {
//         throw std::runtime_error(
//             "API returned HTTP status " + std::to_string(status));
//     }

//     return response;
// }

// int main() {
//     curl_global_init(CURL_GLOBAL_DEFAULT);

//     try {
//         std::string city;

//         std::cout << "=== C++ Weather Dashboard ===\n";
//         std::cout << "Enter a city (Karachi, Lahore, London): ";
//         std::getline(std::cin, city);

//         if (city.empty()) {
//             throw std::runtime_error("Please enter a city name.");
//         }

//         // Encode the city name for use in a URL.
//         CURL* encoder = curl_easy_init();
//         char* encoded = curl_easy_escape(
//             encoder, city.c_str(), static_cast<int>(city.size()));

//         if (!encoded) {
//             curl_easy_cleanup(encoder);
//             throw std::runtime_error("Could not encode city name.");
//         }

//         std::string geo_url =
//             "https://geocoding-api.open-meteo.com/v1/search?name=";
//         geo_url += encoded;
//         geo_url += "&count=1&language=en&format=json";

//         curl_free(encoded);
//         curl_easy_cleanup(encoder);

//         // Request city coordinates.
//         json geo = json::parse(get_url(geo_url));

//         if (!geo.contains("results") || geo["results"].empty()) {
//             throw std::runtime_error("City not found.");
//         }

//         auto place = geo["results"][0];
//         double lat = place.at("latitude").get<double>();
//         double lon = place.at("longitude").get<double>();

//         // Request current weather at those coordinates.
//         std::string weather_url =
//             "https://api.open-meteo.com/v1/forecast?latitude=" +
//             std::to_string(lat) + "&longitude=" + std::to_string(lon) +
//             "&current=temperature_2m,relative_humidity_2m,"
//             "wind_speed_10m,wind_direction_10m,weather_code"
//             "&temperature_unit=celsius&wind_speed_unit=kmh&timezone=auto";

//         json data = json::parse(get_url(weather_url));
//         auto current = data.at("current");

//         std::cout << "\n--- Current Weather ---\n";
//         std::cout << "City: " << place.value("name", city) << "\n";
//         std::cout << "Country: "
//                   << place.value("country", std::string("Unknown")) << "\n";
//         std::cout << "Temperature: "
//                   << current.at("temperature_2m") << " °C\n";
//         std::cout << "Humidity: "
//                   << current.at("relative_humidity_2m") << " %\n";
//         std::cout << "Wind speed: "
//                   << current.at("wind_speed_10m") << " km/h\n";
//         std::cout << "Wind direction: "
//                   << current.at("wind_direction_10m") << " degrees\n";
//         std::cout << "Weather code: "
//                   << current.at("weather_code") << "\n";
//         std::cout << "API observation time: "
//                   << current.at("time") << "\n";

//     } catch (const std::exception& error) {
//         std::cerr << "\nError: " << error.what() << "\n";
//         std::cerr << "Check your Internet connection and city name.\n";
//         curl_global_cleanup();
//         return 1;
//     }

//     curl_global_cleanup();
//     return 0;
// }

#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <sqlite3.h>

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

using json = nlohmann::json;

size_t write_callback(void* data, size_t size, size_t count,
                      void* output) {
    auto* response = static_cast<std::string*>(output);
    response->append(static_cast<char*>(data), size * count);
    return size * count;
}

std::string http_get(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Could not initialize HTTP.");

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);

    CURLcode result = curl_easy_perform(curl);
    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curl);

    if (result != CURLE_OK)
        throw std::runtime_error(curl_easy_strerror(result));
    if (status < 200 || status >= 300)
        throw std::runtime_error("HTTP status: " + std::to_string(status));

    return response;
}

std::string encode_url(const std::string& input) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("URL encoding failed.");

    char* result = curl_easy_escape(
        curl, input.c_str(), static_cast<int>(input.size()));

    if (!result) {
        curl_easy_cleanup(curl);
        throw std::runtime_error("Could not encode city name.");
    }

    std::string encoded(result);
    curl_free(result);
    curl_easy_cleanup(curl);
    return encoded;
}

std::string timestamp_now() {
    std::time_t now = std::time(nullptr);
    std::tm local{};

#ifdef _WIN32
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif

    std::ostringstream out;
    out << std::put_time(&local, "%Y-%m-%d %H:%M:%S");
    return out.str();
}

std::string describe_weather(int code) {
    if (code == 0) return "Clear sky";
    if (code == 1) return "Mainly clear";
    if (code == 2) return "Partly cloudy";
    if (code == 3) return "Overcast";
    if (code == 45 || code == 48) return "Fog";
    if (code >= 51 && code <= 57) return "Drizzle";
    if (code >= 61 && code <= 67) return "Rain";
    if (code >= 71 && code <= 77) return "Snow";
    if (code >= 80 && code <= 82) return "Rain showers";
    if (code == 85 || code == 86) return "Snow showers";
    if (code == 95 || code == 96 || code == 99)
        return "Thunderstorm";
    return "Unknown (code " + std::to_string(code) + ")";
}

sqlite3* open_database() {
    sqlite3* db = nullptr;

    if (sqlite3_open("weather_history.db", &db) != SQLITE_OK) {
        std::string error = db ? sqlite3_errmsg(db) : "unknown error";
        if (db) sqlite3_close(db);
        throw std::runtime_error("Database error: " + error);
    }

    const char* sql =
        "CREATE TABLE IF NOT EXISTS snapshots ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "city TEXT NOT NULL,"
        "country TEXT,"
        "api_time TEXT,"
        "saved_at TEXT NOT NULL,"
        "temperature REAL,"
        "humidity INTEGER,"
        "wind_speed REAL,"
        "wind_direction INTEGER,"
        "weather_code INTEGER,"
        "description TEXT);";

    char* error = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &error) != SQLITE_OK) {
        std::string message = error ? error : "unknown error";
        sqlite3_free(error);
        sqlite3_close(db);
        throw std::runtime_error("Table creation failed: " + message);
    }

    return db;
}

void fetch_and_save(sqlite3* db, const std::string& query) {
    std::string geo_url =
        "https://geocoding-api.open-meteo.com/v1/search?name=" +
        encode_url(query) + "&count=1&language=en&format=json";

    json geo = json::parse(http_get(geo_url));

    if (!geo.contains("results") || geo["results"].empty())
        throw std::runtime_error("City not found. Try Karachi, Lahore, or London.");

    auto place = geo["results"][0];
    std::string city = place.value("name", query);
    std::string country = place.value("country", "Unknown");
    double lat = place.at("latitude").get<double>();
    double lon = place.at("longitude").get<double>();

    std::ostringstream coordinates;
    coordinates << std::fixed << std::setprecision(6) << lat;

    std::ostringstream longitude;
    longitude << std::fixed << std::setprecision(6) << lon;

    std::string weather_url =
        "https://api.open-meteo.com/v1/forecast?latitude=" +
        coordinates.str() + "&longitude=" + longitude.str() +
        "&current=temperature_2m,relative_humidity_2m,"
        "wind_speed_10m,wind_direction_10m,weather_code"
        "&temperature_unit=celsius&wind_speed_unit=kmh&timezone=auto";

    json data = json::parse(http_get(weather_url));
    if (!data.contains("current"))
        throw std::runtime_error("API response has no current weather.");

    auto current = data.at("current");

    double temperature = current.at("temperature_2m").get<double>();
    int humidity = current.at("relative_humidity_2m").get<int>();
    double wind = current.at("wind_speed_10m").get<double>();
    int direction = current.at("wind_direction_10m").get<int>();
    int code = current.at("weather_code").get<int>();
    std::string api_time = current.value("time", "unknown");
    std::string saved_at = timestamp_now();
    std::string description = describe_weather(code);

    const char* sql =
        "INSERT INTO snapshots "
        "(city,country,api_time,saved_at,temperature,humidity,"
        "wind_speed,wind_direction,weather_code,description)"
        " VALUES (?,?,?,?,?,?,?,?,?,?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error("Could not prepare database insert.");

    sqlite3_bind_text(stmt, 1, city.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, country.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, api_time.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, saved_at.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 5, temperature);
    sqlite3_bind_int(stmt, 6, humidity);
    sqlite3_bind_double(stmt, 7, wind);
    sqlite3_bind_int(stmt, 8, direction);
    sqlite3_bind_int(stmt, 9, code);
    sqlite3_bind_text(stmt, 10, description.c_str(), -1, SQLITE_TRANSIENT);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (result != SQLITE_DONE)
        throw std::runtime_error("Could not save weather snapshot.");

    std::cout << "\n=== CURRENT WEATHER ===\n"
              << "City: " << city << ", " << country << "\n"
              << "Temperature: " << temperature << " °C\n"
              << "Humidity: " << humidity << " %\n"
              << "Wind speed: " << wind << " km/h\n"
              << "Wind direction: " << direction << " degrees\n"
              << "Condition: " << description << "\n"
              << "API observation time: " << api_time << "\n"
              << "Latest successful update: " << saved_at << "\n"
              << "Snapshot saved to SQLite.\n";
}

void show_history(sqlite3* db) {
    const char* sql =
        "SELECT city,country,saved_at,temperature,humidity,"
        "wind_speed,description FROM snapshots ORDER BY id DESC LIMIT 30;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error("Could not read history.");

    std::cout << "\n=== SAVED WEATHER HISTORY ===\n";
    int count = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto text_column = [&](int col) {
            const unsigned char* value = sqlite3_column_text(stmt, col);
            return value ? std::string(
                reinterpret_cast<const char*>(value)) : std::string();
        };

        std::cout << text_column(0) << ", " << text_column(1)
                  << " | Saved: " << text_column(2)
                  << " | " << sqlite3_column_double(stmt, 3) << " °C"
                  << " | Humidity: " << sqlite3_column_int(stmt, 4) << " %"
                  << " | Wind: " << sqlite3_column_double(stmt, 5) << " km/h"
                  << " | " << text_column(6) << "\n";
        ++count;
    }

    sqlite3_finalize(stmt);
    if (count == 0) std::cout << "No snapshots saved yet.\n";
}

int main() {
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        std::cerr << "Could not initialize libcurl.\n";
        return 1;
    }

    sqlite3* db = nullptr;

    try {
        db = open_database();
        bool running = true;

        while (running) {
            std::cout << "\n=== C++ Weather Dashboard ===\n"
                      << "1. Fetch weather for a city\n"
                      << "2. View saved history\n"
                      << "3. Exit\n"
                      << "Select: ";

            std::string choice;
            if (!std::getline(std::cin, choice)) break;

            try {
                if (choice == "1") {
                    std::cout << "Enter city: ";
                    std::string city;
                    std::getline(std::cin, city);

                    if (city.empty()) {
                        std::cout << "Please enter a city name.\n";
                        continue;
                    }

                    fetch_and_save(db, city);
                } else if (choice == "2") {
                    show_history(db);
                } else if (choice == "3") {
                    running = false;
                } else {
                    std::cout << "Choose 1, 2, or 3.\n";
                }
            } catch (const std::exception& e) {
                std::cerr << "Update failed: " << e.what() << "\n"
                          << "No new snapshot was saved. Previous history remains available.\n";
            }
        }

        sqlite3_close(db);
        db = nullptr;
    } catch (const std::exception& e) {
        std::cerr << "Application error: " << e.what() << "\n";
        if (db) sqlite3_close(db);
        curl_global_cleanup();
        return 1;
    }

    curl_global_cleanup();
    std::cout << "Goodbye!\n";
    return 0;
}
