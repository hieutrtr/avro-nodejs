var addon = require('../build/Release/addon');
var assert = require("assert");

describe("Read Files with Avro spec", function(){

  it("should be able to construct a new FileReader", function(){
    var avroFile = new addon.AvroFileReader("test.bin");
    avroFile.close();
    assert.equal(true, true);
  });

  it("should throw error if incorrect file name", function(){
    try{
      var avroFile = new addon.AvroFileReader("homedepot");
    }catch(exception){
      assert.equal("Cannot open file: No such file or directory", exception);
    }
  });
  it("should throw error if no file given", function(){
    try{
      var avroFile = new addon.AvroFileReader;
    }catch(exception){
      assert.equal("Cannot open file: No such file or directory", exception);
    }
    
  });

  it("should be able to read avro data", function(){
    var avroFile = new addon.AvroFileReader("test.bin");
    var datum = avroFile.read();
    var expected = {
      re: 0,
      im: 0,
      name: '',
      array: [ '16', '2', '77', '29'],
      map: { first: 1, second: 2, third: 3}
    };
    assert.deepEqual(datum, expected);

  });

});
