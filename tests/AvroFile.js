var addon = require('../build/Release/addon');
var assert = require('assert');

describe("Avro File decoding", function() {
  it("should return the datum contained in the file", function(done){
    var avro = new addon.Avro();
    var datumCount = 0;
    avro.decodeFile("test.bin",
      function(datum){
		  assert.deepEqual(datum.im, datumCount++);
		  if(datumCount >= 99){
			  avro.close();
			  done();
		  }
		},
		function(error){
		  console.log(error);
	    }
	  );
	});
});
