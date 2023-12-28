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

#ifndef OPENMETEOFORECASTSOURCE_H
#define OPENMETEOFORECASTSOURCE_H

#include <QUrl>
#include <QVariant>

class Mountain;

class OpenMeteoForecastSource : public QObject
{
    Q_OBJECT
public:
    explicit OpenMeteoForecastSource(QObject* parent = nullptr);

    void MakeRequest(const double mountainLong, const double mountainLat, const double mountainElev, Mountain* mountain);

private:
    QUrl m_requestUrl;

    void processResponse(const QJsonDocument& response, Mountain* mountain) const;
    void assignHourlyDataToMountain(const QMap<QString, QVariant>& hourlyData, Mountain* mountain) const;
    void assignDailyDataToMountain(const QMap<QString, QVariant>& dailyData, Mountain* mountain) const;

    template<typename T>
    QList<T> convertQVariantListToTypedList(QList<QVariant> qVariantList) const
    {
        QList<T> convertedList;
        convertedList.reserve(qVariantList.size());
        for (QVariant element : qVariantList)
        {
            T value = element.value<T>();
            convertedList.emplaceBack(value);
        }
        return convertedList;
    }
};

#endif // OPENMETEOFORECASTSOURCE_H
