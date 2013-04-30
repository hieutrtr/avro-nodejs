var fs = require('fs');
var addon = require('./build/Release/avro');
var stream = require('stream');
var Buffer = require('buffer').Buffer;
var constants = require('constants');
var readStream = new stream.Readable();

var FFI = require("node-ffi");
var libc = new FFI.Library(null, {
  "system": ["int32", ["string"]]
});

var run = libc.system;
run("avrogencpp -i cpx.json -o cpx.hh -n c");
var output = "";
fs.open("test.bin", 'r', function(status, fd) {
  fs.fstat(fd,function(err, stats){
    var i=0
    var s=stats.size
    var buffer = new Buffer(100);

    console.log('.'+"test.bin"+' '+s);
    for(i=0;i<s;){

      var readbytes = fs.readSync(fd,buffer,0,buffer.length,i);
      //console.log(buffer.toString('utf8'));
      addon.decodeAppend(buffer.slice(0,readbytes));
      //console.log(buffer);
      //console.log(buffer.toString('utf8', 0, readbytes));
      i=i+buffer.length;
    }
    console.log(addon.decode());

    fs.close(fd)
  })
});
console.log("Calling Addon end");
//console.log(addon.decode());

//console.log(addon.decode()); // 'world'
//console.log(addon.encode({}));