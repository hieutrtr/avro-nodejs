var fs = require('fs');
var addon = require('../build/Release/avro');


describe("An Async test for Avro", function(){
	it("should emit an event when the avro object has been closed",
    function(done){
      var avro = new addon.Avro();
	  	avro.onclose = function(){
	  		expect(true).toEqual(true);
	  		done();
	  	}
	  	avro.close();
	  }
  );

  it("should not throw an error if no callback for close is defined",
    function(done){
      var avro = new addon.Avro();
      avro.close();
      expect(true).toEqual(true);
      done();
    }
  );
});
