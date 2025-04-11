#include "datasheet.h"

Data_Sheet_t dsheet; // 数据表结构体
void Data_Sheet_Init() {
    if(!prefs.begin("data_sheet", false))
    {
        Serial.println("Failed to initialize preferences");
        return;
    }
    if (prefs.isKey("data_sheet")) {
        prefs.getBytes("data_sheet", &dsheet, sizeof(dsheet));
        Serial.println("Loaded data_sheet from flash");
        return;
    }
    prefs.putBytes("data_sheet", &dsheet, sizeof(dsheet));
    Serial.println("Saved data_sheet to flash");
}

void Data_Sheet_Read() {
    for (int i = 0; i < MAX_KEY_NUM; i++) {
        Serial.printf("key[%d]: %d\n", i, dsheet.key[i]);
    }
    Serial.println("Data_Sheet Read Successfully");
}

void Data_Sheet_Modify(int operation) {
    dsheet.key[operation/10000] = operation%10000;
}

void Data_Sheet_Save() {
    if(prefs.putBytes("data_sheet", &dsheet, sizeof(dsheet))) {
        Serial.println("Data_Sheet Saved Successfully");
        return;
    }
    Serial.println("Data_Sheet Save Failed");
}