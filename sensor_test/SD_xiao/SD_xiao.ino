#include <SPI.h>
#include <SD.h>

// #define SD_SPI_SCK (8)
// #define SD_SPI_MOSI (10)
// #define SD_SPI_MISO (9)


const int cs_SD = 1;
const int spi_mosi   = 9;
const int spi_miso   = 8;
const int spi_sck    = 7;

File myFile;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; 
  }

  Serial.print("Initializing SD card...");

  SPI.begin(spi_sck, spi_miso, spi_mosi); 
  
  if (!SD.begin(cs_SD)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("testing 1, 2, 3.");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  // re-open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("test.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}

void loop() {
  // nothing happens after setup
}