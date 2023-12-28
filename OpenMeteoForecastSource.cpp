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

#include "OpenMeteoForecastSource.h"
#include "Mountain.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>
#include <QVariantMap>

OpenMeteoForecastSource::OpenMeteoForecastSource(QObject* parent) :
    QObject{parent}
{
    m_requestUrl.setScheme("https");
    m_requestUrl.setHost("api.open-meteo.com");
    m_requestUrl.setPath("/v1/forecast");
}

void OpenMeteoForecastSource::MakeRequest(const double mountainLong, const double mountainLat, const double mountainElev, Mountain* mountain)
{
    // Weather data is accessed from https://open-meteo.com/
    // License information: https://open-meteo.com/en/license
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("latitude", QString::number(mountainLat));
    urlQuery.addQueryItem("longitude", QString::number(mountainLong));
    urlQuery.addQueryItem("elevation", QString::number(mountainElev));
    urlQuery.addQueryItem("timezone", "auto");
    urlQuery.addQueryItem("hourly", "temperature_2m,apparent_temperature,precipitation,visibility");
    urlQuery.addQueryItem("daily", "precipitation_sum,weathercode,windspeed_10m_max,windgusts_10m_max,winddirection_10m_dominant");
    m_requestUrl.setQuery(urlQuery);

    const QNetworkRequest networkRequest(m_requestUrl);
    QNetworkAccessManager* const networkManager = new QNetworkAccessManager(this);

    connect(networkManager, &QNetworkAccessManager::finished, this, [this, mountain](QNetworkReply* reply){
        if (reply->error() == QNetworkReply::NetworkError::NoError)
        {
            if (mountain == nullptr)
                return;
            const QByteArray jsonBytes = reply->readAll();
            reply->deleteLater();
            const QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonBytes);
            processResponse(jsonDocument, mountain);
        }
    });
    networkManager->get(networkRequest);
}

void OpenMeteoForecastSource::processResponse(const QJsonDocument& response, Mountain* mountain) const
{
    if (!response.isObject() || mountain == nullptr)
        return;

    const QJsonObject jsonObject = response.object();

    if (jsonObject.isEmpty())
        return;

    const QVariantMap responseVariantMap = jsonObject.toVariantMap();

    const QMap<QString, QVariant> hourlyData = responseVariantMap.value("hourly").toMap();
    assignHourlyDataToMountain(hourlyData, mountain);

    const QMap<QString, QVariant> dailyData = responseVariantMap.value("daily").toMap();
    assignDailyDataToMountain(dailyData, mountain);

    mountain->identifyMaxAndMinValues();
}

void OpenMeteoForecastSource::assignHourlyDataToMountain(const QMap<QString, QVariant>& hourlyData, Mountain* mountain) const
{
    const QList<QDateTime> time = convertQVariantListToTypedList<QDateTime>(hourlyData.value("time").toList());
    mountain->setHourlyDateTime(time);

    const QList<double> apparentTemperature = convertQVariantListToTypedList<double>(hourlyData.value("apparent_temperature").toList());
    mountain->setHourlyApparentTemperature(apparentTemperature);

    const QList<double> precipitation = convertQVariantListToTypedList<double>(hourlyData.value("precipitation").toList());
    mountain->setHourlyPrecipitation(precipitation);

    const QList<double> temperature2m = convertQVariantListToTypedList<double>(hourlyData.value("temperature_2m").toList());
    mountain->setHourlyTemperature(temperature2m);

    const QList<int> visibility = convertQVariantListToTypedList<int>(hourlyData.value("visibility").toList());
    mountain->setHourlyVisibility(visibility);
}

void OpenMeteoForecastSource::assignDailyDataToMountain(const QMap<QString, QVariant>& dailyData, Mountain* mountain) const
{
    const QList<QDate> date = convertQVariantListToTypedList<QDate>(dailyData.value("time").toList());
    mountain->setDates(date);

    const QList<int> weatherCode = convertQVariantListToTypedList<int>(dailyData.value("weathercode").toList());
    mountain->setDailyWeatherConditions(weatherCode);

    const QList<int> windDirection = convertQVariantListToTypedList<int>(dailyData.value("winddirection_10m_dominant").toList());
    mountain->setDailyWindDirection(windDirection);

    const QList<double> windGusts = convertQVariantListToTypedList<double>(dailyData.value("windgusts_10m_max").toList());
    mountain->setDailyWindGusts(windGusts);

    const QList<double> windSpeed = convertQVariantListToTypedList<double>(dailyData.value("windspeed_10m_max").toList());
    mountain->setDailyWindSpeed(windSpeed);

    const QList<double> precipitation = convertQVariantListToTypedList<double>(dailyData.value("precipitation_sum").toList());
    mountain->setDailyPrecipitation(precipitation);
}
