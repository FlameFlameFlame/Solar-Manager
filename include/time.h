#pragma once

#include <Arduino.h>

class Time {
    private:
        int hours, minutes, seconds;
        int ContainsSeconds () {
            return hours * 3600 + minutes * 60 + seconds;
        }

    public:
    
    Time (int value) {
        hours = value / 3600;
        value -= hours * 3600;
        Serial.println (value);
        Serial.print ("Hours: ");
        Serial.println (hours);
        minutes = value / 60;
        value -= minutes * 60;
        Serial.print ("Minutes: ");
        Serial.println (minutes);
        Serial.println (value);
        seconds = value;
        Serial.print ("Seconds: ");
        Serial.println (seconds);
        Serial.println (value);


        Serial.print ("New time:");
        Serial.println (AsString());
    } 

    Time (const int &_hours, const int &_minutes, const int &_seconds) {
        hours = _hours;
        minutes = _minutes % 60;
        hours += minutes / 60;

        seconds = _seconds % 60;
        minutes += seconds / 60;
    }

    int GetSeconds() const {
        return seconds;
    }

    int GetMinutes() const {
        return minutes;
    }

    int GetHours() const {
        return hours;
    }

    void SetSeconds (const int& value) {
        seconds = value;
    }

    void SetMinutes (const int& value) {
        minutes = value;
    }

    void SetHours (const int& value) {
        hours = value;
    }   

    String AsString () {
        String res = "";
        res += minutes < 10 ? "0" + (String) minutes : minutes;
        res += ":";
        res += seconds < 10 ? "0" + (String) seconds : seconds;
        return res;
    } 

    
    Time& operator-- (int lhs) {
        int t = ContainsSeconds();
        Serial.print ("Contains seconds: ");
        Serial.println (t);
        t--;
        *this = Time (t);
        return *this;
    }
};

