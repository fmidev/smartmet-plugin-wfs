#!/bin/bash
OUTDIR="./test-msgs/"
INPUTDIR="./filters/"

echo "Output dir is "
echo $OUTDIR

rm -f $OUTDIR*

OUTPUTFILE="$OUTDIR"nothing.get

echo "get propertyvalue storedquery kvp to $OUTPUTFILE"

./formatter \
-s fmi::observations::weather::timevaluepair \
-v /wfs:FeatureCollection/wfs:member/omso:PointTimeSeriesObservation/om:featureOfInterest \
-o $OUTPUTFILE

OUTPUTFILE="$OUTDIR"weather-timevaluepair.get

echo "get propertyvalue storedquery kvp to $OUTPUTFILE"

./formatter \
-s fmi::observations::weather::timevaluepair \
-q starttime=2013-05-16T18:00:00Z \
-q endtime=2013-05-17T00:00:00Z \
-q maxlocations=2 \
-q parameters=t2m \
-q place=Kumpula,Helsinki \
-v /wfs:FeatureCollection/wfs:member/omso:PointTimeSeriesObservation/om:featureOfInterest \
-o $OUTPUTFILE

# NOTE: Since stored query needs parameters, this result will be empty
INPUTFILE="$INPUTDIR"filsu-gpv-2013-kumpula.xml
OUTPUTFILE="$OUTDIR"filsu-bbox-gpv.get

echo "get propertyvalue filter $INPUTFILE to $OUTPUTFILE"

./formatter \
-i $INPUTFILE \
-t omso:PointTimeSeriesObservation \
-v /wfs:FeatureCollection/wfs:member/omso:PointTimeSeriesObservation/om:featureOfInterest \
-o $OUTPUTFILE

echo "GetFeature-files"

OUTPUTFILE="$OUTDIR"surface.get
echo "get feature BBOX to $OUTPUTFILE"

./formatter \
-t avi:VerifiableMessage \
-b 24.00,60.00,25.00,61.00,EPSG::4326 \
-o $OUTPUTFILE

# NOTE: GetFeature with projection and GetPropertyValue looks identical?

OUTPUTFILE="$OUTDIR"projection.get
echo "GetFeature BBOX and projection to $OUTPUTFILE"

./formatter \
-t avi:VerifiableMessage \
-b 24.00,60.00,25.00,61.00,EPSG::4326 \
-p saf:Aerodrome \
-o $OUTPUTFILE

OUTPUTFILE="$OUTDIR"projection-gpv.get
echo "GetPropertyValue BBOX to $OUTPUTFILE"

./formatter \
-t avi:VerifiableMessage \
-b 24.00,60.00,25.00,61.00,EPSG::4326 \
-v //*:Aerodrome \
-o $OUTPUTFILE

INPUTFILE="$INPUTDIR"filsu-bbox-time.xml
OUTPUTFILE="$OUTDIR"filsu-bbox.get

echo "get feature filter: $INPUTFILE to $OUTPUTFILE"

./formatter \
-i $INPUTFILE \
-t avi:VerifiableMessage \
-o $OUTPUTFILE

INPUTFILE="$INPUTDIR"filsu-bbox-time-for-winterweather-query.xml
OUTPUTFILE="$OUTDIR"filsu-bbox-time-for-winterweather-contour-query.get

echo "get feature filter: $INPUTFILE to $OUTPUTFILE"

./formatter \
-i $INPUTFILE \
-t wp:WinterWeatherContours \
-o $OUTPUTFILE

INPUTFILE="$INPUTDIR"filsu-bbox-time-for-winterweather-query.xml
OUTPUTFILE="$OUTDIR"filsu-bbox-time-for-winterweather-general-contour-query.get

echo "get feature filter: $INPUTFILE to $OUTPUTFILE"

./formatter \
-i $INPUTFILE \
-t wp:WinterWeatherGeneralContours \
-o $OUTPUTFILE

INPUTFILE="$INPUTDIR"filsu-bbox-time-for-winterweather-query.xml
OUTPUTFILE="$OUTDIR"filsu-bbox-time-for-winterweather-probabilities-query.get

echo "get feature filter: $INPUTFILE to $OUTPUTFILE"

./formatter \
-i $INPUTFILE \
-t wp:WinterWeatherProbabilities \
-o $OUTPUTFILE


INPUTFILE="$INPUTDIR"filsu-bbox-time-for-coverage-contour-query.xml
OUTPUTFILE="$OUTDIR"filsu-bbox-time-for-coverage-contour-query.get

echo "get feature filter: $INPUTFILE to $OUTPUTFILE"

./formatter \
-i $INPUTFILE \
-t fmicov:CoverageContours \
-o $OUTPUTFILE

INPUTFILE="$INPUTDIR"filsu-bbox-time-for-isoline-contour-query.xml
OUTPUTFILE="$OUTDIR"filsu-bbox-time-for-isoline-contour-query.get

echo "get feature filter: $INPUTFILE to $OUTPUTFILE"

./formatter \
-i $INPUTFILE \
-t fmicov:IsolineContours \
-o $OUTPUTFILE

# NOTE: This works fine, with * namespace.

INPUTFILE="$INPUTDIR"filsu-enontekio.xml
OUTPUTFILE="$OUTDIR"enontekio-gpv.get

echo "get feature filter: $INPUTFILE to $OUTPUTFILE"

./formatter \
-i $INPUTFILE \
-t avi:VerifiableMessage \
-o $OUTPUTFILE \
-v //*:timePosition \
-v //*:name \
-v //*:airTemperature

# NOTE: Previous filtering loses namespaces, so using qualified namespaces usually does not work.
#-v /wfs:member\
#/avi:VerifiableMessage\
#/avi:message\
#/iwxxm:METAR\
#/iwxxm:observation\
#/om:OM_Observation\
#/om:result\
#/iwxxm:MeteorologicalAerodromeObservationRecord\
#/iwxxm:airTemperature

