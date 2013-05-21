var fs = require('fs');
var addon = require('./build/Release/avro');
var Buffer = require('buffer').Buffer;


var avro1 = new addon.Avro();

avro1.onschema = function(schema){
  console.log("schema 1");
}

avro1.onerror = function(error){
  console.log("error");
}

avro1.ondatum = function(datum){
  console.log("onDatum",datum);
}

var jsonSchema = '{ "type": "record", "name": "cpx", "fields" : [{"name": "re", "type": ["double", "null"]},{"name": "im", "type" : "double", "default" : "2.0"},{"name": "name", "type": "string", "default" : "hello"},{"name": "array", "type": [{ "type": "array", "items": "string" }, "null"], "default": "null"},{"name": "map", "type": [{ "type": "map", "values": "int"}]}]}'

avro1.queueSchema(jsonSchema);

//console.log(avro1.getSchema());

//avro1.encode("hello");


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
      avro1.push(b.slice(0,l));

      i=i+buffer.length;
      setTimeout(function(){
        buf(fs,fd,i,s,buffer)}, 1);
    });
  }else{
    fs.read(fd,buffer,0,buffer.length,i,function(e,l,b){
      avro1.push(b.slice(0,l));
      fs.close(fd);
    });
  }
}


