// Copyright 2023 Esri

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "Mountain.h"

// ------------------------------------- //
//              Constructor              //
// ------------------------------------- //

Mountain::Mountain(QString name, double latitude, double longitude, double elevation, QObject* parent) :
    QObject{parent},
    m_elevation(elevation),
    m_latitude(latitude),
    m_longitude(longitude),
    m_name(std::move(name))
{
    initialiseDailyConditionsMap();
}

// ------------------------------------- //
//    Static Property Initialisation     //
// ------------------------------------- //

double Mountain::maxPrecipitationMeasurement = 0.0;
double Mountain::maxTemperatureMeasurement = 0.0;
double Mountain::minTemperatureMeasurement = 0.0;

// ------------------------------------- //
//     Property Getters and Setters      //
// ------------------------------------- //

const QList<double> Mountain::getDailyPrecipitation() const
{
    return m_precipitation_daily.values();
}

void Mountain::setDailyPrecipitation(const QList<double>& newData)
{
    m_precipitation_daily.clear();
    for (int counter = 0; counter < m_dates.size(); ++counter)
    {
        const QDate date = m_dates[counter];
        const double precipitation = newData[counter];
        m_precipitation_daily[date] = precipitation;
    }
}

const QList<QString> Mountain::getDailyWeatherConditions() const
{
    return m_weatherconditions_daily.values();
}

void Mountain::setDailyWeatherConditions(const QList<int>& newData)
{
    m_weatherconditions_daily.clear();
    for (int counter = 0; counter < m_dates.size(); ++counter)
    {
        const QDate date = m_dates[counter];
        const int weatherCode = newData[counter];
        const QString weatherDescription = m_dailyConditionsMap[weatherCode];
        m_weatherconditions_daily[date] = weatherDescription;
    }
}

const QList<QString> Mountain::getDailyWindDirection() const
{
    return m_winddirection_daily.values();
}

void Mountain::setDailyWindDirection(const QList<int>& newData)
{
    m_winddirection_daily.clear();
    for (int counter = 0; counter < m_dates.size(); ++counter)
    {
        const QDate date = m_dates[counter];
        const int windDirectionInDegrees = newData[counter];
        const QString windOrientation = convertWindDirectionToOrientation(windDirectionInDegrees);
        m_winddirection_daily[date] = windOrientation;
    }
}

const QList<double> Mountain::getDailyWindGusts() const
{
    return m_windgusts_daily.values();
}

void Mountain::setDailyWindGusts(const QList<double>& newData)
{
    m_windgusts_daily.clear();
    for (int counter = 0; counter < m_dates.size(); ++counter)
    {
        const QDate date = m_dates[counter];
        const double windGust = newData[counter];
        m_windgusts_daily[date] = windGust;
    }
}

const QList<double> Mountain::getDailyWindSpeed() const
{
    return m_windspeed_daily.values();
}

void Mountain::setDailyWindSpeed(const QList<double>& newData)
{
    m_windspeed_daily.clear();
    for (int counter = 0; counter < m_dates.size(); ++counter)
    {
        const QDate date = m_dates[counter];
        const double windspeed = newData[counter];
        m_windspeed_daily[date] = windspeed;
    }
}

const QList<QDate> Mountain::getDates() const
{
    return m_dates;
}

void Mountain::setDates(const QList<QDate>& dates)
{
    m_dates = dates;
}

const QList<QString> Mountain::getDays() const
{
    QList<QString> days;
    for (const auto date : m_dates)
        days.append(date.toString("ddd"));
    return days;
}

const double Mountain::getElevation() const
{
    return m_elevation;
}

const QList<double> Mountain::getHourlyApparentTemperature() const
{
    return m_apparentTemperature_hourly;
}

void Mountain::setHourlyApparentTemperature(const QList<double>& newData)
{
    m_apparentTemperature_hourly = newData;
}

const QList<QDateTime> Mountain::getHourlyDateTime() const
{
    return m_dateTime_hourly;
}

void Mountain::setHourlyDateTime(const QList<QDateTime>& newData)
{
    m_dateTime_hourly = newData;

    // To ensure the lines marking the days on the date/time axis on the results plots
    // are in the correct place, add an extra hour to the data to make the last data point
    // exactly 7 days after the first data point.
    const QDateTime lastMeasurement = m_dateTime_hourly.last();
    m_dateTime_hourly.append(lastMeasurement.addSecs(3600));
}

const QList<double> Mountain::getHourlyPrecipitation() const
{
    return m_precipitation_hourly;
}

void Mountain::setHourlyPrecipitation(const QList<double>& newData)
{
    m_precipitation_hourly = newData;
}

const QList<double> Mountain::getHourlyTemperature() const
{
    return m_temperature_hourly;
}

void Mountain::setHourlyTemperature(const QList<double>& newData)
{
    m_temperature_hourly = newData;
}

const QList<int> Mountain::getHourlyVisibility() const
{
    return m_visibility_hourly;
}

void Mountain::setHourlyVisibility(const QList<int>& newData)
{
    m_visibility_hourly = newData;
}

const double Mountain::getLatitude() const
{
    return m_latitude;
}

const double Mountain::getLongitude() const
{
    return m_longitude;
}

const double Mountain::getMaxPrecipitationMeasurement() const
{
    return Mountain::maxPrecipitationMeasurement;
}

