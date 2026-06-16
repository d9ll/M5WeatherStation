#include <M5Unified.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include "html_pages.h"

#define AP_SSID "M5WeatherStation"
#define AP_PASS "weather123"
#define UPDATE_INTERVAL 60000

Preferences prefs;
WebServer server(80);
HTTPClient http;

float temperature = 0;
int humidity = 0;
int pressure = 0;
int windSpeed = 0;
int feelsLike = 0;
String weatherDesc = "";
String cityName = "";
String units = "metric";
String localIP = "";
unsigned long lastUpdate = 0;
bool apModeActive = false;
bool webServerStarted = false;

void setupDisplay() {
    M5.Display.setRotation(1);
    M5.Display.setBrightness(100);
}

String getComfortIcon(float temp) {
    if (temp >= 25) return "🔥";
    else if (temp >= 18) return "☀️";
    else if (temp >= 10) return "🌤️";
    else if (temp >= 0) return "❄️";
    else return "🥶";
}

void drawWeatherIcon(int x, int y) {
    String desc = weatherDesc;
    desc.toLowerCase();
    
    if (desc.indexOf("rain") >= 0 || desc.indexOf("drizzle") >= 0) {
        M5.Display.fillCircle(x - 10, y, 10, TFT_LIGHTGRAY);
        M5.Display.fillCircle(x + 5, y - 6, 12, TFT_LIGHTGRAY);
        M5.Display.fillCircle(x + 18, y, 9, TFT_LIGHTGRAY);
        M5.Display.fillRect(x - 18, y - 2, 44, 12, TFT_LIGHTGRAY);
        M5.Display.fillTriangle(x - 5, y + 10, x - 8, y + 18, x - 2, y + 18, TFT_CYAN);
        M5.Display.fillTriangle(x + 5, y + 12, x + 2, y + 20, x + 8, y + 20, TFT_CYAN);
        M5.Display.fillTriangle(x + 15, y + 10, x + 12, y + 18, x + 18, y + 18, TFT_CYAN);
    }
    else if (desc.indexOf("cloud") >= 0 || desc.indexOf("overcast") >= 0) {
        M5.Display.fillCircle(x - 10, y, 10, TFT_LIGHTGRAY);
        M5.Display.fillCircle(x + 5, y - 6, 12, TFT_LIGHTGRAY);
        M5.Display.fillCircle(x + 18, y, 9, TFT_LIGHTGRAY);
        M5.Display.fillRect(x - 18, y - 2, 44, 12, TFT_LIGHTGRAY);
    }
    else if (desc.indexOf("snow") >= 0) {
        M5.Display.fillCircle(x, y, 12, TFT_WHITE);
        for (int i = 0; i < 6; i++) {
            float angle = i * 60 * 3.14159 / 180;
            int dx = cos(angle) * 18;
            int dy = sin(angle) * 18;
            M5.Display.drawLine(x, y, x + dx, y + dy, TFT_WHITE);
        }
    }
    else if (desc.indexOf("thunder") >= 0 || desc.indexOf("storm") >= 0) {
        M5.Display.fillCircle(x - 10, y, 10, TFT_LIGHTGRAY);
        M5.Display.fillCircle(x + 5, y - 6, 12, TFT_LIGHTGRAY);
        M5.Display.fillCircle(x + 18, y, 9, TFT_LIGHTGRAY);
        M5.Display.fillRect(x - 18, y - 2, 44, 12, TFT_LIGHTGRAY);
        M5.Display.fillTriangle(x, y + 5, x - 5, y + 15, x + 5, y + 10, TFT_YELLOW);
    }
    else {
        M5.Display.fillCircle(x, y, 12, TFT_YELLOW);
        for (int i = 0; i < 12; i++) {
            float angle = i * 30 * 3.14159 / 180;
            int dx = cos(angle) * 18;
            int dy = sin(angle) * 18;
            M5.Display.drawLine(x + dx, y + dy, x + dx * 1.4, y + dy * 1.4, TFT_YELLOW);
        }
    }
}

