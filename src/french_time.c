#include "french_time.h"

static const char* STR_HEURE = "past";
static const char* STR_HEURE2 = "five p.";
static const char* STR_MOINS = "to";
static const char* STR_MOINS2 = "five to";
static const char* const HEURES[] = {
	
  "twelve",
  "one",
  "two",
  "three",
  "four",
  "five",
  "six",
  "seven",
  "eight",
  "nine",
  "ten",
  "eleven",
  "twelve"
};

static const char* const MINS[] = {
  "o'clock!",
  "five",
  "ten",
  "quarter",
  "twenty",
  "half",
  "it's",
  "now"
};

static const char* const JOURS[] = {
  "Sunday",
  "Monday",
  "Tuesday",
  "Wednesday",
  "Thursday",
  "Friday",
  "Saturday"
};

//static const char* const MOIS[] = {
  //"Jan.",
//  "Feb.",
//  "Mar.",
//  "Apr",
//  "May",
//  "June",
//  "July",
//  "Aug.",
//  "Sep.",
//  "Oct.",
//  "Nov.",
//  "Dec."
//};



void fuzzy_time(struct tm* t, char* line1, char* line2, char* line3) {

  int hours = t->tm_hour;
  int minutes = t->tm_min;

  strcpy(line1, "");
  strcpy(line2, "");
  strcpy(line3, "");



  if (minutes >= 33) hours++;
  if (hours >= 24) hours = 0;
  if (hours > 12) hours -= 12;

 strcat(line3, HEURES[hours]); // HOURS

  if (minutes >= 0 && minutes < 3) {
   if(hours >= 0 && hours <= 12) {
        strcat(line2, MINS[7]);
      }
 	  strcat(line1, MINS[6]); // O'CLOCK
  }
	
  else if (minutes < 8) {
    if(hours >= 0 && hours <= 12) {
        strcat(line2, STR_HEURE);
    }
	  strcat(line1, MINS[1]); // FIVE PAST
  }
	
  else if (minutes < 13) {
    if(hours >= 0 && hours <= 12) {
        strcat(line2, STR_HEURE);
    }
    strcat(line1, MINS[2]); // TEN PAST
  }
	
  else if (minutes < 18) {
    if(hours >= 0 && hours <= 12) {
        strcat(line2, STR_HEURE);
    }
    strcat(line1, MINS[3]); // QUARTER PAST
  }
	
  else if (minutes < 23) {
    if(hours >= 0 && hours <= 12) {
        strcat(line2, STR_HEURE);
    }
    strcat(line1, MINS[4]); // TWENTY PAST
  }
	
 else if (minutes < 28) {
    if(hours >= 0 && hours <= 12) {
        strcat(line2, STR_HEURE2);
    }
    strcat(line1, MINS[4]); // TWENTY FIVE PAST
  }
	
   else if (minutes < 33) {
    if(hours >= 0 && hours <= 12) {
        strcat(line2, STR_HEURE);
    }
    strcat(line1, MINS[5]); // HALF PAST
  }
	
	 else if (minutes < 38) {
    if(hours >= 0 && hours <= 12) {
        strcat(line2, STR_MOINS2);
    }
    strcat(line1, MINS[4]); // TWENTY FIVE TO
  }
	
	 else if (minutes < 43) {
    if(hours >= 0 && hours <= 12) {
        strcat(line2, STR_MOINS);
    }
    strcat(line1, MINS[4]); // TWENTY  TO
  }
	
  	 else if (minutes < 48) {
    if(hours >= 0 && hours <= 12) {
        strcat(line2, STR_MOINS);
    }
    strcat(line1, MINS[3]); // QUARTER TO
  }
	
	 else if (minutes < 53) {
    if(hours >= 0 && hours <= 12) {
        strcat(line2, STR_MOINS);;
    }
    strcat(line1, MINS[2]); // TEN TO
  }
	
 	 else if (minutes < 58) {
    if(hours >= 0 && hours <= 12) {
        strcat(line2, STR_MOINS);
    }
    strcat(line1, MINS[1]); // FIVE TO
  }
	
 	 else if (minutes < 60) {
    if(hours >= 0 && hours <= 12) {
        strcat(line2, MINS[7]);
    }
	  strcat(line1, MINS[6]); // IT'S NOW
  }
  }
 


void info_lines(struct tm* t, char* line1, char* line2) {

  //char wday[LINE_BUFFER_SIZE];
 // char mday[LINE_BUFFER_SIZE];
 // char mon[LINE_BUFFER_SIZE];

  strcpy(line1, "");
  strcpy(line2, "");

//  string_format_time(line1, LINE_BUFFER_SIZE, "%A • %e %b", t);
 // mini_snprintf(wday, LINE_BUFFER_SIZE, "%s", JOURS[t->tm_wday]);
  //mini_snprintf(mday, LINE_BUFFER_SIZE, "%d", t->tm_mday);
 // mini_snprintf(mon, LINE_BUFFER_SIZE, "%s", MOIS[t->tm_mon]);
 // strcat(line1, wday);
 // strcat(line1, " ");
 // strcat(line1, mday);
 // strcat(line1, " ");
 // strcat(line1, mon);

  strftime(line1, LINE_BUFFER_SIZE, "%A • %H:%M", t);
  strftime(line2, LINE_BUFFER_SIZE, " Week %V • %B %e", t);

}



























