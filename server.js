var fs = require('fs');
var avro = require('./build/Release/avro');
var stream = require('stream');
var Buffer = require('buffer').Buffer;
var constants = require('constants');
var readStream = new stream.Readable();

var FFI = require("ffi");
var libc = new FFI.Library(null, {
  "system": ["int32", ["string"]]
});

var run = libc.system;
run("avrogencpp -i cpx.json -o cpx.hh -n c");
var output = "";

//simple file read case

avro.decode("test.bin", function(event, data){
  console.log(event);
  console.log(data.length);
});

//avro.setSchema("cpx.json");

console.log(avro.getSchema());

/*
fs.open("test.bin", 'r', function(status, fd) {
  fs.fstat(fd,function(err, stats){
    var i=0
    var s=stats.size
    var buffer = new Buffer(100);

    console.log('.'+"test.bin"+' '+s);
    for(i=0;i<s;){
      var readbytes = fs.readSync(fd,buffer,0,buffer.length,i);
      avro.decodeAppend(buffer.slice(0,readbytes));
      console.log(avro.decode().length);
      i=i+buffer.length;
    }
    //console.log(addon.decode().length);

    fs.close(fd)
  })
});
console.log("Calling Addon end");

*/