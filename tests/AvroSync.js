var fs = require('fs');
var addon = require('../build/Release/avro');

var avro = new addon.Avro();

avro.onerror = function(error){
  console.error("Error: ",error);
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

var requestSchema = '{\
    "type":"record",\
    "name":"properties.GetProperty",\
    "fields":[{\
      "type":[\
        {\
          "type":"record",\
          "name":"models.common.GUID",\
          "fields":[{"type":"bytes","name":"bytes"}]\
        },\
        {\
          "type":"record",\
          "name":"Reference",\
          "fields":[{"type":"long","name":"id"}]\
        }\
      ],\
    "name":"id"}\
 ]}';

var linkedList = '{\
  "name":"com.gensler.scalavro.test.SinglyLinkedStringList",\
  "type":"record",\
  "fields":[{\
    "name":"data",\
    "type":"string"\
    },\
    {\
    "name":"next",\
    "type":[\
      "null",\
      ["com.gensler.scalavro.test.SinglyLinkedStringList",\
      {"name":"com.gensler.scalavro.Reference",\
        "type":"record",\
        "fields":[{\
          "name":"id",\
          "type":"long"\
        }]\
      }]\
    ]}\
 ]}';
var array = '{\
  "name": "com.common.test.array",\
  "type": "array",\
  "items": "string"\
}';


var toyBox = '{\
    "name": "com.gensler.scalavro.test.ToyBox",\
    "type": "record",\
    "fields": [{\
      "name": "contents",\
      "type": {\
        "type": "array",\
        "items": [{\
          "name": "com.gensler.scalavro.test.Toy",\
          "type": "record",\
          "fields": [{\
            "name": "name",\
            "type": "string"\
          }]\
          }, {\
          "name": "com.gensler.scalavro.Reference",\
          "type": "record",\
          "fields": [{\
            "name": "id",\
            "type": "long"\
          }]\
          }\
        ]\
      }\
    }]\
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
describe("Testing the sync input checking", function(){

  it("should require valid schema", function(done){
    var avroInput = new addon.Avro();

    avroInput.onerror = function(error){
      expect(true).toEqual(true);
      done();
    }
    avroInput.decodeDatum("bob", []);
    avroInput.close();
  });

  it("should require intance of string for schema", function(done){
    var avroInput = new addon.Avro();

    avroInput.onerror = function(error){
      expect(error).toEqual("arg[0] must be a Schema String and arg[1] must be an instance of Buffer.");
      done();
    }
    avroInput.decodeDatum(3, []);
    avroInput.close();
  });

})

describe("Testing the sync encoding and decoding types", function(){
  it("should encode decode complex union", function(){
    var complexBinary = avro.encodeDatum(complexUnion, 
      { "A": {"x": {string: "a String"}}}
    );
    var complexUnionResult = avro.decodeDatum(complexUnion,
          complexBinary
        );
    expect({x: 'a String'}).toEqual(complexUnionResult);

  });

  it("should encode decode a map", function(){
    var sequence = avro.encodeDatum(map,
      {sequence: [242, 192, 1]}
    );
    var mapResult = avro.decodeDatum(map,sequence);
    expect({sequence: [242, 192, 1]}).toEqual(mapResult);
  });

  it("should encode decode union", function(){
    var unionResult = avro.decodeDatum(union, avro.encodeDatum(union, { string: "we have a string"}));
    expect("we have a string").toEqual(unionResult);

  });

  it("should encode decode boolean", function(){
    var booleanResult = avro.decodeDatum('"boolean"', avro.encodeDatum('"boolean"', true ));
    expect(true).toEqual(booleanResult);

  });

  it("should encode decode arrays", function(){
    var arrayTestData = ["hello", "bye", "YOLO"];
    var arrayResult = avro.decodeDatum(array, 
      avro.encodeDatum(array, arrayTestData)
      );
    expect(arrayTestData).toEqual(arrayResult);

  });

  it("should encode decode long", function(){
    var longResult = avro.decodeDatum('"long"', avro.encodeDatum('"long"', 12345));
    expect(12345).toEqual(longResult);

  });

  it("should encode decode string", function(){
    var stringResult = avro.decodeDatum('"string"', avro.encodeDatum('"string"', "A string to parse" ));
    expect("A string to parse").toEqual(stringResult);

  });

  it("should encode decode fixed", function(){

    var fixedResult = avro.decodeDatum(fixedExample, 
      avro.encodeDatum(fixedExample, { 
          clientHash:  [ 120, 231, 49, 2, 125, 143, 213, 14, 214, 66, 52, 11, 124, 154, 99, 179 ],
          clientProtocol: { string: "client"},
          serverHash:  [ 120, 231, 49, 2, 125, 143, 213, 14, 214, 66, 52, 11, 124, 154, 99, 179 ],
          meta: null
      })
    );

    expect(
      { 
        clientHash: [ 120, 231, 49, 2, 125, 143, 213, 14, 214, 66, 52,11, 124, 154, 99, 179 ],
        clientProtocol: 'client',
        serverHash: [ 120, 231, 49, 2, 125, 143, 213, 14, 214, 66, 52, 11, 124, 154, 99, 179 ],
        meta: null 
      }).toEqual(fixedResult);
  });

  it("should encode decode complex type", function(){
    var complexResult = avro.decodeDatum(complexSchema, 
        avro.encodeDatum(
          complexSchema,
          { id: { bytes: ([8,-85,-51,18,52]) }}
        )
    );

    expect({id: {bytes: [8, 171, 205, 18, 52]}}).toEqual(complexResult);

  });

  it("should encode deocde schema handshake", function(){
    var handshakeResponseResult = avro.decodeDatum(handshakeResponse, 
      avro.encodeDatum(handshakeResponse, {
        match: "CLIENT",
        serverProtocol: null,
        serverHash: null,
        meta: null    
      })
    );

    expect({match: 1, serverProtocol: null, serverHash: null, meta: null}).toEqual(handshakeResponseResult);
  });


  it("should support scalavro reference type", function(){
    //This is the a byte stream representing what a reference would 
    //look like for the personExample schema
    var referenceScalavroType = avro.decodeDatum(personExample,
      [
        20,0, 6, 66, 101, 110, 0, 12, 67, 111, 110, 110, 111, 
        114, 0, 10, 78, 105, 97, 108, 108, 2, 0, 0, 8, 66, 
        114, 97, 100, 2, 4, 2, 6, 2, 4, 2, 2, 2, 0, 0 
        ]
      );
    var array = [
      { name: 'Ben' },
      { name: 'Connor' },
      { name: 'Niall' },
      { name: 'Ben' },
      { name: 'Brad' },
      { name: 'Niall' },
      { name: 'Brad' },
      { name: 'Niall' },
      { name: 'Connor' },
      { name: 'Ben' } ];
  
    expect(array).toEqual(referenceScalavroType);

  });

  it("should support linked lists", function(){

    var linkedListType = avro.decodeDatum(linkedList,
      [
        6, 111, 110, 101, 2, 0, 6, 116, 119,
        111, 2, 0, 10, 116, 104, 114, 101, 101,
        2, 0, 8, 102, 111, 117, 114, 0
        ]
      );
    //At this time linkedList schema can not be resolved by
    // C++ implementation of Avro
    expect(true).toEqual(false);

  });


  it("should support reference objects", function(){
    var Toy = function Toy(name){
      Object.defineProperty(this, "namespace",{
        value: "com.gensler.scalavro.test.Toy"
      });
      Object.defineProperty(this, "name", {
        enumerable: true,
        get: function(){
          return _name;
        },
        set: function(val){
          _name = val;
        }
      });
      
      var _name;
      this.name= name;
    }

    var dinosaur = new Toy('dinosaur');
    var contents = { contents: [
      new Toy('doll'),
      new Toy('truck'),
      dinosaur,
      new Toy('teddy roxspin'),
      dinosaur,
      dinosaur,
      dinosaur,
      dinosaur
    ]
    };
    var toyBoxType = avro.encodeDatum(toyBox,contents);
    var result = (avro.decodeDatum(toyBox,toyBoxType));
    expect(contents).toEqual(result);

  });

  it("should be able to parse a request schema", function(){
    var GUID = function GUID(bytes){
      Object.defineProperty(this, "namespace",{value: "models.common.GUID"});
      Object.defineProperty(this, "bytes", {
        enumerable: true,
        get: function(){
          return _bytes;
        },
        set: function(val){
          _bytes = val;
        }
      });
      
      var _bytes;
      this.bytes = bytes;
    }

    var guid = new GUID([8,-85,-51,18,52]);
    var obj = {id: guid};

    var encodedRequestType = avro.encodeDatum(
          requestSchema, obj
        );
    var guid2 = new GUID([2,3,4,4,5,6,7,7]);
    obj.id = guid2;

    var encodedRequestType = avro.encodeDatum(
          requestSchema, obj
        );

    var requestType = avro.decodeDatum(requestSchema, 
          encodedRequestType);
    expect(obj).toEqual(requestType);
  });

  avro.close();
});

