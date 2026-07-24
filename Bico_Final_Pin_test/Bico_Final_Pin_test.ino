constexpr int pin[] = {1,2,3,4,5,11,12,13,14,17,18,20,21,26,27};
constexpr int led_pin[] = {8,9,10,22,23,24};

void setup() {
  for(int i = 0; i < sizeof pin / sizeof pin[0]; i++){
    pinMode(pin[i], OUTPUT);
  }
  for (int i=0; i < sizeof led_pin / sizeof led_pin[0]; i++){
    pinMode(led_pin[i], OUTPUT);
  }
}

void loop() {
  for (int i = 0; i < sizeof pin / sizeof pin[0]; i++){
    digitalWrite(pin[i], HIGH);
  }
  for (int i = 0; i < sizeof led_pin / sizeof led_pin[0]; i++){
    digitalWrite(led_pin[i], HIGH);
  }
}