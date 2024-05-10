#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>

#define TX_PIN 03
#define BUTTON_PIN 02 // Change this to the pin number connected to your push button D4
#define YELLOW_LED_PIN 13 // Define the pin number for the yellow LED d7
#define RED_LED_PIN 12 // Define the pin number for the red LED d6
#define BUZZER_PIN 14 // Define the pin number for the buzzer d5

SoftwareSerial RFIDSerial(TX_PIN, -1);

// Arrays to store RFID tag numbers, product names, and prices
char tagNumbers[][13] = {
  "540070EC29E1", // Milk
  "09003B27B8AD", // Tea
  "09005B39E883", // Dairy Milk
  "09005B39147F", // Toothbrush
  "09005B390C67", // Hairbrush
  "09008531A01D", // Shampoo
  "09003D316762"  // Book
};

char* productNames[] = {
  "Milk",
  "Tea",
  "Dairy Milk",
  "Toothbrush",
  "Hairbrush",
  "Shampoo",
  "Book"
};

double productPrices[] = {
  25.0,  // Price of Milk
  10.0,  // Price of Tea
  15.0,  // Price of Dairy Milk
  5.0,   // Price of Toothbrush
  8.0,   // Price of Hairbrush
  20.0,  // Price of Shampoo
  30.0   // Price of Book
};

int itemCounts[] = {0, 0, 0, 0, 0, 0, 0}; // Initialize item counts to zero

const char* ssid = "Manoj"; // WiFi name
const char* password = "manoj123"; // WiFi password
ESP8266WebServer server(80);

unsigned long lastButtonPressTime = 0;
bool buttonPressed = false;
bool deleteMode = false; 
int productToDelete = -1;
int lastScannedItemIndex = -1; // Index of the last scanned item

void setup() {
  Serial.begin(9600);
  RFIDSerial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT); // Initialize buzzer pin

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", []() {
    String mode = deleteMode ? "Delete Mode" : "Add Mode";
    String page = "<html><head><title>Smart Shopping Cart</title><style>table {width: 80%; margin: auto; border-collapse: collapse;} th, td {padding: 12px; text-align: left;} th {background-color: #4CAF50; color: white;} tr:nth-child(even) {background-color: #f2f2f2;} tr:hover {background-color: #ddd;} h1 {text-align: center; font-size: 36px;} h2 {text-align: center; font-size: 24px;} #mode {text-align: center; font-size: 20px;}</style></head><body><h1>Smart Shopping Cart</h1><div id='mode'><h2>Mode: " + mode + "</h2></div><table><tr><th>Item Name</th><th>Quantity</th><th>Price</th></tr>";
    double totalCost = 0.0;
    for (int i = 0; i < sizeof(productNames) / sizeof(productNames[0]); i++) {
      double itemCost = itemCounts[i] * productPrices[i];
      totalCost += itemCost;
      page += "<tr><td>" + String(productNames[i]) + "</td><td>" + String(itemCounts[i]) + "</td><td>Rs " + String(itemCost) + "</td></tr>";
    }
    page += "</table><h2>Total Cost: Rs " + String(totalCost) + "</h2></body></html>";
    server.send(200, "text/html", page);
  });

  server.begin();
}

void loop() {
  server.handleClient();

  if (digitalRead(BUTTON_PIN) == LOW) { 
    if (!buttonPressed && (millis() - lastButtonPressTime > 1000)) { 
      lastButtonPressTime = millis();
      buttonPressed = true;
      if (!deleteMode) { 
        deleteMode = true;
        Serial.println("Switched to delete mode"); 
      } else {
        deleteMode = false;
        Serial.println("Switched to add mode"); 
      }
    }
  } else {
    buttonPressed = false;
  }

  if (RFIDSerial.available() >= 12) {
    char cardNumber[13];
    RFIDSerial.readBytes(cardNumber, 12);
    cardNumber[12] = '\0'; 

    Serial.print("RFID Tag: ");
    Serial.println(cardNumber);

    // Check if the scanned tag matches any known tag
    bool found = false;
    for (int i = 0; i < sizeof(tagNumbers) / sizeof(tagNumbers[0]); i++) {
      if (strcmp(cardNumber, tagNumbers[i]) == 0) {
        if (!deleteMode) {
          Serial.print("Item added: ");
          Serial.println(productNames[i]);
          itemCounts[i]++; 
          // Blink the yellow LED when an item is added
          for (int j = 0; j < 3; j++) {
            digitalWrite(YELLOW_LED_PIN, HIGH);
            digitalWrite(BUZZER_PIN, HIGH); // Turn on buzzer
            delay(250);
            digitalWrite(YELLOW_LED_PIN, LOW);
            digitalWrite(BUZZER_PIN, LOW); // Turn off buzzer
            delay(250);
          }
        } else {
          if (itemCounts[i] > 0) { 
            itemCounts[i]--; 
            Serial.print("Item deleted: ");
            Serial.println(productNames[i]);
            // Blink the red LED when an item is deleted
            for (int j = 0; j < 3; j++) {
              digitalWrite(RED_LED_PIN, HIGH);
              digitalWrite(BUZZER_PIN, HIGH); // Turn on buzzer
              delay(250);
              digitalWrite(RED_LED_PIN, LOW);
              digitalWrite(BUZZER_PIN, LOW); // Turn off buzzer
              delay(250);
            }
          } else {
            Serial.print("Item not present: ");
            Serial.println(productNames[i]);
          }
        }
        found = true;
        break; 
      }
    }

    if (!found) {
      Serial.println("Unknown item");
    }

    if (!deleteMode) {
      lastScannedItemIndex = -1; // Reset last scanned item index when in add mode
    } else {
      // Store last scanned item index only in delete mode
      for (int i = 0; i < sizeof(tagNumbers) / sizeof(tagNumbers[0]); i++) {
        if (strcmp(cardNumber, tagNumbers[i]) == 0) {
          lastScannedItemIndex = i;
          break;
        }
      }
    }

    static unsigned long lastRefreshTime = 0;
    if (millis() - lastRefreshTime >= 1000) {
      server.send(200, "text/html", ""); 
      lastRefreshTime = millis();
    }
  }
}
