# Configuring the WFS Plugin for SmartMet Server

In order to use the WFS plugin we need to edit _two_ configuration files. These files are the main configuration file of the SmartMet Server environment and the WFS plugin specific configuration file.

* [SmartMet Server's main configuration file](#smartmet-server's-main-configuration-file)
* [WFS plugin's configuration file](#wfs-plugin's-configuration-file)
  * [featuresDir](#featuresDir)
  * [Administrative requests](#administrative-requests)

## SmartMet Server's main configuration file

Note: this documentation describes the configuration of WFS plugin and further details of the main configuration file is out of its scope.

The main configuration file is by default named as `smartmet.conf` (configuration may vary depending on installation). In this configuration file is defined which plugins and engines are in use and which configuration files these plugins and engines are using.

In order to use the WFS plugin edit SmartMet Server's main configuration file as follows.

Add required engines in the `engines` attribute. Required engines can be checked from the `.spec` file in source-code.
The plugin itself requires the `gis` engine (the coordinate reference systems known to the plugin come from its CRS registry);
the stored query handlers use the `querydata`, `geonames`, `observation`, `contour` and `grid` engines.

These engines must also be installed on the system.

```javascript
engines = {
    querydata: {configfile = "engines/querydata.conf";};
    geonames: {configfile = "engines/geonames.conf";};
    observation: {configfile = "engines/observation.conf";};
    gis: {configfile = "engines/gis.conf";};
    contour: {configfile = "engines/contour.conf";};
    grid: {configfile = "engines/grid.conf";};
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
storedQueryTemplateDir = "/usr/share/smartmet/wfs";
xmlGrammarPoolDump = "/usr/share/smartmet/wfs/XMLGrammarPool.dump";
serializedXmlSchemas = "/usr/share/smartmet/wfs/XMLSchemas.cache";
cacheSize = 1000;
cacheTimeConstant = 60;
validateXmlOutput = false;
enableDemoQueries = false;
featuresDir = "/etc/smartmet/plugins/wfs/features";
capabilitiesConfig = "/etc/smartmet/plugins/wfs/capabilities.conf";
geoserverConnStr = "dbname=xxxxxx  user=xxxx password=xxxxxxxxxx host=xxxxxx";
getCapabilitiesTemplate = "capabilities.c2t";
listStoredQueriesTemplate = "list_stored_queries.c2t";
describeStoredQueriesTemplate = "describe_stored_queries.c2t";
featureTypeTemplate = "feature_type.c2t";
exceptionTemplate = "exception.c2t";
ctppDumpTemplate = "hash_dump_html.c2t";

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
  type_name = "wp:SleetPrecipitation";
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

The coordinate reference systems are not defined in this file: they are read by the GIS engine from the
directory given by its `crsDefinitionDir` setting.

The table below describes the configuration attributes that can be  used in the configuration file.

Plugin's configuration file

| Attribute                       | Type     | Use |Description  |
|---------------------------------|----------|-----|-------------|
| `url`                           | string   | optional   | The URL (or actually the URL path) of the plugin. The default is `/wfs`. Language specific handlers are registered as `<url>/<language>` for each language in `languages`. |
| `languages`                     | string[] | mandatory  | An array of the supported languages ("eng", "fin", "swe", etc.). The first language in the array is used as the default language. At least 1 language must be specified. Language can be changed by adding its name to URL after the plugin name so that `http://smartmet/wfs?` becomes for example `http://smartmet/wfs/swe?`. At least one language must be specifies. The first specified language is the default one. |
| `storedQueryConfigDirs`         | string[] | mandatory  | An array of directories where the stored query configuration files are searched. All "*.conf" files in these directories are read and interpreted as a stored query configuration. If these configuration files contains errors (parsing fails, mandatory parameters are missing, etc.) then the SmartMet server does not start.|
| `storedQueryTemplateDir`        | string   | optional   | The directory where the [CTPP](http://ctpp.havoc.ru/en/) templates are searched. This directory is used for looking up the CTPP template files when the relative path of the template files is specified. The default is `/usr/share/smartmet/wfs`, where the RPM package installs the compiled templates.|
| `storedQueryRestrictions`       | boolean  | optional   | This attribute defines whether the stored query restrictions (e.g. check time interval, max stations in result) should be used. The default is true.|
| [`featuresDir`](#featuresDir)   | string   | mandatory  | The directory of the features description files.|
| `capabilitiesConfig`            | string or group | optional | The service metadata of the _GetCapabilities_ response (`title`, `abstract`, `keywords`, `fees`, `providerName`, `providerSite`, `phone`, `address`, `accessConstraints`, `onlineResource`, `contactInstructions` and `outputFormats`), either as a group or as the name of a separate file containing these settings (see `cnf/capabilities.conf`). The strings can be translated.|
| `xmlGrammarPoolDump`            | string[] | mandatory  | The path of the Xerces-C grammar pool dump file. The RPM package installs it as `/usr/share/smartmet/wfs/XMLGrammarPool.dump`. If an array is given, the first file that exists is used. The WFS plugin loads the XML grammar pool in order to validate incoming requests and also outgoing responses (if enabled). |
| `serializedXmlSchemas`          | string   | optional   | The file containing serialized XML schemas (installed as `/usr/share/smartmet/wfs/XMLSchemas.cache`). The schemas read from this file are used when looking up XML shemas according to its location URI. Usually schema data should already be in the grammar pool (see xmlGrammarPoolDump), but with newer Xerces-C versions the grammar pool alone may not be sufficient.|
| `getFeatureById`                | string   | optional   | Specifies the stored query ID for the mandatory stored query "GetFeatureById". The default value is "urn:ogc:def:query:OGC-WFS::GetFeatureById".|
| `validateXmlOutput`             | boolean  | optional   | Specifies whether to perform validation of generated XML responses. The default is false (do not validate). It is recommended to use this option for testing only to ensure that correct XML responses are generated. Note that XML format WFS requests are validated always and their validation cannot be turned off. This option is enabled when running WFS plugin tests.  |
| `failOnValidateErrors`          | boolean  | optional   | Specifies whether to return failure in case of XML output validation errors. Default value is false. This option is enabled when running WFS plugin tests. |
| `getCapabilitiesTemplate`       | string   | mandatory  | Specifies the CTPP template for the "GetCapabilities" WFS request.|
| `listStoredQueriesTemplate`     | string   | mandatory  | Specifies the CTPP  template for the "ListStoredQueries" WFS request.|
| `describeStoredQueriesTemplate` | string   | mandatory  | Specifies the CTPP  template for the "DescribeStoredQueries" WFS request.|
| `featureTypeTemplate`           | string   | mandatory  | Specifies the CTPP  template for the "DescribeFeatureType"  WFS request.|
| `exceptionTemplate`             | string   | mandatory  | Specifies the CTPP template for the error response.|
| `ctppDumpTemplate`              | string   | mandatory  | Specifies the CTPP template for the CTPP dump.|
| `enableDemoQueries`             | boolean  | optional   | Enable stored queries which have configuration parameter demo equal to true. The default value is false: ignore demo queries. Some of possible uses of demo queries are: creating some queries for testing purpose and dumping the entire contents of template preprocessor hash.|
| `enableTestQueries`             | boolean  | optional   | Enable stored queries which have configuration parameter test equal to true. The default value is false: ignore test queries|
| `enableConfigurationPolling`    | boolean  | optional   | Enable stored query configuration file polling. If enabled, changes made to a stored query configuration file is taken into production on runtime. Changes to the `id` value of a stored query require a server restart. The default value is false.|
| `geoserverConnStr`              | string   | optional   | String which describes database connection to Geoserver. Format is the following: "dbname=THE_DATABASE user=THE_USER password=THE_PASSWORD host=THE_HOST" (note: actual user password and database host name for use). Default value "" means - no connection |
| `cacheSize`                     | integer  | optional   | The number of query responses to keep in memory for faster response. The default value is 100.|
| `defaultExpiresSeconds`         | integer  | optional   | Specifies default expires seconds for the Expires header field of a requests response. This value is override by a value defined in a request configuration e.g. stored query configuration. The value is supposed to use if no other value is available. Some requests use this as a default value, so the value should be select wisely.  Default value is 60 seconds.|
| `cacheTimeConstant`             | integer  | optional   | The query response cache time constant as seconds. The default value is 60. The value is read but currently not used.|
| `locale`                        | string   | optional   | The locale to use. WFS plugin tries to find locale from system when not specified. |
| `silence_init_warnings`         | boolean  | optional   | Specifies whether to hide warning and lover priority messages from stored query initialization. The default value is false - do not hide. |
| `case_sensitive_params`         | boolean  | optional   | Specifies whether to treate parameter names submitted to stored query as case sensitive. The default value is false. |
| `httpProxy`                     | string   | optional   | Specifies HTTP proxy to use for XML schema download when performing XML schema validation. The default valus is "" - no proxy. XML style requests are always validated, response validation os optional.  Normally file specified in parameter xmlGrammarPoolDump should be sufficient for validation. |
| `noProxy`                       | string   | optional   | Specifies addresses to connect directly for XML schema download when performing XML schema download. Rge deafult value is "". XML style requests are always validated, response validation os optional.  Normally file specified in parameter xmlGrammarPoolDump should be sufficient for validation. |
| `fallbackEncoding`              | string or group | optional | The character encoding of location names in requests which are not valid UTF-8. The value can be given per language. The default is `ISO-8859-1`. |
| `parallelInit`                  | integer  | optional   | A non-zero value initializes the stored query handlers in separate threads. The default is 0. |
| `debugLevel`                    | integer  | optional   | The amount of diagnostic output written to the server log during stored query initialization and request processing. The default is 1. |
| `fallback_hostname`             | string   | optional   | The host name used in the _GetCapabilities_ and _DescribeFeatureType_ responses when it cannot be determined from the request. The default is `localhost`. |
| `fallback_protocol`             | string   | optional   | The protocol used in the _GetCapabilities_ and _DescribeFeatureType_ responses when it cannot be determined from the request. The default is `http`. |
| `primaryForecastSource`         | string   | optional   | The default data source of the contour (isoline and coverage) stored queries: `grid` uses the grid engine, other values the querydata engine. A request can override it with the `source` parameter. The default is `querydata`. |
| `gridengine_disabled`           | boolean  | optional   | Disable the use of the grid engine by the contour stored queries even if `grid` is requested as the data source. The default is false. |
| `lockedTimeStamp`               | string   | optional   | A fixed "current time" used for all requests. Intended for testing only (the plugin tests use it to get reproducible responses). |
| `hosts`                         | list     | optional   | Host specific settings for the download links of the atom (download) stored queries. Each item contains `server` (mandatory, the requested host name), `wms` (the host name of the WMS service, default `wms.fmi.fi`) and `keep_apikey` (whether to keep the API key in the generated links, default true). |
| `typename-storedquery-mapping`  | list     | optional   | Maps feature type names to stored queries for ad hoc _GetFeature_ and _GetPropertyValue_ queries. Each item contains `type_name` and `stored_queries`; several stored query identifiers are separated with `;`. Items missing either setting are ignored. |


### Administrative requests

The plugin registers the following server administration requests (served at the admin URI of the server, for example `/admin?what=wfs:constructors`):

| Request                | Description |
|------------------------|-------------|
| `wfs:constructors`     | Lists the stored query handler factories (`constructor_name` values) in use, the stored queries using them and their handler parameters. Optional parameters: `format` (`html`, the default, or `json`) and `handler`. |
| `wfs:xmlSchemaCache`   | Dumps the XML schema cache. |
| `wfs:reload`           | Reloads the WFS plugin configuration. Requires authentication. |

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
| `boundingBox`     | double[4] | no       | The default coordinate bounding box. The default is `[-180, -90, 180, 90]` |
| `hidden`          | boolean   | no       | Hide the feature type from the service metadata. The default is false |
