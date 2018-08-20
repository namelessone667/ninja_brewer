/*
 * Copyright 2012-2013 BrewPi/Elco Jacobs.
 *
 * This file is part of BrewPi.
 *
 * BrewPi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BrewPi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BrewPi.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef JSONKeys_h_
#define JSONKeys_h_

static const char JSONKEY_mode[] = "mode";
static const char JSONKEY_beerSetting[] = "beerSet";
static const char JSONKEY_fridgeSetting[] = "fridgeSet";
static const char JSONKEY_heatEstimator[] = "heatEst";
static const char JSONKEY_coolEstimator[] = "coolEst";

// constant;
static const char JSONKEY_tempFormat[] = "tempFormat";
static const char JSONKEY_tempSettingMin[] = "tempSetMin";
static const char JSONKEY_tempSettingMax[] = "tempSetMax";
static const char JSONKEY_pidMax[] = "pidMax";
static const char JSONKEY_Kp[] = "Kp";
static const char JSONKEY_Ki[] = "Ki";
static const char JSONKEY_Kd[] = "Kd";
static const char JSONKEY_iMaxError[] = "iMaxErr";
static const char JSONKEY_idleRangeHigh[] = "idleRangeH";
static const char JSONKEY_idleRangeLow[] = "idleRangeL";
static const char JSONKEY_heatingTargetUpper[] = "heatTargetH";
static const char JSONKEY_heatingTargetLower[] = "heatTargetL";
static const char JSONKEY_coolingTargetUpper[] = "coolTargetH";
static const char JSONKEY_coolingTargetLower[] = "coolTargetL";
static const char JSONKEY_maxHeatTimeForEstimate[] = "maxHeatTimeForEst";
static const char JSONKEY_maxCoolTimeForEstimate[] = "maxCoolTimeForEst";
static const char JSONKEY_fridgeFastFilter[] = "fridgeFastFilt";
static const char JSONKEY_fridgeSlowFilter[] = "fridgeSlowFilt";
static const char JSONKEY_fridgeSlopeFilter[] = "fridgeSlopeFilt";
static const char JSONKEY_beerFastFilter[] = "beerFastFilt";
static const char JSONKEY_beerSlowFilter[] = "beerSlowFilt";
static const char JSONKEY_beerSlopeFilter[] = "beerSlopeFilt";
static const char JSONKEY_lightAsHeater[] = "lah";
static const char JSONKEY_rotaryHalfSteps[] = "hs";
//#if SettableMinimumCoolTime
static const char JSONKEY_minCoolTime[] = "minCoolTime";
static const char JSONKEY_minCoolIdleTime[] = "minCoolIdleTime";
static const char JSONKEY_minHeatTime[] = "minHeatTime";
static const char JSONKEY_minHeatIdleTime[] = "minHeatIdleTime";
static const char JSONKEY_mutexDeadTime[] = "deadTime";
//#endif
// variable;
static const char JSONKEY_beerDiff[] = "beerDiff";
static const char JSONKEY_diffIntegral[] = "diffIntegral";
static const char JSONKEY_beerSlope[] = "beerSlope";
static const char JSONKEY_p[] = "p";
static const char JSONKEY_i[] = "i";
static const char JSONKEY_d[] = "d";
static const char JSONKEY_estimatedPeak[] = "estPeak"; // current peak estimate
static const char JSONKEY_negPeakEstimate[] = "negPeakEst"; // last neg peak estimate before switching to idle
static const char JSONKEY_posPeakEstimate[] = "posPeakEst";
static const char JSONKEY_negPeak[] = "negPeak"; // last true neg peak
static const char JSONKEY_posPeak[] = "posPeak";

static const char JSONKEY_logType[] = "logType";
static const char JSONKEY_logID[] = "logID";

#endif
