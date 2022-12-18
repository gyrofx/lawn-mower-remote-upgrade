#include <Button.h>

#define ON_OFF_SWITCH_PIN A3
#define CUTTER_SWITCH_PIN A5
#define CUTTER_VELOCITY_PIN A4
#define RC_TURN_PIN A0
#define RC_LEFT_FW_PIN A1
#define RC_RIGHT_FW_PIN A2

//#define ENGINE_CONTROL_WHEEL_PIN A2
//#define REMOTE_CONTROL_MODE_SWITCH_PIN 

#define ENGINE_RIGHT_BACKWARD_PIN 10
#define ENGINE_RIGHT_FORWARD_PIN 9
#define ENGINE_LEFT_BACKWARD_PIN 11
#define ENGINE_LEFT_FORWARD_PIN 13

#define KNIFE_EN_L_PIN 2
#define KNIFE_PWM_L_PIN 3
#define KNIFE_EN_R_PIN 4
#define KNIFE_PWM_R_PIN 5

#define LED_ON_PIN 6
#define LED_PAUSE_PIN 7

class TreeStateSwitch {
    bool read();

    bool isHigh();
    bool isIdle();
    bool isLow();
};

class RemotePWMChannel {
  public:
    RemotePWMChannel(uint8_t pin) {
      pin_ = pin;
      pinMode(pin_, INPUT);
    }

    void update() {
      value_ = pulseIn(pin_, 100);
    }

    uint16_t value() {
      return value_;
    }

    uint8_t pin() {
      return pin_;
    }
  private:
    uint8_t pin_;
    uint16_t value_;
};

class RemoteSwitch {
  public:

    RemoteSwitch(uint8_t pin, uint16_t lowMin, uint16_t lowMax, uint16_t highMin, uint16_t highMax)
      : channel_(pin) {
      lowMin_ = lowMin;
      lowMax_ = lowMax;
      highMin_ = highMin;
      highMax_ = highMax;
      state_ = -1;
    }

    update() {
      channel_.update();
      value_ = channel_.value();

      if (value_ >= lowMin_ && value_ <= lowMax_) {
        state_ = 0;
      }
      else if (value_ >= highMin_ && value_ <= highMax_) {
        state_ = 1;
      }
      else {
        state_ = -1;
      }
      lastState_ = state_;
    }

    bool isOff() {
      return state_ == 0;
    }

    bool isOn() {
      return state_ == 1;
    }

    int state() {
      return state_;
    }

    uint16_t value() {
      return value_;
    }

  protected:
    int state_;
    int lastState_;
    uint16_t lowMin_;
    uint16_t lowMax_;
    uint16_t highMin_;
    uint16_t highMax_;
    uint16_t value_;
    RemotePWMChannel channel_;
};

class RemoteThreeStateSwitch : public RemoteSwitch {
  public:

    RemoteThreeStateSwitch(uint8_t pin, uint16_t lowMin, uint16_t lowMax, uint16_t idleMin, uint16_t idleMax, uint16_t highMin, uint16_t highMax)
      : RemoteSwitch(pin, lowMin, lowMax, highMin, highMax) {
      idleMin_ = idleMin;
      idleMax_ = idleMax;
    }

    update() {

      RemoteSwitch::update();

      if (state_ == -1) {
        if (value_ >= idleMin_ && value_ <= idleMax_) {
          state_ = 2;
        }
      }
    }

    bool isIdle() {
      return state_ == 2;
    }

  private:

    uint16_t idleMin_;
    uint16_t idleMax_;
};

class RCChannel {
  public :

    RCChannel(uint8_t pin): channel_(pin) {
      forwardValue_ = 0;
      backwardValue_ = 0;
    }

    void update() {
      channel_.update();
      uint16_t value = channel_.value();
      int16_t vRemoteNorm = value - 1500;
      //Serial.println(vRemoteNorm);
      //vRemoteNorm = vRemoteNorm < 0 ? 0 : vRemoteNorm;


      if (vRemoteNorm < -20) {
        forwardValue_ = 0;
        backwardValue_ = -vRemoteNorm / 450.0 * 255;
      }
      else if (vRemoteNorm > 20) {
        backwardValue_ = 0;
        forwardValue_ = vRemoteNorm / 450.0 * 255 + 40;
      }
      else {
        forwardValue_ = 0;
        backwardValue_ = 0;
      }

      //value_ = vRemoteNorm/500.0*255;
      /*
        Serial.print(value);
        Serial.print(" ");
        Serial.print(vRemoteNorm);
        Serial.print(" ");
        Serial.print(value_);
        Serial.println();
      */
    }