void drawWeatherScreen() {
    M5.Display.fillScreen(TFT_BLACK);
    
    int batteryLevel = M5.Power.getBatteryLevel();
    String comfort = getComfortIcon(temperature);
    
    // Строка 1: Город + Батарея
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(5, 5);
    M5.Display.print(cityName);
    
    M5.Display.setCursor(165, 5);
    M5.Display.print("Bat: ");
    if (batteryLevel > 50) M5.Display.setTextColor(TFT_GREEN);
    else if (batteryLevel > 20) M5.Display.setTextColor(TFT_YELLOW);
    else M5.Display.setTextColor(TFT_RED);
    M5.Display.printf("%d%%", batteryLevel);
    
    // Строка 2: Температура (крупно)
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(5, 18);
    M5.Display.print("Temp: ");
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(TFT_ORANGE);
    M5.Display.printf("%.0f", temperature);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.print(" C");
    
    // Строка 3: Ощущается как
    M5.Display.setTextSize(1);
    M5.Display.setCursor(5, 46);
    M5.Display.print("Feels: ");
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.printf("%d C", feelsLike);
    
    // Строка 4: Влажность
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(5, 60);
    M5.Display.print("Hum:   ");
    M5.Display.setTextColor(TFT_CYAN);
    if (humidity > 0) {
        M5.Display.printf("%d %%", humidity);
    } else {
        M5.Display.print("-- %");
    }
    
    // Строка 5: Давление
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(5, 74);
    M5.Display.print("Press: ");
    M5.Display.setTextColor(TFT_GREEN);
    if (pressure > 0) {
        M5.Display.printf("%d hPa", pressure);
    } else {
        M5.Display.print("-- hPa");
    }
    
    // Строка 6: Ветер
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(5, 88);
    M5.Display.print("Wind:  ");
    M5.Display.setTextColor(TFT_WHITE);
    if (windSpeed > 0) {
        M5.Display.printf("%d km/h", windSpeed);
    } else {
        M5.Display.print("-- km/h");
    }
    
    // Строка 7: Описание погоды
    M5.Display.setCursor(5, 102);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.print("Cond:  ");
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.printf("%s", weatherDesc.substring(0, 14).c_str());
    
    // Иконка комфорта и погоды
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(145, 40);
    M5.Display.print(comfort);
    
    drawWeatherIcon(190, 48);
}

void drawIPScreen() {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.setCursor(10, 20);
    M5.Display.println("WiFi Connected!");
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(10, 55);
    M5.Display.println("Open in browser:");
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setCursor(10, 80);
    M5.Display.println(localIP);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setCursor(10, 110);
    M5.Display.println("/setup - choose city");
}

void drawAPScreen() {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setCursor(10, 10);
    M5.Display.println("AP MODE");
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(10, 45);
    M5.Display.print("SSID: ");
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.println(AP_SSID);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(10, 65);
    M5.Display.print("Pass:  ");
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.println(AP_PASS);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(10, 85);
    M5.Display.print("IP:    ");
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.println("192.168.4.1");
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.setCursor(10, 110);
    M5.Display.println("Open in browser");
}

void drawConnectingScreen(int attempt) {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(10, 35);
    M5.Display.print("Connecting");
    for (int i = 0; i < (attempt % 4); i++) M5.Display.print(".");
}

void parseWeatherLine(String line, float &temp, int &hum, int &press, int &wind, String &desc) {
    temp = 0;
    hum = 0;
    press = 0;
    wind = 0;
    desc = "";
    
    int pos = 0;
    while (pos < line.length()) {
        int nextSpace = line.indexOf(' ', pos);
        if (nextSpace == -1) nextSpace = line.length();
        
        String part = line.substring(pos, nextSpace);
        part.trim();
        
        if (part.indexOf("°C") >= 0 || part.indexOf("°F") >= 0) {
            temp = part.toInt();
        }
        else if (part.indexOf("%") >= 0) {
            hum = part.toInt();
        }
        else if ((part.indexOf("km/h") >= 0 || part.indexOf("m/s") >= 0) && part.length() > 0) {
            String numPart = part;
            numPart.replace("km/h", "");
            numPart.replace("m/s", "");
            numPart.replace("↗", "");
            numPart.replace("↘", "");
            numPart.replace("→", "");
            numPart.replace("←", "");
            numPart.replace("↑", "");
            numPart.replace("↓", "");
            numPart.trim();
            if (numPart.length() > 0) {
                wind = numPart.toInt();
            }
        }
        else if (part.indexOf("hPa") >= 0) {
            String numPart = part;
            numPart.replace("hPa", "");
            numPart.trim();
            press = numPart.toInt();
        }
        else if (part.length() > 0) {
            if (desc.length() == 0) {
                desc = part;
            } else {
                desc += " " + part;
            }
        }
        
        pos = nextSpace + 1;
    }
    
    if (desc.length() == 0) {
        desc = line;
    }
}

void updateWeather() {
    if (WiFi.status() != WL_CONNECTED) return;
    
    String url = "http://wttr.in/" + cityName + "?format=%t+%h+%P+%w+%C&m";
    
    http.begin(url);
    int httpCode = http.GET();
    
    if (httpCode == 200) {
        String response = http.getString();
        response.trim();
        Serial.println("Raw: " + response);
        
        parseWeatherLine(response, temperature, humidity, pressure, windSpeed, weatherDesc);
        
        feelsLike = temperature;
        if (humidity > 70) feelsLike -= 2;
        if (windSpeed > 10) feelsLike -= 1;
        
        Serial.println("Parsed: Temp=" + String(temperature) + " Hum=" + String(humidity) + 
                       " Press=" + String(pressure) + " Wind=" + String(windSpeed) + 
                       " Desc=" + weatherDesc);
    } else {
        Serial.println("HTTP error: " + String(httpCode));
    }
    http.end();
}

