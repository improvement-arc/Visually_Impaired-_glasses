#include <WiFi.h>
#include <WebServer.h>  // Include the web server library

// Define your Wi-Fi credentials
const char* ssid = "sid04";
const char* password = "1234abcd";

// Define pins for ultrasonic sensors
const int trigPin1 = 15;  // Sensor 1 Trig (front)
const int echoPin1 = 14;  // Sensor 1 Echo (front)
const int trigPin2 = 4;   // Sensor 2 Trig (left)
const int echoPin2 = 16;  // Sensor 2 Echo (left)
const int trigPin3 = 17;  // Sensor 3 Trig (right)
const int echoPin3 = 5;   // Sensor 3 Echo (right)

// Threshold for obstacle detection (in cm)
const int threshold = 30;

// Create a web server object on port 80
WebServer server(80);

// Function to read distance from an ultrasonic sensor
long readUltrasonic(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  long distance = (duration * 0.034) / 2;  // Convert to cm
  return distance;
}

// Function to handle the root webpage
void handleRoot() {
  String htmlPage = R"=====(
    <!DOCTYPE html>
    <html>
    <head>
      <title>Ultrasonic Sensor Data with Voice Feedback</title>
      <style>
        body {
          font-family: Arial, sans-serif;
          background-color: #f4f4f9;
          color: #333;
          text-align: center;
          margin: 0;
          padding: 0;
        }

        h1 {
          background-color: #4CAF50;
          color: white;
          padding: 20px;
          margin-bottom: 40px;
        }

        .container {
          max-width: 800px;
          margin: 0 auto;
          padding: 20px;
        }

        .data-box {
          background-color: #fff;
          border: 1px solid #ddd;
          border-radius: 8px;
          padding: 20px;
          margin-bottom: 20px;
          box-shadow: 0px 4px 8px rgba(0, 0, 0, 0.1);
        }

        p {
          font-size: 18px;
          font-weight: bold;
        }

        .status {
          color: #4CAF50;
          font-size: 20px;
        }

        .alert {
          color: #FF5722;
          font-size: 20px;
        }

        footer {
          background-color: #4CAF50;
          color: white;
          padding: 10px 0;
          position: fixed;
          width: 100%;
          bottom: 0;
        }
      </style>
      <script>
        function getData() {
          fetch('/getData')
            .then(response => response.json())
            .then(data => {
              document.getElementById('distance1').innerText = "Distance 1 (Front): " + data.distance1 + " cm";
              document.getElementById('distance2').innerText = "Distance 2 (Left): " + data.distance2 + " cm";
              document.getElementById('distance3').innerText = "Distance 3 (Right): " + data.distance3 + " cm";

              // Handle voice feedback for obstacles and movement suggestions
              let message = "";
              if (data.distance1 < 30) {
                message = "Object in front, move left or right";
                document.getElementById('status').className = 'alert';
                document.getElementById('status').innerText = message;
              } else if (data.distance2 < 30 && data.distance3 < 30) {
                message = "Objects on both sides, stop and wait";
                document.getElementById('status').className = 'alert';
                document.getElementById('status').innerText = message;
              } else if (data.distance2 < 30) {
                message = "Object on the left, move right";
                document.getElementById('status').className = 'alert';
                document.getElementById('status').innerText = message;
              } else if (data.distance3 < 30) {
                message = "Object on the right, move left";
                document.getElementById('status').className = 'alert';
                document.getElementById('status').innerText = message;
              } else {
                message = "All clear, move forward";
                document.getElementById('status').className = 'status';
                document.getElementById('status').innerText = message;
              }

              // Call the speak function to give voice feedback
              speak(message);
            });
        }

        // Text to speech function
        function speak(message) {
          window.speechSynthesis.cancel();  // Cancel any ongoing speech
          var msg = new SpeechSynthesisUtterance();
          msg.text = message;
          window.speechSynthesis.speak(msg);
        }

        setInterval(getData, 2500);  // Update every 2.5 seconds
      </script>
    </head>
    <body>
      <h1>Ultrasonic Sensor Data with Voice Feedback</h1>
      <div class="container">
        <div class="data-box">
          <p id="distance1">Distance 1 (Front): N/A</p>
          <p id="distance2">Distance 2 (Left): N/A</p>
          <p id="distance3">Distance 3 (Right): N/A</p>
          <p id="status" class="status">Awaiting data...</p>
        </div>
      </div>
      <footer>
        <p>Ultrasonic Navigation System &copy; 2024</p>
      </footer>
    </body>
    </html>
  )=====";
  server.send(200, "text/html", htmlPage);
}

// Function to send ultrasonic data in JSON format
void handleGetData() {
  long distance1 = readUltrasonic(trigPin1, echoPin1);
  long distance2 = readUltrasonic(trigPin2, echoPin2);
  long distance3 = readUltrasonic(trigPin3, echoPin3);

  String jsonData = "{\"distance1\": " + String(distance1) + 
                    ", \"distance2\": " + String(distance2) + 
                    ", \"distance3\": " + String(distance3) + "}";
  server.send(200, "application/json", jsonData);
}

void setup() {
  Serial.begin(115200);

  // Initialize ultrasonic sensor pins
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(trigPin3, OUTPUT);
  pinMode(echoPin3, INPUT);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Start the web server
  server.on("/", handleRoot);  // Serve the main page
  server.on("/getData", handleGetData);  // Serve the sensor data
  server.begin();
  Serial.println("Server started");
}

void loop() {
  // Handle client requests
  server.handleClient();
}