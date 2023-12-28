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

#ifndef MOUNTAIN_H
#define MOUNTAIN_H

#include "QString"
#include "QList"
#include "QDateTime"

namespace Esri::ArcGISRuntime
{
  class Graphic;
}

class Mountain : public QObject
{
  Q_OBJECT

public:
    explicit Mountain(QString name, double latitude, double longitude, double elevation, QObject* parent = nullptr);

    Esri::ArcGISRuntime::Graphic* mountainGraphic = nullptr;
    static double maxPrecipitationMeasurement;
    static double maxTemperatureMeasurement;
    static double minTemperatureMeasurement;

    Q_INVOKABLE const QList<double> getDailyPrecipitation() const;
    Q_INVOKABLE const QList<QString> getDailyWeatherConditions() const;
    Q_INVOKABLE const QList<QString> getDailyWindDirection() const;
    Q_INVOKABLE const QList<double> getDailyWindGusts() const;
    Q_INVOKABLE const QList<double> getDailyWindSpeed() const;
    Q_INVOKABLE const QList<QDate> getDates() const;
    Q_INVOKABLE const QList<QString> getDays() const;
    Q_INVOKABLE const double getElevation() const;
    Q_INVOKABLE const QList<double> getHourlyApparentTemperature() const;
    Q_INVOKABLE const QList<QDateTime> getHourlyDateTime() const;
    Q_INVOKABLE const QList<double> getHourlyPrecipitation() const;
    Q_INVOKABLE const QList<double> getHourlyTemperature() const;
    Q_INVOKABLE const QList<int> getHourlyVisibility() const;
    Q_INVOKABLE const double getLatitude() const;
    Q_INVOKABLE const double getLongitude() const;
    Q_INVOKABLE const double getMaxPrecipitationMeasurement() const;
    Q_INVOKABLE const double getMaxTemperatureMeasurement() const;
    Q_INVOKABLE const double getMinTemperatureMeasurement() const;
    Q_INVOKABLE const QString getName() const;

    void setDailyPrecipitation(const QList<double>& newData);
    void setDailyWeatherConditions(const QList<int>& newData);
    void setDailyWindDirection(const QList<int>& newData);
    void setDailyWindGusts(const QList<double>& newData);
    void setDailyWindSpeed(const QList<double>& newData);
    void setDates(const QList<QDate>& dates);
    void setHourlyApparentTemperature(const QList<double>& newData);
    void setHourlyDateTime(const QList<QDateTime>& newData);
    void setHourlyPrecipitation(const QList<double>& newData);
    void setHourlyTemperature(const QList<double>& newData);
    void setHourlyVisibility(const QList<int>& newData);

    void identifyMaxAndMinValues() const;

private:
    QList<double> m_apparentTemperature_hourly;
    QList<QDate> m_dates;
    QList<QDateTime> m_dateTime_hourly;
    QMap<int, QString> m_dailyConditionsMap;
    const double m_elevation;
    const double m_latitude;
    const double m_longitude;
    const QString m_name;
    QMap<QDate,double> m_precipitation_daily;
    QList<double> m_precipitation_hourly;
    QList<double> m_temperature_hourly;
    QList<int> m_visibility_hourly;
    QMap<QDate,QString> m_weatherconditions_daily;
    QMap<QDate,QString> m_winddirection_daily;
    QList<int> m_winddirection_hourly;
    QMap<QDate,double> m_windgusts_daily;
    QList<double> m_windgusts_hourly;
    QMap<QDate,double> m_windspeed_daily;
    QList<double> m_windspeed_hourly;

    const QString convertWindDirectionToOrientation(const int windDirection);
    void initialiseDailyConditionsMap();
};

#endif // MOUNTAIN_H
