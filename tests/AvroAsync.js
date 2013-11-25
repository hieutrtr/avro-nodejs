var addon = require('../build/Release/addon');
var assert = require('assert');

var avro = new addon.Avro();

var jsonSchema = '{\
  "type": "record",\
  "name": "cpx",\
  "fields" : [{\
    "name": "re",\
    "type": ["double", "null"]\
    },\
    {\
      "name": "im",\
      "type" : "double",\
      "default" : "2.0"\
    },\
    {\
      "name": "name",\
      "type": "string",\
      "default" : "hello"\
    },\
    {\
      "name": "array",\
      "type": [{\
        "type": "array",\
        "items": "string"\
      }, "null"],\
      "default": "null"\
    },\
    {\
      "name": "map",\
      "type": {\
        "type": "map",\
        "values": "int"\
      }\
    }\
  ]}';


describe("An Async test for Avro", function(){
  it("should parse the datum from a byte stream", function(done){
    avro.addSchema(jsonSchema);
    var object = { re: {double: 0}, im: 105, name: '', array: {array: []}, map: [] };

    var binary = avro.encodeDatum(object, "cpx");
    avro.push(binary);
    
    var ondatum = function(datum){
      assert.deepEqual({ re: 0, im: 105, name: '', array: [], map: [] }, datum);
      done();
      avro.close()
    }
    var onerror = function(error){
      done();
      avro.close()
    }

    avro.queueSchema(jsonSchema,ondatum, onerror);

  });
});
