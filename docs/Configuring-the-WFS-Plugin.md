# Configuring the WFS Plugin for SmartMet Server

In order to use the WFS plugin we need to edit _two_ configuration files. These files are the main configuration file of the SmartMet Server environment and the WFS plugin specific configuration file.

* [SmartMet Server's main configuration file](#smartmet-server's-main-configuration-file)
* [WFS plugin's configuration file](#wfs-plugin's-configuration-file)
  * [CRS](#CRS)
  * [featuresDir](#featuresDir)

## SmartMet Server's main configuration file

Note: this documentation describes the configuration of WFS plugin and further details of the main configuration file is out of its scope.

The main configuration file is by default named as `smartmet.conf` (configuration may vary depending on installation). In this configuration file is defined which plugins and engines are in use and which configuration files these plugins and engines are using.

In order to use the WFS plugin edit SmartMet Server's main configuration file as follows.

Add required engines in the `engines` attribute. Required engines can be checked from the `.spec` file in source-code.

Notice that the `geoengine` entry must be before the `observation` entry because of some linking reasons such as `observation` uses `geoengine`'s functions.

These engines must also be installed on the system.

```javascript
engines = {
    querydata: {configfile = "engines/querydata.conf";};
    geoengine: {geonames = "engines/geonames.conf";};
    observation: {configfile = "engines/observation.conf";};
    gis: {configfile = "engines/gis.conf";};
    contour: {configfile = "engines/contour.conf";};
};
```

Add a dedicated configuration entry for the WFS plugin in the main configuration file `smartmet.conf` and define file path of the WFS plugin configuration file.

```javascript
plugins: {
    wfs: {
        configfile = "plugins/wfs.conf";
    };
}
```

## WFS plugin's configuration file

The filename and the location of the WFS configuration file is defined in the [main configuration file](#smartmet-server's-main-configuration-file).

A typical WFS configuration file looks like this:

```javascript
url = "/wfs";
languages = [ "eng", "fin" ];
debugLevel = 0;
storedQueryRestrictions = true;
storedQueryConfigDirs = [
  "/etc/smartmet/plugins/wfs/opendata_stored_queries",
  "/etc/smartmet/plugins/wfs/commercial_stored_queries"
];
storedQueryTemplateDir = "/etc/smartmet/plugins/wfs/templates";
xmlGrammarPoolDump = "/etc/smartmet/plugins/wfs/XMLGrammarPool.dump";
cacheSize = 1000;
cacheTimeConstant = 60;
validateXmlOutput = false;
enableDemoQueries = false;
featuresDir = "/etc/smartmet/plugins/wfs/features";
crsDefinitionDir = "/etc/smartmet/plugins/wfs/crs";
geoserverConnStr = "dbname=xxxxxx  user=xxxx password=xxxxxxxxxx host=demohost.fmi.fi";
getCapabilitiesTemplate = "capabilities.c2t";
listStoredQueriesTemplate = "list_stored_queries.c2t";
describeStoredQueriesTemplate = "describe_stored_queries.c2t";
featureTypeTemplate = "feature_type.c2t";
exceptionTemplate = "exception.c2t";
ctppDumpTemplate = "hash_dump_html.c2t";

CRS : (
{
  name  = "WGS84";
  epsg = 4326;
  swapCoord = true;
  projEpochUri = "http://xml.fmi.fi/gml/crs/compoundCRS.php?crs=4326&amp;time=unixtime";
  projUri = "http://www.opengis.net/def/crs/EPSG/0/4326";
},
{
  name = "WGS 84 / Pseudo-Mercator";
  epsg = 3857;
  projEpochUri = "http://xml.fmi.fi/gml/crs/compoundCRS.php?crs=3857&amp;time=unixtime";
  projUri = "http://www.opengis.net/def/crs/EPSG/0/3857";
}
);

typename-storedquery-mapping : (
{
  type_name = "wp:DrizzlePrecipitation";
  stored_queries = "fmi::forecast::ecmwf::winterweather::drizzleprecipitation";
},
{
  type_name = "wp:RainPrecipitation";
  stored_queries = "fmi::forecast::ecmwf::winterweather::rainprecipitation";
},
{
  stored_queries = "fmi::forecast::ecmwf::winterweather::sleetprecipitation";
},
{
  type_name = "wp:SnowPrecipitation";
  stored_queries = "fmi::forecast::ecmwf::winterweather::snowprecipitation";
},
{
  type_name = "wp:WinterWeatherGeneralContours";
  stored_queries = "fmi::forecast::ecmwf::winterweather::general::contours";
}
);
```

The table below describes the configuration attributes that can be  used in the configuration file.

Plugin's configuration file

| Attribute                       | Type     | Use |Description  |
|---------------------------------|----------|-----|-------------|
| `url`                           | string   | optional   | The URL (or actually the URL path) of the plugin (for example `/wfs`). |
| `languages`                     | string[] | mandatory  | An array of the supported languages ("eng", "fin", "swe", etc.). The first language in the array is used as the default language. At least 1 language must be specified. Language can be changed by adding its name to URL after the plugin name so that `http://smartmet/wfs?` becomes for example `http://smartmet/wfs/swe?`. At least one language must be specifies. The first specified language is the default one. |
| `storedQueryConfigDirs`         | string[] | mandatory  | An array of directories where the stored query configuration files are searched. All "*.conf" files in these directories are read and interpreted as a stored query configuration. If these configuration files contains errors (parsing fails, mandatory parameters are missing, etc.) then the SmartMet server does not start.|
| `storedQueryTemplateDir`        | string   | mandatory  | The directory where the [CTPP](http://ctpp.havoc.ru/en/) templates are searched. This directory is used for looking up the CTPP template files when the relative path of the template files is specified.|
| `storedQueryRestrictions`       | boolean  | optional   | This attribute defines whether the stored query restrictions (e.g. check time interval, max stations in result) should be used. The default is true.|
| [`featuresDir`](#featuresDir)   | string   |            | The directory of the features description files.|
| `xmlGrammarPoolDump`            | string[] | mandatory  | The directory for the Xerces-C grammar pool dump file. This file can be generated by the helper program included with the WFS plugin. The WFS plugin loads the XML grammar pool in order to validate incoming requests and also outgoing responses (if enabled). Minimal array size 1. One can also provide single value. |
| `serializedXmlSchemas`          | string   | optional   | The file containing serialized XML schemas. The schemas read from this file are used when looking up XML shemas according to its location URI. Usually schema data should already be in the grammar pool (see xmlGrammarPoolDump). In RHEL6 (Xerces-C 3.0.0) the grammar pool seems to be sufficient. It looks however that it is not sufficient in case of newer Xerces-C versions (like 3.1.1 in Ubuntu)|
| `getFeatureById`                | string   | optional   | Specifies the stored query ID for the mandatory stored query "GetFeatureById". The default value is "urn:ogc:def:query:OGC-WFS::GetFeatureById".|
| `validateXmlOutput`             | boolean  | optional   | Specifies whether to perform validation of generated XML responses. The default is false (do not validate). It is recommended to use this option for testing only to ensure that correct XML responses are generated. Note that XML format WFS requests are validated always and their validation cannot be turned off. This option is enabled when running WFS plugin tests.  |
| `failOnValidateErrors`          | boolean  | optional   | Specifies whether to return failure in case of XML output validation errors. Default value is false. This option is enabled when running WFS plugin tests. |
| `getCapabilitiesTemplate`       | string   | mandatory  | Specifies the CTPP template for the "GetCapabilities" WFS request.|
| `listStoredQueriesTemplate`     | string   | mandatory  | Specifies the CTPP  template for the "ListStoredQueries" WFS request.|
| `describeStoredQueriesTemplate` | string   | mandatory  | Specifies the CTPP  template for the "DescribeStoredQueries" WFS request.|
| `featureTypeTemplate`           | string   | mandatory  | Specifies the CTPP  template for the "DescribeFeatureType"  WFS request.|
| `exceptionTemplate`             | string   | mandatory  | Specifies the CTPP template for the error response.|
| `cttpDumpTemplate`              | string   | mandatory  | Specifies the CTPP  template for the CTPP dump.|
| `enableDemoQueries`             | boolean  | optional   | Enable stored queries which have configuration parameter demo equal to true. The default value is false: ignore demo queries. Some of possible uses of demo queries are: creating some queries for testing purpose and dumping the entire contents of template preprocessor hash.|
| `enableTestQueries`             | boolean  | optional   | Enable stored queries which have configuration parameter test equal to true. The default value is false: ignore test queries|
| `enableConfigurationPolling`    | boolean  | optional   | Enable stored query configuration file polling. If enabled, changes made to a stored query configuration file is taken into production on runtime. Changes to the `id` value of a stored query require a server restart. The default value is false.|
| `geoserverConnString`           | string   | optional   | String which describes database connection to Geoserver. Format is the following: "dbname=open_data_gis user=open_data_gis_user password=THE_PASSWORD host=THE_HOST" (note: actual user password and database host name for use). Default value "" means - no connection |
| `cacheSize`                     | integer  | optional   | The number of query responses to keep in memory for faster response. The default value is 100.|
| `defaultExpiresSeconds`         | integer  | optional   | Specifies default expires seconds for the Expires header field of a requests response. This value is override by a value defined in a request configuration e.g. stored query configuration. The value is supposed to use if no other value is available. Some requests use this as a default value, so the value should be select wisely.  Default value is 60 seconds.|
| `cacheTimeConstant`             | integer  | optional   | The query response cache time constant as seconds. The default value is 60.|
| `locale`                        | string   | optional   | The locale to use. WFS plugin tries to find locale from system when not specified. |
| `silence_init_warnings`         | boolean  | optional   | Specifies whether to hide warning and lover priority messages from stored query initialization. The default value is false - do not hide. |
| `case_sensitive_params`         | boolean  | optional   | Specifies whether to treate parameter names submitted to stored query as case sensitive. The default value is false. |
| `httpProxy`                     | string   | optional   | Specifies HTTP proxy to use for XML schema download when performing XML schema validation. The default valus is "" - no proxy. XML style requests are always validated, response validation os optional.  Normally file specified in parameter xmlGrammarPoolDump should be sufficient for validation. |
| `noProxy`                       | string   | optional   | Specifies addresses to connect directly for XML schema download when performing XML schema download. Rge deafult value is "". XML style requests are always validated, response validation os optional.  Normally file specified in parameter xmlGrammarPoolDump should be sufficient for validation. |
| `fallbackEncoding`              | boolean  | optional   | ???. Default value is false |


### featuresDir

Feature description files directory

According to the WFS 2.0 standard, the WFS interface must support the _GetCapabilities_ request. The response for this request tells what kind of capabilities the current WFS interface implementation has. In this case the WFS plugin is the implementation and it should send the response to the caller.

The WFS plugin features are described in the feature descriptions files. The directory of these files  is defined with the __featuresDir__ attribute in the plugin's configuration file.

A typical feature description file looks like this:

```javascript
name = "wp:RainPrecipitation";
xmlType = "RainPrecipitation";
xmlNamespace = "http://xml.fmi.fi/namespace/aviation-weather/special-products/2015/09/29";
xmlNamespaceLoc = "http://xml.fmi.fi/schema/aviation-weather/special-products/2015/09/29/WinterPrecipitation.xsd";

title:
{
  eng = "Winter weather index probabilities";
  fin = "Talvisäändeksin todennäisyydet";
};

abstract:
{
  eng = "Winter weather index probabilities.";
  fin = "Talvisääindeksin todennäkösyydet.";
};

defaultCRS =  "EPSG:4326";
otherCRS = [];
````

The feature description file can contain the following attributes:

Feature description file

| Attribute         | Type      | Required | Description |
|-------------------|-----------|----------|-------------|
| `name`            | string    | yes      | The internal name of the feature. This name is usually referred from the "returnTypeNames" attribute used in stored query configuration files |
| `xmlType`         | string    | yes      | The XML type (top element name) of the returned element. The type is expressed without the namespace or the namespace prefix |
| `xmlNamespace`    | string    | yes      | The XML namespace of the returned element type |
| `xmlNamespaceLoc` | string    | no       | The XML namespace location |
| `title`           | object    | yes      | The title of the feature with translations |
| `abstract`        | object    | yes      | A detailed description of the feature with translations |
| `defaultCRS`      | string    | yes      | The default coordinate projection |
| `otherCRS`        | string[]  | no       | An array of available coordinate projections |
| `boundingBox`     | double[4] | no       | The default coordinate bounding box |
