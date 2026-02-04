#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Longitude {
private:
    double m_lonValue{};

public:
    Longitude() = default;

    explicit Longitude(double value) : m_lonValue(value) {}

    explicit operator double() const {
        return m_lonValue;
    }

    friend std::istream& operator>>(std::istream&, Longitude&);
    friend std::ostream& operator<<(std::ostream&, const Longitude&); 
};

std::istream& operator>>(std::istream& input, Longitude& object) {
    input >>object.m_lonValue;
    return input;
}

std::ostream& operator<<(std::ostream& out, const Longitude& obj) {
    out << obj.m_lonValue;
    return out;
}

Longitude operator""_lon(long double value){
        return Longitude(value);
}

class Latitude {

private:
    double m_latValue{};

public:
    Latitude() = default;

    explicit Latitude(double value) : m_latValue(value) {}

    explicit operator double() const {
        return m_latValue;
    }

    friend std::istream& operator>>(std::istream&, Latitude&);
    friend std::ostream& operator<<(std::ostream&, const Latitude&); 
};

std::istream& operator>>(std::istream& input, Latitude& object) {
    input >> object.m_latValue;
    return input;
}

std::ostream& operator<<(std::ostream& out, const Latitude& obj) {
    out << obj.m_latValue;
    return out;
}

Latitude operator""_lat(long double value) {
        return Latitude(value);
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void displayLocation(const Latitude& lat, const Longitude& lon) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if(curl) {
        std::string url = "https://nominatim.openstreetmap.org/reverse?format=json&lat="
                          + std::to_string(static_cast<double>(lat))
                          + "&lon=" + std::to_string(static_cast<double>(lon));

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Вот это добавляем
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "MyLatitudeApp/1.0 (hostpost3@gmail.com)");

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if(res == CURLE_OK) {
            try {
                auto j = json::parse(readBuffer);
                std::cout << "Местоположение: " << j["display_name"] << std::endl;
            } catch (std::exception& e) {
                std::cerr << "Ошибка парсинга JSON: " << e.what() << std::endl;
            }
        } else {
            std::cerr << "Ошибка запроса: " << res << std::endl;
        } 
   }
}

void OpenMap(const Latitude& lat, const Longitude& lon) {

    std::string url =
        "https://www.google.com/maps?q=" +
        std::to_string(static_cast<double>(lat)) + "," +
        std::to_string(static_cast<double>(lon));

#ifdef __APPLE__
    std::string command = "open \"" + url + "\"";
#elif __linux__
    std::string command = "xdg-open \"" + url + "\"";
#elif _WIN32
    std::string command = "start \"\" \"" + url + "\"";
#endif

    system(command.c_str());
}

int main() {
    Latitude lat;
    Longitude lon;

    std::cout << "set latitude :";
    std::cin >> lat;
    std::cout << "set longitude :";
    std::cin >> lon;
    displayLocation(lat, lon);
    OpenMap(lat, lon);
}
