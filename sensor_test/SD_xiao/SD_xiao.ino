#include <TORICA_SD.h>
#include <SD.h>

#define SD_SPI_SCK  (8)
#define SD_SPI_MOSI (10)
#define SD_SPI_MISO (9)

int cs_SD = 7; // XIAO RP2040ならRP2040のGPIO番号で指定
TORICA_SD my_torica_sd(cs_SD);

void listFiles(File dir, int numTabs) {
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      // No more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      listFiles(entry, numTabs + 1);
    } else {
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

void setup()
{
  Serial.begin(115200);
  while (!Serial) delay(10);

  if (!my_torica_sd.begin()) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");

  File root = SD.open("/");
  listFiles(root, 0);
  root.close();
}

void loop()
{
  // 何もしない
}