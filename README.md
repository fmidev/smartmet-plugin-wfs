# WFS interface for SmartMet Server

## Table of Contents

* [SmartMet Server](#SmartMet Server)
* [Introduction](#introduction)
  * [WFS Requests](#wfs-requests)
* [Using the WFS Interface](#using-the-wfs-interface)
  * [WFS Request Types](#wfs-request-types)
  * [Show Available Stored Queries](#show-available-stored-queries)
  * [Show Details for a Stored Query](#show-details-for-a-stored-query)
  * [Making a HTTP Request](#making-a-http-request)
* [Configuring the WFS plugin](#configuring-the-wfs-plugin)
* [Configuring Stored Queries](#configuring-stored-queries)

## SmartMet Server

[SmartMet Server](https://github.com/fmidev/smartmet-server) is a data and product server for MetOcean data. It
provides a high capacity and high availability data and product server
for MetOcean data. The server is written in C++, since 2008 it has
been in operational use by the Finnish Meteorological Institute FMI.

## Introduction

The WFS plugin offers a convenient way to fetch the meteorological, climatological and geographical data  over the Web using the HTTP protocol. The WFS plugin fetches this data from the SmartMet Server engines such as QEngine, ObsEngine, GeoEngine, etc. The data is delivered in the XML format.

The WFS plugin meets the requirements of the [INSPIRE (Infrastructure for spatial information in the European Community)](http://inspire.ec.europa.eu/) compliant interfaces. The following links give the details of the INSPIRE workprogramme, the technical guidance and data specification of INSPIRE.

* [INSPIRE Workprogramme: Data Specifications](http://inspire.ec.europa.eu/index.cfm/pageid/2/list/5)
* [Technical Guidance for the implementation of INSPIRE Download Services (PDF)](http://inspire.ec.europa.eu/documents/Network_Services/Technical_Guidance_Download_Services_v3.1.pdf)
* [Data Specification on Atmospheric Conditions and Meteorological Geographical Features–Technical Guidelines (PDF)](http://inspire.ec.europa.eu/documents/Data_Specifications/INSPIRE_DataSpecification_AC-MF_v3.0.pdf)

The WFS plugin implements the  [Web Feature Service (WFS) interface](http://docs.geoserver.org/latest/en/user/services/wfs/reference.html) specified by [Open Geospatial Consortium, Inc.](http://www.opengeospatial.org). This interface is described in detail in the [Web Feature Service interface standard](http://www.opengeospatial.org/standards/wfs).  

In this document we describe the basic usage and the configuration of the WFS service with SmartMet Server.

## Using the WFS Interface

The WFS interface is located at url `/wfs` (or as defined in the configuration). The requests can be sent to the WFS interface in different ways. The simplest way to send a WFS request is to use an _HTTP GET_ message so that all necessary parameters can be given in the URL. In the WFS standard this type of request is known as a _keyword-value pair (KVP) request_.

Another way to send a request is to use an HTTP POST message so that the actual WFS request is delivered in XML format in the content part of the HTTP message. In this document we use only _KVP_ requests.

The basic steps needed to use the WFS interface are:

* List stored queries provided by the WFS interface
* List which parameters (eg. query string fileds) the stored query supports
* Fetch data using the stored query and parameters

To make a WFS request through the interface define the _request type_ in the query string field `request`.

### WFS Request Types

The WFS interface standard defines the various request types. Not all of them are implemented in the SmartMet Server plugin. Use the _GetCapabilities_ request to list all of the request types supported by the interface.

Most notable request are:

* _GetCapabilities_ (show what can be done with the interface)
* _ListStoredQueries_ (what queries can be performed with GetFeature)
* _DescribeStoredQueries_ (what parameters are availble in a stored query/queries)
* _GetFeature_ (to perform the selected stored query)

```text
http://data.fmi.fi/wfs?request=GetCapabilities
```

The _GetCapabilities_ request returns a lot of information about the capabilities of the current WFS implementation.
The [Web Feature Service interface standard](http://www.opengeospatial.org/standards/wfs) provides more information about the response to this request.

### Show Available Stored Queries

Using the _ListStoredQueries_ request, we can find out the available stored queries in the current WFS interface. The SmartMet Server WFS plugin is specially designed for the _stored queries_ so that large data sets can be treated effectively. 

```text
http://data.fmi.fi/wfs?request=ListStoredQueries
```

The response contains a list of available queries and links to the XML schemas related to these queries.

In the response a `<StoredQuery>` block is returned for each available stored query. Value for the attribute `id` is then used for retrieving the actual data. In the following example the `fmi::forecast::ecmwf::surface::coverage::temperature` is the id for retrieving something titled `Weather parameter coverage as GML-contours`. What this actually does is up to the configuration of this stored query.

```xml
<?xml version="1.0" encoding="UTF-8"?>
<ListStoredQueriesResponse xmlns="http://www.opengis.net/wfs/2.0"
  xmlns:inspire_common="http://inspire.ec.europa.eu/schemas/common/1.0"
  xmlns:ef="http://inspire.ec.europa.eu/schemas/ef/3.0"
  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://www.opengis.net/wfs/2.0 http://schemas.opengis.net/wfs/2.0/wfs.xsd
  http://inspire.ec.europa.eu/schemas/common/1.0 http://inspire.ec.europa.eu/schemas/common/1.0/common.xsd
  http://inspire.ec.europa.eu/schemas/ef/3.0 http://inspire.ec.europa.eu/schemas/ef/3.0/EnvironmentalMonitoringFacilities.xsd">
  <StoredQuery id="fmi::forecast::ecmwf::surface::coverage::temperature">
    <Title>Weather parameter coverage as GML-contours</Title>
    <ReturnFeatureType xmlns:wfstype="http://inspire.ec.europa.eu/schemas/omso/3.0"
    xsi:schemaLocation="http://inspire.ec.europa.eu/schemas/omso/3.0
    http://inspire.ec.europa.eu/schemas/omso/3.0/SpecialisedObservations.xsd">wfstype:GridSeriesObservation
    </ReturnFeatureType>
  </StoredQuery>
  <StoredQuery id="fmi::forecast::ecmwf::surface::coverage::wind">
    <Title>Weather parameter coverage as GML-contours</Title>
    <ReturnFeatureType xmlns:wfstype="http://inspire.ec.europa.eu/schemas/omso/3.0"
          xsi:schemaLocation="http://inspire.ec.europa.eu/schemas/omso/3.0
          http://inspire.ec.europa.eu/schemas/omso/3.0/SpecialisedObservations.xsd">wfstype:GridSeriesObservation
    </ReturnFeatureType>
  </StoredQuery>
  <StoredQuery id="fmi::forecast::ecmwf::surface::isoline::pressure">
    <Title>Pressure isolines as GML-contours</Title>
    <ReturnFeatureType xmlns:wfstype="http://inspire.ec.europa.eu/schemas/omso/3.0"
    xsi:schemaLocation="http://inspire.ec.europa.eu/schemas/omso/3.0
    http://inspire.ec.europa.eu/schemas/omso/3.0/SpecialisedObservations.xsd">wfstype:GridSeriesObservation
    </ReturnFeatureType>
  </StoredQuery>
</ListStoredQueriesResponse>
```

### Show Details for a Stored Query

Using the request _DescribeStoredQueries_, the details of a stored query can be obtained. The example below shows how to find out more information about a stored query with the id `fmi::forecast::ecmwf::surface::coverage::temperature` found in the previous step.

```text
http://data.fmi.fi/wfs?request=DescribeStoredQueries&storedquery_id=fmi::forecast::ecmwf::surface::coverage::temperature
```

The response contains a list of configurable fields that can be used in the query when retrieving the actual data.

Each `<Parameter>` block contains the `name` and the `type` for a setting that can be used to define what data should be returned. There _should_ be a description in `<Title>` and `<Abstract>` blocks how this parameter affects the results of the query when it's done. If the parameter has some default value defined, it _should_ be in the description (but usually is not). Please update the stored query's configuration file accordingly (see [Configuring Stored Queries](docs/Configuring-Stored-Queries.md)).

In this example there is a parameter named `starttime` and it is of type `dateTime`. For example, if the beginning of the requested data would be limited to 1. January 2018, one would add in the query string `/wfs?starttime=2018-01-01T00:00:00Z`. The returning data from WFS then shouldn't contain any timesteps prior to that.

```xml
<?xml version="1.0" encoding="UTF-8"?>
<DescribeStoredQueriesResponse xmlns="http://www.opengis.net/wfs/2.0
http://www.opengis.net/gml/3.2 http://schemas.opengis.net/gml/3.2.1/gml.xsd">
<StoredQueryDescription id="fmi::forecast::ecmwf::surface::coverage::temperature">
  <Title>Weather parameter coverage as GML-contours</Title>
  <Abstract>
    Weather parameter forecast for an area returned in GML-contours. Contours are polygons. Area can be specified as bbox parameter.
  </Abstract>
  <Parameter name="starttime" type="dateTime">
    <Title>Begin of the time interval</Title>
    <Abstract>Parameter begin specifies the begin of time interval in ISO-format (for example 2012-02-27T00:00:00Z).</Abstract>
  </Parameter>
  <Parameter name="origintime" type="dateTime">
    <Title>Analysis time</Title>
    <Abstract>Analysis time specifies the time of analysis in ISO-format (for example 2012-02-27T00:00:00Z).</Abstract>
  </Parameter>
  <Parameter name="endtime" type="dateTime">
    <Title>End of time interval</Title>
    <Abstract>End of time interval in ISO-format (for example 2012-02-27T00:00:00Z).</Abstract>
  </Parameter>
  <Parameter name="timestep" type="int">
    <Title>The time step of data in minutes</Title>
    <Abstract>The time step of data in minutes. Notice that timestep is calculated from start of the ongoing hour or day. </Abstract>
  </Parameter>
  <Parameter name="timesteps" type="int">
    <Title>Number of timesteps</Title>
    <Abstract>Number of timesteps in result set.</Abstract>
  </Parameter>
  <Parameter name="crs" type="xsi:string">
    <Title>Coordinate projection to use in results</Title>
    <Abstract> Coordinate projection to use in results. For example EPSG::3067</Abstract>
  </Parameter>
  <Parameter name="bbox" type="xsi:string">
    <Title>Bounding box of area for which to return data.</Title>
    <Abstract>Bounding box of area for which to return data (lon,lat,lon,lat). For example 21,61,22,62</Abstract>
  </Parameter>
  <Parameter name="timezone" type="xsi:string">
    <Title>Time zone</Title>
    <Abstract>Time zone of the time instant of the data point in the form Area/Location (for example America/Costa_Rica). Default value is UTC.
  </Abstract>
  </Parameter>
  <QueryExpressionText xmlns:wfs_001="http://inspire.ec.europa.eu/schemas/omso/3.0"
  xsi:schemaLocation="http://inspire.ec.europa.eu/schemas/omso/3.0 http://inspire.ec.europa.eu/schemas/omso/3.0/SpecialisedObservations.xsd"
  returnFeatureTypes="wfs_001:GridSeriesObservation"
  language="urn:ogc:def:queryLanguage:OGC-WFS::WFS_QueryExpression"
  isPrivate="true"/>
</StoredQueryDescription>
</DescribeStoredQueriesResponse>
```

### Making a HTTP Request

To get data with a stored query use the _GetFeature_ request with a field `storedquery_id` that has the same value as attribute `id` in the _ListStoredQueries_ response. Set rest of the query string fields as necessary with parameters defined in previous step.

Some of the parameters are mandatory, some are not. Some have a default value, some don't. These are defined in the stored query settings. Please update the title and abstract in the stored query's configuration file accordingly (see [Configuring Stored Queries](docs/Configuring-Stored-Queries.md)).

The example here has

* `request` set to `GetFeature` to tell WFS to fetch data
* `storedquery_id` defines stored query to use as `fmi::forecast::ecmwf::surface::coverage::temperature`
* `bbox` defines area limited by coordinates `21,66` and `22,6`
* `starttime` set to 9th of May 2012 at 09:00 UTC
* `endtime` also set to 9th of May 2012 at 09:00 UTC

The endtime here is the same as starttime, so only one exact moment in time (timestep) is expected in the returning data.

```text
http://data.fmi.fi/wfs?request=GetFeature&storedquery_id=fmi::forecast::ecmwf::surface::coverage::temperature	&bbox=21,66,22,6&starttime=2016-05-09T09:00:00Z&endtime=2016-05-09T09:00:00Z
```

The response could look something like this:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<wfs:FeatureCollection timeStamp="2016-05-12T06:50:57Z" numberMatched="2" numberReturned="2" xmlns:wfs="http://www.opengis.net/wfs/2.0" >
  <!-- One member per time and parameter type -->
  <wfs:member>
    <omso:GridSeriesObservation gml:id="wfs-member-1">
      <om:phenomenonTime>	<gml:TimeInstant gml:id="phenomenon-time-wfs-member-1">
        <gml:timePosition>2016-05-09T09:00:00Z</gml:timePosition> <!-- Data time stamp -->
      </gml:TimeInstant></om:phenomenonTime>
      <om:resultTime> <!-- Model analysis time -->
        <gml:TimeInstant gml:id="result-time-wfs-member-1"><gml:timePosition>2016-05-12T06:50:57Z</gml:timePosition></gml:TimeInstant>
      </om:resultTime>
      <om:procedure xlink:href="http://xml.fmi.fi/inspire/process/hirlam_pressurelevels"/>
      <om:parameter>
        <om:NamedValue>
          <om:name xlink:href="http://inspire.ec.europa.eu/codeList/ProcessParameterValue/value/numericalModel/analysisTime"/>
          <om:value><gml:TimeInstant gml:id="analysis-time-wfs-member-1">
            <gml:timePosition>2016-05-11T12:00:00Z</gml:timePosition>
          </gml:TimeInstant></om:value>
        </om:NamedValue>
      </om:parameter>
      <om:observedProperty xlink:href="http://127.0.0.1:8080/meta?observableProperty=forecast&amp;param=temperature"/>
      <om:featureOfInterest>
        <sams:SF_SpatialSamplingFeature gml:id="spatial-sampling-feature-wfs-member-1">
          <sam:sampledFeature/> <!-- Sampling feature is actually a requested bbox -->
          <sams:shape>
            <gml:Polygon gml:id="spatial-sampling-feature-requested-bounding-box-wfs-member-1">
              <gml:exterior><gml:LinearRing><gml:coordinates>
              61.000000,21.000000 62.000000,21.000000 62.000000,22.000000 61.000000,22.000000 61.000000,21.000000
              </gml:coordinates></gml:LinearRing></gml:exterior>
            </gml:Polygon>
          </sams:shape>
        </sams:SF_SpatialSamplingFeature>
      </om:featureOfInterest>
      <om:result>
        <fmicov:phenomenonArea dateTime="2016-05-09T09:00:00Z" parameter="Temperature" unit="C" low="1" high="9" srsDimension="2"
        gml:id="fmicov-phenomenon-area-wfs-member-1-Temperature" srsName="http://www.opengis.net/def/crs/EPSG/0/4326">
          <gml:surfaceMember>
            <gml:Surface gml:id="fmicov-phenomenon-area-Temperature-wfs-member-1-surface-member-1">
              <gml:patches><gml:PolygonPatch> <!-- exterior ring -->
                <gml:exterior><gml:LinearRing><gml:coordinates>
                61.000000 21.000000,62.000000 21.000000,62.000000 21.224317,62.000000
                21.224317,61.982857 21.238757,61.968421 21.255904,61.927957 21.283869,61.900000
                21.332396,61.885965 21.341893,61.866667 21.355932,61.818391 21.374328,61.800000
                21.413091,61.565714 21.421665,61.500000 21.455960,61.479775 21.376163,61.467857
                21.355932,61.439175 21.316746,61.400000 21.298340,61.363158 21.292757,61.325000
                21.191181,61.146154 21.155877,61.129268 21.126600,61.100000 21.109710,61.000000
                21.110864,61.000000 21.000000
                </gml:coordinates></gml:LinearRing></gml:exterior>
              </gml:PolygonPatch></gml:patches>
            </gml:Surface>
          </gml:surfaceMember>
        </fmicov:phenomenonArea>
      </om:result>
    </omso:GridSeriesObservation>
  </wfs:member>
</wfs:FeatureCollection>
```

## Configuring the WFS plugin

In order to use the WFS plugin we need to edit two configuration files. The main configuration file of the SmartMet Server environment and the WFS plugin specific configuration file.
See page [Configuring the WFS Plugin](docs/Configuring-the-WFS-Plugin.md) for help.

## Configuring Stored Queries

For each stored query a separate configuration file is required.
See page [Configuring Stored Queries](docs/Configuring-Stored-Queries.md) for help.
