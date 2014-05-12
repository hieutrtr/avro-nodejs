var addon = require('../build/Release/addon');
var assert = require("assert");

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
  },\
  {\
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

var arrayOfRecords = '{\
  "name": "com.common.test.arrayOfRecords",\
  "name": "contents",\
  "type": "array",\
  "items": {\
    "name": "B",\
    "type": "record",\
    "fields": [{\
      "name": "x",\
      "type": "string"\
    }]\
  }\
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
  "name": "com.common.test.complexUnion",\
  "type": "record",\
  "fields": [{"name": "x", "type": ["int", "string"]}]\
  }, {\
    "name": "B",\
    "type": "record",\
    "fields": [{"name": "x", "type": "string"}]\
  }]';

var union ='["string", "double"]';

var veryComplexRecord = '{\
  "name": "nutcracker",\
  "type": "record",\
  "fields": [\
    {\
      "type": {\
        "type": "map",\
        "values": {\
          "type": "array",\
          "items": [\
            {\
              "name": "A",\
              "type": "record",\
              "fields": [{"name": "x", "type": "string"}]\
            },\
            {\
              "name": "B",\
              "type": "record",\
              "fields": [{"name": "x", "type": "string"}]\
            }\
          ]\
        }\
      },\
      "name": "result"\
    }\
  ]\
  }';

    

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

var mapToArray = '{\
  "name": "mapToArray",\
  "type": "map",\
  "values": {\
    "type": "array",\
    "items": "string"\
  }\
}';

var mapToArrayOfRecords = '{\
  "name": "mapToArray",\
  "type": "map",\
  "values": {\
    "type": "array",\
    "items":\
      {\
        "name": "B",\
        "type": "record",\
        "fields": [{"name": "x", "type": "string"}]\
      }\
  }\
}';
// Some of the types supported
// TODO finish off examples.
describe("Testing the sync input checking", function(){

  it("should require valid schema", function(done){
    var avroInput = new addon.Avro();

    avroInput.onerror = function(error){
      assert.equal(true, true);
      done();
    }
    avroInput.decodeDatum([],"bob");
    avroInput.close();
  });

  //TODO fix test
  it("should require intance of string for schema", function(done){
    var avroInput = new addon.Avro();

    avroInput.onerror = function(error){
      assert.equal("Invalid Avro type: 3", error);
      done();
    }
    avroInput.decodeDatum([],"3");
    avroInput.close();
  });

  it("should allow array of bytes or a nodejs Buffer for decodeDatum", function(){
    avro.addSchema(complexUnion);
    var binary = avro.encodeDatum( 
      { "x": {string: "a String"}, namespace: "com.common.test.complexUnion"}
    );
    var resultFromArray = avro.decodeDatum(binary,"com.common.test.complexUnion");
    var resultFromBuffer = avro.decodeDatum(new Buffer(binary), "com.common.test.complexUnion");

    assert.deepEqual(resultFromArray, resultFromBuffer);

  });

});

describe("Testing the avro type dictionary", function(){

  /**
    * Needed right now until the dictionary is fixed.
    */
  beforeEach(function(){
    avro.clearDictionary();
  });

  it("should be able to add two schemas of the same type", function(){
    avro.addSchema(complexUnion);
    avro.addSchema(complexUnion);
    var object = { "x": {string: "a String"}, "namespace": "com.common.test.complexUnion"}

    var binary = avro.encodeDatum(object, "com.common.test.complexUnion");
    var result = avro.decodeDatum(binary, "com.common.test.complexUnion");

    assert.deepEqual(result, {x: "a String"});
    
  });

  it("should be able to addSchema and encodeDatum the same way", function(){
    avro.addSchema(complexUnion);
    var object = { "x": {string: "a String"}, "namespace": "com.common.test.complexUnion"}

    var binaryAddSchema = avro.encodeDatum(object, "com.common.test.complexUnion");
    var resultAddSchema = avro.decodeDatum(binaryAddSchema, "com.common.test.complexUnion");

    var binary = avro.encodeDatum(object, complexUnion);
    var result = avro.decodeDatum(binary, complexUnion);

    assert.deepEqual(result, resultAddSchema);

  });

});

