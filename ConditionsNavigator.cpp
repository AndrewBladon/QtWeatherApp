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

#include "ConditionsNavigator.h"

#include <QFuture>
#include <QQmlProperty>

#include "AttributeListModel.h"
#include "Envelope.h"
#include "GeometryEngine.h"
#include "Graphic.h"
#include "GraphicListModel.h"
#include "GraphicsOverlay.h"
#include "GraphicsOverlayListModel.h"
#include "IdentifyGraphicsOverlayResult.h"
#include "LabelDefinition.h"
#include "LabelDefinitionListModel.h"
#include "LinearUnit.h"
#include "Map.h"
#include "MapTypes.h"
#include "MapQuickView.h"
#include "MultilayerPointSymbol.h"
#include "Point.h"
#include "Polygon.h"
#include "Portal.h"
#include "SimpleLabelExpression.h"
#include "SpatialReference.h"
#include "SymbolLayer.h"
#include "SymbolLayerListModel.h"
#include "SymbolStyle.h"
#include "SymbolTypes.h"
#include "TextSymbol.h"
#include "Viewpoint.h"

#include "Mountain.h"
#include "MountainLocations.h"
#include "OpenMeteoForecastSource.h"

using namespace Esri::ArcGISRuntime;

namespace
{
    OpenMeteoForecastSource openMeteoForecast;
}

// ------------------------------------- //
//       Constructor & Destructor        //
// ------------------------------------- //

ConditionsNavigator::ConditionsNavigator(QObject* parent /* = nullptr */):
    QObject(parent),
    m_map(new Map(BasemapStyle::ArcGISTopographic, this))
{
    getPinSymbolFromPortalThenInitialiseApp();
}

ConditionsNavigator::~ConditionsNavigator() = default;

// ------------------------------------- //
//     Property Getters and Setters      //
// ------------------------------------- //

MapQuickView* ConditionsNavigator::mapView() const
{
    return m_mapView;
}

// Set the view (created in QML)
void ConditionsNavigator::setMapView(MapQuickView* const mapView)
{
    if (!mapView || mapView == m_mapView)
    {
        return;
    }

    m_mapView = mapView;
    m_mapView->setMap(m_map);

    getReferencesToFilterOptionToggles();
    assignLabelsToUIFilterOptions();

    emit mapViewChanged();
}

Mountain* ConditionsNavigator::selectedMountain() const
{
    return m_selectedMountain;
}

// ------------------------------------- //
//            Public Methods             //
// ------------------------------------- //

void ConditionsNavigator::clearCurrentFilter() const
{
    // Loop through each QML toggle element and set checked property to false.
    for (QObject* toggle : m_filterToggles)
        toggle->setProperty("checked", false);

    // Loop through each mountain and reset symbol to dull red colour.
    for (Mountain* mountain : m_mountains)
        mountain->mountainGraphic->setSymbol(m_baseSymbol);
}

void ConditionsNavigator::filterOptionsChanged()
{
    const QList<int> indicesOfSelectedDays = identifyWhichFilterOptionsAreChecked();

    if (indicesOfSelectedDays.isEmpty())
        clearCurrentFilter();
    else
        applyFilter(indicesOfSelectedDays);
}

// ------------------------------------- //
//            Private Methods            //
// ------------------------------------- //

QList<int> ConditionsNavigator::identifyWhichFilterOptionsAreChecked() const
{
    const int daysInAWeek = 7;
    QList<int> indicesOfSelectedToggleOptions;
    for (int index = 0; index < daysInAWeek; ++index)
    {
        const QObject* toggle = m_filterToggles.at(index);
        const bool toggleChecked = QQmlProperty::read(toggle, "checked").toBool();
        if (toggleChecked)
            indicesOfSelectedToggleOptions.append(index);
    }
    return indicesOfSelectedToggleOptions;
}

void ConditionsNavigator::getPinSymbolFromPortalThenInitialiseApp()
{
  SymbolStyle* style = new SymbolStyle("Esri2DPointSymbolsStyle", new Portal(this), this);
      style->fetchSymbolAsync({"esri-pin-2"}).then(this, [this](Symbol* symbol) {
        createDifferentColouredVersionsOfPinSymbol(symbol);
          if (m_map->loadStatus() == LoadStatus::Loaded)
              initialiseApp();
          else
              waitUntilMapIsLoadedThenInitialiseApp();
          });
}

void ConditionsNavigator::waitUntilMapIsLoadedThenInitialiseApp()
{
    connect(m_map, &Map::loadStatusChanged, this, [this](){
        if (m_map->loadStatus() == LoadStatus::Loaded)
            initialiseApp();
    });
}

