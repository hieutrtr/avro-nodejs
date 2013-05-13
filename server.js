var fs = require('fs');
var addon = require('./build/Release/avro');
var Buffer = require('buffer').Buffer;


var avro1 = new addon.Avro();

var avro2 = new addon.Avro();

avro1.onschema = function(schema){
  console.log("schema 1");
}

avro2.onschema = function(schema){
  console.log("schema 2");
}

avro1.onerror = function(error){
  console.log("error");
}

avro1.ondatum = function(datum){
  console.log("datum");
}

//avro1.setSchema("cpx.json");

//avro1.decode("test.bin");
//console.log(avro1);
//console.log(avro1.getSchema());


fs.open("test.bin", 'r', function(status, fd) {
  fs.fstat(fd,function(err, stats){
    var i=0
    var s=stats.size
    var buffer = new Buffer(100);

    console.log('.'+"test.bin"+' '+s);
    for(i=0;i<s;){
      var readbytes = fs.readSync(fd,buffer,0,buffer.length,i);
      avro1.decodeBytes(buffer.slice(0,readbytes));
      i=i+buffer.length;
    }
    //console.log(addon.decode().length);

    fs.close(fd)
  })
});
console.log("Calling Addon end");

