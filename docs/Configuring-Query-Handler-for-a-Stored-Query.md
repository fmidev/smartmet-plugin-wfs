# A.K.A. constructor_name

Value                                                                                                           | Description
----------------------------------------------------------------------------------------------------------------|------------------
[wfs_obs_handler_factory](#wfs_obs_handler_factory)                                                             | Observation data (no lightning, aviation or multi-sensor)
[wfs_flash_handler_factory](#wfs_flash_handler_factory)                                                         | Lightning observation data
[wfs_stored_aviation_observation_handler_factory](#wfs_stored_aviation_observation_handler_factory)             | Aviation observation data
[wfs_stored_mast_handler_factory](#wfs_stored_mast_handler_factory)                                             | Multi-sensor observation data
[wfs_stored_sounding_handler_factory](#wfs_stored_sounding_handler_factory)                                     | Sounding observation data
[wfs_stored_air_nuclide_handler_factory](#wfs_stored_air_nuclide_handler_factory)                               | Air radionuclide activity concentration data
[wfs_stored_env_monitoring_network_handler_factory](#wfs_stored_env_monitoring_network_handler_factory)         | Environmental monitoring networks (INSPIRE EF)
[wfs_stored_env_monitoring_facility_handler_factory](#wfs_stored_env_monitoring_facility_handler_factory)       | Environmental monitoring facilities (INSPIRE EF)
[wfs_forecast_handler_factory](#wfs_forecast_handler_factory)                                                   | Forecast data
[wfs_grid_forecast_handler_factory](#wfs_grid_forecast_handler_factory)                                         | Forecast data from the grid engine
[wfs_stored_qe_download_handler_factory](#wfs_stored_qe_download_handler_factory)                               | Forecast data downloading (the data source is QEngine)
[wfs_stored_file_handler_factory](#wfs_stored_file_handler_factory)                                             | Forecast data downloading (the data source is query files)
[wfs_stored_grid_handler_factory](#wfs_stored_grid_handler_factory)                                             | Forecast data in grid format
[wfs_winterweather_probabilities_query_handler_factory](#wfs_winterweather_probabilities_query_handler_factory) | Forecast data for different winter weather conditions
[wfs_isoline_query_handler_factory](#wfs_isoline_query_handler_factory)                                         | Forecast parameter values of the given geographical area returned as GML isolines
[wfs_coverage_query_handler_factory](#wfs_coverage_query_handler_factory)                                       | Forecast parameter values of the given geographical area returned as GML isobars
[wfs_winterweather_coverage_query_handler_factory](#wfs_winterweather_coverage_query_handler_factory)           | Forecast for winter weather conditions of the given geographical area returned as GML isobars
[wfs_stored_geoserver_handler_factory](#wfs_stored_geoserver_handler_factory)                                   | Geographical data downloading
[wfs_get_data_set_by_id_handler_factory](#wfs_get_data_set_by_id_handler_factory)                               | Predefined data sets
[wfs_get_feature_by_id_handler_factory](#wfs_get_feature_by_id_handler_factory)                                 | The mandatory implementation of the "GetFeatureById" stored query defined in the WFS 2.0 standard
wfs_stored_atom_handler_base_factory                                                                            | Download links generated from the `url_template` setting only (for testing)
wfs_dummy_handler_factory                                                                                       | Simple stored query handler for testing purposes only

The value of `constructor_name` is the name of the factory object, which the plugin looks up with `dlsym()`.
The handler parameters actually supported by each factory can be listed with the server administration request `/admin?what=wfs:constructors`.

## wfs_obs_handler_factory

This query handler is responsible for queries that are used to fetch the meteorological observation data from SmartMet Server's ObsEngine module.

The observation data is _usually_ fetched over the given time interval from the given geographical area or from the given observation stations.

The following optional attributes can be defined on the top level of the query configuration file.

Attribute name      | Type    | Description
--------------------|---------|--------------
maxHours            | double  | Limit total length of requested time interval to this many hours. The default value is `168` (7 days). This parameter is only valid, if the `storedQueryRestrictions` parameter is set to `true` in the WFS Plugin configuration file (see [Configuring the WFS Pugin](Configuring-the-WFS-Plugin))
maxStationCount     | integer | Limit the number of requested observation stations. The default value "0" means unlimited. This parameter is only valid, if the `storedQueryRestrictions` parameter is set to `true` in the WFS Plugin configuration file (see [Configuring the WFS Pugin](Configuring-the-WFS-Plugin)). Also this parameter is only valid, if the request contains `bbox` (bounding box) setting. _A `ProcessingError` exception is returned if the actual count exceeds the given limit_
separateGroups      | boolean | Return the results of each observation station as a separate group in the XML response. By default the results of all stations form a single group. The default value is `false`
supportQCParameters | boolean | Allow `qc_` prefixed names (the quality codes of the observations) in the `meteoParameters` array. They are accepted only if also `qualityInfo` is enabled. The default value is `false`
meteo_parameter_options | _MeteoParameterOption[]_ | Per parameter output options (see below)

`handler_params` attributes regarding the requested time period.

Name      | Type      | Description
----------|-----------|-------------
beginTime | time      | start time of the requested time period
endTime   | time      | end time of the requested time period
timeStep  | integer   | Interval length between the requested observations in minutes
timeSteps | integer   | The number of the requested time steps
startStep | integer   | The index number of the first selected time step since the start time
hours     | integer[] | requested times expressed in the list of hours (for example “12,13,18,19”)
times     | integer[] | requested times expressed in the list of hours and minutes HHMM (for example “1200,1230”)
weekDays  | integer[] | requested times expressed in the list of weekdays

`handler_params` attributes regarding the requested geographical location/area

Name                 | Type      | Description
---------------------|-----------|--------------
places               | string[]  | Array of names for location of observations
latlons              | double[]  | Array of latitude-longitude pairs for location of observations
geoids               | integer[] | Array of geoids for location of observations
wmos                 | integer[] | An array of observation station WMO identifiers
fmisids              | integer[] | An array of observation station FMISID identifiers
keyword              | string    | The geographical location(s) expressed by a keyword
keyword_overwritable | boolean   | The default values listed in the "keyword" parameter can be overwritten by using the location related input parameters only if the value of this parameter is true"
boundingBox          | double[]  | The geographical location / area expressed by the bounding box coordinates
maxDistance          | double    | The maximum search distance of the observation stations from the given geographical location(s)
lpnns                | integer[] | An array of observation station LPNN identifiers
stationType          | string    | The type of the observation station (defined in the ObsEngine configuration)
numOfStations        | integer   | The maximum number of the observation stations returned around the given geographical location (inside the radius of "maxDistance")

`handler_params` attributes regarding the requested data

Name  | Type | Description |
------|------|--------------|
meteoParameters | string[] | array of fields whose values should be returned in the response.|
crs             | string   | coordinate projection used in the response.|
timeZone        | string   | time zone used in the response.|
locale          | string   | value of the "Locale" (for example fi_FI.utf8). |
language        | string   | The language to use. |
missingText     | string   | value that is returned when the value of the requested field is missing.|
maxEpochs       | integer  | maximum number of time epochs that can be returned. If the estimated number before query is larger than the specified one then the query is aborted. This parameter is not alid if the "storedQueryRestrictions" parameter is set to "false" in the WFS Plugin configuration file.|
qualityInfo     | boolean  | quality code information of the measured values will be included in the response if the parameter value is true.|
latest          | boolean  | Return only the latest observation of each station. |

The _MeteoParameterOption_ structure (top level `meteo_parameter_options` list) can contain the following attributes:

Name         | Type    | Description |
-------------|---------|-------------|
name         | string  | The name of the parameter in `meteoParameters` (case insensitive). Mandatory |
precision    | integer | The number of decimals (0-15). The default is `1` |
missing_value| double  | The data value regarded as missing. The default is `32700.0` |
missing_text | string  | The text returned for missing values. The default is `NaN` |
sensor_first | integer | The first sensor number to query. The default is `1` |
sensor_last  | integer | The last sensor number to query. The default is `1` |
sensor_step  | integer | The step between the queried sensor numbers. The default is `1` |

## wfs_flash_handler_factory

This query handler is used to  fetch the  lightning observation data.

The following table contains the attributes used on the top level of the query configuration file.

Top level configuration parameters

|Name  | Type | Description |
|------|------|--------------|
|constructor_name|string|The name of the stored query factory that is used to create the stored query handler  object. The expected value is "wfs_flash_handler_factory".|
|stationType| string|The observation station type. The default value is "flash".|
|maxHours| double|The maximum allowed length of the requested time interval expressed in hours. The default value is "168" (7 days). This parameter is not valid, if the "storedQueryRestrictions" parameter is set to "false" in the WFS Plugin configuration file.|
|missingText| string|The value that is returned when the value of the requested field is missing. The default value is "NaN".|
|timeBlockSize| integer|The length of a time step used for counting the lightning observations. The length is expressed in seconds. Only time steps with full length are supported. The default value is "300". This parameter is not valid, if the "storedQueryRestrictions" parameter is set to "false" in the WFS Plugin configuration file.|

The following table contains the attributes used inside the  _HandlerParam_ structure.

HandlerParam structure

|Name  | Type | Description |
|------|------|--------------|
|**Requested time period** |||
|beginTime| time|The start time of the requested time period.|
|endTime| time|The end time of the requested time period.|
|**Requested geographical location/ area**|||
|boundingBox| double[]|The geographical location / area expressed by the bounding box coordinates.|
|**Requested data** |||
|meteoParameters| string[]|An array of fields whose values should be returned in the response.|
|crs| string|Specifies the coordinate projection. For example crs = "${crs:EPSG::4326}" maps to request parameter crs with the default value EPSG::4326.|
|timeZone| string|The time zone used in the response.|

## wfs_stored_aviation_observation_handler_factory

This query handler is used for fetching the _aviation observation data_. The  data is returned in the IWXXM (ICAO Meteorological Information Exchange Model) format.

The following table contains the attributes used on the top level of the query configuration file.

Top level configuration parameters

|Name  | Type | Description |
|------|------|--------------|
|constructor_name|string|The name of the stored query factory that is used to create the stored query handler  object. The expected value is "wfs_stored_aviation_observation_handler_factory".|
|maxHours|double|The maximum permitted time interval expressed in hours. The default value is 168 hours (7 days). This attribute is not valid, if the "storedQueryRestrictions" attribute is set to "false" in the WFS Plugin configuration file.|

The following table contains the  attributes used inside the  _HandlerParam_ structure.

HandlerParam structure

|Name  | Type | Description |
|------|------|--------------|
|**Requested time period** |||
|beginTime| time|The start time of the requested time period.|
|endTime| time|The end time of the requested time period.|
|**Requested geographical location/ area**|||
|places|string[]|The geographical location(s) expressed by an array of location names.|
|latlons|double[]|The geographical location(s) expressed by an array of latitude-longitude pairs.|
|geoids|integer[]|The geographical location(s) expressed by an array of geoids. |
|icaoCode|string[]|An array of ICAO identifiers. The ICAO identifier is a four-character alphanumeric code designating each airport around the world.|
|keyword|string|The geographical location(s) expressed by a keyword.|
|keyword_overwritable|boolean|The default values listed in the "keyword" parameter can be overwritten by using the location related input parameters only if the value of this parameter is "true".|
|boundingBox|double[]|The geographical location / area expressed by the bounding box coordinates. |
|maxDistance|double|The maximum search distance of the observation stations from the given geographical  location(s).|
|stationType|string|The observation station type.|
|**Requested data** |||
|returnOnlyLatest|boolean|The attribute indicates whether to return only the latest values from the stations or all.|

## wfs_stored_mast_handler_factory

This query handler is used for fetching the _multi-sensor observation data_.

The following table contains the attributes used on the top level of the query configuration file.

Top level configuration parameters

|Name  | Type | Description |
|------|------|--------------|
|constructor_name|string|The name of the stored query factory that is used to create the stored query handler  object. The expected value is " wfs_stored_mast_handler_factory ".|
|supportQCParameters|boolean|The parameters defines whether the query handler should allow "qc_" prefixed name in the "meteoParameters" array. The default value is "false".|
|maxHours|double|The maximum permitted time interval expressed in hours. The default value is 168 hours (7 days). This attribute is not valid, if the "storedQueryRestrictions" attribute is set to "false" in the WFS Plugin configuration file.|

The following table contains the attributes used inside the  _HandlerParam_ structure.

HandlerParam structure

|Name  | Type | Description |
|------|------|--------------|
|**Requested time period** |||
|beginTime|time|The start time of the requested time period.|
|endTime|time|The end time of the requested time period.|
|timeStep|integer|The time interval between the requested data (observations).|
|maxEpochs|integer|The maximum number of time epochs to return.|
|**Requested geographical location/ area**|||
|places|string[]|The geographical location(s) expressed by an array of location names.|
|latlons|double[]|The geographical location(s) expressed by an array of latitude-longitude pairs.|
|geoids|integer[]|The geographical location(s) expressed by an array of geoids.|
|wmos|integer[]|An array of observation station WMO identifiers.|
|fmisids|integer[]|An array of observation station FMISID identifiers.|
|boundingBox|double[]|The geographical location / area expressed by the bounding box coordinates.|
|maxDistance|double|The maximum search distance of the observation stations from the given geographical location(s).|
|stationType|string|The type of the observation station (defined in the ObsEngine configuration).|
|numOfStations|integer|The maximum number of the observation stations returned around the given geographical location (inside the radius of "maxDistance"). |
|**Requested data** |||
|producerId|integer[]|Producer id values are found for example from PRODUCERS_V1 Oracle database view.|
|meteoParameters|string[]|An array of fields whose  values should be returned in the response. Available data fields depend on the value of the "stationType" attribute.|
|crs|string|The coordinate projection used in the response. |
|missingText|string|The value that is returned when the value of the requested field is missing.|
|qualityInfo|boolean|The quality code information of the measured values will be included into the response if the parameter value is true.|

## wfs_stored_sounding_handler_factory

This query handler is used for fetching the _sounding observation data_.

The following table contains the attributes used on the top level of the query configuration file.

Top level configuration parameters

|Name  | Type | Description |
|------|------|--------------|
|constructor_name|string|The name of the stored query factory that is used to create the stored query handler  object. The expected value is "wfs_stored_sounding_handler_factory".|
|supportQCParameters|boolean|The parameters defines whether the query handler should allow "qc_" prefixed name in the "meteoParameters" array. The default value is "false".|
|maxHours|double|The maximum permitted time interval expressed in hours. The default value is 168 hours (7 days). This attribute is not valid, if the "storedQueryRestrictions" attribute is set to "false" in the WFS Plugin configuration file.|
|maxSoundings|integer|The maximum number of soundings in the requested time interval. The default value is 15. This attribute is not valid, if the "storedQueryRestrictions" attribute is set to "false" in the WFS Plugin configuration file.|

The following table contains the attributes used inside the  _HandlerParam_ structure.

HandlerParam structure

|Name  | Type | Description |
|------|------|--------------|
|**Requested time period** |||
|beginTime|time|The start time of the requested time period.|
|endTime|time|The end time of the requested time period.|
|latest|boolean|Return only the latest sounding.|
|**Requested geographical location/ area**|||
|places|string[]|The geographical location(s) expressed by an array of location names.|
|latlons|double[]|The geographical location(s) expressed by an array of latitude-longitude pairs.|
|geoids|integer[]|The geographical location(s) expressed by an array of geoids.|
|wmos|integer[]|An array of observation station WMO identifiers.|
|fmisids|integer[]|An array of observation station FMISID identifiers.|
|keyword|string|The geographical location(s) expressed by a keyword.|
|keyword_overwritable|boolean|The default values listed in the "keyword" parameter can be overwritten by using the location related input parameters only if the value of this parameter is "true".|
|boundingBox|double[]|The geographical location / area expressed by the bounding box coordinates.|
|maxDistance|double|The maximum search distance of the observation stations from the given geographical location(s).|
|stationType|string|The type of the observation station (defined in the ObsEngine configuration).|
|numOfStations|integer|The maximum number of the observation stations returned around the given geographical location (inside the radius of "maxDistance"). |
|**Requested data** |||
|meteoParameters|string[]|An array of fields whose  values should be returned in the response. At least one is required.|
|soundingType|integer[]|The sounding types to return (types 1, 2 and 3 are supported).|
|publicity|integer[]|The publicity flags of the soundings to return. At least one is required.|
|altitudeRanges|double[]|Altitude ranges as (min, max) pairs restricting the returned levels.|
|pressureRanges|double[]|Pressure ranges as (min, max) pairs restricting the returned levels. The old misspelt name `pressureRages` is also accepted.|
|crs|string|The coordinate projection used in the response. |
|missingText|string|The value that is returned when the value of the requested field is missing.|
|qualityInfo|boolean|The quality code information of the measured values will be included into the response if the parameter value is true.|

## wfs_stored_air_nuclide_handler_factory

This query handler is used for fetching the _air radionuclide activity concentration data_.

The following table contains the attributes used on the top level of the query configuration file.

Top level configuration parameters

|Name  | Type | Description |
|------|------|--------------|
|constructor_name|string|The name of the stored query factory that is used  to create the stored query handler  object. The expected value is "wfs_stored_air_nuclide_handler_factory".|
|maxHours|double|The maximum permitted time interval expressed in hours. The default value is 8760 hours (365 days). This attribute is not valid, if the "storedQueryRestrictions" attribute is set to "false" in the WFS Plugin configuration file.|

The following table contains the attributes used inside the  _HandlerParam_ structure.

HandlerParam structure

|Name  | Type | Description |
|------|------|--------------|
|**Requested time period** |||
|beginTime|time|The start time of the requested time period (YYYY-MM-DDTHHMIZ).|
|endTime|time|The end time of the requested time period  (YYYY-MM-DDTHHMIZ).|
|timeStep|integer|The time interval between the requested records expressed in minutes.|
|**Requested geographical location/ area**|||
|places|string[]|The geographical location(s) expressed by an array of location names.|
|latlons|double[]|The geographical location(s) expressed by an array of latitude-longitude pairs.|
|geoids|integer[]|The geographical location(s) expressed by an array of geoids.|
|keyword|string|The geographical location(s) expressed by a keyword.|
|keyword_overwritable|boolean|The default values listed in the "keyword" parameter can be overwritten by using the location related input parameters only if the value of this parameter is "true".|
|wmos|integer[]|An array of observation station WMO identifiers.|
|fmisids|integer[]|An array of observation station FMISID identifiers.|
|boundingBox|double[]|The geographical location / area expressed by the bounding box coordinates.|
|stationType|string|The observation station type.|
|maxDistance|double|The maximum data search distance from the given geographical location(s).|
|numOfStations|integer|The maximum number of the observation stations returned around the given geographical location (inside the radius of "maxDistance"). |
|**Requested data** |||
nuclideCodes|string[]|An array of nuclide codes. If at least one listed nuclide code match the nuclide code in the analysis, the analysis will be included into the result. |
|crs|string|The coordinate projection used in the response.|
|qualityInfo|boolean|The quality code information of the measured values will be included into the response if the parameter value is true.|
|latest|boolean|The attribute indicates whether to return only the latest values from the stations or all.|

## wfs_stored_env_monitoring_network_handler_factory

This query handler is used for fetching the _environmental monitoring networks_ (INSPIRE EnvironmentalMonitoringNetwork features) from the ObsEngine module.

Top level configuration parameters

|Name  | Type | Description |
|------|------|--------------|
|constructor_name|string|The name of the stored query factory that is used to create the stored query handler  object. The expected value is "wfs_stored_env_monitoring_network_handler_factory".|
|missingText|string|The value that is returned when the value of the requested field is missing. The default value is "NaN".|

HandlerParam structure

|Name  | Type | Description |
|------|------|--------------|
|networkId|integer[]|The unique identities of the requested networks.|
|classId|integer[]|Station groups are classified into classes. ClassId is a unique identity of a class.|
|className|string[]|The names of the classes. Optional.|
|groupId|integer[]|The identities of the station groups.|
|stationId|integer[]|The local identities of the stations.|
|stationName|string[]|The names of the stations. Optional.|
|inspireNamespace|string|The INSPIRE namespace used in the response.|
|authorityDomain|string|The authority domain used for example in codeSpace names of the XML response.|

## wfs_stored_env_monitoring_facility_handler_factory

This query handler is used for fetching the _environmental monitoring facilities_, i.e. the observation stations (INSPIRE EnvironmentalMonitoringFacility features) from the ObsEngine module.

Top level configuration parameters

|Name  | Type | Description |
|------|------|--------------|
|constructor_name|string|The name of the stored query factory that is used to create the stored query handler  object. The expected value is "wfs_stored_env_monitoring_facility_handler_factory".|
|missingText|string|The value that is returned when the value of the requested field is missing. The default value is "NaN".|

HandlerParam structure

|Name  | Type | Description |
|------|------|--------------|
|beginTime|time|The begin of the operational activity period.|
|endTime|time|The end of the operational activity period.|
|classId|integer[]|Station groups are classified into classes. ClassId is a unique identity of a class.|
|groupId|integer[]|The identities of the station groups.|
|stationId|integer[]|The local identities of the stations.|
|stationName|string[]|The names of the stations. Optional.|
|basePhenomenon|string[]|Restrict the result to stations measuring the given phenomena.|
|aggregateFunction|string[]|Restrict the result to stations having measurands with the given aggregate functions.|
|aggregatePeriod|string[]|Restrict the result to stations having measurands with the given aggregate periods.|
|measurandCode|string[]|Restrict the result to stations having measurands with the given measurand codes.|
|storageId|integer[]|Restrict the database search to the given storages.|
|inspireNamespace|string|The INSPIRE namespace used in the response.|
|authorityDomain|string|The authority domain used for example in codeSpace names of the XML response.|
|showObservingCapability|boolean|Include the observing capabilities in the response. Optional.|

## wfs_forecast_handler_factory

This query handler is responsible for queries that are used  to fetch _meteorological forecast data_ from the _QEngine_ module. The forecast data is usually fetched over a given geographical area and time interval.

The following table contains the attributes used on the top level of the query configuration file.

Top level configuration parameters

|Name  | Type | Description |
|------|------|--------------|
|constructor_name|string|The name of the stored query factory that is used in order to create the stored query handler  object. The expected value is "wfs_forecast_handler_factory".|
|maxNpDistance|  double|The maximum distance (in meters) when looking the nearest valid forecast point. The value of this parameter is used only if the value of the "findNearestValid" parameter is not "0" (zero). The default value is "-1" (no limit).|
|separateGroups|  boolean|The parameter defines whether the returned forecast records should be grouped (in the XML response) by the sites. The default value is "false".|
|meteo_parameter_options|_MeteoParameterOption[]_|Per parameter output options, see [wfs_obs_handler_factory](#wfs_obs_handler_factory).|

The following table contains the attributes used inside the  _HandlerParam_ structure.

HandlerParam structure

|Name  | Type | Description |
|------|------|--------------|
|**Requested time period** |||
|beginTime| time|The start time of the requested time period (YYYY-MM-DDTHHMIZ).|
|endTime| time|The end time of the requested time period  (YYYY-MM-DDTHHMIZ).|
|timeStep| integer|The time interval between the requested records expressed in minutes.|
|timeSteps| integer|The number of the requested time steps (= number of data records).|
|startStep| integer|The index number of the first selected time step since the start time.|
|hours| integer[]|The requested times expressed in the list of hours HH (for example “12,13,18,19”).|
|times| integer[]|The requested times expressed in the list of hours and minutes HHMM (for example “1200,1230,1300,1330”).|
|**Requested geographical location / area**|||
|places| string[]|The geographical location(s) expressed by an array of location names.|
|latlons| double[]|The geographical location(s) expressed by an array of latitude-longitude pairs.|
|geoids| integer[]|The geographical location(s) expressed by an array of geoids.|
|fmisids| integer[]|An array of observation station FMISID identifiers.|
|wmos| integer[]|An array of observation station WMO identifiers.|
|lpnns| integer[]|An array of observation station LPNN identifiers.|
|keyword | string|The geographical location(s) expressed by a keyword.|
|keyword_overwritable| boolean|The default values listed in the "keyword" parameter can be overwritten by using the location related input parameters only if the value of this parameter is "true".|
|maxDistance| double|The maximum data search distance from the given geographical location(s).|
|findNearestValid| integer|A non zero value of this parameters causes look-up of the nearest point from the  model.|
|**Requested data** |||
|model| strings[]|An array of weather models (see QEngine configuration) that can be used in the query. An empty array means that all the available weather models can be used.|
|originTime| time|The origin time of the weather models that should be used. This might be omitted in the query. |
|param| string[]|An array of fields which values should be returned in the response.|
|level| integer[]|An array of the model levels that can be used in the query. An empty array means that all the levels can be used.|
|levelHeights|double[]|An array of geometric heights (GeomHeight) above the topography of model. An empty array means that all the levels can be used. The parameter is usable only with hybrid data and interpolated data values will be returned if exact match is not found. The use of parameter also require that the dataset contains e.g. GeomHeight data and it is not allowed to use with the level parameter at the same time.|
|levelType| string|The level type that can be used in the query.|
|crs| string|The coordinate projection used in the response.|
|timeZone| string|The time zone used in the response.|
|locale| string|The value of the "Locale" (for example fi_FI.utf8). This is used for example for formatting time and date information in the response.|
|missingText| string|The value that is returned when the value of the requested field is missing.|

## wfs_grid_forecast_handler_factory

This query handler is the grid engine counterpart of [wfs_forecast_handler_factory](#wfs_forecast_handler_factory): it fetches _meteorological forecast data_ for the requested locations from the _grid engine_ instead of the QEngine.

Top level configuration parameters

|Name  | Type | Description |
|------|------|--------------|
|constructor_name|string|The name of the stored query factory that is used in order to create the stored query handler  object. The expected value is "wfs_grid_forecast_handler_factory".|
|maxNpDistance|  double|The maximum distance (in meters) when looking the nearest valid forecast point. The value of this parameter is used only if the value of the "findNearestValid" parameter is not "0" (zero). The default value is "-1" (no limit).|
|separateGroups|  boolean|The parameter defines whether the returned forecast records should be grouped (in the XML response) by the sites. The default value is "false".|

HandlerParam structure

|Name  | Type | Description |
|------|------|--------------|
|**Requested time period** |||
|beginTime| time|The start time of the requested time period.|
|endTime| time|The end time of the requested time period.|
|timeStep| integer|The time interval between the requested records expressed in minutes.|
|timeSteps| integer|The number of the requested time steps (= number of data records).|
|startStep| integer|The index number of the first selected time step since the start time.|
|hours| integer[]|The requested times expressed in the list of hours HH (for example “12,13,18,19”).|
|times| integer[]|The requested times expressed in the list of hours and minutes HHMM (for example “1200,1230,1300,1330”).|
|**Requested geographical location / area**|||
|places| string[]|The geographical location(s) expressed by an array of location names.|
|latlons| double[]|The geographical location(s) expressed by an array of latitude-longitude pairs.|
|geoids| integer[]|The geographical location(s) expressed by an array of geoids.|
|keyword | string|The geographical location(s) expressed by a keyword.|
|keyword_overwritable| boolean|The default values listed in the "keyword" parameter can be overwritten by using the location related input parameters only if the value of this parameter is "true".|
|maxDistance| double|The maximum data search distance from the given geographical location(s).|
|findNearestValid| integer|A non zero value of this parameters causes look-up of the nearest point from the  model.|
|**Requested data** |||
|model| string[]|An array of producers (see the grid engine configuration) that can be used in the query.|
|originTime| time|The origin time of the weather models that should be used. This might be omitted in the query. |
|param| string[]|An array of fields which values should be returned in the response (1-99 items).|
|level| integer[]|An array of the model levels that can be used in the query.|
|levelHeights|double[]|An array of geometric heights above the topography of the model.|
|levelType| string|The level type that can be used in the query.|
|crs| string|The coordinate projection used in the response.|
|timeZone| string|The time zone used in the response.|
|locale| string|The value of the "Locale" (for example fi_FI.utf8).|
|missingText| string|The value that is returned when the value of the requested field is missing.|

## wfs_stored_qe_download_handler_factory

Query handler: Forecast data downloading

This query handler is responsible for queries that are used to fetch _download URLs for meteorological forecast data_ from the _QEngine_ module. The query handler does not return the requested data directly. Instead, it stores the data to the file system and returns the URL to this data. This  URL can be used to download the actual data. This makes it  easier to fetch large amounts of data or to fetch data in different formats than XML.

The following table contains the attributes used on the top level of the query configuration file.

Top level configuration parameters

|Name  | Type | Description |
|------|------|--------------|
|constructor_name|string|The name of the stored query factory that is used in order to create the stored query handler  object. The expected value is "wfs_stored_qe_download_handler_factory".|
|formats|string[]|An array of supported formats. The default value is "grib2,grib1,netcdf"). The "grib2" format is supported even if it is not defined at the array.|
|producers|string[]|An array of supported producers. If the array is not given then all producers used by the  QEngine module are supported.|
|url_template|_UrlTemplate_|The UrlTemplate structure used for defining the download URL.|

The following table contains the attributes used inside the  _HandlerParam_ structure.

HandlerParam structure

|Name  | Type | Description |
|------|------|--------------|
|**Requested time period** |||
|beginTime|time|The start time of the requested time period (YYYY-MM-DDTHHMIZ).|
|endTime|time|The end time of the requested time period  (YYYY-MM-DDTHHMIZ).|
|fullInterval|integer|If non zero then the full specified time interval must be present in the  model data for the model to be used. Otherwise the overlapping is sufficient.|
|**Requested geographical location / area**|||
|boundingBox|double[]|The geographical location / area expressed by the bounding box coordinates.|
|**Requested data** |||
|producer|string[0..1]|The producer (weather model, see QEngine configuration) to use. If omitted, all the producers allowed by the "producers" setting can be used.|
|originTime|time|The origin time of the weather models that should be used. This might be omitted in the query. |
|meteoParam|string[]|An array of fields whose values should be returned in the response.|
|levelType|string[]|An array of level types used in the query.|
|levelValue|double[]|An array of the model levels that can be used in the query. An empty array means that all the levels can be used.|
|format|string|The format used in the response. The default value is "grib2".|
|projection|string|The coordinate projection used in the response.|

The _UrlTemplate_ structure  contains the following attributes:

UrlTemplate structure

|Name  | Type | Description |
|------|------|--------------|
|url| string |Describes the URL to which parameters are appended. Substitutions of stored query handler named parameter values are supported in this string.|
|params |string[] |Describes the URL parameters to be added to the generated URL. |

## wfs_stored_file_handler_factory

Query handler: Forecast file data downloading

This query handler is used for fetching _download URLs of the data from the query files_ instead of using the QEngine module. The query handler returns a URL that can be used for downloading the actual query data.  This makes it easier to fetch bigger amounts of data or to fetch data in different formats than XML.

The following table contains the attributes used on the top level of the query configuration file.

Top level configuration parameters

|Name  | Type | Description |
|------|------|--------------|
|constructor_name|string|The name of the stored query factory that is used in order to create the stored query handler  object. The expected value is "wfs_stored_file_handler_factory".|
|dataSets[]|_DataSet_|An array of _DataSet_ structures used for defining the available data sets.|
|url_template|UrlTemplate|The UrlTemplate structure used for defining the download URL. |

The following table contains the attributes used inside the  _HandlerParam_ structure.

HandlerParam structure

|Name  | Type | Description |
|------|------|--------------|
|**Requested time period** |||
|beginTime|time|The start time of the requested time period (YYYY-MM-DDTHHMIZ).|
|endTime|time|The end time of the requested time period  (YYYY-MM-DDTHHMIZ).|
|**Requested geographical location / area**|||
|bbox|double[]|The geographical location / area expressed by the bounding box coordinates.|
|**Requested data** |||
|name|string|The name of the requested data set (see the "dataSets" attribute). Optional.|
|params| strings[]|An array of the requested data fields. The data fields for the each data set are specified with the "dataSets" attribute. All available data fields are selected if the value of this attribute is empty.|
|levels|integer[]|An array of the requested  levels.|

The _DataSet_ structure can contain the following attributes:

DataSet structure

|Name  | Type | Description |
|------|------|--------------|
|name|string|The name of the file set. Mandatory.|
|dir|string|The name of the directory where the query files can be found. The directory must exist. Mandatory.|
|serverDir|string|The directory name of the server (the directory part of the URL). The default is empty.|
|file_regex|string|Regular expressions used for filtering file names (only the base name). Mandatory.|
|origin_time_extract|string|Specifies regular expressions for extracting the origin time string. Mandatory.|
|origin_time_translate|string[2]|The first element in the array specifies how to extract the time fields from the origin time string. The second element specifies how to translate the origin time to the ISO extended time format. The default value is: ["^(\\d{4})(\\d{2})(\\d{2})(\\d{2})(\\d{2})(\\d{2})", "\\1\\2\\3T\\4\\5\\6Z"]|
|params|string[]|The data fields available in the query files. Mandatory.|
|levels|integer[]|The levels available in the query files. Mandatory.|
|bbox|double[4]|The bounding box of the query files. Mandatory.|

The _UrlTemplate_ structure  contains the following attributes:

UrlTemplate structure

|Name  | Type | Description |
|------|------|--------------|
|url| string |Describes the URL to which parameters are appended. Substitutions of stored query handler named parameter values are supported in this string.|
|params |string[] |Describes the URL parameters to be added to the generated URL. |

## wfs_stored_grid_handler_factory

This query handler is responsible for queries that are used to fetch _download URLs for forecast data in the grid format_.

The following table contains the attributes used on the top level of the query configuration file.

Top level configuration parameters

|Name  | Type | Description |
|------|------|--------------|
|constructor_name|string|The name of the stored query factory that is used in order to create the stored query handler  object. The expected value is "wfs_stored_grid_handler_factory".|

The  _HandlerParam_ structure contains the following parameters when using this query handler.

HandlerParam structure

|Name  | Type | Description |
|------|------|--------------|
|**Requested time period** |||
|beginTime|time|The start time of the requested time period (YYYY-MM-DDTHHMIZ).|
|endTime|time|The end time of the requested time period  (YYYY-MM-DDTHHMIZ).|
|timeStep|integer|The time interval between the requested records expressed in minutes.|
|timeSteps|integer|The number of the requested time steps (= number of data records).|
|startStep|integer|The index number of the first selected time step since the start time.|
|hours|integer[]|The requested times expressed in the list of hours HH (for example “12,13,18,19”).|
|times|integer[]|The requested times expressed in the list of hours and minutes HHMM (for example “1200,1230,1300,1330”).|
|**Requested geographical location / area**|||
|boundingBox|double[]|The geographical location / area expressed by the bounding box coordinates. |
|**Requested data** |||
|producer|string|The data producer name.|
|originTime|time|The origin time of the weather models that should be used. This might be omitted in the query.|
|param|string[]|An array of fields whose values should be returned in the response.|
|levelValue|double[]|An array of the model levels that can be used in the query. An empty array means that all the levels can be used.|
|levelType|string|The level type that can be used in the query.|
|scaleFactor|integer|The scale factor for the returned data.|
|datastep|integer|The spatial data interval for output.|
|precision|integer|This attribute specifies the precision (= number of decimals) of the response data. |
|missingText|string|The value that is returned when the value of the requested field is missing.|
|dataCRS|string|The coordinate projection used in the response.|

## wfs_winterweather_probabilities_query_handler_factory

Query handler: Winter weather forecast data

This query handler is responsible for fetching the _forecasts for the probabilities of the different winter weather conditions_.

The following table contains the attributes used on the top level of the query configuration file.

Top level configuration parameters

|Name  | Type | Description |
|------|------|--------------|
|constructor_name|string|The name of the stored query factory that is used in order to create the stored query handler  object. The expected value is "wfs_winterweather_probabilities_query_handler_factory".|
|probability_params|_ProbabilityParam_|The _ProbabilityParam_ structure.|

The following table contains the attributes used inside the _HandlerParam_ structure of this query handler.

HandlerParam structure

|Name  | Type | Description |
|------|------|--------------|
|**Requested time period** |||
|beginTime| time|The start time of the requested time period.|
|endTime| time|The end time of the requested time period.|
|timeStep| integer|The time interval between the requested records expressed in minutes.|
|timeSteps| integer|The number of the requested time steps (= number of data records).|
|startStep| integer|The index number of the first selected time step since the start time.|
|hours| integer[]|The requested times expressed in the list of hours (for example “12,13,18,19”).|
|times| integer[]|The requested times expressed in the list of hours and minutes HHMM (for example “1200,1230,1300,1330”).|
|**Requested geographical location / area**|||
|places|string[]|The geographical location(s) expressed by an array of location names.|
|latlons|double[]|The geographical location(s) expressed by an array of latitude-longitude pairs.|
|geoids|integer[]|The geographical location(s) expressed by an array of geoids. |
|icaoCode|string[]|An array of ICAO identifiers. The ICAO identifier is a four-character alphanumeric code designating each airport around the world.|
|keyword|string|The geographical location(s) expressed by a keyword.|
|keyword_overwritable|boolean|The default values listed in the "keyword" parameter can be overwritten by using the location related input parameters only if the value of this parameter is "true".|
|boundingBox|double[]|The geographical location / area expressed by the bounding box coordinates. |
|maxDistance|double|The maximum search distance from the given geographical  location(s).|
|findNearestValid| integer|A non zero value of this parameters causes look-up of the nearest point from the model.|
|**Requested data** |||
|producer|string|The data producer name.|
|originTime|time|The origin time of the weather models that should be used. This might be omitted in the query. |
|crs|string|The coordinate projection used in the response.|
|timeZone|string|The time zone used in the response.|
|locale|string|The value of the "Locale" (for example fi_FI.utf8). |
|missingText|string|The value that is returned when the value of the requested field is missing.|

The following table contains the attributes used inside the  _ProbabilityParam_ structure. All of them are mandatory.

ProbabilityParam structure

|Name  | Type | Description |
|------|------|--------------|
|param_id|string[]|An array of parameter identifiers. Mandatory.|
|probabilityUnit|string|The probability unit (for example "percent")|
|precipitationIntensityLight|string|A symbolic name for the light precipitation intensity.  |
|precipitationIntensityModerate|string|A symbolic name for the moderate precipitation intensity.|
|precipitationIntensityHeavy|string|A symbolic name for the heavy precipitation intensity.|
|&lt;param_id&gt;|_ProbParam_|There is one _ProbParam_ structure for the each parameter listed in the "param_id" attribute.|

The _ProbParam_ structure can contain the following attributes:

ProbParam structure

|Name  | Type | Description |
|------|------|--------------|
|precipitationType|string|The precipitation type (for example "Rain"). Mandatory.|
|idLight|integer|The field identifier used for fetching data for the light precipitation of this type. Mandatory.|
|idModerate|integer|The field identifier used for fetching data for the moderate precipitation of this type. Mandatory.|
|idHeavy|integer|The field identifier used for fetching data for the heavy precipitation of this type. Mandatory.|

## wfs_isoline_query_handler_factory

Query handler: Forecast isoline data

This query handler is used for fetching the _forecast parameter values of a  given geographical area returned as the GML contours (isolines)_.

The following table contains the attributes used on the top level of the query configuration file.

Top level configuration parameters

|Name  | Type | Description |
|------|------|--------------|
|constructor_name|string|The name of the stored query factory that is used in order to create the stored query handler  object. The expected value is "wfs_isoline_query_handler_factory".|
|contour_param| _ContourParam_|The _ContourParam_ structure used for defining the contour attributes.|

The following table contains the attributes used inside the _HandlerParam_ structure of this query handler.

HandlerParam structure

|Name  | Type | Description |
|------|------|--------------|
|**Requested time period** |||
|beginTime|time|The start time of the requested time period (YYYY-MM-DDTHHMIZ).|
|endTime|time|The end time of the requested time period  (YYYY-MM-DDTHHMIZ).|
|timeStep|integer|The time interval between the requested records expressed in minutes.|
|timeSteps|integer|The number of the requested time steps (= number of data records).|
|startStep|integer|The index number of the first selected time step since the start time.|
|hours|integer[]|The requested times expressed in the list of hours HH (for example “12,13,18,19”).|
|times|integer[]|The requested times expressed in the list of hours and minutes HHMM (for example “1200,1230,1300,1330”).|
|**Requested geographical location / area**|||
|boundingBox| double[]|The geographical location / area expressed by the bounding box coordinates.|
|**Requested data** |||
|producer|string|The data producer name.|
|originTime|time|The origin time of the weather models that should be used. This might be omitted in the query.|
|crs|string|The coordinate projection used in the response.|
|timeZone|string|The time zone used in the response.|
|smoothing|boolean|Enable smoothing of the data before contouring. Recognized only if present in the configuration.|
|smoothing_degree|integer|The degree of the smoothing. Recognized only if present in the configuration.|
|smoothing_size|integer|The size of the smoothing. Recognized only if present in the configuration.|

The following table contains the  attributes used inside the  _ContourParam_ structure.

ContourParam structure

|Name  | Type | Description |
|------|------|--------------|
|name|string|The name of the data field used for the contours (for example "Pressure"). Mandatory.|
|unit|string|The unit of the contour (for example "hPa"). Optional.|
|id|integer|The id of the data field used for the contours. Mandatory.|
|isovalues|double[]|Mandatory. An array of values that are used for the isolines. For example, if we want to get an isoline that follows the pressure value P then we add this value to the array. In practice, we usually want multiple isolines that follows the pressure values / steps we define.|

## wfs_coverage_query_handler_factory

Query handler for forecast coverage data.
This query handler is used for fetching forecast parameter values of the given area returned as GML contours (isobars).
The information can then be used for example to draw weather forecast maps.

The query handler adds requirements to stored query configuration in top level attributes and in handler_params.

Top level configuration parameters

Attribute name  | Type           | Description
----------------|----------------|--------------
`contour_param` | _ContourParam_ | What data is to be formed as isobars (see below table for details)

The _ContourParam_ structure for this query handler

Name     | Type     | Description
---------|----------|--------------
`name`   | string   | The name of the data field used for the contours (e.g. `Pressure`). Mandatory
`unit`   | string   | The unit of the contour (e.g. `hPa`). Optional
`id`     | integer  | The identification number of the data field in the source data files. Mandatory
`limits` | double[] | An array of comma separated value ranges which are used in the contours. The value ranges are expressed as comma separated value pairs. For example, two ranges: from `1.05` to `3` and from `4` to `7.85` would be expressed with an array `[1.05,3,4,7.85]`. Optional: the limits can also be given with the `limits` handler parameter.

The attributes for the _HandlerParam_ structure (for `handler_params` settings) for this query handler.
Some of the attributes are mutually exclusive and not all of them can of should be defined at the same time.

Name                   | Type      | Description
-----------------------|-----------|------------------
`beginTime`            | string    | Start of the requested time period in ISO format timestamp (`YYYY-MM-DDThh:mmZ`)
`endTime`              | string    | The end time of the requested time period  (`YYYY-MM-DDThh:mmZ`)
`timeStep`             | integer   | The time interval between the requested records expressed in minutes
`timeSteps`            | integer   | The number of the requested time steps (= number of data records)
`startStep`            | integer   | The index number of the first selected time step since the start time
`hours`                | integer[] | Request data only for exact hours of day (_HH_) (for example `12,13,18,19`)
`times`                | integer[] | Request data only for exact times of day (_HHMM_) (for example `1200,1230,1300,1330`)
`boundingBox`          | double[]  | The geographical location / area expressed by the bounding box coordinates
`producer`             | string    | The data producer name
`originTime`           | time      | The origin time of the weather models that should be used. This might be omitted in the query
`crs`                  | string    | The coordinate projection used in the response
`timeZone`             | string    | The time zone used in the response
`limits`               | double[]  | The contour value ranges as value pairs (see `contour_param.limits`). Recognized only if present in the configuration
`smoothing`            | boolean   | Enable smoothing of the data before contouring. Recognized only if present in the configuration
`smoothing_degree`     | integer   | The degree of the smoothing. Recognized only if present in the configuration
`smoothing_size`       | integer   | The size of the smoothing. Recognized only if present in the configuration

## wfs_winterweather_coverage_query_handler_factory

Query handler: Winter weather coverage data

This query handler is used for fetching the winter forecast probabilities of a given geographical area returned as GML contours (isobars). This information can be used in order to draw weather forecast maps.

The following table contains the attributes used on the top level of the query configuration file.

Top level configuration parameters

|Name  | Type | Description |
|------|------|--------------|
|constructor_name|string|The name of the stored query factory that is used in order to create the stored query handler  object. The expected value is "wfs_winterweather_coverage_query_handler_factory".|
|contour_param| _ContourParam_|The _ContourParam_ structure used for defining the contour attributes|

The following table contains the attributes used inside the _HandlerParam_ structure of this query handler.

HandlerParam structure

|Name  | Type | Description |
|------|------|--------------|
|**Requested time period** |||
|beginTime|time|The start time of the requested time period (YYYY-MM-DDTHHMIZ).|
|endTime|time|The end time of the requested time period  (YYYY-MM-DDTHHMIZ).|
|timeStep|integer|The time interval between the requested records expressed in minutes.|
|timeSteps|integer|The number of the requested time steps (= number of data records).|
|startStep|integer|The index number of the first selected time step since the start time.|
|hours|integer[]|The requested times expressed in the list of hours HH (for example “12,13,18,19”).|
|times|integer[]|The requested times expressed in the list of hours and minutes HHMM (for example “1200,1230,1300,1330”).|
|**Requested geographical location / area**|||
|boundingBox|double[]|The geographical location / area expressed by the bounding box coordinates.|
|**Requested data** |||
|producer|string|The data producer name.|
|originTime|time|The origin time of the weather models that should be used. This might be omitted in the query. |
|crs|string|The coordinate projection used in the response.|
|timeZone|string|The time zone used in the response.|

The following table contains the attributes used inside the  _ContourParam_ structure

ContourParam structure

|Name  | Type | Description |
|------|------|--------------|
|name|string|The name of the data field used for the contours (for example "TopLinkIndex2"). Mandatory.|
|id|integer|The id of the data field used for the contours. Mandatory.|
|unit|string|The unit of the contour. Optional.|
|limits|double[]|An array of value ranges which are used in the contours. The value ranges are expressed as value pairs (for example, ranges [1..3] and [4..7] are expressed with the value array "[1,3,4,7]").|
|limitNames|string[]|An array of value range names. This defines a name for each value range defined with the "limits" attribute. Mandatory.|

## wfs_stored_geoserver_handler_factory

Query handler: Geographical data downloading

This query handler is used for fetching the _download URLs for geographical data from the GeoEngine module_. Instead of returning the actual data, the query handler returns a URL that can be used for fetching the actual data.  This makes it easier to fetch bigger amounts of data or fetch data in different formats than XML.

The following table contains the attributes used on the top level of the query configuration file.

Top level configuration parameters

|Name  | Type | Description |
|------|------|--------------|
|constructor_name|string|The name of the stored query factory that is used in order to create the stored query handler  object. The expected value is "wfs_stored_geoserver_handler_factory".|
|layerDbTableNameFormat|string|Specifies how to generate the PostGIS database table name from the layer name in "boost::format" format string. The default is "mosaic.%1%" if neither this attribute nor "layerMap" is given. |
|layerMap|_LayerMapItem[]_|Specifies the mapping of the layers (and their aliases) to actual database tables. Ignored if the "layerDbTableNameFormat" attribute is specified|
|layerParamNameMap|_LayerParamNameItem[]_|Specifies the mapping between the layer names and the data field names. Mandatory.|
|url_template|_UrlTemplate_|The _UrlTemplate_ structure used for defining the download URL. |
|dbSelectParams|string[]|Specifies some extra database parameters which can be used for additional filtering of the queried data. Currently only the filtering by the equal string value is supported|

The following table contains the attributes used inside the _HandlerParam_ structure of this query handler.

HandlerParam structure

|Name  | Type | Description |
|------|------|--------------|
|**Requested time period** |||
|begin|time|The start time of the requested time period (YYYY-MM-DDTHHMIZ).|
|end|time|The end time of the requested time period  (YYYY-MM-DDTHHMIZ).|
|**Requested geographical location / area**|||
|boundingBox|double[]|The geographical location / area expressed by the bounding box coordinates.|
|**Requested data** |||
|layers| strings[]|An array of layer names used for the image.|
|width| int|The width of the returned image.|
|height| int|The height of the returned image.|
|crs| string|The coordinate projection used in the response.|

The _LayerMapItem_ structure can contain the following attributes:

LayerMapItem structure

|Name  | Type | Description |
|------|------|--------------|
|name|string|The layer name. Mandatory.|
|alias|string[]|An array of the alias names for the layer.|
|db_table|string|The PostGIS database table name for the layer. The default is "mosaic.&lt;name&gt;".|

The _LayerParamNameItem_ structure can contain the following attributes:

LayerParamNameItem structure

|Name  | Type | Description |
|------|------|--------------|
|layer|string|The layer name.|
|param|string|The data field name used in the layer.|

The _UrlTemplate_ structure can contain the following attributes:

UrlTemplate structure

|Name  | Type | Description |
|------|------|--------------|
|url|string|Describes the URL to which parameters are appended. Substitutions of stored query handler named parameter values are supported in this string.|
|params|string[]|Describes the URL parameters to be added to the generated URL. |

## wfs_get_data_set_by_id_handler_factory

Query handler: Predefined data sets

This query handler is used to fetch the _predefined data sets by using only the data set identifier_. The idea is that this data set identifier can be mapped to another stored query that will be executed.

The following table contains the attributes used on the top level of the query configuration file.

Top level configuration parameters

|Name  | Type | Description |
|------|------|--------------|
|constructor_name|string|The name of the stored query factory that is used in order to create the stored query handler  object. The expected value is "wfs_get_data_set_by_id_handler_factory".|
|datasetids|_DataSetDef[]_|An array of _DataSetDef_ structures. These structures are needed for mapping the data set identifiers to the stored queries.|

The following table contains the attributes used inside the _HandlerParam_ structure of this query handler.

HandlerParam structure

|Name  | Type | Description |
|------|------|--------------|
|**Requested data** |||
|datasetid|string|The identifier of the requested data set. The _DataSetDef_ structure with this identifier is searched from the "datasetids" array. If found then the "stored_query" attribute contains the name of the stored query that will be executed in order to get the data.|

The _DataSetDef_ structure can contain the following attributes:

DataSetDef structure

|Name  | Type | Description |
|------|------|--------------|
|data_set|string|The data set identifier.|
|stored_query|string|The name of the stored query that will be executed when this data set is requested.|
|namespace|string|The namespace value used in the "GetCapabilities" response. The default value is "FI".|

## wfs_get_feature_by_id_handler_factory

This query handler is used for _implementing the mandatory "GetFeatureById" stored query defined in the WFS 2.0 standard_.  This stored query accepts a single argument named "id" and _returns a single feature_ whose identifier is equal to the specified value of the "id" argument.

When using the SmartMet Server's WFS plugin this query is mostly useless, because most of the features only have temporary IDs that are generated on-the-fly during the response generation, and thus cannot be used as permanent identifiers for those features.

The following table contains the attributes used on the top level of the query configuration file.

Top level configuration parameters

|Name  | Type | Description |
|------|------|--------------|
|constructor_name|string|The name of the stored query factory that is used in order to create the stored query handler  object. The expected value is "wfs_get_feature_by_id_handler_factory".|

The following table contains the attributes used inside the _HandlerParam_ structure of this query handler.

HandlerParam structure

|Name  | Type | Description |
|------|------|--------------|
|**Requested data** |||
|feature_id|string|The feature identifier of the requested feature.|
