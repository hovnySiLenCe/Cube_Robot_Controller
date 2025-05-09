#include "datasheet.h"

Data_Sheet_t dsheet, tmp_ds; // 数据表结构体
Preferences prefsDataSheet; // 用于存储数据的对象
void Data_Sheet_Init() {
    Serial.println("--------- Initializing Data_Sheet ----------");
    if(!prefsDataSheet.begin("data_sheet", false)) {
        Serial.println("[FETAL] Failed to initialize preferences");
        return;
    }
    if (prefsDataSheet.isKey("data_sheet")) {
        prefsDataSheet.getBytes("data_sheet", &tmp_ds, sizeof(tmp_ds));
        dsheet.Merge(tmp_ds);
        Serial.println("[SUCCESS] Loaded data_sheet from flash");
        dsheet.Show();
        return;
    }
    prefsDataSheet.putBytes("data_sheet", &dsheet, sizeof(dsheet));
    Serial.println("[SUCCESS] Saved data_sheet to flash");
}

void Data_Sheet_Read() {
    for (int i = 0; i < MAX_KEY_NUM; i++) {
        Serial.printf("#DS%02d%04d\n", i, dsheet.key[i]);
    }
    Serial.println("#DSEND");
}

void Data_Sheet_Modify(int operation) {
    if(operation%10000) dsheet.key[operation/10000] = operation%10000;
}

void Data_Sheet_Save() {
    if(prefsDataSheet.putBytes("data_sheet", &dsheet, sizeof(dsheet))) {
        Serial.println("[SUCCESS] Data_Sheet Saved Successfully");
        Serial.println("Current Data_Sheet:");
        dsheet.Show();
        reGenerateSCurveStepTimes();
        SaveAccArrays();
        return;
    }
    Serial.println("[FETAL] Data_Sheet Save Failed");
}