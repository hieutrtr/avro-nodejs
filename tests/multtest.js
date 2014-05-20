var fs = require('fs');
var addon = require('../build/Release/avro');

var avro = new addon.Avro();
var datumCount = 0;
avro.decodeFile("test.bin",
  function(datum){
    console.log(datum.im, datumCount++);
    if(datumCount >= 99){
	    avro.close();
    }
  },
  function(error){
    console.log(error);
  }
);

var avro1 = new addon.Avro();
var datumCount = 0;
avro1.decodeFile("test.bin",
  function(datum){
    console.log(datum.im, datumCount++);
    if(datumCount >= 99){
	    avro1.close();
    }
  },
  function(error){
    console.log(error);
  }
);

