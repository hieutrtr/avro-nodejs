var fs = require('fs');
var addon = require('../build/Release/avro');

var avro = new addon.Avro();

avro.onerror = function(error){
  console.log(error);
};

var buildingSchema = '{\
    "name": "Organization",\
    "type": "record",\
    "fields": [\
      { "name": "id", "type": \
        {\
          "name": "GUID",\
          "type": "record",\
          "fields": [\
            { "name": "bytes", "type": "bytes"}\
          ]\
        }\
      },\
      { "name": "name", "type": "string" },\
      { "name": "organizationType", "type": "string" }\
    ]\
  }';

var fixedExample = '{\
  "name": "com.gensler.scalavro.protocol.HandshakeRequest",\
  "type": "record",\
  "fields": [{\
    "name": "clientHash",\
    "type": {\
      "name": "MD5",\
      "type": "fixed",\
      "size": 16,\
      "namespace": "com.gensler.scalavro.protocol",\
      "aliases": []\
    }\
  }, {\
    "name": "clientProtocol",\
    "type": ["null", "string"]\
  }, {\
    "name": "serverHash",\
    "type": "com.gensler.scalavro.protocol.MD5"\
  }, {\
    "name": "meta",\
    "type": ["null", {\
      "type": "map",\
      "values": "bytes"\
    }]\
  }]\
}';

// Schema of Person
var personExample = '{\
  "type": "array",\
  "items": [{\
    "name": "com.gensler.scalavro.util.BenTest.Person",\
    "type": "record",\
    "fields": [{\
      "name": "name",\
      "type":"string"\
    }]},\
    {\
      "name": "com.gensler.scalavro.Reference",\
      "type": "record",\
      "fields": [{\
        "name": "id",\
        "type": "long"\
      }]\
    }\
  ]}';


var complexSchema = '{\
  "name": "GetOrganization",\
  "type": "record",\
  "fields": [\
    { "name": "id", "type": \
      {\
        "name": "common.GUID",\
        "type": "record",\
        "fields": [\
          { "name": "bytes", "type": "bytes"}\
        ]\
      }\
    }\
  ]\
}';

var complexUnion = '[{\
  "name": "A",\
  "type": "record",\
  "fields": [{"name": "x", "type": ["int", "string"]}]\
  }, {\
    "name": "B",\
    "type": "record",\
    "fields": [{"name": "x", "type": "string"}]\
  }]';

var union ='["string", "double"]';

var handshakeResponse = '{\
    "type": "record",\
    "name": "handshakeResponse",\
    "namespace": "com.gensler.scalavro.protocol",\
    "fields": [\
      {"name": "match",\
       "type": {"type": "enum", "name": "HandshakeMatch",\
                "symbols": ["BOTH", "CLIENT", "NONE"]}},\
      {"name": "serverProtocol",\
       "type": ["null", "string"]},\
      {"name": "serverHash",\
       "type": ["null", {"type": "fixed", "name": "MD5", "size": 16}]},\
      {"name": "meta",\
       "type": ["null", {"type": "map", "values": "bytes"}]}\
    ]\
  }';

var map = '{"type": "map","values": "bytes"}';
// Some of the types supported
// TODO finish off examples.

describe("Testing the sync encoding and decoding types", function(){
  it("should encode decode complex union", function(){
    var complexUnionResult = avro.decodeDatum(complexUnion,
                               new Buffer(avro.encodeDatum(complexUnion, { "A": {"x": {string: "a String"}}})));
    expect({x: 'a String'}).toEqual(complexUnionResult);

  });

  it("should encode decode a map", function(){
    var mapResult = avro.decodeDatum(map,
                      new Buffer(avro.encodeDatum(map, {sequence: new Buffer(avro.encodeDatum('"long"', 12345))})));
    expect({sequence: [242, 192, 1]}).toEqual(mapResult);
  });

  it("should encode decode union", function(){
    var unionResult = avro.decodeDatum(union, new Buffer(avro.encodeDatum(union, { string: "we have a string"})));
    expect("we have a string").toEqual(unionResult);

  });

  it("should encode decode boolean", function(){
    var booleanResult = avro.decodeDatum('"boolean"', new Buffer(avro.encodeDatum('"boolean"', true )));
    expect(true).toEqual(booleanResult);

  });

  it("should encode decode long", function(){
    var longResult = avro.decodeDatum('"long"', new Buffer(avro.encodeDatum('"long"', 12345)));
    expect(12345).toEqual(longResult);

  });

  it("should encode decode string", function(){
    var stringResult = avro.decodeDatum('"string"', new Buffer(avro.encodeDatum('"string"', "A string to parse" )));
    expect("A string to parse").toEqual(stringResult);

  });

  it("should encode decode fixed", function(){

    var fixedResult = avro.decodeDatum(fixedExample, new Buffer(
      avro.encodeDatum(fixedExample, { 
          clientHash:  new Buffer([ 120, 231, 49, 2, 125, 143, 213, 14, 214, 66, 52, 11, 124, 154, 99, 179 ]),
          clientProtocol: { string: "client"},
          serverHash:  new Buffer([ 120, 231, 49, 2, 125, 143, 213, 14, 214, 66, 52, 11, 124, 154, 99, 179 ]),
          meta: null
      })
    ));

    expect(
      { 
        clientHash: [ 120, 231, 49, 2, 125, 143, 213, 14, 214, 66, 52,11, 124, 154, 99, 179 ],
        clientProtocol: 'client',
        serverHash: [ 120, 231, 49, 2, 125, 143, 213, 14, 214, 66, 52, 11, 124, 154, 99, 179 ],
        meta: null 
      }).toEqual(fixedResult);
  });

  it("should encode decode complex type", function(){
    var complexResult = avro.decodeDatum(complexSchema, new Buffer(avro.encodeDatum(complexSchema, { id: { bytes: new Buffer([8,-85,-51,18,52]) }})));

    expect({id: {bytes: [8, 171, 205, 18, 52]}}).toEqual(complexResult);

  });

  it("should encode deocde schema handshake", function(){
    var handshakeResponseResult = avro.decodeDatum(handshakeResponse, new Buffer(
      avro.encodeDatum(handshakeResponse, {
        match: "CLIENT",
        serverProtocol: null,
        serverHash: null,
        meta: null    
      })
    ));

    expect({match: 1, serverProtocol: null, serverHash: null, meta: null}).toEqual(handshakeResponseResult);
  });


  it("should support scalavro reference type", function(){
    //This is the a byte stream representing what a reference would 
    //look like for the personExample schema
    var referenceScalavroType = avro.decodeDatum(personExample,
      new Buffer([
        20,0, 6, 66, 101, 110, 0, 12, 67, 111, 110, 110, 111, 
        114, 0, 10, 78, 105, 97, 108, 108, 2, 0, 0, 8, 66, 
        114, 97, 100, 2, 4, 2, 6, 2, 4, 2, 2, 2, 0, 0 
        ])
      );
    expect([
      { name: 'Ben' },
      { name: 'Connor' },
      { name: 'Niall' },
      { name: 'Ben' },
      { name: 'Brad' },
      { name: 'Niall' },
      { name: 'Ben' },
      { name: 'Niall' },
      { name: 'Connor' },
      { name: 'Ben' } ]).toEqual(referenceScalavroType);

  });

  avro.close();
});

