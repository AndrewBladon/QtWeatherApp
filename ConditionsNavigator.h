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

#ifndef CONDITIONSNAVIGATOR_H
#define CONDITIONSNAVIGATOR_H

namespace Esri::ArcGISRuntime {
class GraphicsOverlay;
class IdentifyGraphicsOverlayResult;
class Map;
class MapQuickView;
class MultilayerPointSymbol;
class Symbol;
} // namespace Esri::ArcGISRuntime

class QMouseEvent;

#include <QObject>

#include "Mountain.h"

Q_MOC_INCLUDE("MapQuickView.h")

class ConditionsNavigator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Esri::ArcGISRuntime::MapQuickView* mapView READ mapView WRITE setMapView NOTIFY mapViewChanged)
    Q_PROPERTY(Mountain* selectedMountain READ selectedMountain NOTIFY selectedMountainChanged)

public:
    explicit ConditionsNavigator(QObject* parent = nullptr);
    ~ConditionsNavigator() override;

    Q_INVOKABLE void clearCurrentFilter() const;
    Q_INVOKABLE void filterOptionsChanged();

signals:
    void mapViewChanged();
    void selectedMountainChanged();

private:
    bool anyBadConditionForecastForDay(Mountain* mountain, int day) const;
    bool anyMarginalConditionForecastForDay(Mountain* mountain, int day) const;
    void applyFilter(const QList<int>& selectedDays) const;
    void assignLabelsToUIFilterOptions();
    bool conditionsDescriptionIsConcerning(const QString& condition) const;
    Esri::ArcGISRuntime::MultilayerPointSymbol* createCopyOfPointSymbol(Esri::ArcGISRuntime::MultilayerPointSymbol* const symbol);
    void createDifferentColouredVersionsOfPinSymbol(Esri::ArcGISRuntime::Symbol* const symbol);
    void displayMountainsOnMap();
    void getPinSymbolFromPortalThenInitialiseApp();
    Mountain* getSelectedMountain(const QString& name) const;
    QList<int> identifyWhichFilterOptionsAreChecked() const;
    void initialiseApp();
    Esri::ArcGISRuntime::MapQuickView* mapView() const;
    void retrieveForecastData() const;
    Mountain* selectedMountain() const;
    void selectMountain(Esri::ArcGISRuntime::IdentifyGraphicsOverlayResult* const rawIdentifyResult);
    void setInitialViewpoint();
    void setMapView(Esri::ArcGISRuntime::MapQuickView* const mapView);
    void setupInteractionBehaviour();
    void setupLabeling();
    void getReferencesToFilterOptionToggles();
    void waitUntilMapIsLoadedThenInitialiseApp();

    Esri::ArcGISRuntime::MultilayerPointSymbol* m_baseSymbol = nullptr;
    QList<QObject*> m_filterToggles;
    Esri::ArcGISRuntime::MultilayerPointSymbol* m_greenSymbol = nullptr;
    QList<Mountain*> m_mountains;
    Esri::ArcGISRuntime::GraphicsOverlay* m_mountainsOverlay = nullptr;
    Esri::ArcGISRuntime::Map* m_map = nullptr;
    Esri::ArcGISRuntime::MapQuickView* m_mapView = nullptr;
    Esri::ArcGISRuntime::MultilayerPointSymbol* m_orangeSymbol = nullptr;
    Esri::ArcGISRuntime::MultilayerPointSymbol* m_redSymbol = nullptr;
    Mountain* m_selectedMountain = nullptr;
};

#endif // CONDITIONSNAVIGATOR_H
