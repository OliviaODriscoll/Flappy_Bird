/*
 * Code for Flappy Bird game
 * ENGI E1002
 * Spring 2023
 * Olivia O'Driscoll, Jungyun Kim, Maite Penahererra, Emily Yin
 */

#include <LiquidCrystal.h> // includes the LiquidCrystal Library
#include <Servo.h>

/** PORTS  */

// analog
const int SW_pin = 8;   // digital pin connected to switch output
const int X_pin = 4;    // analog pin connected to X output
const int Y_pin = 5;    // analog pin connected to Y output
const int FLEX_PIN = 0; // analog

// digital
const int LCD_RS = 12;
const int LCD_ENABLE = 11;
const int LCD_D4 = 5;
const int LCD_D5 = 4;
const int LCD_D6 = 3;
const int LCD_D7 = 2;
const int CAROUSEL_MOTOR_PIN = 9;
const int BIRD_SERVO_PIN = 10;
const int BEAM_BREAK_PIN = 7;

/** GAME VARIABLES */

// smoothing
const int MAX_READINGS = 10;
int Xreadings[MAX_READINGS];
int XreadIndex = 0;
int Xtotal = 0;
int X_Pos = 0;

int Yreadings[MAX_READINGS];
int YreadIndex = 0;
int Ytotal = 0;
int Y_Pos = 0;

// bird
int birdPos = 0; int pos = 0;

// beam break
int beamState;
int lastBeamState = 0;

// flex
int flexADC;
float flexV;
float flexR;

// game data
int score = 0;
bool startToggle = false;

/** GAME CONSTANTS */
const int MAX_ANALOG_READ = 1023.0;
const int MAX_SERVO_POS = 180;

// flex sensor
const float VCC = 4.98;                     // Measured voltage of Ardunio 5V line
const float R_DIV = 47500.0;                // Measured resistance of 3.3k resistor
const float FLEX_BOUND = 930.0;      // TODO: measure ourselves

// motors
const int CAROUSEL_MOTOR_SPEED = 180; // 0 to 255 scale TODO:tune

/** OBJECTS */
Servo birdServo;                                                       // create servo object to control a servo
LiquidCrystal lcd(LCD_RS, LCD_ENABLE, LCD_D4, LCD_D5, LCD_D6, LCD_D7); // Creates an LCD object.

/**
 * initializes pins to appropriate read/write setting. displays welcome message.
 */
void setup()
{
    lcd.begin(16, 2); // Initializes LCD screen interface and specifies display dimensions (width and height)

    // set up button
    pinMode(SW_pin, INPUT);
    digitalWrite(SW_pin, HIGH);
    pinMode(FLEX_PIN, INPUT);
    pinMode(CAROUSEL_MOTOR_PIN, OUTPUT);
    pinMode(BEAM_BREAK_PIN, INPUT);
    Serial.begin(9600);

    // set up bird servo motor
    birdServo.attach(BIRD_SERVO_PIN);

    // initialize arrays to 0 for smoothing method
    for (int i = 0; i < MAX_READINGS; i++)
    {
        Xreadings[i] = 0;
        Yreadings[i] = 0;
    }

    // display welcome message
    lcd.setCursor(0, 0);
    lcd.print("Welcome to"); // Prints "Welcome to Flappy Bird!" on the LCD
    lcd.setCursor(0, 1);
    lcd.print("Flappy Bird!");
}

/**
 * repeating portion of code
 */
void loop()
{

    analogWrite(6, 9);

    if ((digitalRead(SW_pin) == 0) && (!startToggle))
    { // button has been pressed to start game
        startToggle = true;
        lcd.clear();
        lcd.setCursor(0, 0);
        rotateCarousel();
    }

    while (startToggle)
    {
        game();

        //Serial.println(measureFlex());
        if (FLEX_BOUND > measureFlex()) //TODO: change to <
        {
            gameOver();
            delay(2000); // does the delay need to be this large?
            startToggle = false;
        }
    }
}

/**
 * the master method to control in game functions
 */
void game()
{
    incrementScore();
    printScore();
    positionBird();
}

/**
 * checks if a post has passed
 */
void incrementScore()
{
    beamState = digitalRead(BEAM_BREAK_PIN);

    if (!beamState && lastBeamState) // if the beam was just broken, increase score
        score++;
        lcd.print(score);
    lastBeamState = beamState;
}

/**
 * prints the score to the screem
 */
void printScore()
{
    lcd.setCursor(0, 0);
    lcd.println(score); // Prints the score
}

/**
 * sets desired servo position and from joystick reading
 */
void positionBird()
{
    smoother(X_pin,Y_pin);
    birdPos = map(X_Pos, 0, MAX_ANALOG_READ, 0, MAX_SERVO_POS);
    birdServo.write(birdPos);
}

/**
 * measures the flex sensor resistance to detect when the game is over
 */
float measureFlex()
{
    // Read the ADC, and calculate voltage and resistance from it
    flexADC = analogRead(FLEX_PIN);
    flexV = flexADC * VCC / MAX_ANALOG_READ;
    flexR = R_DIV * (VCC / flexV - 1.0);
    return flexR;
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
    birdServo.detach();
    analogWrite(CAROUSEL_MOTOR_PIN, 0);
}

/**
 * smooths the joystick input for the servo control
 * @param x_pin x axis analog connection
 * @param y_pin y axis analog connection
 */
void smoother(int x_pin, int y_pin)
{
    Xtotal = Xtotal - Xreadings[XreadIndex];
    Ytotal = Ytotal - Yreadings[YreadIndex];
    delay(1);
    Xreadings[XreadIndex] = analogRead(x_pin);
    Yreadings[YreadIndex] = analogRead(y_pin);
    delay(1);
    Xtotal = Xtotal + Xreadings[XreadIndex];
    Ytotal = Ytotal + Yreadings[YreadIndex];
    delay(1);
    XreadIndex = XreadIndex + 1;
    YreadIndex = YreadIndex + 1;
    if (XreadIndex >= MAX_READINGS)
        XreadIndex = 0;
    if (YreadIndex >= MAX_READINGS)
        YreadIndex = 0;
    delay(1);
    X_Pos = Xtotal / MAX_READINGS;
    Y_Pos = Ytotal / MAX_READINGS;
}

/**
 * stops all motors and goes back to display screen when game is over
 */
void gameOver()
{
    stopMotors();
    lcd.clear();
    Serial.print("Your score was");
    lcd.setCursor(0, 1);
    Serial.print(score);
}