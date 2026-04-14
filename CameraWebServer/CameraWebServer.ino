#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

// 1. Your WiFi Credentials
const char* ssid = "Gal";
const char* password = "Ydan20190616";

// 2. Set up a simple web server on port 80
WebServer server(80);

// 3. AI Thinker Camera Pins
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

// 4. The updated function that streams live MJPEG video
void streamVideo() {
  WiFiClient client = server.client();

  // Step A: Send the HTTP headers telling the browser to expect a continuous stream of images
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  client.print(response);

  camera_fb_t* fb = NULL;

  // Step B: Endlessly loop to send frames as long as the browser remains connected
  while (client.connected()) {
    fb = esp_camera_fb_get();  // Grab a frame

    if (!fb) {
      Serial.println("Camera failed to take a picture!");
      break;  // Exit the loop if capture fails
    }

    // Step C: Send the boundary marker and the size of the current frame
    client.printf("--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", fb->len);

    // Step D: Write the raw image bytes directly to the web client
    client.write(fb->buf, fb->len);
    client.print("\r\n");  // Add a newline to separate frames

    esp_camera_fb_return(fb);  // Clear the memory so it's ready for the next frame
  }

  // Step E: Stop the client connection if the user closes the browser tab
  client.stop();
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  // 5. Configure the Camera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 12;

  // CHANGED: Use 2 frame buffers for smoother live streaming
  config.fb_count = 10;

  // Start the Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed! Error: 0x%x\n", err);
    return;  // Stop here if camera is broken or loose
  }

  // --- ADD THESE LINES TO FLIP THE CAMERA ---
  sensor_t* s = esp_camera_sensor_get();
  s->set_vflip(s, 1);    // 1 flips it upside down (or right-side up)
  s->set_hmirror(s, 1);  // 1 mirrors it so left and right aren't swapped
  // ------------------------------------------

  // 6. Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected!");

  // 7. Tell the server what to do when someone visits the IP address
  server.on("/", streamVideo);
  server.begin();

  Serial.println("==============================================");
  Serial.print("All Good! Open your browser and go to: http://");
  Serial.println(WiFi.localIP());
  Serial.println("==============================================");
}

void loop() {
  // Keep the server listening for visitors
  server.handleClient();
}