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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCharts
import Esri.ConditionsNavigator
import Source.Mountain 1.0
import Qt.labs.qmlmodels 1.0

Item {

    property Mountain mountain: null;

    // Create MapQuickView here, and create its Map etc. in C++ code
    MapView {
        id: view
        anchors.fill: parent
        // set focus to enable keyboard navigation
        focus: true

        Button {
            id: filterButton
            anchors {
                top: parent.top
                left: parent.left
                margins: 5
            }
            checkable: true
            onPressed: {
                if (!checked) {
                    // Reset state of infoButton when filter menu is closed.
                    infoButton.checked = false;
                }
            }
            icon.source: "/Resources/icon-filter-10.jpg"
        }

        // Filter options UI - scrollable if the UI is too tall for the screen.
        ScrollView {
            property int padding_value: 3

            id: filterOptions
            height: {
                let distanceBetweenUIElements = 5;

                let desiredHeight = contentHeight + (2 * padding_value) + filterButton.height;
                let availableHeight = parent.height - parent.attributionRect.height;

                if (desiredHeight > availableHeight) {
                    return availableHeight - (4 * distanceBetweenUIElements) - filterButton.height;
                }
                else {
                    return contentHeight + (2 * padding_value);
                }
            }
            width: contentWidth + (2 * padding_value);
            visible: filterButton.checked
            anchors {
                top: filterButton.bottom
                left: parent.left
                margins: 5
            }
            background: Rectangle {}
            padding: padding_value

            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AsNeeded
            GroupBox {
                anchors.fill: parent
                ColumnLayout {
                    CheckBox {
                        objectName: "day1FilterOption"
                        onToggled: model.filterOptionsChanged();
                    }
                    CheckBox {
                        objectName: "day2FilterOption"
                        onToggled: model.filterOptionsChanged();
                    }
                    CheckBox {
                        objectName: "day3FilterOption"
                        onToggled: model.filterOptionsChanged();
                    }
                    CheckBox {
                        objectName: "day4FilterOption"
                        onToggled: model.filterOptionsChanged();
                    }
                    CheckBox {
                        objectName: "day5FilterOption"
                        onToggled: model.filterOptionsChanged();
                    }
                    CheckBox {
                        objectName: "day6FilterOption"
                        onToggled: model.filterOptionsChanged();
                    }
                    CheckBox {
                        objectName: "day7FilterOption"
                        onToggled: model.filterOptionsChanged();
                    }

                    Button {
                        text: "Clear"
                        onPressed: model.clearCurrentFilter();
                    }

                    Button {
                        id: infoButton
                        text: "Info"
                        checkable: true
                    }
                }
            }
        }

        Label {
            property int margin_value: 5

            id: filterCriteriaInfoBox
            anchors {
                left: filterOptions.right
                top: filterOptions.top
                margins: margin_value
            }
            width: {
                const availableWidth = parent.width - filterOptions.width - (3 * margin_value);
                if (availableWidth > 200)
                    return 200;
                else {
                    return availableWidth;
                }
            }
            leftInset: -1 * margin_value
            rightInset: -1 * margin_value
            visible: infoButton.checked && filterOptions.visible
            textFormat: Text.RichText
            background: Rectangle {
                color: "white"
                border.color: "black"
                border.width: 1
            }
            wrapMode: Text.Wrap
            text: " <html>
                        <p><b><u>Filter Criteria:</u></b></p>
                        <p><b><i>Bad Conditions:</i></b> Precipitation &gt; 5mm, Windspeed &gt; 40km/h, or
                                &quot;Thunderstorms&quot;.</p>
                        <p><b><i>Marginal Conditions:</i></b> Precipitation &gt; 1mm, Windspeed &gt; 20km/h,
                                or any conditions not mentioned in Bad or Good conditions (e.g. &quot;Fog&quot;,
                                &quot;Drizzle&quot;, and &quot;Rain&quot;).</p>
                        <p><b><i>Good Conditions:</i></b> Precipitation &lt; 1mm, Windspeed &lt; 20km/h, and
                                &quot;clear&quot;, &quot;Mainly Clear&quot;, &quot;Partly cloudy&quot;,
                                &quot;Overcast&quot;, or &quot;Unknown&quot;.</p>
                    </html>"
        }
    }

    // Declare the C++ instance which creates the map etc. and supply the view
    ConditionsNavigator {
        id: model
        mapView: view

        onSelectedMountainChanged: {
            if (selectedMountain) {
                mountain = selectedMountain;
                createCharts();
                createTable();
                windowContainingForecastData.visible = true;
            }
            else {
                mountain = null;
                windowContainingForecastData.visible = false;
            }
        }
    }

    Drawer {
        id: windowContainingForecastData
        width: view.width
        height: view.height
        interactive: false

        ScrollView {
            id: scrollView
            width: parent.width
            height: parent.height
            clip : true
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            ColumnLayout {
                id: column
                width: scrollView.availableWidth
                spacing: 2

                RowLayout {
                    id: layoutForMountainName
                    Layout.minimumHeight: 50
                    Label {
                        property int margin_value: 5

                        id: mountainName
                        text: mountain ? mountain.getName() : "";
                        Layout.preferredWidth: scrollView.availableWidth - closeButton.width - (2 * margin_value)
                        horizontalAlignment: Text.AlignHCenter
                        font.bold: true
                        font.underline: true
                        font.pointSize: 25
                        wrapMode: Text.Wrap
                    }

                    Button {
                        id: closeButton
                        text: "X"
                        onPressed: windowContainingForecastData.close();
                        Layout.preferredWidth: 50
                        Layout.preferredHeight: 50
                        Layout.alignment: Qt.AlignRight
                    }
                }

                Label {
                    id: mountainLatLongAndElevation
                    text: {
                        if (mountain) {
                            "[" + mountain.getLatitude() + ", " + mountain.getLongitude() + "] (" + mountain.getElevation() + "m)";
                        }
                        else
                            "";
                    }
                    Layout.preferredWidth: mountainName.width
                    horizontalAlignment: Text.AlignHCenter
                    font.italic: true
                    font.pointSize: 10
                }

                Item {
                    implicitWidth: scrollView.availableWidth
                    implicitHeight: forecastTableView.height

                    HorizontalHeaderView {
                        id: tableHeader
                        syncView: forecastTableView
                        implicitWidth: scrollView.availableWidth
                        implicitHeight: 30
                        columnSpacing: 1
                        clip: true
                        interactive: false
                        columnWidthProvider: function (column) { return columnWidths[column] }

                        model: ["Day", "Condition", "Precipitation (mm)", "Wind Speed (km/h)", "Wind Dir. (from)"]
                        delegate: Rectangle {
                            implicitHeight: 50
                            border.width: 1.2
                            border.color: "black"
                            color: "light grey"
                            Text {
                                anchors.fill: parent
                                text: modelData
                                wrapMode: Text.Wrap
                                horizontalAlignment: Text.AlignHCenter
                                font.bold: true
                                font.italic: true
                                font.pointSize: 10
                            }
                        }
                    }

                    TableView {
                        id: forecastTableView
                        implicitWidth: scrollView.availableWidth
                        implicitHeight: 270
                        topMargin: tableHeader.implicitHeight + 1
                        columnSpacing: 1
                        rowSpacing: 1
                        clip: true
                        interactive: false
                        ScrollBar.horizontal: ScrollBar { }

                        property var columnWidths: [40, 140, 75, 75, 60]
                        columnWidthProvider: function (column) { return columnWidths[column] }

                        model: TableModel {
                            id: forecastTableModel

                            TableModelColumn {display: "Day"}
                            TableModelColumn {display: "Condition"}
                            TableModelColumn {display: "Precipitation"}
                            TableModelColumn {display: "Wind"}
                            TableModelColumn {display: "WindDirection"}
                        }

                        delegate: Rectangle {
                            implicitHeight: 30
                            border.width: 1
                            border.color: "black"

                            Text {
                                text: display
                                anchors.centerIn: parent
                                wrapMode: Text.WordWrap
                            }
                        }
                    }
                }

                ChartView {
                    id: precipitationChart
                    implicitWidth: scrollView.availableWidth
                    implicitHeight: 250
                }

                ChartView {
                    id: temperatureChart
                    implicitWidth: scrollView.availableWidth
                    implicitHeight: 250
                }

                ChartView {
                    id: visibilityChart
                    implicitWidth: scrollView.availableWidth
                    implicitHeight: 250
                }
            }
        }
    }

    DateTimeAxis {
        id: dateTimeAxisForPrecipitationPlot
        format: "ddd"
        tickCount: 8
        truncateLabels: false
    }

    DateTimeAxis {
        id: dateTimeAxisForTempPlot
        format: "ddd"
        tickCount: 8
        truncateLabels: false
    }

    DateTimeAxis {
        id: dateTimeAxisForVisPlot
        format: "ddd"
        tickCount: 8
        truncateLabels: false
    }

    ValuesAxis {
        id: precipitationAxis
        min: 0
    }

    ValuesAxis {
        id: temperatureAxis
        tickCount: 6
    }

    ValuesAxis {
        id: visibilityAxis
        min: 0
        max: 25
        tickCount: 6
    }

    function createCharts() {
        const dates = model.selectedMountain.getHourlyDateTime();
        const numberOfData = dates.length;

        createPrecipitationChart(dates, numberOfData);
        createTemperatureChart(dates, numberOfData);
        createVisibilityChart(dates, numberOfData);
    }

    function createPrecipitationChart(dates, numberOfData) {
        const precipitation = model.selectedMountain.getHourlyPrecipitation();

        precipitationChart.removeAllSeries();
        const precipitationSeries = precipitationChart.createSeries(ChartView.SeriesTypeLine, "Precipitation (mm)", dateTimeAxisForPrecipitationPlot, precipitationAxis)

        dateTimeAxisForPrecipitationPlot.min = dates[0];
        dateTimeAxisForPrecipitationPlot.max = dates[numberOfData - 1];

        precipitationAxis.max = Math.min(mountain.getMaxPrecipitationMeasurement(), 25);

        for (let counter = 0; counter < numberOfData; counter++) {
            precipitationSeries.append(dates[counter], precipitation[counter]);
        }
    }

    function createTemperatureChart(dates, numberOfData) {
        const temperature = model.selectedMountain.getHourlyTemperature();
        const apparentTemperature = model.selectedMountain.getHourlyApparentTemperature();

        temperatureChart.removeAllSeries();

        dateTimeAxisForTempPlot.min = dates[0];
        dateTimeAxisForTempPlot.max = dates[numberOfData - 1];

        temperatureAxis.max = mountain.getMaxTemperatureMeasurement();
        temperatureAxis.min = mountain.getMinTemperatureMeasurement();

        const temperatureSeries = temperatureChart.createSeries(ChartView.SeriesTypeLine, "Temperature (°C)", dateTimeAxisForTempPlot, temperatureAxis);
        const apparentTemperatureSeries = temperatureChart.createSeries(ChartView.SeriesTypeLine, "Apparent Temperature (°C)", dateTimeAxisForTempPlot, temperatureAxis);

        for (let counter = 0; counter < numberOfData; counter++) {
            temperatureSeries.append(dates[counter], temperature[counter]);
            apparentTemperatureSeries.append(dates[counter], apparentTemperature[counter]);
        }
    }

    function createVisibilityChart(dates, numberOfData) {
        const visibility = model.selectedMountain.getHourlyVisibility();

        visibilityChart.removeAllSeries();

        dateTimeAxisForVisPlot.min = dates[0];
        dateTimeAxisForVisPlot.max = dates[numberOfData - 1];

        const visibilitySeries = visibilityChart.createSeries(ChartView.SeriesTypeLine, "Visibility (Km)", dateTimeAxisForVisPlot, visibilityAxis)

        for (let counter = 0; counter < numberOfData; counter++) {
            const visibilityInKm = visibility[counter] / 1000;
            visibilitySeries.append(dates[counter], visibilityInKm);
        }
    }

    function createTable() {
        forecastTableModel.clear()

        const days = model.selectedMountain.getDays();
        const conditions = model.selectedMountain.getDailyWeatherConditions();
        const precipitation = model.selectedMountain.getDailyPrecipitation();
        const windspeed = model.selectedMountain.getDailyWindSpeed();
        const windgusts = model.selectedMountain.getDailyWindGusts();
        const winddirection = model.selectedMountain.getDailyWindDirection();

        const numberOfData = days.length;

        for (let counter = 0; counter < numberOfData; ++counter)
        {
            const _day = days[counter];
            const _condition = conditions[counter];
            const _precipitation = precipitation[counter];
            const _windspeed = windspeed[counter];
            const _windgust = windgusts[counter];
            const _winddirection = winddirection[counter];


            forecastTableModel.appendRow({Day: _day,
                                  Condition: _condition,
                                  Precipitation: _precipitation,
                                  Wind: _windspeed + " (" + _windgust + ")",
                                  WindDirection: _winddirection})
        }
    }
}