    uint8_t forwardValue() {
      if (forwardValue_ > 255) {
        return 255;
      }
      return forwardValue_;
    }

    uint8_t backwardValue() {
      if (backwardValue_ > 255) {
        return 255;
      }
      return backwardValue_;
    }

  private:
    uint16_t forwardValue_;
    uint16_t backwardValue_;
    RemotePWMChannel channel_;
};

class Knife {
  public:
    Knife(uint8_t en_l_pin, uint8_t pwm_l_pin, uint8_t en_r_pin, uint8_t pwm_r_pin) :
      en_l_pin_(en_l_pin), en_r_pin_(en_r_pin), pwm_l_pin_(pwm_l_pin), pwm_r_pin_(pwm_r_pin) {

    }

    void setup() {
      pinMode(en_l_pin_, OUTPUT);
      pinMode(pwm_l_pin_, OUTPUT);
      pinMode(en_r_pin_, OUTPUT);
      pinMode(pwm_r_pin_, OUTPUT);
    
    
      digitalWrite(en_l_pin_, LOW);
      digitalWrite(pwm_l_pin_, LOW);
      digitalWrite(en_r_pin_, LOW);
      digitalWrite(pwm_r_pin_, LOW);
    }

    void enable() {
      digitalWrite(KNIFE_EN_L_PIN, HIGH);
      digitalWrite(KNIFE_EN_R_PIN, HIGH);
    }

    void disable() {
      digitalWrite(KNIFE_EN_L_PIN, LOW);
      digitalWrite(KNIFE_EN_R_PIN, LOW);
      analogWrite(KNIFE_PWM_L_PIN, 0);
    }

    void setVelocity(uint8_t v) {
      analogWrite(KNIFE_PWM_L_PIN, v);
    }

  private:
    uint8_t en_l_pin_;
    uint8_t en_r_pin_;
    uint8_t pwm_l_pin_;
    uint8_t pwm_r_pin_;

};

int pinSwitch1 = A0;
int led1Pin = 8;
int duration = 0;
int led1 = LOW;
/*
  #define DEBOUNCE_MS 20     //A debounce time of 20 milliseconds usually works well for tactile button switches.
  #define LED_PIN 13         //The standard Arduino "Pin 13" LED.
  #define LONG_PRESS 1000    //We define a "long press" to be 1000 milliseconds.
  #define BLINK_INTERVAL 100 //In the BLINK state, switch the LED every 100 milliseconds.

  Button switchOnOff(BUTTON_PIN, PULLUP, INVERT, DEBOUNCE_MS);
*/
RemoteThreeStateSwitch cutterVelocitySwitch(CUTTER_VELOCITY_PIN, 1900, 2100, 1400, 1600, 950, 1050);
RemoteSwitch onOffSwitch(ON_OFF_SWITCH_PIN, 1200, 2100, 950, 1050);
RemoteThreeStateSwitch cutterOnOffSwitch(CUTTER_SWITCH_PIN, 1900, 2100, 1400, 1600, 950, 1050);

RCChannel rcLeftStraightChannel(RC_LEFT_FW_PIN);
//RCChannel rcRightStraight(RC_RIGHT_FW_PIN);
RCChannel rcRightTurnChannel(RC_TURN_PIN);

Knife knife(KNIFE_EN_L_PIN, KNIFE_PWM_L_PIN, KNIFE_EN_R_PIN, KNIFE_PWM_R_PIN);

enum {OFF, ON, CUT};
uint8_t STATE;
uint8_t LAST_STATE;
unsigned long ms;

void setup() {
  // put your setup code here, to run once:
  pinMode(pinSwitch1, INPUT);
  pinMode(led1Pin, OUTPUT);
  pinMode(ENGINE_RIGHT_BACKWARD_PIN, OUTPUT);
  pinMode(ENGINE_RIGHT_FORWARD_PIN, OUTPUT);
  pinMode(ENGINE_LEFT_BACKWARD_PIN, OUTPUT);
  pinMode(ENGINE_LEFT_FORWARD_PIN, OUTPUT);
  pinMode(LED_ON_PIN, OUTPUT);
  pinMode(LED_PAUSE_PIN, OUTPUT);

  digitalWrite(LED_ON_PIN, HIGH);
  digitalWrite(LED_PAUSE_PIN, HIGH);

  knife.setup();
}


