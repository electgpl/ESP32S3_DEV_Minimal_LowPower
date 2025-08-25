#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

#define TFT_CS    10
#define TFT_DC    9
#define TFT_RST   8
#define TFT_MOSI  11  //SDA en Pantalla
#define TFT_SCLK  12  //SCK en Pantalla
#define TFT_BLK   7

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

struct APData {
  String bssid;
  float rssi_sum = 0;
  float rssi_sum_sq = 0;
  int count = 0;
  float mean = 0;
  float stddev = 1.0;
  float lastDiffNorm = 0;
};

#include <vector>
std::vector<APData> calibrationData;

const unsigned long CALIBRATION_TIME_MS = 30000;
const unsigned long SCAN_INTERVAL_MS = 2000;
const float THRESHOLD = 25.0;

unsigned long calibrationStart = 0;
bool calibrated = false;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);  //Demora porque el S3 a veces tarda un poco mas en mandar los datos por UART tras el boot

  pinMode(TFT_BLK, OUTPUT);
  digitalWrite(TFT_BLK, HIGH); // Enciende la retroiluminación

  tft.init(240, 240, SPI_MODE3);
  tft.setRotation(2);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3);
  tft.setCursor(10, 10);
  tft.print("Electgpl");
  tft.setTextSize(2);
  tft.setCursor(10, 50);
  tft.print("Huella Espectral");
  tft.setCursor(10, 70);
  tft.print("Espera Colocacion");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  Serial.println("Coloque el ESP32 sin movimiento...");
  calibrationStart = millis();
}

void loop() {
  if (!calibrated) {
    if (millis() - calibrationStart < CALIBRATION_TIME_MS) {
      scanAndAccumulate();
      delay(SCAN_INTERVAL_MS);
    } else {
      computeCalibrationStats();
      calibrated = true;
      Serial.println("Calibracion terminada.");
      tft.fillScreen(ST77XX_BLACK);
      tft.setCursor(10, 10);
      tft.println("Ararma Activa");
      delay(1000);
    }
  } else {
    bool alarm = detectAndDraw();
    if (alarm) {
      Serial.println(">>> ALARMA <<<");
      drawAlarm();
      delay(1000);
    }
    delay(SCAN_INTERVAL_MS);
  }
}

void scanAndAccumulate() {
  int n = WiFi.scanNetworks(false, true, false, 300);
  if (n == 0) return;

  for (int i = 0; i < n; i++) {
    String bssid = WiFi.BSSIDstr(i);
    int rssi = WiFi.RSSI(i);
    int idx = findAPIndex(bssid);
    if (idx < 0) {
      APData newAP;
      newAP.bssid = bssid;
      newAP.rssi_sum = rssi;
      newAP.rssi_sum_sq = rssi * rssi;
      newAP.count = 1;
      calibrationData.push_back(newAP);
    } else {
      calibrationData[idx].rssi_sum += rssi;
      calibrationData[idx].rssi_sum_sq += rssi * rssi;
      calibrationData[idx].count++;
    }
  }
  WiFi.scanDelete();
}

int findAPIndex(const String &bssid) {
  for (int i = 0; i < (int)calibrationData.size(); i++) {
    if (calibrationData[i].bssid == bssid) return i;
  }
  return -1;
}

void computeCalibrationStats() {
  for (auto &ap : calibrationData) {
    if (ap.count > 0) {
      ap.mean = ap.rssi_sum / ap.count;
      float var = (ap.rssi_sum_sq / ap.count) - (ap.mean * ap.mean);
      ap.stddev = sqrt(var);
      if (ap.stddev < 1.0) ap.stddev = 1.0;
    }
    Serial.printf("AP %s: mean=%.1f, std=%.1f, count=%d\n",
                  ap.bssid.c_str(), ap.mean, ap.stddev, ap.count);
  }
}

bool detectAndDraw() {
  int n = WiFi.scanNetworks(false, true, false, 100);
  if (n == 0) return false;

  float totalDiff = 0;
  int maxBars = min(16, (int)calibrationData.size());
  int barWidth = 240 / maxBars;

  tft.fillScreen(ST77XX_BLACK);
  // Dibujar línea de umbral
  for (int x = 0; x < 240; x += 4) {
    tft.drawPixel(x, 30, ST77XX_CYAN);
  }

  for (int i = 0; i < maxBars; i++) {
    APData &ap = calibrationData[i];
    int idx = -1;
    for (int j = 0; j < n; j++) {
      if (WiFi.BSSIDstr(j) == ap.bssid) {
        idx = j;
        break;
      }
    }

    float diffNorm = 0;
    if (idx >= 0) {
      int rssi_now = WiFi.RSSI(idx);
      diffNorm = fabs(rssi_now - ap.mean) / ap.stddev;
    } else {
      diffNorm = 3.0;
    }
    ap.lastDiffNorm = diffNorm;
    totalDiff += diffNorm;

    // Dibujar barra
    int barHeight = min(200, (int)(diffNorm * 40)); // escala ajustable
    int x = i * barWidth;
    int y = 240 - barHeight;

    uint16_t color = ST77XX_GREEN;
    if (diffNorm > 2.0) color = ST77XX_RED;
    else if (diffNorm > 1.0) color = ST77XX_YELLOW;

    tft.fillRect(x + 2, y, barWidth - 4, barHeight, color);

    // UART
    Serial.printf("%s, diff=%.2f\n", ap.bssid.c_str(), diffNorm);
  }

  tft.setCursor(10, 10);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.printf("Diff: %.1f\n", totalDiff);

  WiFi.scanDelete();

  return (totalDiff > THRESHOLD);
}

void drawAlarm() {
  tft.fillScreen(ST77XX_RED);
  tft.setCursor(60, 100);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3);
  tft.println("ALARMA");
}