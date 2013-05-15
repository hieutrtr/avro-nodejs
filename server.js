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

avro1.setSchema("cpx.json");

//avro1.decode("test.bin");
//console.log(avro1);

console.log(avro1.getSchema());


//avro1.encode("hello");

/*
fs.open("data.bin", 'r', function(status, fd) {
  fs.fstat(fd,function(err, stats){
    var i=0
    var s=stats.size
    var buffer = new Buffer(100);

    console.log('.'+"test.bin"+' '+s);
    buf(fs,fd,0,s,buffer);

    //console.log(addon.decode().length);
  })
});


var buf=function(fs,fd,i,s,buffer){
  if(i+buffer.length<s){
    fs.read(fd,buffer,0,buffer.length,i,function(e,l,b){
      avro1.decodeBytes(b.slice(0,l));

      i=i+buffer.length;
      setTimeout(function(){
        buf(fs,fd,i,s,buffer)}, 1000);
    });
  }else{
    fs.read(fd,buffer,0,buffer.length,i,function(e,l,b){
      avro1.decodeBytes(b.slice(0,l));
      fs.close(fd);
    });
  }
}

*/
console.log("Calling Addon end");

