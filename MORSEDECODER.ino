#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <Wire.h>

#define TFT_CS   D6
#define TFT_DC   D7
#define TFT_RST  D9
#define VCNL_ADDR 0x60

Adafruit_ST7735 ROSE = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

bool detectedNow;
bool detectedBefore = false;

unsigned long startTime = 0;
unsigned long duration = 0;
unsigned long lastInputTime = 0;

int buzzerPin = D1;
int ledPin = D2;
int sensorPin = D0; 

String currentLetter = "";
String decodedMessage = "";
String fullMorse = "";

bool recording = true;

char decodeMorse(String code)
{
    if (code == ".-") return 'A';
    if (code == "-...") return 'B';
    if (code == "-.-.") return 'C';
    if (code == "-..") return 'D';
    if (code == ".") return 'E';
    if (code == "..-.") return 'F';
    if (code == "--.") return 'G';
    if (code == "....") return 'H';
    if (code == "..") return 'I';
    if (code == ".---") return 'J';
    if (code == "-.-") return 'K';
    if (code == ".-..") return 'L';
    if (code == "--") return 'M';
    if (code == "-.") return 'N';
    if (code == "---") return 'O';
    if (code == ".--.") return 'P';
    if (code == "--.-") return 'Q';
    if (code == ".-.") return 'R';
    if (code == "...") return 'S';
    if (code == "-") return 'T';
    if (code == "..-") return 'U';
    if (code == "...-") return 'V';
    if (code == ".--") return 'W';
    if (code == "-..-") return 'X';
    if (code == "-.--") return 'Y';
    if (code == "--..") return 'Z';

    return '?';
}

bool sensorReading()
{
    return digitalRead(sensorPin) == LOW;
}

void initSensor()
{
    Wire.beginTransmission(VCNL_ADDR);
    Wire.write(0x00);
    Wire.write(0x01); 
    Wire.endTransmission();
}

void setup() {
    lastInputTime = millis();
    pinMode(sensorPin, INPUT_PULLUP);
    pinMode(buzzerPin, OUTPUT);
    pinMode(ledPin, OUTPUT);

    Serial.begin(115200);
    Wire.begin();
    initSensor();
    
    ROSE.initR(INITR_BLACKTAB);
    ROSE.setRotation(1);
    ROSE.fillScreen(ST77XX_BLACK);
    ROSE.setTextColor(ST77XX_WHITE);
    ROSE.setTextSize(2);
    ROSE.setCursor(0,0);
    ROSE.setTextWrap(true);

     
}

void loop()
{
    detectedNow = sensorReading();

    // started sensing
    if (!detectedBefore && detectedNow)
    {
        startTime = millis();
    }

    // ended sensing
    if (detectedBefore && !detectedNow)
    {
        duration = millis() - startTime;

        if (duration < 1500)
        {
            currentLetter += ".";
            fullMorse += ".";
            Serial.println("DOT");
        }
        else
        {
            currentLetter += "-";
            fullMorse += "-";
            Serial.println("DASH");
        }

        lastInputTime = millis();
    }

    // 4 secs not sensed makes a space so it can move to next letter in word
    if (currentLetter.length() > 0 &&
        millis() - lastInputTime >= 4000 &&
        recording)
    {
        char letter = decodeMorse(currentLetter);

        decodedMessage += letter;
        fullMorse += " ";

        Serial.print("Code: ");
        Serial.print(currentLetter);
        Serial.print(" = ");
        Serial.println(letter);
        ROSE.print(letter);

        currentLetter = "";
        lastInputTime = millis();
    }

    if (millis() - lastInputTime >= 6000 &&
    recording)
    {
        recording = false;

        Serial.print("Final Message: ");
        Serial.println(decodedMessage);

        ROSE.fillScreen(ST77XX_BLACK);
        ROSE.setCursor(0,0);
        ROSE.print(decodedMessage);
        ROSE.setCursor(0,40);
        ROSE.print(fullMorse);

        replayMorseLED();

        if (decodedMessage == "SONG")
        {
            playSong();
        }
        else
        {
            Serial.println("Unknown Command");
        }
        delay(1000);
        resetSystem();
    }

    detectedBefore = detectedNow;
}

void playSong() // rn its just buzzer sounds, I will replace this with library pitches.h to make a proper melody
{
    tone(buzzerPin, 262, 300);
    delay(350);
    tone(buzzerPin, 330, 300);
    delay(350);
    tone(buzzerPin, 392, 400);
    delay(450);
    noTone(buzzerPin);
    ROSE.setCursor(0,60);
    ROSE.println("playing melody");
}

void flashLED()
{
    for (int i = 0; i < 3; i++)
    {
        digitalWrite(ledPin, HIGH);
        delay(200);
        digitalWrite(ledPin, LOW);
        delay(200);
    }
}

void resetSystem()
{
    currentLetter = "";
    decodedMessage = "";
    fullMorse = "";
    recording = true;
    lastInputTime = millis();
    ROSE.fillScreen(ST77XX_BLACK);
    ROSE.setCursor(0,0);
    Serial.println("Ready for next message");
}

void replayMorseLED()
{
    for (int i = 0; i < fullMorse.length(); i++)
    {
        char symbol = fullMorse[i];

        if (symbol == ' ')
        {
            delay(1200);
            continue;
        }

        digitalWrite(ledPin, HIGH);

        if (symbol == '.')
            delay(300);
        else
            delay(900);

        digitalWrite(ledPin, LOW);
        delay(300);
    }
}