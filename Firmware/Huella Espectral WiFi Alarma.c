#include <WiFi.h>

struct APData {
  String bssid;
  float rssi_sum = 0;
  float rssi_sum_sq = 0;
  int count = 0;
  float mean = 0;
  float stddev = 0;
};

#include <vector>
std::vector<APData> calibrationData;

const unsigned long CALIBRATION_TIME_MS = 30000;
const unsigned long SCAN_INTERVAL_MS = 2000;
const float THRESHOLD = 25.0;  // Ajustar según sensibilidad deseada

unsigned long calibrationStart = 0;
bool calibrated = false;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
     delay(10);
  }
  Serial.println("UART Init");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); // Para evitar conexión automática

  Serial.println("Colocar el ESP32S3 sin movimiento para calibracion (30 segundos)...");
  calibrationStart = millis();
}

void loop() {
  if (!calibrated) {
    // Durante calibracion
    if (millis() - calibrationStart < CALIBRATION_TIME_MS) {
      scanAndAccumulate();
      delay(SCAN_INTERVAL_MS);
    } else {
      computeCalibrationStats();
      calibrated = true;
      Serial.println("Calibracion terminada. Modo activo.");
    }
  } else {
    // Modo activo
    bool alarm = detectMovement();
    if (alarm) {
      Serial.println("<<<<<<<<<<  ALARMA  >>>>>>>>>>");
    }
    delay(SCAN_INTERVAL_MS);
  }
}

void scanAndAccumulate() {
  int n = WiFi.scanNetworks(false, true, false, 300);
  if (n == 0) {
    Serial.println("No se detectaron APs durante calibracion");
    return;
  }

  for (int i = 0; i < n; i++) {
    String bssid = WiFi.BSSIDstr(i);
    int rssi = WiFi.RSSI(i);

    int idx = findAPIndex(bssid);
    if (idx < 0) {
      // Nuevo AP
      APData newAP;
      newAP.bssid = bssid;
      newAP.rssi_sum = rssi;
      newAP.rssi_sum_sq = rssi * rssi;
      newAP.count = 1;
      calibrationData.push_back(newAP);
    } else {
      // AP existente, acumular
      calibrationData[idx].rssi_sum += rssi;
      calibrationData[idx].rssi_sum_sq += rssi * rssi;
      calibrationData[idx].count++;
    }
  }
  WiFi.scanDelete();
}

int findAPIndex(const String &bssid) {
  for (int i = 0; i < (int)calibrationData.size(); i++) {
    if (calibrationData[i].bssid == bssid) {
      return i;
    }
  }
  return -1;
}

void computeCalibrationStats() {
  for (auto &ap : calibrationData) {
    if (ap.count > 0) {
      ap.mean = ap.rssi_sum / ap.count;
      float variance = (ap.rssi_sum_sq / ap.count) - (ap.mean * ap.mean);
      ap.stddev = (variance > 0) ? sqrt(variance) : 1.0; // mínimo stddev para evitar división por cero
    } else {
      ap.mean = 0;
      ap.stddev = 1.0;
    }
    // Debug
    Serial.printf("AP %s: media=%.2f dBm, std=%.2f dBm, muestras=%d\n",
                  ap.bssid.c_str(), ap.mean, ap.stddev, ap.count);
  }
}

bool detectMovement() {
  int n = WiFi.scanNetworks(false, true, false, 300); // passive scan 300ms
  if (n == 0) {
    Serial.println("No se detectaron APs en modo activo");
    return false;
  }

  float totalDiff = 0;
  for (auto &ap : calibrationData) {
    // Buscar mismo AP en scan actual
    int idx = -1;
    for (int i = 0; i < n; i++) {
      if (WiFi.BSSIDstr(i) == ap.bssid) {
        idx = i;
        break;
      }
    }
    if (idx >= 0) {
      int rssi_now = WiFi.RSSI(idx);
      float diff_norm = fabs(rssi_now - ap.mean) / ap.stddev;
      totalDiff += diff_norm;
    } else {
      // AP no encontrado, contar como diferencia grande
      totalDiff += 3.0; // valor arbitrario, ajustar según prueba
    }
  }
  WiFi.scanDelete();

  // Debug
  Serial.printf("Diferencia total: %.2f\n", totalDiff);

  return (totalDiff > THRESHOLD);
}
