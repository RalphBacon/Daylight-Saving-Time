/* 
 _______  __    __  .__   __.   ______ .___________. __    ______   .__   __.      _______.
|   ____||  |  |  | |  \ |  |  /      ||           ||  |  /  __  \  |  \ |  |     /       |
|  |__   |  |  |  | |   \|  | |  ,----'`---|  |----`|  | |  |  |  | |   \|  |    |   (----`
|   __|  |  |  |  | |  . `  | |  |         |  |     |  | |  |  |  | |  . `  |     \   \    
|  |     |  `--'  | |  |\   | |  `----.    |  |     |  | |  `--'  | |  |\   | .----)   |   
|__|      \______/  |__| \__|  \______|    |__|     |__|  \______/  |__| \__| |_______/    
*/

enum buttonType { none, fn, inc, all };

enum dateSegment { segNothing, segDayOfMonth, segMonths, segYears, segHours, segMins };

void readDS3231time(
  byte *second,
  byte *minute,
  byte *hour,
  byte *dayOfWeek,
  byte *dayOfMonth,
  byte *month,
  byte *year);

bool const DEBUG = true;
bool const TRACE = true;

// Button pins
const int fnButton = 2;
const int incButton = 3;

// -----------------------------------------------------------
// BUTTON PRESS BUTTON PRESS BUTTON PRESS BUTTON PRESS
// -----------------------------------------------------------
int getButtonPress(buttonType btnPress, bool waitForPress) {

  do {
    // Function button
    if (btnPress == fn || btnPress == all) {
      if (digitalRead(fnButton)) {
        if (DEBUG) Serial.println("Function button press detected");
        delay(50);
        while (digitalRead(fnButton)) {}
        if (DEBUG) Serial.println("Function button returning 1");
        return 1;
      }
    }

    // Increment button
    if (btnPress == inc || btnPress == all) {
      if (digitalRead(incButton)) {
        if (DEBUG) Serial.println("Up (increment) button press detected");
        delay(50);
        while (digitalRead(incButton)) {}
        if (DEBUG) Serial.println("Up button returning 2");
        return 2;
      }
    }
  } while (waitForPress);

  // Nothing pressed of interest
  return 0;
}

// -----------------------------------------------------------
// Check for valid date in respect of days in month
// -----------------------------------------------------------
bool isValidDate(int dayOfMonth, int Month, int Years, dateSegment &errorSegment) {

  // Year must be divisible by 4, and, if a century, be divisible by 400 too
  Years = Years < 2000 ? 2000 + Years : Years;

  bool isLeapYear = Years % 4 == 0 && ((Years % 100 == 0 && Years % 400 == 0) || (Years % 100 > 0));

  int days30[] = { 9, 4, 6, 11 };
  int daysInMonth = Month == 2 ? 28 + isLeapYear : 31;
  for (int cnt = 0; cnt < 4; cnt++) {
    if (days30[cnt] == Month) {
      daysInMonth = 30;
      break;
    }
  }

  if (dayOfMonth > daysInMonth) {
    errorSegment = segDayOfMonth;
    return false;
  }

  if (Month > 12) {
    errorSegment = segMonths;
    return false;
  }

  if (Years > 2030) {
    errorSegment = segYears;
    return false;
  }

  // All OK
  errorSegment = segNothing;
  return true;
}

// -----------------------------------------------------------
// Tries to make the date valid by flipping over or reducing values
// -----------------------------------------------------------
void makeValidDate(int &dayOfMonth, int &Month, int &Year, dateSegment updatePart) {

  dateSegment errorPart;

  while (!isValidDate(dayOfMonth, Month, Year, errorPart)) {

    switch (errorPart) {
      case 0:
        break;
      case 1:
        updatePart == segDayOfMonth ? dayOfMonth = 1 : --dayOfMonth;
        break;
      case 2:
        Month = 1;
        break;
      case 3:
        Year = 0;
        break;
    }
  }
}

// -----------------------------------------------------------
// Calculate day of week from date
// -----------------------------------------------------------
int getDayOfWeekFromDate(int d, int m, int y) {
  int dayOfWeek = (d += m < 3 ? y-- : y - 2, 23 * m / 9 + d + 4 + y / 4 - y / 100 + y / 400) % 7;
  return dayOfWeek == 0 ? 7 : dayOfWeek; // Mon = 1 Sun = 7 (not zero, makes DS3231 go mad)
}

// -----------------------------------------------------------
// Adjust display (not actual RTC) with +1 when BST
// -----------------------------------------------------------
bool isBST(int dayOfMonth, int Month, int Year, int Hour) {

  if (DEBUG) {
    Serial.print("Date:");
    Serial.print(dayOfMonth);
    Serial.print("/");
    Serial.print(Month);
    Serial.print("/");
    Serial.print(Year);
    Serial.print(" ");
    Serial.print(Hour);
    Serial.print(":00\n");
  }

  // Get last Sunday in March
  int lastDayInMarch = getDayOfWeekFromDate(31, 03, Year);
  int BSTStartSunday = 31 - (lastDayInMarch == 7 ? 0 : lastDayInMarch);
  if (TRACE) {
    Serial.print("BST starts on ");
    Serial.print(BSTStartSunday);
    Serial.print(" March (Sunday 2am)\n");
  }

  // Get last Sunday in October
  int lastDayInOctober = getDayOfWeekFromDate(31, 10, Year);
  int BSTEndSunday = 31 - (lastDayInOctober == 7 ? 0 : lastDayInOctober);
  if (TRACE) {
    Serial.print("BST ends on ");
    Serial.print(BSTEndSunday);
    Serial.print(" October (Sunday 2am)\n");
  }

  if ( (Month >= 4 && Month <= 9)
    || (Month == 03 && dayOfMonth > BSTStartSunday)
    || (Month == 03 && dayOfMonth == BSTStartSunday && Hour >= 2)
    || (Month == 10 && dayOfMonth < BSTEndSunday )
    || (Month == 10 && dayOfMonth == BSTEndSunday && Hour <= 1))
  {
    if (DEBUG) Serial.print("BST\n");
    return true;
  }

  // Nope, back to GMT
  if (DEBUG) Serial.print("GMT\n");
  return false;
}

