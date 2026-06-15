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
String weatherDesc = "";
String cityName = "Minsk";
String units = "metric";
String localIP = "";
unsigned long lastUpdate = 0;
bool apModeActive = false;
bool webServerStarted = false;

// Поворот экрана в горизонталь
void setupDisplay() {
    M5.Display.setRotation(1);
    M5.Display.setBrightness(100);
}

// Рисование иконки WiFi
void drawWifiIcon(int x, int y, int strength = 4) {
    for (int i = 0; i < 3; i++) {
        int radius = 14 - i * 4;
        int arcStart = 210 + i * 15;
        int arcEnd = 330 - i * 15;
        if (strength > i) {
            M5.Display.fillArc(x, y, radius + 2, radius - 2, arcStart, arcEnd, TFT_WHITE);
        } else {
            M5.Display.drawArc(x, y, radius + 2, radius - 2, arcStart, arcEnd, TFT_DARKGREY);
        }
    }
    M5.Display.fillCircle(x, y + 14, 3, TFT_WHITE);
}

// Рисование иконки батареи (только пиктограмма)
void drawBatteryIcon(int x, int y) {
    int batteryLevel = M5.Power.getBatteryLevel();
    
    // Корпус батареи
    M5.Display.drawRect(x, y, 22, 11, TFT_WHITE);
    M5.Display.fillRect(x + 22, y + 3, 3, 5, TFT_WHITE);
    
    // Заливка в зависимости от заряда
    int fillWidth = map(batteryLevel, 0, 100, 0, 20);
    fillWidth = constrain(fillWidth, 0, 20);
    
    uint16_t batteryColor;
    if (batteryLevel > 50) {
        batteryColor = TFT_GREEN;
    } else if (batteryLevel > 20) {
        batteryColor = TFT_YELLOW;
    } else {
        batteryColor = TFT_RED;
    }
    
    if (fillWidth > 0) {
        M5.Display.fillRect(x + 1, y + 1, fillWidth, 9, batteryColor);
    }
    
    // Если заряжается — рисуем молнию
    if (M5.Power.isCharging()) {
        M5.Display.drawLine(x + 10, y + 2, x + 7, y + 6, TFT_GREEN);
        M5.Display.drawLine(x + 7, y + 6, x + 12, y + 6, TFT_GREEN);
        M5.Display.drawLine(x + 12, y + 6, x + 9, y + 9, TFT_GREEN);
    }
}

// Рисование иконки градусника
void drawThermometerIcon(int x, int y, float temp) {
    M5.Display.fillRoundRect(x, y, 10, 35, 5, TFT_WHITE);
    M5.Display.fillCircle(x + 5, y + 38, 8, TFT_WHITE);
    
    int fillHeight = constrain(map(temp, -30, 50, 0, 30), 0, 30);
    if (fillHeight > 0) {
        M5.Display.fillRoundRect(x, y + 35 - fillHeight, 10, fillHeight, 5, TFT_RED);
        M5.Display.fillCircle(x + 5, y + 38, 7, TFT_RED);
    }
}

// Рисование иконки влажности
void drawHumidityIcon(int x, int y, int hum) {
    M5.Display.fillTriangle(x, y + 10, x + 8, y + 10, x + 4, y, TFT_CYAN);
    M5.Display.fillCircle(x + 4, y + 14, 6, TFT_CYAN);
    M5.Display.fillCircle(x + 4, y + 12, 3, TFT_BLACK);
}

