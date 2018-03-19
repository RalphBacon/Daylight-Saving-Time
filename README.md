# Daylight Saving Time
How to work out whether a date is in the DST period

The main routine return a boolean if the date supplied is in the DST period  
**bool isBST(int dayOfMonth, int Month, int Year, int Hour)**

This requires the day of week of the date so it can work out the last Sunday in March and October (may be different in your part of the world)  
**int getDayOfWeekFromDate(int d, int m, int y)**

This is a single line of C++ code that works out the day of the week, taking into account leap years an everything. I wish I could patent it!

Other functions can determine whether we have a valid date:  
**bool isValidDate(int dayOfMonth, int Month, int Years, dateSegment &errorSegment)**

And if the date is not valid, try and fix it:  
**void makeValidDate(int &dayOfMonth, int &Month, int &Year, dateSegment updatePart)**

Some use of pointers is made to make the coding simpler. You can identify these because the variable name either has an asterisk in front of it like this:  
**byte \*hour**

If it references a pointer value, it will have an ampersand attached like this:  
**dateSegment &errorSegment**

One day I really will create a library for this.
