##avro_nodejs
===========

An avro c++ module connected to nodejs.

#Why

  Want to be able to decode avro files and streams on nodejs. Haven't found a great soluction out there that has a good node like api. 

  This project will support a good event call back api. Example

      avro.onerror = function(error){
        console.log(error);
      }

      avro.onschema = function(schema){
        //Do something with the schema that was parsed.
      }

      avro.ondatum = function(datum){
        //Do something with a javascript object parsed from avro. 
      }

      avro.decodeFile("avro.file");

#Install and Run 

    npm install

    node-gyp configure build

    node server.js




