/*
 * Code for Flappy Bird game
 * ENGI E1002
 * Spring 2023
 * Olivia O'Driscoll, Jungyun Kim, Maite Penahererra, Emily Yin
 */

#include <LiquidCrystal.h> // includes the LiquidCrystal Library
#include <Servo.h>

/** PINS  */

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
const int BIRD_SERVO_PIN = 6;
const int BEAM_BREAK_PIN = 13;

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
int birdPos = 0;
int pos = 0;

// beam break
int beamState;
int lastBeamState = 0;

// flex
float flexR;

// game data
int score = 0;
bool startToggle = false;

/** GAME CONSTANTS */
const int MAX_ANALOG_READ = 1023.0;
const int MAX_SERVO_POS = 180;

// flex sensor
const float FLEX_BOUND = 230;

// motors
const int CAROUSEL_MOTOR_SPEED = 100;

/** OBJECTS */
Servo birdServo;         // create servo object to control  servo
LiquidCrystal lcd(LCD_RS, LCD_ENABLE, LCD_D4, LCD_D5, LCD_D6, LCD_D7); // Creates an LCD object.

/**
 * initializes pins to appropriate read/write setting.
 * displays welcome message.
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
    lcd.setCursor(0, 1);     // print half the message on the second line
    lcd.print("Flappy Bird!");
}

/**
 * repeating portion of code
 */
void loop()
{
    checkStart();
    while (startToggle)
        game();
}

/**
 * the master method to control in game functions
 */
void game()
{
    incrementScore();
    rotateCarousel();
    positionBird();
    isGameFinished();
}

/**
 * checks if the game has finished
 */
void isGameFinished()
{
    if (FLEX_BOUND > measureFlex())
    {
        gameOver();
        delay(2000);
        startToggle = false;
    }
}

/**
 * checks if the game has started
 */
void checkStart()
{
    if ((digitalRead(SW_pin) == 0) && (!startToggle))
    { // button has been pressed to start game
        score = 0;
        startToggle = true;
        lcd.clear();
        lcd.setCursor(0, 0);
        rotateCarousel();
    }
}

/**
 * checks if a post has passed
 */
void incrementScore()
{
    beamState = digitalRead(BEAM_BREAK_PIN);
    if (!beamState && lastBeamState)
        score++;

    lastBeamState = beamState;
    lcd.setCursor(0, 0);
    lcd.print(score);
}

/**
 * sets desired servo position and from joystick reading
 */
void positionBird()
{
    smoother(X_pin, Y_pin);
    birdPos = map(X_Pos, 0, MAX_ANALOG_READ, 0, MAX_SERVO_POS);
    birdServo.write(birdPos);
}

/**
 * measures the flex sensor resistance to detect when the game is over
 */
float measureFlex()
{
    // Read the ADC, and calculate voltage and resistance from it
    flexR = analogRead(FLEX_PIN);         // Read and save analog value from potentiometer
    flexR = map(flexR, 700, 900, 0, 255); // Map value 0-1023 to 0-255 (PWM)
    Serial.println(flexR);
    return flexR;
}

/**
 * spins the carousel base
 */
void rotateCarousel()
{
    digitalWrite(CAROUSEL_MOTOR_PIN, CAROUSEL_MOTOR_SPEED);
}

/**
 * stop all motors
 */
void stopMotors()
{
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
    lcd.print("Your score was");
    lcd.setCursor(0, 1);
    lcd.print(score);
}