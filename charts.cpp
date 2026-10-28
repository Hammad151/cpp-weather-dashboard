
#include <iostream>
#include <fstream>
#include <sqlite3.h>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

struct Reading {
    string city;
    string time;
    double temperature;
    double humidity;
    double wind;
};

vector<Reading> readings;

int loadReading(void* data, int argc, char** argv, char** columns) {
    auto* rows = static_cast<vector<Reading>*>(data);

    Reading r;
    r.city = argv[0] ? argv[0] : "";
    r.time = argv[1] ? argv[1] : "";
    r.temperature = argv[2] ? atof(argv[2]) : 0;
    r.humidity = argv[3] ? atof(argv[3]) : 0;
    r.wind = argv[4] ? atof(argv[4]) : 0;

    rows->push_back(r);
    return 0;
}

string escapeXml(string s) {
    for (char& c : s) {
        if (c == '&') c = ' ';
        if (c == '<' || c == '>') c = ' ';
        if (c == '"') c = '\'';
    }
    return s;
}

void drawChart(const string& filename, const string& title,
               const vector<Reading>& rows, int metric) {
    const int W = 900, H = 500;
    const int left = 90, right = 40, top = 80, bottom = 100;
    const int plotW = W - left - right;
    const int plotH = H - top - bottom;

    ofstream out(filename);
    if (!out) {
        cerr << "Could not create " << filename << "\n";
        return;
    }

    double maxValue = 10;
    for (const auto& r : rows) {
        double v = metric == 0 ? r.temperature :
                   metric == 1 ? r.humidity : r.wind;
        maxValue = max(maxValue, v);
    }

    double minValue = metric == 0 ? 0 : 0;
    if (metric == 0) {
        minValue = 0;
        for (const auto& r : rows)
            minValue = min(minValue, r.temperature);
    }

    maxValue = maxValue * 1.15;
    if (maxValue <= minValue) maxValue = minValue + 10;

    auto yPos = [&](double v) {
        return top + plotH - (v - minValue) / (maxValue - minValue) * plotH;
    };

    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
        << "width=\"" << W << "\" height=\"" << H
        << "\" viewBox=\"0 0 " << W << " " << H << "\">\n";
    out << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n";
    out << "<text x=\"" << W/2 << "\" y=\"38\" text-anchor=\"middle\" "
        << "font-size=\"24\" font-family=\"Arial\" font-weight=\"bold\">"
        << escapeXml(title) << "</text>\n";

    for (int i = 0; i <= 5; i++) {
        double value = minValue + (maxValue - minValue) * i / 5.0;
        int y = static_cast<int>(yPos(value));
        out << "<line x1=\"" << left << "\" y1=\"" << y
            << "\" x2=\"" << W-right << "\" y2=\"" << y
            << "\" stroke=\"#dddddd\"/>\n";
        out << "<text x=\"" << left-12 << "\" y=\"" << y+5
            << "\" text-anchor=\"end\" font-size=\"13\" font-family=\"Arial\">"
            << value << "</text>\n";
    }

    out << "<line x1=\"" << left << "\" y1=\"" << top
        << "\" x2=\"" << left << "\" y2=\"" << H-bottom
        << "\" stroke=\"black\"/>\n";
    out << "<line x1=\"" << left << "\" y1=\"" << H-bottom
        << "\" x2=\"" << W-right << "\" y2=\"" << H-bottom
        << "\" stroke=\"black\"/>\n";

    const string colors[] = {"#1769aa", "#d97706", "#14804a"};
    int n = static_cast<int>(rows.size());

    if (n == 1) {
        double v = metric == 0 ? rows[0].temperature :
                   metric == 1 ? rows[0].humidity : rows[0].wind;
        out << "<circle cx=\"" << left + plotW/2 << "\" cy=\""
            << yPos(v) << "\" r=\"6\" fill=\"" << colors[metric] << "\"/>\n";
    } else if (n > 1) {
        for (int i = 0; i < n-1; i++) {
            auto value = [&](int j) {
                return metric == 0 ? rows[j].temperature :
                       metric == 1 ? rows[j].humidity : rows[j].wind;
            };
            int x1 = left + i * plotW / (n-1);
            int x2 = left + (i+1) * plotW / (n-1);
            out << "<line x1=\"" << x1 << "\" y1=\"" << yPos(value(i))
                << "\" x2=\"" << x2 << "\" y2=\"" << yPos(value(i+1))
                << "\" stroke=\"" << colors[metric] << "\" stroke-width=\"3\"/>\n";
            out << "<circle cx=\"" << x1 << "\" cy=\"" << yPos(value(i))
                << "\" r=\"5\" fill=\"" << colors[metric] << "\"/>\n";
        }
        double last = metric == 0 ? rows.back().temperature :
                      metric == 1 ? rows.back().humidity : rows.back().wind;
        out << "<circle cx=\"" << W-right << "\" cy=\"" << yPos(last)
            << "\" r=\"5\" fill=\"" << colors[metric] << "\"/>\n";
    }

    out << "<text x=\"" << W/2 << "\" y=\"" << H-25
        << "\" text-anchor=\"middle\" font-size=\"15\" font-family=\"Arial\">"
        << "Saved readings (oldest to newest)</text>\n";
    out << "</svg>\n";
}

int main() {
    sqlite3* db = nullptr;
    if (sqlite3_open("weather_history.db", &db) != SQLITE_OK) {
        cerr << "Could not open weather_history.db\n";
        if (db) sqlite3_close(db);
        return 1;
    }

    const char* sql =
        "SELECT city, saved_at, temperature, humidity, wind_speed "
        "FROM snapshots ORDER BY saved_at ASC;";

    char* error = nullptr;
    int rc = sqlite3_exec(db, sql, loadReading, &readings, &error);
    if (rc != SQLITE_OK) {
        cerr << "Database query failed: "
             << (error ? error : "unknown error") << "\n";
        sqlite3_free(error);
        sqlite3_close(db);
        return 1;
    }
    sqlite3_close(db);

    if (readings.empty()) {
        cout << "No saved readings found. Fetch weather for a few cities first.\n";
        return 0;
    }

    drawChart("temperature_chart.svg", "Temperature Trend (Celsius)",
              readings, 0);
    drawChart("humidity_chart.svg", "Humidity Trend (Percent)",
              readings, 1);
    drawChart("wind_chart.svg", "Wind Speed Trend (km/h)",
              readings, 2);

    cout << "Charts generated successfully:\n"
         << " - temperature_chart.svg\n"
         << " - humidity_chart.svg\n"
         << " - wind_chart.svg\n";
    return 0;
}