void ConditionsNavigator::createDifferentColouredVersionsOfPinSymbol(Symbol* const symbol)
{
    m_baseSymbol = dynamic_cast<MultilayerPointSymbol*>(symbol);
    m_baseSymbol->setColor(QColor::fromString("indianred"));

    m_greenSymbol = createCopyOfPointSymbol(m_baseSymbol);
    m_greenSymbol->setColor(QColor::fromString("green"));

    m_orangeSymbol = createCopyOfPointSymbol(m_baseSymbol);
    m_orangeSymbol->setColor(QColor::fromString("orange"));

    m_redSymbol = createCopyOfPointSymbol(m_baseSymbol);
    m_redSymbol->setColor(QColor::fromString("red"));
}

MultilayerPointSymbol* ConditionsNavigator::createCopyOfPointSymbol(MultilayerPointSymbol* const symbol)
{
    // MultilayerPointSymbol does not have copy constructor, so a new MultilayerPointSymbol object
    // needs to be created from a list containing each symbol layer.
    const SymbolLayerListModel* listModel = symbol->symbolLayers();
    QList<SymbolLayer*> listOfSymbolLayers;
    for (int counter = 0; counter < listModel->size(); ++counter)
        listOfSymbolLayers.append(listModel->at(counter)->clone());
    return new MultilayerPointSymbol(listOfSymbolLayers, this);
}

void ConditionsNavigator::initialiseApp()
{
    MountainLocations mountainLocations{this};
    m_mountains = mountainLocations.getLocations();
    displayMountainsOnMap();
    setInitialViewpoint();
    retrieveForecastData();
    setupInteractionBehaviour();
}

void ConditionsNavigator::displayMountainsOnMap()
{
    if (m_mountainsOverlay == nullptr)
    {
        m_mountainsOverlay = new GraphicsOverlay(this);
        setupLabeling();
    }

    for (Mountain* mountain : m_mountains)
    {
        const double mountainsLongitude = mountain->getLongitude();
        const double mountainsLatitude = mountain->getLatitude();
        const QString mountainName = mountain->getName();

        const Point mountainPoint(mountainsLongitude, mountainsLatitude, SpatialReference::wgs84());
        Graphic* pointGraphic = new Graphic(mountainPoint, m_baseSymbol, this);
        pointGraphic->attributes()->insertAttribute("Name", mountainName);
        mountain->mountainGraphic = pointGraphic;
        m_mountainsOverlay->graphics()->append(pointGraphic);
    }

    m_mapView->graphicsOverlays()->append(m_mountainsOverlay);
}

void ConditionsNavigator::setupLabeling()
{
    SimpleLabelExpression* labelExpression = new SimpleLabelExpression("[Name]", this);

    TextSymbol* textSymbol = new TextSymbol(this);
    textSymbol->setFontWeight(FontWeight::Bold);
    textSymbol->setSize(11);
    textSymbol->setColor(Qt::black);

    LabelDefinition* labelDefinition = new LabelDefinition(labelExpression, textSymbol, this);
    m_mountainsOverlay->labelDefinitions()->append(labelDefinition);
    m_mountainsOverlay->setLabelsEnabled(true);
}

void ConditionsNavigator::setInitialViewpoint()
{
    const Envelope extent = m_mountainsOverlay->extent();
    const double bufferSize = extent.height() * 0.1;
    const double maxDeviationBetweenPoints = 100;
    const Polygon expandedExtent = GeometryEngine::bufferGeodetic(
                extent,
                bufferSize,
                LinearUnit::meters(),
                maxDeviationBetweenPoints,
                GeodeticCurveType::Geodesic);
    const Viewpoint viewpoint(expandedExtent);
    m_mapView->setViewpointAsync(viewpoint);
}

void ConditionsNavigator::retrieveForecastData() const
{
    for (Mountain* mountain : m_mountains)
        openMeteoForecast.MakeRequest(mountain->getLongitude(), mountain->getLatitude(), mountain->getElevation(), mountain);
}

void ConditionsNavigator::setupInteractionBehaviour()
{
    connect(m_mapView, &MapQuickView::mouseClicked, this, [this](QMouseEvent& mouseEvent)
    {
        const double identifyTolerance = 15;
        const bool returnPopups = false;

        m_mapView->identifyGraphicsOverlayAsync(m_mountainsOverlay,
                                                mouseEvent.position(),
                                                identifyTolerance,
                                                returnPopups).then([this](IdentifyGraphicsOverlayResult* rawIdentifyResult){
          selectMountain(rawIdentifyResult);
        });
    });

    connect(m_mapView, &MapQuickView::touched, this, [this](QTouchEvent& touchEvent)
    {
        const QEventPoint touchPoint = touchEvent.point(0);

        const double identifyTolerance = 15;
        const bool returnPopups = false;

        m_mapView->identifyGraphicsOverlayAsync(m_mountainsOverlay,
                                                touchEvent.points().first().position(),
                                                identifyTolerance,
                                                returnPopups).then([this](IdentifyGraphicsOverlayResult* rawIdentifyResult){
          selectMountain(rawIdentifyResult);
        });
    });
}