void loop() {
  ms = millis();               //record the current time
  onOffSwitch.update();
  cutterOnOffSwitch.update();

  switch (STATE) {
    case OFF:
      stateOff();
      break;

    case ON:
      stateOn();
      break;

    case CUT:
      stateCut();
      break;
  }

  Serial.print(STATE);
  Serial.print(" ");
  Serial.print(onOffSwitch.state());
  Serial.print(" ");
  Serial.print(cutterOnOffSwitch.state());
  Serial.print(" ");
  Serial.print(cutterVelocitySwitch.state());
  Serial.print(" ");
  Serial.print(rcLeftStraightChannel.backwardValue());
  Serial.print(" ");
  Serial.print(rcLeftStraightChannel.forwardValue());
  Serial.print(" ");
  Serial.print(rcRightTurnChannel.backwardValue());
  Serial.print(" ");
  Serial.print(rcRightTurnChannel.forwardValue());

  Serial.println("");

  if (LAST_STATE != STATE) {
    Serial.print("Transition to state: ");
    Serial.println(STATE);
  }

  LAST_STATE = STATE;
}

void stateOff() {
  if (onOffSwitch.isOn()) {
    STATE = ON;
    switchOn();
  }

  digitalWrite(LED_PAUSE_PIN, HIGH);
}

void stateOn() {
  if (cutterOnOffSwitch.isOn()) {
    STATE = CUT;
    switchOnCut();
    return;
  }
  if (onOffSwitch.isOff()) {
    STATE = OFF;
    switchOff();
    return;
  }
  
  digitalWrite(LED_PAUSE_PIN, HIGH);
  drive();
}

void stateCut() {
  if (cutterOnOffSwitch.isOff()) {
    STATE = ON;
    switchOffCut();
    return;
  }
  if (onOffSwitch.isOff()) {
    STATE = OFF;
    switchOffCut();
    switchOff();
    return;
  }

  cut();
  drive();
  digitalWrite(LED_PAUSE_PIN, LOW);
}

void switchOff() {
  switchOffCut();
}

void switchOn() {
}

void switchOnCut() {
  knife.enable();
}

void switchOffCut() {
  knife.disable();
}

int readCutterVelocity() {
  cutterVelocitySwitch.update();
  if (cutterVelocitySwitch.isOn()) {
    return 255;
  }
  if (cutterVelocitySwitch.isIdle()) {
    return 168;
  }
  return 128;
}

void cut() {  
  knife.setVelocity(readCutterVelocity());
}

uint8_t to8Bit(int16_t value) {
  if (value > 255) {
    return 255;
  }
  if (value < 0) {
    return 0;
  }
  return value;
}

void drive() {
  rcLeftStraightChannel.update();
  rcRightTurnChannel.update();

  uint8_t turnLeft = rcRightTurnChannel.backwardValue();
  uint8_t turnRight = rcRightTurnChannel.forwardValue();
  uint8_t turnLeftValue = 0;
  uint8_t turnRightValue = 0;
  if (turnLeft > 0) {
    turnLeftValue = turnLeft;
  }
  else if (turnRight > 0) {
    turnRightValue = turnRight;
  }
  
  uint8_t fw = rcLeftStraightChannel.forwardValue();
  uint8_t bw = rcLeftStraightChannel.backwardValue();

  int16_t bwRight = bw;
  int16_t bwLeft = bw;
  int16_t fwRight = fw;
  int16_t fwLeft = fw;

  if (fw > 0) {
    if (turnLeftValue > 0) {
      fwLeft -= turnLeftValue;
    }
    else if (turnRightValue > 0) {
      fwRight -= turnRightValue;
    }
  }
  else if (bw > 0) {
    if (turnLeftValue > 0) {
      bwLeft -= turnLeftValue;
    }
    else if (turnRightValue > 0) {
      bwRight -= turnRightValue;
    }
  }
  else if (fw == 0 && bw == 0) {
    if (turnLeftValue > 0) {
      fwLeft = bwRight = turnLeftValue / 2;
    }
    else if (turnRightValue > 0) {
      fwRight = bwLeft = turnRightValue / 2;
    }
  }

  

  uint8_t bwRight_8 = to8Bit(bwRight);
  uint8_t fwRight_8 = to8Bit(fwRight);
  uint8_t bwLeft_8 = to8Bit(bwLeft);
  uint8_t fwLeft_8 = to8Bit(fwLeft);

  analogWrite(ENGINE_RIGHT_BACKWARD_PIN, bwRight_8);
  analogWrite(ENGINE_RIGHT_FORWARD_PIN, fwRight_8);

  analogWrite(ENGINE_LEFT_BACKWARD_PIN, bwLeft_8);
  analogWrite(ENGINE_LEFT_FORWARD_PIN, fwLeft_8);

  Serial.print(" ");
  Serial.print(bwRight_8);
  Serial.print(" ");
  Serial.print(fwRight_8);
  Serial.print(" ");
  Serial.print(bwLeft_8);
  Serial.print(" ");
  Serial.print(fwLeft_8);
  Serial.print(" | ");
}


