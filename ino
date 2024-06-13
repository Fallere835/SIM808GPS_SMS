#include <SoftwareSerial.h>

// Configuración de los módulos
SoftwareSerial SIM808(2, 3); // RX, TX para SIM808

// Variables de almacenamiento
String simResponse = "";
const int maxResponses = 10;
String responses[maxResponses];
int responseIndex = 0;
double latitude = 0.0;
double longitude = 0.0;

// Configuración de pines
const int LED = 6;
const int SIM808PowerPin = 9;

// Función para alimentar el módulo SIM808
void SIM808power() {
  digitalWrite(SIM808PowerPin, HIGH);
  delay(1000);
  digitalWrite(SIM808PowerPin, LOW);
  delay(5000);
}

// Función para enviar un SMS
void sendSMS(String message) {
  SIM808.print("AT+CMGF=1\r"); // Poner en modo SMS de texto
  delay(100);
  SIM808.println("AT+CMGS=\"958734473\""); // Número de destino
  delay(100);
  SIM808.println(message); // Contenido del mensaje
  delay(100);
  SIM808.println((char)26); // Ctrl+Z para finalizar el mensaje
  delay(5000); // Tiempo para enviar el mensaje
}

// Función para llamar a un número
void callSomeone() {
  delay(1000);
  SIM808.println("ATD958734473;"); 
  delay(1000);            
  SIM808.println("ATH"); // Finalizar la llamada
  delay(1000);
}

void setup() {
  // Inicialización de los módulos y pines
  Serial.begin(9600);
  SIM808.begin(9600);
  SIM808.begin(9600);
  pinMode(LED, OUTPUT);
  pinMode(SIM808PowerPin, OUTPUT);

  // Esperar a que los módulos estén listos
  while (!Serial) { ; }
  while (!SIM808) { ; }
  while (!SIM808) { ; }

  Serial.println("Serial PC conectado");
  Serial.println("SIM808 Conectada");
  delay(2000);

  SIM808power(); // Encender el módulo SIM808

  // Enviar comando para obtener datos del GPS
  SIM808.println("AT+CGNSINF");
}

void loop() {
  // Leer datos del módulo SIM808
  if (SIM808.available()) {
    char c = SIM808.read();
    simResponse += c;
    
    if (c == '\n') {
      if (responseIndex < maxResponses) {
        responses[responseIndex] = simResponse;
        responseIndex++;
      } else {
        Serial.println("Warning: Response array is full.");
      }

      Serial.print("SIM808 Response: ");
      Serial.print(simResponse);
      simResponse = "";
    }
  }

  // Procesar y extraer coordenadas GPS
  for (int i = 0; i < responseIndex; i++) {
    if (responses[i].startsWith("+CGNSINF")) {
      String data = responses[i].substring(responses[i].indexOf(":") + 1);
      data.trim();
      int commaIndex3 = data.indexOf(',', data.indexOf(',', data.indexOf(',') + 1) + 1) + 1;
      int commaIndex4 = data.indexOf(',', commaIndex3);
      int commaIndex5 = data.indexOf(',', commaIndex4 + 1);

      String latString = data.substring(commaIndex3, commaIndex4);
      String lonString = data.substring(commaIndex4 + 1, commaIndex5);

      latitude = latString.toDouble();
      longitude = lonString.toDouble();

      Serial.print("Latitude: ");
      Serial.println(latitude, 6);
      Serial.print("Longitude: ");
      Serial.println(longitude, 6);

      // Enviar coordenadas por SMS
      String message = "Latitud:     " + String(latitude, 6) + "\nLongitud:  " + String(longitude, 6);
      sendSMS(message);

      // Encender LED
      digitalWrite(LED, HIGH);
      delay(10000);
      digitalWrite(LED, LOW);

      // Realizar llamada (opcional)
      callSomeone();

      break;
    }
  }

  // Enviar comando AT+CGNSINF periódicamente
  static unsigned long lastCommandTime = 0;
  unsigned long currentTime = millis();
  if (currentTime - lastCommandTime > 2000) {
    SIM808.println("AT+CGNSINF");
    lastCommandTime = currentTime;
  }
}
