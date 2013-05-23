var fs = require('fs');
var addon = require('./build/Release/avro');

var Buffer = require('buffer').Buffer;

var avro = new addon.Avro();

avro.onschema = function(schema){
  console.log("schema 1");
};

avro.onerror = function(error){
  console.log(error);
};

avro.ondatum = function(datum){
  console.log("onDatum",datum);
};

var complexSchema = '{\
  "name": "com.gensler.organizations.GetOrganization",\
  "type": "record",\
  "fields": [\
    { \"name\": \"id\", \"type\": \
      {\
        \"name\": \"com.gensler.models.common.GUID\",\
        \"type\": \"record\",\
        \"fields\": [\
          { \"name\": \"bytes\", \"type\": \"bytes\"}\
        ]\
      }\
    }\
  ]\
}';



var booleanResult = avro.parseDatum('"boolean"', new Buffer(avro.encodeDatum('"boolean"', true )));
var stringResult = avro.parseDatum('"string"', new Buffer(avro.encodeDatum('"string"', "A string to parse" )));
var complexResult = avro.parseDatum(complexSchema, new Buffer(avro.encodeDatum(complexSchema, { id: { bytes: new Buffer([8,-85,-51,18,52]) }})));
console.log("boolean result: ", booleanResult);
console.log("string result: ", stringResult, " byte length: ",avro.encodeDatum('"string"', "A string to parse" ).length);
console.log("complex result: ", complexResult, " byte length: ",avro.encodeDatum(complexSchema, { id: { bytes: new Buffer([8,-85,-51,18,52]) }}).length);


//complex schema 
