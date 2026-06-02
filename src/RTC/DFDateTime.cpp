/*
 * This file is part of the "DieselFire" distribution 
 * (https://dieselfire.wabo.cc) 
 *
 * Copyright (C) 2019  Ray Jones <ray@mrjones.id.au>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 */

#include "DFDateTime.h"
#include "../Utility/macros.h"

const char*
DFDateTime::dowStr() const
{
  return daysOfTheWeek[dayOfTheWeek()];
}

const char*
DFDateTime::monthStr()  const
{
  return months[month()-1];
}

DFDateTime& 
DFDateTime::operator=(const DateTime& rhs)
{
  yOff = rhs.year()-2000;
  m = rhs.month();
  d = rhs.day();
  hh = rhs.hour();
  mm = rhs.minute();
  ss = rhs.second();
  return *this;
}

DFDateTime& 
DFDateTime::operator=(const DFDateTime& rhs)
{
  yOff = rhs.yOff;
  m = rhs.m;
  d = rhs.d;
  hh = rhs.hh;
  mm = rhs.mm;
  ss = rhs.ss;
  return *this;
}

void 
DFDateTime::adjustDay(int dir)
{
  int days = daysInMonth(m, yOff);
  if(dir > 0) {
    if(d == days)  d = 1;
    else  d++;
  }
  else {
    if(d == 1) d = days;
    else  d--;
  }
}

void
DFDateTime::adjustMonth(int dir)
{
  if(dir > 0) {
    if(m == 12)  m = 1;
    else m++;
  }
  else {
    if(m == 1)  m = 12;
    else  m--;
  }
  int days = daysInMonth(m, yOff);   // trap shorter months
  UPPERLIMIT(d, days);
}

void
DFDateTime::adjustYear(int dir)
{
  yOff += dir;
  int days = daysInMonth(m, yOff);
  UPPERLIMIT(d, days);              // pick up 29 Feb
}

void
DFDateTime::adjustHour(int dir)
{
  if(dir > 0) {
    if(hh == 23)  hh = 0;
    else  hh++;
  }
  else {
    if(hh == 0)  hh = 23;
    else  hh--;
  }
}

void
DFDateTime::adjustHour12()
{
  hh += 12;
  if(hh >= 24)  
    hh -= 24;
}

void
DFDateTime::adjustMinute(int dir)
{
  if(dir > 0) {
    if(mm == 59)  mm = 0;
    else  mm++;
  }
  else {
    if(mm == 0)  mm = 59;
    else  mm--;
  }
}

void
DFDateTime::adjustSecond(int dir)
{
  if(dir > 0) {
    if(ss == 59)  ss = 0;
    else  ss++;
  }
  else {
    if(ss == 0)  ss = 59;
    else  ss--;
  }
}

int 
DFDateTime::daysInMonth(int month, int year) const
{
  if(month >= 1 && month <= 12) {
    int days = monthDays[month-1];
    if((month == 2) && ((year & 0x03) == 0))
      days++;
    return days;
  }
  return -1;
}