void ConditionsNavigator::selectMountain(IdentifyGraphicsOverlayResult* const rawIdentifyResult)
{
  // Delete rawIdentifyResult on leaving scope.
  const auto identifyResult = std::unique_ptr<IdentifyGraphicsOverlayResult>(rawIdentifyResult);

  const bool identifyResultNullOrEmpty = !identifyResult || identifyResult.get()->graphics().isEmpty();

  if (identifyResultNullOrEmpty)
  {
      m_selectedMountain = nullptr;
  }
  else
  {
      const QString nameOfSelectedMountain = identifyResult.get()->graphics().first()->attributes()->attributeValue("Name").toString();
      m_selectedMountain = getSelectedMountain(nameOfSelectedMountain);
  }

  emit selectedMountainChanged();
}

void ConditionsNavigator::getReferencesToFilterOptionToggles()
{
    m_filterToggles.clear();
    const int numberOfFilterOptions = 7;

    const QString templateForToggleNames("day%1FilterOption");
    for (int i = 1; i <= numberOfFilterOptions; i++)
    {
        const QString filterOptionName = templateForToggleNames.arg(QString::number(i));
        if (QObject* filterOption = m_mapView->findChild<QObject*>(filterOptionName); filterOption)
            m_filterToggles.append(filterOption);
    }
}

void ConditionsNavigator::assignLabelsToUIFilterOptions()
{
    const QDate currentDate = QDate::currentDate();

    for (int counter = 0; counter < 7; ++counter)
        m_filterToggles.at(counter)->setProperty("text", currentDate.addDays(counter).toString("ddd"));
}

Mountain* ConditionsNavigator::getSelectedMountain(const QString& name) const
{
    for (Mountain* mountain : m_mountains)
    {
        if (mountain->getName() == name)
             return mountain;
    }
    return nullptr;
}

void ConditionsNavigator::applyFilter(const QList<int>& selectedDays) const
{
    for (Mountain* mountain : m_mountains)
    {
      bool MoveToNextMountain = false;
      mountain->mountainGraphic->setSymbol(m_greenSymbol);

        // Check if forecast for this mountain meets criteria for bad conditions on any of the selected days.
        for (int day : selectedDays)
        {
            if (anyBadConditionForecastForDay(mountain, day))
            {
                mountain->mountainGraphic->setSymbol(m_redSymbol);
                MoveToNextMountain = true;
            }

            if (MoveToNextMountain)
              break;
        }

        if (MoveToNextMountain)
            continue;

        // Check if forecast for this mountain meets criteria for marginal conditions on any of the selected days.
        for (int day : selectedDays)
        {
            if (anyMarginalConditionForecastForDay(mountain, day))
            {
                mountain->mountainGraphic->setSymbol(m_orangeSymbol);
                MoveToNextMountain = true;
            }

            if (MoveToNextMountain)
              break;
        }
    }
}

bool ConditionsNavigator::anyBadConditionForecastForDay(Mountain* mountain, int day) const
{
    const QString condition = mountain->getDailyWeatherConditions().at(day);
    const double windspeed = mountain->getDailyWindSpeed().at(day);
    const double precipitation = mountain->getDailyPrecipitation().at(day);

    if (windspeed >= 40 || precipitation >= 5 || condition == "Thunderstorms")
        return true;

    return false;
}

bool ConditionsNavigator::anyMarginalConditionForecastForDay(Mountain* mountain, int day) const
{
    const QString condition = mountain->getDailyWeatherConditions().at(day);
    const double windspeed = mountain->getDailyWindSpeed().at(day);
    const double precipitation = mountain->getDailyPrecipitation().at(day);

    if (windspeed >= 20 || precipitation >= 1 || conditionsDescriptionIsConcerning(condition))
        return true;

    return false;
}

bool ConditionsNavigator::conditionsDescriptionIsConcerning(const QString& condition) const
{
    if (condition == "Clear" || condition == "Mainly Clear" ||
        condition == "Partly cloudy" || condition == "Overcast" ||
        condition == "Unknown")
        return false;
    else
        return true;
}
