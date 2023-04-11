/*
 * Code for Flappy Bird game
 * ENGI E1002
 * Spring 2023
 * Olivia O'Driscoll, Jungyun Kim, Maite Penahererra, Emily Yin
 */

#include <LiquidCrystal.h> // includes the LiquidCrystal Library
#include <Servo.h>

// define constants
const int START_BUTTON_PORT = 8;

// LCD
const int LCD_RS = 12;
const int LCD_ENABLE = 11;
const int LCD_D4 = 5;
const int LCD_D5 = 4;
const int LCD_D6 = 3;
const int LCD_D7 = 2;

// flex sensor
const int FLEX_PIN = A0; // Pin connected to voltage divider output
// Measure the voltage at 5V and the actual resistance of your
// 47k resistor, and enter them below:
const float VCC = 4.98;      // Measured voltage of Ardunio 5V line
const float R_DIV = 47500.0; // Measured resistance of 3.3k resistor
// Upload the code, then try to adjust these values to more
// accurately calculate bend degree.
const float STRAIGHT_RESISTANCE = 16860.93; // resistance when straight
const float BEND_RESISTANCE = 24248750.00;  // resistance at 90 deg
const float LOWER_FLEX_BOUND = 6000.0;      // TODO: measure ourselves
const float UPPER_FLEX_BOUND = 3000000.0;   // TODO: measure ourselves
const int MAX_ANALOG_READ = 1023.0;

// motors
const int CAROUSEL_MOTOR_PIN = 10;
const int CAROUSEL_MOTOR_SPEED = 10; // 0 to 255 scale TODO:tune
const int BIRD_MOTOR_PIN = 6;
Servo birdServo;                // create servo object to control a servo
int birdPos = 0;                // variable to store the servo position
const int BIRD_MOTOR_SPEED = 2; // 0 to 255 scale TODO: tune
const int BIRD_CONTROL_POT_PIN = 13;

// beam break
const int BEAM_BREAK_PIN = 7;
int beamState;
int lastBeamState = 0;

int flexADC;
float flexV;
float flexR;

// game data
int score = 0;

bool startToggle = false;

LiquidCrystal lcd(LCD_RS, LCD_ENABLE, LCD_D4, LCD_D5, LCD_D6, LCD_D7); // Creates an LCD object.

void setup()
{
    lcd.begin(16, 2); // Initializes the interface to the LCD screen, and specifies the dimensions (width and height) of the display }

    // set up button
    pinMode(START_BUTTON_PORT, INPUT);
    pinMode(BIRD_DOWN_BUTTON_PORT, INPUT);
    pinMode(FLEX_PIN, INPUT);
    pinMode(CAROUSEL_MOTOR_PIN, OUTPUT);
    pinMode(BEAM_BREAK_PIN, INPUT);
    birdServo.attach(BIRD_MOTOR_PIN);
    Serial.begin(9600);

    lcd.setCursor(0, 0);
    lcd.print("Welcome to"); // Prints "Welcome to Flappy Bird!" on the LCD
    lcd.setCursor(0, 1);
    lcd.print("Flappy Bird!");
}

void loop()
{

    if ((digitalRead(START_BUTTON_PORT) == 1) && (!startToggle))
    { // button has been pressed to start game
        startToggle = true;
        lcd.clear();
        lcd.setCursor(0, 0);
    }

    while (startToggle)
    {
        game();

        Serial.println(measureFlex());
        if ((LOWER_FLEX_BOUND > measureFlex()) || (UPPER_FLEX_BOUND < measureFlex()))
        {
            gameOver();
            delay(2000);
            startToggle = false;
        }
    }
}

/**
 * the master method to control all game functions
 */
void game()
{

    beamState = digitalRead(BEAM_BREAK_PIN);

    if (!beamState && lastBeamState) // if the beam was just broken, increase score
        score++;
    lastBeamState = beamState;

    lcd.setCursor(0, 0);
    lcd.print(score); // Prints the score

    moveBird();
    rotateCarousel();
}


/**
 * sets bird position using servo controlled by potentiometer
 */
void moveBird()
{
    birdPos = analogRead(BIRD_CONTROL_POT_PIN);        // reads the value of the potentiometer (value between 0 and 1023)
    birdPos = map(birdPos, 0, 1023, 0, 180); // scale it to use it with the servo (value between 0 and 180)
    myservo.write(birdPos);              // sets the servo position according to the scaled value
}

/**
 * measures the flex sensor resistance to detect when the game is over
 */
float measureFlex()
{
    // Read the ADC, and calculate voltage and resistance from it
    flexADC = analogRead(FLEX_PIN);
    flexV = flexADC * VCC / 1023.0;
    flexR = R_DIV * (VCC / flexV - 1.0);
    // Serial.println("Resistance: " + String(flexR) + " ohms");
    return flexR;

    // Use the calculated resistance to estimate the sensor's
    // bend angle:
    // float angle = map(flexR, STRAIGHT_RESISTANCE, BEND_RESISTANCE,
    // 0, 90.0);
    // return angle;
    //  Serial.println("Bend: " + String(angle) + " degrees");
    //  Serial.println();
}

/**
 * spins the carousel base
 */
void rotateCarousel()
{
    analogWrite(CAROUSEL_MOTOR_PIN, CAROUSEL_MOTOR_SPEED);
}

/**
 * stop all motors
 */
void stopMotors()
{
    analogWrite(BIRD_MOTOR_PIN, 0);
    analogWrite(CAROUSEL_MOTOR_PIN, 0);
}

/**
 * stops all motors and goes back to display screen when game is over
 */
void gameOver()
{
    lcd.clear();
    lcd.print("Your score was");
    lcd.setCursor(0, 1);
    lcd.print(score);
    stopMotors();
}
