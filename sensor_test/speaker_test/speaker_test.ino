#define SPPIN 15

int freq[]={262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988, 1047};

struct NOTE {
  int tone;
  float length;
};

NOTE sheet[]={
    {0, 1}, {0, 1}, {7, 1}, {7, 1}, {9, 1}, {9, 1}, {7, 2}, {5, 1}, {5, 1}, {4, 1}, {4, 1}, {2, 1}, {2, 1}, {0, 2},
    {7, 1}, {7, 1}, {5, 1}, {5, 1}, {4, 1}, {4, 1}, {2, 2}, {7, 1}, {7, 1}, {5, 1}, {5, 1}, {4, 1}, {4, 1}, {2, 2}, 
    {0, 1}, {0, 1}, {7, 1}, {7, 1}, {9, 1}, {9, 1}, {7, 2}, {5, 1}, {5, 1}, {4, 1}, {4, 1}, {2, 1}, {2, 1}, {0, 2}
};

void setup() {
  pinMode(SPPIN, OUTPUT);
}

void loop() {
  int count = sizeof(sheet)/sizeof(NOTE);
  for(int i=0; i<count; i++){
    int length = 500*sheet[i].length;
    tone(SPPIN, freq[sheet[i].tone], length*0.95);
    delay(length);
  }
  delay(5000);
}