#define BLYNK_TEMPLATE_ID "TMPL6zow1kl3l"
#define BLYNK_TEMPLATE_NAME "Sistem Pemantauan Kualitas Air"
#define BLYNK_AUTH_TOKEN "89ISBM33PP3u9Oq-q6SBjRIvaWvWR6mh"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ================= WIFI =================
char ssid[] = "Themalau";
char pass[] = "24011982";

// ================= pH SENSOR =================
float calibration_value = 21.34 + 1.0;
unsigned long int avgval;
int buffer_arr[10], temp;
float ph_act;

#define PH_SENSOR_PIN 34

// ================= TURBIDITY SENSOR =================
#define TURB_SENSOR_PIN 35

// ================= SUHU DS18B20 =================
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float suhuAir;

BlynkTimer timer;

// ================= FUNGSI BACA & KIRIM =================
void sendDataToBlynk() {

  // ===== pH =====
  for (int i = 0; i < 10; i++) {
    buffer_arr[i] = analogRead(PH_SENSOR_PIN);
    delay(30);
  }

  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (buffer_arr[i] > buffer_arr[j]) {
        temp = buffer_arr[i];
        buffer_arr[i] = buffer_arr[j];
        buffer_arr[j] = temp;
      }
    }
  }

  avgval = 0;
  for (int i = 2; i < 8; i++) {
    avgval += buffer_arr[i];
  }

  float volt = (float)avgval * 3.3 / 4095.0 / 6;
  ph_act = -5.70 * volt + calibration_value;

  // ===== TURBIDITY =====
  int turbidityValue = analogRead(TURB_SENSOR_PIN);
  int kal = (turbidityValue - 2745.6) / -18.189;

  // ===== STATUS AIR =====
  String status;
  if (kal > 0 && kal < 24) {
    status = "Jernih";
  } else if (kal >= 25 && kal < 49) {
    status = "Sedikit Keruh";
  } else if (kal >= 50 && kal < 74) {
    status = "Keruh";
  } else if (kal >= 75 && kal < 150) {
    status = "Sangat Keruh";
  } else {
    status = "Tidak Terdefinisi";
  }

  // ===== SUHU =====
  sensors.requestTemperatures();
  suhuAir = sensors.getTempCByIndex(0);


  // ===== KIRIM KE BLYNK =====
  Blynk.virtualWrite(V0, suhuAir);
  Blynk.virtualWrite(V1, ph_act);
  Blynk.virtualWrite(V2, kal);
  Blynk.virtualWrite(V3, status);

  // ===== SERIAL =====
  Serial.println("===== KUALITAS AIR =====");
  Serial.print("pH: "); Serial.println(ph_act, 2);
  Serial.print("Suhu: "); Serial.print(suhuAir); Serial.println(" °C");
  Serial.print("Nilai Kekeruhan: "); Serial.println(kal);
  Serial.print("Status Air: "); Serial.println(status);
  Serial.println("------------------------\n");
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  sensors.begin();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(2000L, sendDataToBlynk);
}

void loop() {
  Blynk.run();
  timer.run();
}