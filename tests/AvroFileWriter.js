var addon = require('../build/Release/addon');
var assert = require("assert");

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

describe("Tests for AvroFileWriter", function(){

  it("should be able to create new AvroFileWriter", function(){
    var obj = { id: { bytes: ([8, 171, 205, 18, 52]) }};
    var avroWriter = new addon.AvroFileWriter("writeData.bin",complexSchema);  
    avroWriter.write(obj);
    avroWriter.close();

  });

});
