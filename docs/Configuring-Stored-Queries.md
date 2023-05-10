# Configuring Stored Queries

Each stored query requires its own configuration file that contains all the necessary information needed to call and execute the stored query.

* [Example of a Stored Query File](#example-of-a-stored-query-file)
* [Configuration File Attributes](#configuration-file-attributes)
  * [disabled](#disabled)
  * [hidden](#hidden)
  * [demo](#demo)
  * [test](#test)
  * [debugLevel](#debugLevel)
  * [id](#id)
  * [constructor_name](#constructor_name)
  * [title](#title)
  * [abstract](#abstract)
  * [template](#template)
  * [defaultLanguage](#defaultLanguage)
  * [expiresSeconds](#expiresSeconds)
  * [parameters](#parameters)
  * [named_params](#named_params)
  * [returnTypeNames](#returnTypeNames)
  * [handler_params](#handler_params)

## Example of a Stored Query File

The file syntax is Json-ish and allows line comments with `//`

```javascript
disabled = false;
id = "fmi::forecast::ecmwf::surface::coverage::temperature";
constructor_name = "wfs_coverage_query_handler_factory";
template = "coverage_countours.c2t";
expiresSeconds = 21600;

title :
{
  eng = "Weather parameter coverage as GML-contours";
  fin = "Sääparametrin peittoalue esitettynä GML-kontuureina";
};

abstract :
{
  eng = "Weather parameter forecast for an area returned in GML-contours. Contours are polygons. Area can be specified as bbox parameter.";
  fin = "Sääparametrin ennuste alueelle palautettuna GML-konttuureina. Haluttu alue voidaan määrittää bounding boxina (bbox). Tuloksena saatavat kontuurit ovat polygoneja."
};

parameters : (
{
  name = "starttime";
  title : { eng = "Begin of the time interval"; fin = "Alkuaika"; };
  abstract : {
    eng = "Parameter begin specifies the begin of time interval in ISO-format (for example 2012-02-27T00:00:00Z).";
    fin = "Aikajakson alkuaika ISO-muodossa (esim. 2012-02-27T00:00:00Z).";
  };
  xmlType = "dateTime";
  type = "time";
},
{
  name = "origintime";
  title : { eng = "Analysis time"; fin = "Analyysiaika"; };
  abstract : {
    eng = "Analysis time specifies the time of analysis in ISO-format (for example 2012-02-27T00:00:00Z).";
    fin = "Analyysiaika ISO-muodossa (esim. 2012-02-27T00:00:00Z).";
  };
  xmlType = "dateTime";
  type = "time";
},
{
  name = "endtime";
  title : { eng = "End of time interval"; fin = "Loppuaika"; };
  abstract : {
    eng = "End of time interval in ISO-format (for example 2012-02-27T00:00:00Z).";
    fin = "Aikajakson loppuaika ISO-muodossa (esim. 2012-02-27T00:00:00Z).";
  };
  xmlType = "dateTime";
  type = "time";
},
{
  name = "timestep";
  title : { eng = "The time step of data in minutes";  fin = "Aika-askel minuutteina"; };
  abstract : {
    eng = "The time step of data in minutes. Notice that timestep is calculated from start of the ongoing hour or day. ";
    fin = "Aika-askel minuutteina. Huomaa, että aika-askel lasketaan tasaminuuteille edellisestä tasatunnista tai vuorokaudesta.";
  };
  xmlType = "int";
  type = "uint";
},
{
  name = "timesteps";
  title : { eng = "Number of timesteps"; fin = "Aika-askelten lukumäärä"; };
  abstract : {
    eng = "Number of timesteps in result set.";
    fin = "Tulosjoukossa palautettavien aika-askelten lukumäärä.";
  };
  xmlType = "int";
  type = "uint";
},
{
  name = "crs";
  title : { eng = "Coordinate projection to use in results"; fin = "Projektio"; };
  abstract : {
    eng = "Coordinate projection to use in results. For example EPSG::3067";
    fin = "Projektiotieto. Esimerkiksi EPSG::3067";
  };
  xmlType = "xsi:string";
  type = "string";
},
{
  name = "bbox";
  title : { eng = "Bounding box of area for which to return data."; fin = "Aluerajaus"; };
  abstract : {
    eng = "Bounding box of area for which to return data (lon,lat,lon,lat). For example 21,61,22,62";
    fin = "Aluerajaus (lon,lat,lon,lat). Esimerkiksi 21,61,22,62";
  };
  xmlType = "xsi:string";
  type = "bbox";
  minOccurs = 0;
  maxOccurs = 1;
},
{
  name  = "timezone";
  title : { eng = "Time zone"; fin = "Aikavyöhyke"; };
  abstract = {
  eng = "Time zone of the time instant of the data point in the form Area/Location (for example America/Costa_Rica). Default value is UTC.";
  fin = "Datapisteen aika-arvon aikavyöhyke muodossa Alue/Paikka (esim. Europe/Stockholm). Oletusvyöhyke on UTC.";
  };
  xmlType  = "xsi:string";
  type  = "string";
}
);

returnTypeNames = [ "omso:GridSeriesObservation" ];

handler_params :
{
  hours = [];
  times = [];
  timeSteps = "${timesteps}";
  beginTime = "${starttime: now}";
  endTime = "${endtime: after 3 hours}";
  timeStep = "${timestep:180}";
  producer = "ecmwf_maailma_pinta";
  maxDistance = 50000.0;
  places = [];
  latlons = [];
  geoids = [];
  keyword_overwritable = false;
  keyword = "";
  boundingBox = "${bbox:-180,-90,180,90}";
  originTime = "${origintime}";
  startStep="${}";
  crs = "${crs:EPSG::4326}";
  timeZone = "${timezone:UTC}";
};

contour_param :
{
  name = "Temperature";
  unit = "C";
  id = 4;
  limits = [1.0,9.0,9.1,16.0];
};
```

## Configuration File Attributes

List of common attributes that can be used on the top level in the stored query configuration file.

_Notice that different query handlers (defined with `constructor_name`) can define their own top level attributes._

Name                                    | Type           | Description
----------------------------------------|----------------|--------------
[disabled](#disabled)                   | boolean        | Do not load this stored query in use when SmartMet Server is started. The default value is `false`
[hidden](#hidden)                       | boolean        | Exclude this stored query from the responses of _ListStoredQueries_ and _DescribeStoredQueries_ requests. The default is `false`
[demo](#demo)                           | boolean        | Stored query is used for demo purposes. The default value is "false"
[test](#test)                           | boolean        | This attribute can be used for specifying that the query is used for testing purposes. The default value is "false"
[debugLevel](#debugLevel)               | integer        | This attribute specifies how much debug information is printed into the output. A bigger value means more. The default value is "0"
[id](#id)                               | string         | The unique identifier (id) of the stored query. The identifier must match the configuration filename (without the suffix)
[constructor_name](#constructor_name)   | string         | The symbolic name of the object factory that is used for creating the query handler object
[title](#title)                         | object         | The title of the stored query. The title can be expressed in several languages
[abstract](#abstract)                   | object         | A more detailed description of the stored query. The description can be written in several languages
[template](#template)                   | string         | A [CTPP](http://ctpp.havoc.ru/en/)  template used by the stored query handler
[defaultLanguage](#defaultLanguage)     | string         | The default language to be used in descriptions. Notice that this attribute is mandatory if string translations are required. The default value is "eng"
[expiresSeconds](#expiresSeconds)       | integer        | The query expiration time in seconds. The query should return a response within this time. If there are multiple stored queries in the request then the smallest value is used. The default value is "60"
[parameters](#parameters)               | _Param[]_      | Allow which fields and values are valid in the WFS query string.
[named_params](#named_params)           | _NamedParam[]_ | ???
[returnTypeNames](#returnTypeNames)     | string[]       | An array of XML types this stored query may return
[handler_params](#handler_params)       | _HandlerParam_ | Defines how to map the WFS request parameters and named parameters to the stored query parameters. The content depends on and is documented for each `constructor_name` setting separately

### disabled

Do not load this stored query in use when SmartMet Server is started. The default value is `false`

### hidden

Exclude this stored query from the responses of _ListStoredQueries_ and _DescribeStoredQueries_ requests. The default is `false`

### demo

Stored query is used for demo purposes. The default value is `false`

### test

This attribute can be used for specifying that the query is used for testing purposes. The default value is "false"

### debugLevel

This attribute specifies how much debug information is printed into the output. A bigger value means more. The default value is `0`

### id

The unique identifier (id) of the stored query

### constructor_name

This attribute defines what data source the SmartMet Server should use, how the data is accessed, how it is processed and how is it formatted in the response.
Also what for each consturctor_name there can be additional top level attributes and unique 'handler_params' attributes.
See details for each `constructor_name` on page [Configuring Query Handler for a Stored Query](Configuring-Query-Handler-for-a-Stored-Query.md).

_Setting's name refers to things in the program's source code.
It may have a meaning for C++ developers, but it is lacking when viewed as the end user creating new stored queries._
Other terms used describing this setting are _query handler_, _query handler factory_, _factory name_, _factory constructor_, etc. but these cannot be used in the config files.

Select the value that produces results based on your requirements.
For example, usually any query handler that is designed for loading weather _observations_ may not be used for loading weather _forecasts_.

See each _query handler_'s documentation for some help [Configuring Query Handler for a Stored Query](Configuring-Query-Handler-for-a-Stored-Query).

The WFS plugin contains the following query handler factories (eg. constructor_names)

Value                                                  | Description
-------------------------------------------------------|------------------
wfs_obs_handler_factory                                | Observation data (general, no lightning, aviation or multi-sensor)
wfs_flash_handler_factory                              | Observation data: Lightning
wfs_stored_aviation_observation_handler_factory        | Observation data: Aviation
wfs_stored_mast_handler_factory                        | Observation data: Multi-sensor
wfs_forecast_handler_factory                           | Forecast data (general)
wfs_stored_qe_download_handler_factory                 | Forecast data: download when data source is QEngine
wfs_stored_file_handler_factory                        | Forecast data: download in querydata format
wfs_stored_grid_handler_factory                        | Forecast data: download in grid format (grib1, grib2, NetCDF)
wfs_winterweather_probabilities_query_handler_factory  | Forecast data: for different winter weather conditions
wfs_isoline_query_handler_factory                      | Forecast data: areas as GML isolines
wfs_coverage_query_handler_factory                     | Forecast data: areas as GML isobars
wfs_winterweather_coverage_query_handler_factory       | Forecast data: winter weather conditions of the given geographical area returned as GML isobars
wfs_stored_air_nuclide_handler_factory                 | Air radionuclide activity concentration data
wfs_stored_geoserver_handler_factory                   | Geographical data downloading
wfs_get_data_set_by_id_handler_factory                 | Predefined data sets
wfs_get_feature_by_id_handler_factory                  | The mandatory implementation of the "GetFeatureById" stored query defined in the WFS 2.0 standard

### title

The title of the stored query. The title can be expressed in several languages

### abstract

A more detailed description of the stored query. The description can be written in several languages

### template

TODO: Explain _why_ is this required and _how_ does it affect when using the stored query.

A [CTPP](http://ctpp.havoc.ru/en/)  template used by the stored query handler.

Possible values are filenames in `/etc/smartmet/plugins/wfs/templates`

Available values and their description:

Value                            | Description
---------------------------------|-------------------------------------------------
`something`                      | TODO: How does this affect the stored query's behavior? Why would somebody use it?
`something_else`                 | TODO: How does this affect the stored query's behavior? Why would somebody use it?
`etc`                            | ...

### defaultLanguage

The default language to be used in descriptions. Notice that this attribute is mandatory if string translations are required. The default value is "eng"

### expiresSeconds

The query expiration time in seconds. The default value is `60`

The query should return a response within this time. If there are multiple stored queries in the request then the smallest value is used.

### parameters

The WFS request's _query string fields_ are defined with the `parameters` attribute. It contains an array of the _Param_ structure definitions. 

Include a _Param_ structure for every parameter (query string field) that can be given in the WFS request for a stored query. Notice that the query handlers do not usually use the WFS request parameters directly when executing  a query. The WFS request parameters must be first mapped to the stored query parameters defined in [handler_params](#handler_params).

The following table contains a list of attributes that are supported by the _Param_ structure.

Name            | Type                 |  Description
----------------|----------------------|-------------------
`name`          | string               | Field in the WFS query string eg. the parameter. The name comparison is case insensitive
`title`         | object               | Translated, human readable names for the parameter
`abstract`      | object               | Translated, detailed descriptions of the parameter, including default value and/or limits.
`xmlType`       | string               | Namespaced data type of the parameter used when the request is in the XML format. _See table below for allowed values_
`type`          | string               | Defines how the parameter in handled by the plugin. Can be defined as an array or a single values.
`minOccurs`     | unsigned integer     | The minimum number of the occurrences that are required for the parameter _[What occurrences and where?]_. The default value is `0`
`maxOccurs`     | unsigned integer     | The maximum number of the occurrences that are required for the parameter [Again, what and where?]. The default value is `1`
`lowerLimit`    | as specified by type | The lowest value allowed for the parameter. If the value in the request is lower than the specified value then the parameter value is rejected
`upperLimit`    | as specified by type | The highest value allowed for the parameter. If the value in the request is higher than the specified value then the parameter value is rejected
`conflictsWith` | string[]             | ~~The minimum number of the occurrences that are required for the parameter. The default value is "0"~~ [ed. This can't be right]

#### xmlType

The value of this entry is used:

* to describe the type of parameter in response to DescribeStoredQueries request
* to extract parameter value or values when reading XML format query

The latter case is a bit rare, since requests are usually done with HTTP querystrings.

From source code comments: Currently namespace specification is ignored and only the name is matched.

But also:

Parameter XML data type must be specified in mandatory configuration entry
xmlType.

Fully qualified name must be specified (namespace prefix must be provided according to ones used
DescribeStoredQueries CTTP2 tempalte

Following XML namespaces correspond to namespace prefixes

* xsi http://www.w3.org/2001/XMLSchema-instance
* gml http://www.opengis.net/gml/3.2

Possible values for `xmlType`

Value                    | Description
-------------------------|------------
`gml:doubleList`         | A space separated list of of floating point values
`gml:Envelope`           | gml:EnvelopeType
`gml:NameList`           | A space separated list of names
`gml:integerList`        | A space separated list of integer values
`gml:pos`                | gml:DirectPositionType
`xsi:byte`               | -128 .. 127
`xsi:dateTime`           | ISO time format
`xsi:double`             | ???
`xsi:float`              | ???
`xsi:int`                | -2147483648 .. 2147483647
`xsi:integer`            | -9223372036854775808 .. 9223372036854775807
`xsi:language`           | A language code
`xsi:long`               | -9223372036854775808 .. 9223372036854775807
`xsi:negativeInteger`    | -9223372036854775808 .. -1
`xsi:nonNegativeInteger` | 0 .. 18446744073709551615
`xsi:nonPositiveInteger` | -9223372036854775808 .. 0
`xsi:positiveInteger`    | 0 .. 18446744073709551615
`xsi:short`              | -32768 ..32767
`xsi:string`             | ???
`xsi:unsignedByte`       | 0 .. 255
`xsi:unsignedInt`        | 0 .. 4294967295
`xsi:unsignedInteger`    | 0 .. 18446744073709551615
`xsi:unsignedLong`       | 0 .. 18446744073709551615
`xsi:unsignedShort`      | 0 .. 65535

#### type

The internal definition of what kind of parameter is in use.

Possible values for `type`

Value     | Description
----------|------------
`bbox`    | Bounding box area
`bool`    | Boolean
`double`  | Double
`int`     | Integer
`point`   | Location as (x, y, crs)
`string`  | String
`time`    | Date and time related things
`uint`    | Unsigned integer

Same as previous, but as a fixed size array

Value          | Description
---------------|------------
`bbox[size]`   | ???
`bool[size]`   | ???
`double[size]` | ???
`int[size]`    | ???
`point[size]`  | ???
`string[size]` | ???
`time[size]`   | ???
`uint[size]`   | ???

Same as previous, but as a variable sized array

Value                      | Description
---------------------------|------------
`bbox[minsize..maxsize]`   | ???
`bool[minsize..maxsize]`   | ???
`double[minsize..maxsize]` | ???
`int[minsize..maxsize]`    | ???
`point[minsize..maxsize]`  | ???
`string[minsize..maxsize]` | ???
`time[minsize..maxsize]`   | ???
`uint[minsize..maxsize]`   | ???

### named_params

The `named_params` can be used to define items that can be used in `handler_params` as default values.

In the following example Temperature, WindDirection and WindSpeedMS are defined. Should the WFS request contain no value for `parameters` the default values from `named_params` would be used in the `handler_params`.

```text
named_params = (
  {
    name = "defaultThings";
    def = ["Temperature","WindDirection","WindSpeedMS"];
  }
);

parameters:
(
{
  name = "thingsFromURL";
  xmlType = "NameList";
  type = "string[1..99]";
}
);

handler_params:
{
  param = ["${thingsFromURL > defaultThings}"]
}
```

When `parameters` attribute defines items available in WFS request and `handler_params` attribute defines what is passed on to the Query Handler, the `named_params` stands in between those two. The `named_params` can be used in the same way in the configuration file as `parameters` when mapped to the actual Query Handler in `handler_params`. The named parameters are defined with the `named_params` attribute which contains an array of the _NamedParam_ structures. A typical named parameter definition contains just the name of the parameter and its value definition.

The _NamedParam_ structure contains the following attributes:

Name             | Type             | Description
-----------------|------------------|-------------
`name`           | string           | The name of the parameter
`def`            | scalar\|scalar[] | The definition of the value of scalar parameter or of array parameter items
`min_size`       | integer          | The lower permitted size of the array. The default value is "0"
`max_size`       | integer          | The upper permitted size of the array
`allow_optional` | boolean          | This attribute specifies whether the parameter is optional
`register`       | boolean          | This attribute specifies whether the parameter is registered as a query handler parameter that  are usually defined in the _HandlerParam_ structure). Current implementation registers it as a string parameter and it is required to map to query string parameters if such mapping is being used. No other types are currently supported. The default value is `false`

### returnTypeNames

TODO: Description here...

### handler_params

Usually the query handlers (defined in [constructor_name](#constructor_name)) do not directly use the query string values coming in the WFS requests. The parameters from WFS request and the named parameters are mapped to the stored query parameters defined by the `handler_params` attribute, which is defined as the _HandlerParam_ structure. This structure defines all the parameters needed by the query handler in order to execute the requested query.

The main idea of the _HandlerParam_ structure is that it  can contain information that was received in the WFS request as well as some additional information. This additional information can be delivered to the query handler in the same way as the other query parameters. The parameters defined in the _HandlerParam_ structure depends on the query handler used  as  different query handlers use different parameters.

Actual information about parameters used by different constructors can be found using WFS admin request:

* /wfs/admin?request=constructors[&format=(json|html)]

Some notes about this feature:

* Default format is HTML.
* Only information about constructors, that are used in at least one stored query is available
* /wfs/admin requests are not advertised to frontend and as result are available only directly from backend

The _HandlerParam_ structure supports two basic parameter types: 1) scalar and 2) array. If the parameter type is "scalar" then the parameter has only one value. If the parameter type is "array" then the parameter value is actually an array of values. Notice that the length of this array might be zero i.e. in this case there are no values in the array. The array index starts from 0. So, the second value has index 1, the third value has index 2, and so on.

The value of a query parameter can be defined as 1) a constant value 2) a value mapped from a WFS request parameter or 3) a value mapped from a named parameter

#### Query parameters with constant values

The simplest way to define the parameters for the query handler is to use constant values. The example below demonstrates how to define query parameters with constant values.

```javascript
handler_params:
{
  beginValue = 100;
  title = "My string title";
  sum = 123.45;
  myIntArray = [1, 2, 3, 4];
  myStringArray = ["Mon", "Tue", "Wed"];
  myFloatArray = [1.00, 1.25, 2.00, 2.13];
};
```

#### Query parameters mapped from the WFS request parameters

It is possible to define queries which do not need any parameters delivered in the WFS request. For example, we could have a query that returns all the city names in Finland. In this kind of query we can define all required query parameters in the configuration file and no parameters need to be delivered in the request.

The query parameters can be added to the stored query request. For example, we could have a query that returns all the city names in a given country. In this case, the country name / identifier needs to be delivered as a request parameter. In practice, the request parameters are mapped to the query parameters, which are delivered to the correct query handler. The query handler uses these query parameters for executing the query.

The example below demonstrates how the WFS request parameters are mapped to the query parameters by the query handler.

```javascript
handler_params:
{
  myBegin = "${begin}";
  myEnd = "${end:now}";
  myWidth = "${width:100}";
  myHeight = "${height}";
  myVal = "${val[2]:222}";
  myArray = "[${val[2]}, ${val [4]}, ${val[7]}]";
  myDebug = "%{debug}";
};
```

The value from WFS request parameter `begin` is set as the value for the query parameter `myBegin`.

If available the value from WFS request parameter `end` is mapped to the query parameter `myEnd`, default value being string `now`.

If available the value from WFS request parameter `width` is mapped to the query parameter `myWidth`, default value being integer`100`.

The value from WFS request parameter `height` is mapped to the query parameter `myHeight`.

The third element (index origin = 0) of the WFS request array parameter `val` is mapped to the query parameter `myVal` and its default value is `222`.

The array elements (indexes = 2, 4, 7) in the WFS request parameter `val` are mapped as values for the query parameter `myArray` array.

The value from WFS request parameter `debug` is mapped to the query parameter `myDebug` without the request parameter name checking.

It is also possible to define a default value for the query parameter. This value is  used if the request does not contain the parameter that was used in the mapping.

When using `$` character, the request parameter _must_ be defined in the configuration file as a request parameter. If the parameter cannot be found then the situation is seen as a fatal configuration error.

Notice that the last parameter mapping uses `%` character instead of `$` character. This means that the _name of the mapped request parameter does not need to be defined as a request parameter in the configuration file_ (See parameter's "attribute"). This makes it possible to use undefined parameters in the WFS request, which might be useful when the stored queries are developed or debugged.

#### Query parameters mapped from the named_params

The parameter definitions in the _HandlerParam_ structure in the `handler_params` section can also contain references to the _named parameters_ that are defined in the _NamedParam_ structures in the `named_params` section in the configuration file.

Use character `>` for referring to a `named_params` item.

```javascript
named_params :
(
{
  name = "defaultBoundingBox";
  def = "20.0, 60.0, 30.0, 70.0, EPSG::4326";
}
);

handler_params :
{
  myBBox = "${bbox > defaultBoundingBox}";
};
```

The value of WFS request parameter `bbox` is mapped to the query parameter `myBBox`.
If the `bbox`is not defined in the WFS request the value of `defaultBoundingBox` is used.