// Рисование иконки погоды
void drawWeatherIcon(int x, int y) {
    String desc = weatherDesc;
    desc.toLowerCase();
    
    if (desc.indexOf("rain") >= 0) {
        M5.Display.fillCircle(x - 10, y, 10, TFT_LIGHTGRAY);
        M5.Display.fillCircle(x + 5, y - 6, 12, TFT_LIGHTGRAY);
        M5.Display.fillCircle(x + 18, y, 9, TFT_LIGHTGRAY);
        M5.Display.fillRect(x - 18, y - 2, 44, 12, TFT_LIGHTGRAY);
        M5.Display.fillTriangle(x - 5, y + 10, x - 8, y + 18, x - 2, y + 18, TFT_CYAN);
        M5.Display.fillTriangle(x + 5, y + 12, x + 2, y + 20, x + 8, y + 20, TFT_CYAN);
        M5.Display.fillTriangle(x + 15, y + 10, x + 12, y + 18, x + 18, y + 18, TFT_CYAN);
    }
    else if (desc.indexOf("cloud") >= 0) {
        M5.Display.fillCircle(x - 10, y, 10, TFT_LIGHTGRAY);
        M5.Display.fillCircle(x + 5, y - 6, 12, TFT_LIGHTGRAY);
        M5.Display.fillCircle(x + 18, y, 9, TFT_LIGHTGRAY);
        M5.Display.fillRect(x - 18, y - 2, 44, 12, TFT_LIGHTGRAY);
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

// Экран с IP адресом
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
    
    drawWifiIcon(210, 20);
    drawBatteryIcon(180, 15);
}

// Главный экран с погодой
void drawWeatherScreen() {
    M5.Display.fillScreen(TFT_BLACK);
    
    drawWifiIcon(215, 15);
    drawBatteryIcon(185, 15);
    
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(10, 15);
    M5.Display.println(cityName);
    
    M5.Display.setTextSize(4);
    M5.Display.setTextColor(TFT_ORANGE);
    M5.Display.setCursor(10, 40);
    M5.Display.printf("%.0f", temperature);
    M5.Display.setTextSize(2);
    M5.Display.print(units == "metric" ? "°C" : "°F");
    
    drawThermometerIcon(130, 45, temperature);
    
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setCursor(10, 90);
    M5.Display.printf("%d", humidity);
    M5.Display.setTextSize(1);
    M5.Display.print("%");
    
    drawHumidityIcon(80, 88, humidity);
    drawWeatherIcon(200, 70);
}

// Экран AP режима
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
    
    drawWifiIcon(210, 20);
    drawBatteryIcon(180, 15);
}

// Экран подключения
void drawConnectingScreen(int attempt) {
    M5.Display.fillScreen(TFT_BLACK);
    
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(10, 35);
    M5.Display.print("Connecting");
    
    for (int i = 0; i < (attempt % 4); i++) {
        M5.Display.print(".");
    }
    
    drawWifiIcon(210, 20, 1);
    drawBatteryIcon(180, 15);
}

// Обновление погоды
void updateWeather() {
    if (WiFi.status() != WL_CONNECTED) return;
    
    String url = "http://wttr.in/" + cityName + "?format=%t+%h+%C&m";
    
    http.begin(url);
    int httpCode = http.GET();
    
    if (httpCode == 200) {
        String response = http.getString();
        response.trim();
        
        int firstSpace = response.indexOf(' ');
        int secondSpace = response.indexOf(' ', firstSpace + 1);
        
        if (firstSpace > 0 && secondSpace > 0) {
            temperature = response.substring(0, firstSpace).toInt();
            humidity = response.substring(firstSpace + 1, secondSpace).toInt();
            weatherDesc = response.substring(secondSpace + 1);
            weatherDesc.trim();
        }
    }
    http.end();
}

// Веб-сервер
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

// Сброс настроек
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

// ===== SETUP =====
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
    
    // Принудительно ставим Минск
    prefs.begin("weather", false);
    prefs.putString("city", "Minsk");
    prefs.putString("units", "metric");
    prefs.end();
    
    cityName = "Minsk";
    units = "metric";
    
    // Загрузка сохранённого WiFi
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

// ===== LOOP =====
void loop() {
    M5.update();
    
    if (webServerStarted) {
        server.handleClient();
    }
    
    if (apModeActive) {
        delay(50);
        return;
    }
    
    // Обновление погоды по таймеру
    if (millis() - lastUpdate > UPDATE_INTERVAL) {
        lastUpdate = millis();
        if (WiFi.status() == WL_CONNECTED) {
            updateWeather();
            drawWeatherScreen();
        }
    }
    
    // Обновление заряда батареи каждые 10 секунд (только пиктограмма)
    static unsigned long lastBatteryUpdate = 0;
    if (millis() - lastBatteryUpdate > 10000) {
        lastBatteryUpdate = millis();
        drawBatteryIcon(185, 15);
    }
    
    // Кнопка M5 для ручного обновления
    if (M5.BtnA.wasPressed()) {
        if (WiFi.status() == WL_CONNECTED) {
            updateWeather();
            drawWeatherScreen();
        }
    }
    
    // Долгое нажатие для сброса
    if (M5.BtnA.pressedFor(5000)) {
        resetSettings();
    }
    
    delay(50);
}