describe("Testing the sync encoding and decoding types", function(){

  /**
    * Needed right now until the dictionary is fixed.
    */
  beforeEach(function(){
    avro.clearDictionary();
  });

  it("should encode decode complex union", function(){
    avro.addSchema(complexUnion);
    var object = { "x": {string: "a String"}, "namespace": "com.common.test.complexUnion"}

    var binary = avro.encodeDatum(object, "com.common.test.complexUnion");
    var complexUnionResult = avro.decodeDatum(binary, "com.common.test.complexUnion");

    assert.deepEqual({x: 'a String'}, complexUnionResult);

  });

  it("should encode decode a map", function(){
    var object = 
      {
        sequence: [242, 192, 1],
        sequence2: [32, 23, 2]
      };
    var binary = avro.encodeDatum(object, map);
    var mapResult = avro.decodeDatum(binary, map);
    assert.deepEqual(object, mapResult);
  });

  it("should encode decode an map with the value of array", function(){
    var object = 
      {
        array1: ["hello", "how", "are", "you"]
      };
    var binary = avro.encodeDatum(object, mapToArray);
    var mapResult = avro.decodeDatum(binary, mapToArray);
    assert.deepEqual(object, mapResult);
  });

  it("should encode decode an map with the value of an array of records", function(){
    var object = 
      {
        array1: [
          { 
            "x": "hello"
          },
          { 
            "x": "bye"
          }
        ]
      };
    var binary = avro.encodeDatum(object, mapToArrayOfRecords);
    var mapResult = avro.decodeDatum(binary, mapToArrayOfRecords);
    assert.deepEqual(object, mapResult);

  });

  it("should encode decode union", function(){
    var binary = avro.encodeDatum(
      { string: "we have a string"},
      union
      );
    var unionResult = avro.decodeDatum(binary, union);

    assert.equal("we have a string", unionResult);

  });

  it("should encode decode boolean", function(){
    var binary = avro.encodeDatum(true, '"boolean"');
    
    var booleanResult = avro.decodeDatum(binary,'"boolean"');
    assert.equal(true, booleanResult);

  });


  it("should encode decode arrays", function(){
    var object= ["hello", "bye", "YOLO"];

    var binary = avro.encodeDatum(object, array);
    var result= avro.decodeDatum(binary, array);

    assert.deepEqual(result, object);

  });

  it("should encode decode array of records", function(){
    var object = 
      [
        {
          "namespace": "B",
          "x": "hello"
        },
        {
          "namespace": "B",
          "x": "bye"
        }
      ];

    var binary = avro.encodeDatum(object, arrayOfRecords);
    var result = avro.decodeDatum(binary, arrayOfRecords);

    assert.deepEqual(result, [{x: "hello"},{x: "bye"}]);

  });
 

  it("should encode decode long", function(){
    var binary = avro.encodeDatum(12345, '"long"');

    var longResult = avro.decodeDatum(binary, '"long"');

    assert.equal(12345, longResult);

  });

  it("should encode decode string", function(){
    var binary = avro.encodeDatum("A string to parse", '"string"');

    var stringResult = avro.decodeDatum(binary, '"string"');

    assert.equal("A string to parse", stringResult);

  });

  it("should encode decode fixed", function(){
    var obj = {
          clientHash:  [ 120, 231, 49, 2, 125, 143, 213, 14, 214, 66, 52, 11, 124, 154, 99, 179 ],
          clientProtocol: { string: "client"},
          serverHash:  [ 120, 231, 49, 2, 125, 143, 213, 14, 214, 66, 52, 11, 124, 154, 99, 179 ],
          meta: null
    };

    var binary = avro.encodeDatum(obj, fixedExample);

    var fixedResult = avro.decodeDatum(binary, fixedExample); 

    assert.deepEqual(obj.clientHash, fixedResult.clientHash);
    assert.deepEqual(obj.serverHash, fixedResult.serverHash);
    assert.deepEqual(obj.meta, fixedResult.meta);
    assert.deepEqual(obj.clientProtocol.string, fixedResult.clientProtocol);
  });

  it("should encode decode complex type", function(){
    var obj = { id: { bytes: ([8, 171, 205, 18, 52]) }};

    var binary = avro.encodeDatum(obj, complexSchema);
    var complexResult = avro.decodeDatum(binary,complexSchema); 

    assert.deepEqual(obj, complexResult);

  });

  it("should encode decode a 'very' complex type", function(){
    var obj = {
      "result": {
        "akey": [
          {
            "namespace": "A",
            "x": "Hello"
          },
          {
            "namespace": "A",
            "x": "bye"
          },
        
        ]
      }
    };

    var binary = avro.encodeDatum(obj, veryComplexRecord);
    var result = avro.decodeDatum(binary, veryComplexRecord);

    assert.deepEqual(result,{result: { akey: [{x: "Hello"}, {x: "bye"} ]}});

  });

  it("should encode decode schema handshake", function(){
    var obj = {
        match: "CLIENT",
        serverProtocol: null,
        serverHash: null,
        meta: null    
    }

    var binary = avro.encodeDatum(obj, handshakeResponse);

    var result = avro.decodeDatum(binary, handshakeResponse); 

    assert.deepEqual({match: 1, serverProtocol: null, serverHash: null, meta: null}, result);
  });


  it("should support linked lists", function(){

    var linkedListType = avro.decodeDatum(
      [
        6, 111, 110, 101, 2, 0, 6, 116, 119,
        111, 2, 0, 10, 116, 104, 114, 101, 101,
        2, 0, 8, 102, 111, 117, 114, 0
        ], 
      linkedList
      );
    //At this time linkedList schema can not be resolved by
    // C++ implementation of Avro
    //console.log(linkedListType);
    assert.equal(true, false);

  });


  it("should support reference objects", function(){
    var ToyBox = function ToyBox(contents){
      Object.defineProperty(this, "namespace",{
        value: "com.gensler.scalavro.test.ToyBox"
      });
      Object.defineProperty(this, "contents", {
        enumerable: true,
        get: function(){
          return _contents;
        },
        set: function(val){
          _contents = val;
        }
      });
      var _contents;
      this.contents = contents;
    }

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
    var contents = [
      new Toy('doll'),
      new Toy('truck'),
      dinosaur,
      new Toy('teddy roxspin'),
      dinosaur,
      dinosaur,
      dinosaur,
      dinosaur
    ];
    var box = new ToyBox(contents);

    avro.addSchema(toyBox);
    var binary = avro.encodeDatum(box);
    var result = avro.decodeDatum(binary, "com.gensler.scalavro.test.ToyBox");
    assert.deepEqual(box, result);

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
    var guid = new GUID([8, 171, 205, 18, 52]);
    var obj = {id: guid};

    var binary = avro.encodeDatum(obj, requestSchema);

    var requestType = avro.decodeDatum(binary, requestSchema); 

    assert.deepEqual(obj, requestType);
  });

  avro.close();
});

