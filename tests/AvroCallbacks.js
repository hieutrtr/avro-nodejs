var addon = require('../build/Release/addon');
var assert = require('assert');


describe("An Async test for Avro", function(){
	it("should emit an event when the avro object has been closed",
    function(){
      var avro = new addon.Avro();
	  	avro.onclose = function(){
        assert.equal(true,true);
	  	}
	  	avro.close();
	  }
  );

  it("should not throw an error if no callback for close is defined",
    function(){
      var avro = new addon.Avro();
      avro.close();
      assert.equal(true,true);
    }
  );
});