const double Mountain::getMaxTemperatureMeasurement() const
{
    return Mountain::maxTemperatureMeasurement;
}

const double Mountain::getMinTemperatureMeasurement() const
{
    return Mountain::minTemperatureMeasurement;
}

const QString Mountain::getName() const
{
    return m_name;
}

// ------------------------------------- //
//            Public Methods             //
// ------------------------------------- //

void Mountain::identifyMaxAndMinValues() const
{
    // Establish some defaults to have as a fall-back.
    constexpr double defaultMaxHourlyPrecipitation = 30;
    constexpr double defaultMaxHourlyTemperature = 30;
    constexpr double defaultMinHourlyTemperature = -10;
    double max_precipitation_at_mountain = defaultMaxHourlyPrecipitation;
    double max_temperature_at_mountain = defaultMaxHourlyTemperature;
    double min_temperature_at_mountain = defaultMinHourlyTemperature;

    // Review the hourly data to attempt to find new min or max values.
    auto iterator = std::max_element(m_precipitation_hourly.begin(), m_precipitation_hourly.end());
    if (iterator != m_precipitation_hourly.end())
        max_precipitation_at_mountain = *iterator;
    iterator = std::max_element(m_temperature_hourly.begin(), m_temperature_hourly.end());
    if (iterator != m_temperature_hourly.end())
        max_temperature_at_mountain = *iterator;
    iterator = std::min_element(m_apparentTemperature_hourly.begin(), m_apparentTemperature_hourly.end());
    if (iterator != m_apparentTemperature_hourly.end())
        min_temperature_at_mountain = *iterator;

    // Check if min/max values for this mountain are more extreme than the min/max for all mountains
    if (max_precipitation_at_mountain > Mountain::maxPrecipitationMeasurement)
        Mountain::maxPrecipitationMeasurement = max_precipitation_at_mountain;
    if (max_temperature_at_mountain > Mountain::maxTemperatureMeasurement)
        Mountain::maxTemperatureMeasurement = max_temperature_at_mountain;
    if (min_temperature_at_mountain < Mountain::minTemperatureMeasurement)
        Mountain::minTemperatureMeasurement = min_temperature_at_mountain;
}

// ------------------------------------- //
//            Private Methods            //
// ------------------------------------- //

const QString Mountain::convertWindDirectionToOrientation(const int windDirection)
{
    if (windDirection >= 338 && windDirection <= 360)
        return "N";
    if (windDirection >= 0 && windDirection <= 22)
        return "N";
    if (windDirection >= 23 && windDirection <= 67)
        return "NE";
    if (windDirection >= 68 && windDirection <= 112)
        return "E";
    if (windDirection >= 113 && windDirection <= 157)
        return "SE";
    if (windDirection >= 158 && windDirection <= 202)
        return "S";
    if (windDirection >= 203 && windDirection <= 247)
        return "SW";
    if (windDirection >= 248 && windDirection <= 292)
        return "W";
    if (windDirection >= 293 && windDirection <= 337)
        return "NW";

    return "?";
}

void Mountain::initialiseDailyConditionsMap()
{
    m_dailyConditionsMap[0] = QString{"Clear"};
    m_dailyConditionsMap[1] = QString{"Mainly Clear"};
    m_dailyConditionsMap[2] = QString{"Partly cloudy"};
    m_dailyConditionsMap[3] = QString{"Overcast"};
    m_dailyConditionsMap[45] = QString{"Fog"};
    m_dailyConditionsMap[48] = QString{"Fog (with rime)"};
    m_dailyConditionsMap[51] = QString{"Drizzle (light)"};
    m_dailyConditionsMap[53] = QString{"Drizzle (moderate)"};
    m_dailyConditionsMap[55] = QString{"Drizzle (dense)"};
    m_dailyConditionsMap[56] = QString{"Drizzle (freezing)"};
    m_dailyConditionsMap[57] = QString{"Drizzle (freezing)"};
    m_dailyConditionsMap[61] = QString{"Rain (slight)"};
    m_dailyConditionsMap[63] = QString{"Rain (moderate)"};
    m_dailyConditionsMap[65] = QString{"Rain (heavy)"};
    m_dailyConditionsMap[66] = QString{"Rain (freezing)"};
    m_dailyConditionsMap[67] = QString{"Rain (freezing)"};
    m_dailyConditionsMap[71] = QString{"Snow (slight)"};
    m_dailyConditionsMap[73] = QString{"Snow (moderate)"};
    m_dailyConditionsMap[75] = QString{"Snow (heavy)"};
    m_dailyConditionsMap[77] = QString{"Snow"};
    m_dailyConditionsMap[80] = QString{"Rain Showers (slight)"};
    m_dailyConditionsMap[81] = QString{"Rain Showers (moderate)"};
    m_dailyConditionsMap[82] = QString{"Rain Showers (violent)"};
    m_dailyConditionsMap[85] = QString{"Snow Showers"};
    m_dailyConditionsMap[86] = QString{"Snow Showers"};
    m_dailyConditionsMap[95] = QString{"Thunderstorms"};
    m_dailyConditionsMap[96] = QString{"Thunderstorms"};
    m_dailyConditionsMap[99] = QString{"Thunderstorms"};
}
