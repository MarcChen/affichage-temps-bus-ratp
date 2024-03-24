# Project Name: IDF Transport Information Display

This project aims to provide real-time transport information by displaying the next two arrival times of a specific transport line at a designated stop. It utilizes the IDF Mobilités API for fetching the transport data, integrates WiFi connectivity for Internet access, and displays the information on an LCD screen.

## Features

- Fetch and display real-time transport information from IDF Mobilités API.
- Connect to a public WiFi network automatically.
- Authenticate to a captive portal for Internet access.
- Display the next two arrival times of a specific line at a given stop.
- Refresh the displayed information at a configurable interval.

## Hardware Requirements

- ESP32 Microcontroller
- LCD Display (16x2)
- Push Button
- Basic electronic components (resistors, breadboard, jumper wires)

## Software Dependencies

- Arduino IDE
- ArduinoJson Library
- WiFi Library for ESP32
- LiquidCrystal Library for LCD interaction

## Setup Instructions

1. **Hardware Setup**: Connect the LCD display and the push button to the ESP32 according to the provided schematics.
2. **WiFi Configuration**: Update the `ssid`, `serverName`, and `httpRequestData` with your WiFi network details and captive portal authentication parameters.
3. **API Configuration**: Specify the `LineRef` and `MonitoringRef` for the transport line and stop you want to monitor. Obtain an API key from IDF Mobilités and update the `apiKey` variable.
4. **Time Configuration**: Adjust the `gmtOffset_sec` and `daylightOffset_sec` if necessary to match your local time zone.
5. **Uploading the Code**: Open the project in Arduino IDE, select the appropriate board and port, and upload the code to your ESP32.

## Usage

- The device will automatically connect to the specified WiFi network and authenticate with the captive portal upon startup.
- It will then synchronize the time via NTP and start fetching transport information based on the configured interval.
- The next two arrival times of the specified transport line at the designated stop will be displayed on the LCD.
- Press the push button to manually refresh the displayed information.

## Important Notes

- This project is designed for educational purposes and personal use. It is crucial to respect the usage limits and terms of service of the IDF Mobilités API.
- **It's useful to check the documentation available on the IDF Mobilités website for the API [here](https://prim.iledefrance-mobilites.fr/en/apis/idfm-ivtr-requete_unitaire)** to understand the parameters, response structure, and error codes. Familiarizing yourself with this documentation will help you customize and extend the project functionalities.
- The `extractAndAdjustExpectedArrivalTimes` function in this project may not be as robust as one might ideally want. This limitation stems from the constraint faced due to the size of the JSON payload returned by the IDF Mobilités API. The JSON data for some API requests is too large to be parsed into a JSON object with the available memory on the ESP32. To work around this limitation, the function performs a manual extraction of the needed information from the JSON payload as a string. While this method is functional for the specific structure of data expected from the API, it might not gracefully handle unexpected changes in the data format or anomalies in the data received. We recognize that this is not an ideal solution and invite contributions or suggestions for improving the handling of API responses within the memory constraints of the ESP32. This workaround was chosen to ensure that the project could proceed without requiring additional hardware or significant changes to the software architecture.
- The `httpRequestData` string used in this project for authenticating to the captive portal is specific to my particular scenario. This configuration was determined by analyzing the HTTP request made during the login process on the captive portal using the Developer Tools (DevTools) in Chrome. If you intend to use this project in a different environment or with a different captive portal, you will need to adjust the `httpRequestData` string accordingly. To do this, navigate to the captive portal login page, open the Developer Tools in your web browser, and monitor the network traffic while logging in. Look for the HTTP POST request made during the login process, and examine its form data. This information will provide you with the exact structure and parameters needed to construct your `httpRequestData` string for your specific captive portal scenario. This approach ensures that you can adapt the project to work with a wide range of captive portals by customizing the authentication step to match the specific requirements of the portal you are dealing with.

## Troubleshooting

If you encounter issues with WiFi connectivity or API responses, ensure that:
- Your WiFi SSID and password are correctly entered.
- The captive portal credentials are accurate.
- You have not exceeded the API rate limits.
- The API key is valid and has not expired.

For further assistance, consult the IDF Mobilités API documentation or seek help in relevant online forums.

## Upcoming Features

### Sleep Mode for Enhanced Energy Efficiency

A key enhancement we are planning to introduce is the implementation of a sleep mode feature. This addition aims to significantly improve the energy efficiency of the device, making it more suitable for long-term deployment.

This feature is particularly important for battery-operated deployments or for users seeking to reduce their energy footprint. We are in the process of testing the best approaches to implement this feature without compromising the functionality and responsiveness of the device.

Stay tuned for updates on the development and integration of the sleep mode feature into our project. Your suggestions and contributions towards this goal are welcome and appreciated.


## Contributing

We welcome contributions and suggestions to improve this project. Please feel free to fork the repository, make your changes, and submit a pull request.

