/*
 * This file is part of the "DieselFire" distribution 
 * (https://dieselfire.wabo.cc) 
 *
 * Copyright (C) 2019  Ray Jones, James Clark
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

#ifndef __DF_OTA_H
#define __DF_OTA_H

void initOTA();
void initFOTA();
void doOTA();
bool CheckFirmwareCRC(int size);

#endif