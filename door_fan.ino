#include <WiFi.h>
#include <FirebaseESP32.h>
#include <ESP32Servo.h> // Thư viện servo cho ESP32

// Thông tin WiFi và Firebase
#define WIFI_SSID "ae38"
#define WIFI_PASSWORD "19216801"
#define FIREBASE_HOST "smart-home-52e19-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "Nphho9OXtu9dMyqppI7Jb0aj7Oe8TWYLpXn7yLpE"

// Định nghĩa chân servo và quạt
const int servoPin = 25;   // Chân kết nối servo
const int fanPin = 26;     // Chân điều khiển quạt (nối relay hoặc transistor)

FirebaseData firebaseDataDoor;
FirebaseData firebaseDataFan;
FirebaseConfig config;
FirebaseAuth auth;

Servo doorServo; // Servo điều khiển cửa

// Hàm kết nối WiFi
void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.println("Đang kết nối với WiFi...");
  }
  Serial.println("Đã kết nối với WiFi!");
}

// Hàm xử lý điều khiển cửa
void handleDoorControl() {
  if (Firebase.readStream(firebaseDataDoor)) {
    if (firebaseDataDoor.dataType() == "boolean") {
      bool doorIsOpen = firebaseDataDoor.boolData();

      // Điều khiển servo dựa trên trạng thái Firebase
      if (doorIsOpen) {
        doorServo.write(90); // Servo xoay đến 90 độ (Cửa mở)
        Serial.println("Door OPEN - Servo ở 90 độ");
      } else {
        doorServo.write(0); // Servo xoay về 0 độ (Cửa đóng)
        Serial.println("Door CLOSED - Servo ở 0 độ");
      }
    }
  } else if (firebaseDataDoor.dataType() == "undefined") {
    Serial.print("Lỗi đọc Firebase (Door): ");
    Serial.println(firebaseDataDoor.errorReason());
  }
}

// Hàm xử lý điều khiển quạt
void handleFanControl() {
  if (Firebase.readStream(firebaseDataFan)) {
    if (firebaseDataFan.dataType() == "boolean") {
      bool fanIsOn = firebaseDataFan.boolData();

      // Điều khiển quạt dựa trên trạng thái Firebase
      if (fanIsOn) {
        digitalWrite(fanPin, HIGH); // Bật quạt
        Serial.println("Fan ON");
      } else {
        digitalWrite(fanPin, LOW); // Tắt quạt
        Serial.println("Fan OFF");
      }
    }
  } else if (firebaseDataFan.dataType() == "undefined") {
    Serial.print("Lỗi đọc Firebase (Fan): ");
    Serial.println(firebaseDataFan.errorReason());
  }
}

void setup() {
  // Khởi động Serial Monitor
  Serial.begin(115200);

  // Kết nối WiFi
  connectWiFi();

  // Khởi tạo servo
  doorServo.attach(servoPin);
  doorServo.write(0); // Đặt góc ban đầu là 0 độ (Cửa đóng)

  // Khởi tạo chân điều khiển quạt
  pinMode(fanPin, OUTPUT);
  digitalWrite(fanPin, LOW); // Đặt trạng thái ban đầu là tắt

  // Cấu hình Firebase
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Kết nối đến streams Firebase
  if (Firebase.beginStream(firebaseDataDoor, "/door/isOn")) {
    Serial.println("Stream Door OK");
  } else {
    Serial.print("Lỗi kết nối Firebase Door: ");
    Serial.println(firebaseDataDoor.errorReason());
  }

  if (Firebase.beginStream(firebaseDataFan, "/fan/isOn")) {
    Serial.println("Stream Fan OK");
  } else {
    Serial.print("Lỗi kết nối Firebase Fan: ");
    Serial.println(firebaseDataFan.errorReason());
  }
}

void loop() {
  // Xử lý luồng trạng thái cửa
  handleDoorControl();

  // Xử lý luồng trạng thái quạt
  handleFanControl();

  // Kiểm tra lại kết nối WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Mất kết nối WiFi! Đang thử kết nối lại...");
    WiFi.reconnect(); // Tự động kết nối lại
  }

  delay(100); // Chờ một thời gian ngắn để tránh quá tải
}