void handleRoot() {
    server.send(200, "text/html", CONNECT_HTML);
}

void handleSetup() {
    String html = String(SETUP_HTML);
    html.replace("value=\"Moscow\"", "value=\"" + cityName + "\"");
    if (units == "imperial") {
        html.replace("value=\"metric\" selected", "value=\"metric\"");
        html.replace("value=\"imperial\"", "value=\"imperial\" selected");
    }
    server.send(200, "text/html", html);
}

void handleConnect() {
    if (server.hasArg("ssid") && server.hasArg("pass")) {
        prefs.begin("wifi", false);
        prefs.putString("ssid", server.arg("ssid"));
        prefs.putString("pass", server.arg("pass"));
        prefs.end();
        server.send(200, "text/html", "<html><body><h1>✅ WiFi Saved! Rebooting...</h1></body></html>");
        delay(1000);
        ESP.restart();
    }
}

void handleSave() {
    if (server.hasArg("city") && server.hasArg("units")) {
        String newCity = server.arg("city");
        String newUnits = server.arg("units");
        
        prefs.begin("weather", false);
        prefs.putString("city", newCity);
        prefs.putString("units", newUnits);
        prefs.end();
        
        cityName = newCity;
        units = newUnits;
        
        updateWeather();
        drawWeatherScreen();
        
        server.send(200, "text/html", "<html><body><h1>✅ Saved! City: " + newCity + "</h1></body></html>");
        delay(1500);
        ESP.restart();
    }
}

void startAPMode() {
    apModeActive = true;
    WiFi.softAP(AP_SSID, AP_PASS);
    drawAPScreen();
}

void resetSettings() {
    prefs.begin("wifi", false);
    prefs.clear();
    prefs.end();
    
    prefs.begin("weather", false);
    prefs.clear();
    prefs.end();
    
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_RED);
    M5.Display.setCursor(10, 40);
    M5.Display.println("Settings cleared!");
    M5.Display.setCursor(10, 58);
    M5.Display.println("Rebooting...");
    delay(2000);
    ESP.restart();
}

void setup() {
    delay(100);
    auto cfg = M5.config();
    cfg.output_power = true;
    M5.begin(cfg);
    delay(100);
    setupDisplay();
    Serial.begin(115200);
    delay(100);
    Serial.println("M5WeatherStation starting...");
    
    prefs.begin("weather", true);
    cityName = prefs.getString("city", "Minsk");
    units = prefs.getString("units", "metric");
    prefs.end();
    Serial.println("Loaded city: " + cityName);
    
    prefs.begin("wifi", true);
    String ssid = prefs.getString("ssid", "");
    String pass = prefs.getString("pass", "");
    prefs.end();
    
    if (ssid.length() > 0 && ssid != "") {
        Serial.print("Connecting to WiFi: ");
        Serial.println(ssid);
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), pass.c_str());
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            drawConnectingScreen(attempts);
            delay(500);
            attempts++;
            Serial.print(".");
        }
        Serial.println();
        if (WiFi.status() == WL_CONNECTED) {
            localIP = WiFi.localIP().toString();
            Serial.print("Connected! IP: ");
            Serial.println(localIP);
            drawIPScreen();
            delay(3000);
            updateWeather();
            drawWeatherScreen();
            server.on("/", handleRoot);
            server.on("/setup", handleSetup);
            server.on("/connect", HTTP_POST, handleConnect);
            server.on("/save", HTTP_POST, handleSave);
            server.begin();
            webServerStarted = true;
            Serial.println("Web server started");
        } else {
            Serial.println("WiFi connection failed, starting AP mode");
            startAPMode();
            server.on("/", handleRoot);
            server.on("/setup", handleSetup);
            server.on("/connect", HTTP_POST, handleConnect);
            server.on("/save", HTTP_POST, handleSave);
            server.begin();
            webServerStarted = true;
        }
    } else {
        Serial.println("No WiFi saved, starting AP mode");
        startAPMode();
        server.on("/", handleRoot);
        server.on("/setup", handleSetup);
        server.on("/connect", HTTP_POST, handleConnect);
        server.on("/save", HTTP_POST, handleSave);
        server.begin();
        webServerStarted = true;
    }
    
    lastUpdate = millis();
}

void loop() {
    M5.update();
    if (webServerStarted) {
        server.handleClient();
    }
    if (apModeActive) {
        delay(50);
        return;
    }
    
    if (millis() - lastUpdate > UPDATE_INTERVAL) {
        lastUpdate = millis();
        if (WiFi.status() == WL_CONNECTED) {
            updateWeather();
            drawWeatherScreen();
        }
    }
    
    if (M5.BtnA.wasPressed()) {
        Serial.println("M5 pressed — refreshing");
        if (WiFi.status() == WL_CONNECTED) {
            updateWeather();
            drawWeatherScreen();
        }
    }
    
    if (M5.BtnA.pressedFor(5000)) {
        resetSettings();
    }
    
    delay(50);
}
