# Qt Weather App

This repository contains the source code for an application that retrieves weather forecast data for the 282 mountains in Scotland that are higher than 914.4m (3000ft), called the Munros. The location of each mountain is represented using a pin on a topographic basemap. Once forecast data are retrieved, the forecast for each mountain can be easily reviewed by clicking on the mountain on the map. Additionally, if a user selects a day, or multiple days, the application reviews the forecast data and categorises the mountains according the suitability of the forecast for doing outdoor persuits. Currently, the criteria for the categories are defined using my personal preferences for rock climbing in the mountains. If re-using the application, the criteria should be reviewed and adjusted according to your desired activity and personal preferences.

The application is written using the Qt framework.

![Qt Weather App Demo](AppDemo.gif)

I have also written a [blog](https://www.esri.com/arcgis-blog/products/sdk-qt/developers/creating-app-to-identify-climbing-locations-using-weather-api/) about the application and how I used it on a climbing holiday to the Dolomites in Northern Italy.

## Prerequisites

- Qt 6 (**note:** the app was developed using `Qt 6.5.2`).
- [ArcGIS Maps SDK for Qt](https://developers.arcgis.com/qt/), version `200.3` or newer.
- An Esri Developer API key (see [Create and manage an API key](https://developers.arcgis.com/documentation/mapping-apis-and-services/security/tutorials/create-and-manage-an-api-key/)).

## Instructions for running the application

1. Clone the source code.
2. Open Qt Creator.
3. Go to `File` - `Open File or Project`.
4. Navigate to the `CMakeLists.txt` file and press `Open`.
5. In the configuration page, define the build kits you want to use (e.g. `Desktop Qt 6.5.2 MSVC2019 64-bit`) and press `Configure Project`.

    - **Note:** to deploy the application to an android device, you will need to use an Android build kit. Information about using the Qt for Android toolchain can be found on Qt's [Getting Started with Qt for Android](https://doc.qt.io/qt-6/android-getting-started.html) page.

6. Once configuration has ended, open `main.cpp`.
7. Add your Esri API key to the `const QString apiKey = QString("");` statement on line 42.
8. If building using an Android build kit:

    - Open `CmakeLists.txt`.
    - Specify the correct path to the `openssl` `CMakeLists.txt` file.

9. Press `Build`.
10. If the application builds successfully, press `Run`.

## Issues

Find a bug or want to request a new feature? Please let us know by submitting an issue.

## Contributing

This repositry is currently closed to contributions.

## Licensing

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

[http://www.apache.org/licenses/LICENSE-2.0](http://www.apache.org/licenses/LICENSE-2.0)

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

A copy of the license is available in the repository's license.txt file.

## Acknowledgements

Thanks to the reviewers of the code in the initial commit, who found some significant issues and helped improve the logic significantly:

- James Ballard (Esri).
- Luke Smallwood (Esri).

The concept for the application (i.e. colouring pins according to the suitability of a location for climbing) was inspired by [winterclimbingforecasts.co.uk](https://www.winterclimbingforecasts.co.uk/).
