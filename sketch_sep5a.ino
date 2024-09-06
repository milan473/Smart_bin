#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>

// Initialize the LCD with I2C address (0x27), 16 columns, and 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Initialize the GSM module
SoftwareSerial SIM900A(6, 5);

// Initialize the GPS module
SoftwareSerial gpsSerial(4, 3); // RX (Pin 4), TX (Pin 3)

// Create a TinyGPS++ object
TinyGPSPlus gps;

// HC-SR04 ultrasonic sensor pins
const int trigPin = 8;  // Trigger pin connected to D6 (GPIO12)
const int echoPin = 9;  // Echo pin connected to D7 (GPIO13)

// Define distance threshold in cm for sending an SMS
const float distanceThreshold = 100.0; // Change this value as needed

// Flag for sending SMS
bool sendFlag = false;

void setup() {
  // Start serial communication
  Serial.begin(9600);
  
  // Initialize HC-SR04 sensor pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  
  // Initialize GSM module
  SIM900A.begin(9600);
  
  // Initialize GPS module
  gpsSerial.begin(9600);
  gpsSerial.listen();

  // Print initial message
  lcd.setCursor(0, 0);
  lcd.print("Distance:");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
}

void loop() {
  // Trigger the HC-SR04 sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure the duration of the pulse on the echo pin
  long duration = pulseIn(echoPin, HIGH);

  // Calculate the distance in centimeters
  float distance = (duration / 2.0) * 0.0344;

  // Update the LCD display with the distance
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Distance:");
  lcd.setCursor(0, 1);
  lcd.print(distance);
  lcd.print(" cm");

  // Print the distance to Serial Monitor for debugging
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Check if the distance exceeds the threshold and send SMS if needed
  if (distance > distanceThreshold) {
    if (!sendFlag) {
      sendFlag = true; // Set flag to send message
    }
  }

  // Handle GSM communication
  if (sendFlag) {
    // Ensure GPS data is available
    if (gps.location.isUpdated()) {
      SendMessage(distance, gps.location.lat(), gps.location.lng()); // Send distance and GPS coordinates with live map link
      sendFlag = false; // Reset flag after sending message
    } else {
      Serial.println("Waiting for GPS data...");
    }
  }

  // Wait before the next measurement
  delay(500);

  // Handle GPS data
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  // If no data has been received for a while, notify the user
  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println("No GPS data received: check wiring or move to a location with a clear view of the sky.");
  }
}

void SendMessage(float distance, double latitude, double longitude) {
  Serial.println("Sending message, please wait...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sending Msg...");
  
  // Set SMS to text mode
  SIM900A.println("AT+CMGF=1");
  delay(1000);

  // Set the recipient's phone number
  SIM900A.println("AT+CMGS=\"+917069117694\"\r"); // Replace with the recipient's phone number
  delay(1000);

  // Set the SMS content
  SIM900A.print("Alert: Distance measured is ");
  SIM900A.print(distance);
  SIM900A.println(" cm");
  
  // Generate Google Maps link
  SIM900A.print("Location: ");
  SIM900A.print("https://maps.google.com/?q=");
  SIM900A.print(latitude, 6);
  SIM900A.print(",");
  SIM900A.println(longitude, 6);
  
  delay(100);

  // End the SMS with a CTRL+Z character
  SIM900A.write(26);
  delay(1000); // Give time for the message to be sent

  Serial.println("Message sent successfully");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Msg Sent!");
  delay(2000); // Display sent message status for 2 seconds
}