#include <Arduino.h>
#include <ESP8266WebServer.h>
#include "secret.h"
#include "pins.h"
#include "htmlfile.h"
#define PRINT(...) Serial.println(__VA_ARGS__)
#define UNUSED __attribute__((unused))

// server
ESP8266WebServer server(80);

// byte object to store data
byte state = 0b00000000;

bool handleSwitch(uint8_t *id);

String getStatusObject()
{
    String str = "{\"message\":\"Changed Successful\",\"success\":true,\"data\":";
    str += state;
    str += "}";
    return str;
}

class ProtectedRoute : public RequestHandler
{
public:
    bool canHandle(HTTPMethod method, const String UNUSED &uri) override
    {
        return (method == HTTP_GET || method == HTTP_POST);
    }
    bool handle(ESP8266WebServer &server, HTTPMethod requestMethod, const String &requestUri) override
    {

        if (!server.authenticate(authName, authPas))
        {
            server.sendHeader("Location", "/login");
            server.send(307);
            return true;
        }
        if (requestUri.startsWith("/refresh"))
        {
            server.send(200, "application/json; charset=utf-8", getStatusObject());
        }
        if (requestUri.startsWith("/switch"))
        {
            char id = (char)server.arg("id").charAt(0);
            // char status = (char)server.arg("status");
            // if (status != '0' && status != '1')
            // {
            //     server.send(400, "application/json", "{'message':'Invalid Status','success':false}");
            //     return true;
            // }
            if (id >= '0' && id <= '9')
            {

                uint8_t pin = ((uint8_t)id) - 48;
                handleSwitch(&pin);
                server.send(200, "application/json; charset=utf-8", getStatusObject());
            }
            else
            {
                server.send(400, "application/json; charset=utf-8", "{'message':'Invalid Pin','success':false}");
            }
            return true;
        }
        if (requestUri == "/home")
        {
            server.send(200, "text/html", FPSTR(mainhtml));
            return true;
        }
        return false;
    }
};

bool handleSwitch(uint8_t *id)
{

    if (*id == 8)
    {

        state = 0b00000000;
        digitalWrite(SWITCH1, 0);
        digitalWrite(SWITCH2, 0);
        digitalWrite(SWITCH3, 0);
        digitalWrite(SWITCH4, 0);
        digitalWrite(SWITCH5, 0);
        digitalWrite(SWITCH6, 0);
        digitalWrite(SWITCH7, 0);
        digitalWrite(SWITCH8, 0);
        return true;
    }
    // Turn on all light
    else if (*id == 9)
    {
        state = 0b11111111;
        digitalWrite(SWITCH1, 1);
        digitalWrite(SWITCH2, 1);
        digitalWrite(SWITCH3, 1);
        digitalWrite(SWITCH4, 1);
        digitalWrite(SWITCH5, 1);
        digitalWrite(SWITCH6, 1);
        digitalWrite(SWITCH7, 1);
        digitalWrite(SWITCH8, 1);
        return true;
    }

    state ^= (0b00000001 << *id);
    // Turn off light serial wise
    if (*id == 0)
    {
        digitalWrite(SWITCH1, state & (0b00000001 << 0));
    }
    else if (*id == 1)
    {

        digitalWrite(SWITCH2, state & (0b00000001 << 1));
    }
    else if (*id == 2)
    {

        digitalWrite(SWITCH3, state & (0b00000001 << 2));
    }
    if (*id == 3)
    {

        digitalWrite(SWITCH4, state & (0b00000001 << 3));
    }
    else if (*id == 4)
    {

        digitalWrite(SWITCH5, state & (0b00000001 << 4));
    }
    else if (*id == 5)
    {
        digitalWrite(SWITCH6, state & (0b00000001 << 5));
    }
    else if (*id == 6)
    {

        digitalWrite(SWITCH7, state & (0b00000001 << 6));
    }
    else if (*id == 7)
    {

        digitalWrite(SWITCH8, state & (0b00000001 << 7));
    }
    // Turn off all light

    return true;
}

void initialize()
{
    // define OUTPUT Pins
    pinMode(SWITCH1, OUTPUT);
    pinMode(SWITCH2, OUTPUT);
    pinMode(SWITCH3, OUTPUT);
    pinMode(SWITCH4, OUTPUT);
    pinMode(SWITCH5, OUTPUT);
    pinMode(SWITCH6, OUTPUT);
    pinMode(SWITCH7, OUTPUT);
    pinMode(SWITCH8, OUTPUT);
    digitalWrite(SWITCH1, 0);
    digitalWrite(SWITCH2, 0);
    digitalWrite(SWITCH3, 0);
    digitalWrite(SWITCH4, 0);
    digitalWrite(SWITCH5, 0);
    digitalWrite(SWITCH6, 0);
    digitalWrite(SWITCH7, 0);
    digitalWrite(SWITCH8, 0);
}

void setup()
{
    delay(3000);
    // Serial Printing
    Serial.begin(115200);
    Serial.flush();

    // initialize
    initialize();

    // connect to WIFI
    PRINT("Connecting WIFI");
    WiFi.mode(WIFI_STA);
    WiFi.setHostname("HomeAutomation");
    WiFi.begin(wifin, wifip);
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }
    PRINT("Connected Successfully.");

    // print local IP Address
    PRINT("Local IP:");
    PRINT(WiFi.localIP());
    

    // Allow CORS
    server.enableCORS(true);

    // set HostName

    PRINT("Server Starting...");
    server.begin();
    PRINT("Server started....");
    server.on("/login", []()
              {
        if(!server.authenticate(authName,authPas))
        return server.requestAuthentication();
        server.sendHeader("location","/home");
        return server.send(302); });
    server.addHandler(new ProtectedRoute());

    server.onNotFound([]()
                      {
        // PRINT("Got Request");
        server.send(404,"application/json","{'status':404,message:'No Endpoint point'}"); });
}

void loop()
{
    server.handleClient();
}
