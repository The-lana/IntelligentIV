#include "menu.h"


enum MENU CurrentMenu = FUNCTION_SELECTION;
enum FUNCTION_SELECTION_MENU CurrentFuntion = DRIP_RATE;


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
void initialize(){

    display.clearDisplay();
    display.setTextSize(TEXT_SIZE_USED);
    drawMenu();    
}

void checkIIC(){

    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
    }
}

void drawMenu(){

    display.clearDisplay();
    display.setTextSize(TEXT_SIZE_USED);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println(F("Menu 1"));
    display.setCursor(0,16);
    display.println(F("Menu 2"));
    //display.setCursor(0,);
    //display.println(F("Exit"));

    display.display();
    delay(20);
}

void drawWords(int location, char* text){

    display.setTextColor(SSD1306_WHITE,SSD1306_BLACK);
    display.setCursor(0,8*location*TEXT_SIZE_USED);
    display.printf("%s",text);
    display.display();
    delay(50);

}

void drawInvertedWords(int location, char* text){

    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    display.setCursor(0,8*location*TEXT_SIZE_USED);
    display.printf("%s",text);
    display.display();
    delay(50);

}
