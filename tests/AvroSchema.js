var addon = require('../build/Release/addon');
var assert = require('assert');


var complexSchema = '{\
    "name": "GetOrganization",\
    "type": "record",\
    "fields": [\
    { "name": "id", "type": \
      {\
        "name": "common.GUID",\
        "type": "record",\
        "fields": [\
          { "name": "bytes", "type": "bytes"}\
        ]\
      }\
    }\
  ]\
}';

var protocolTypes = '[{"type":"record","name":"com.gensler.scalavro.Reference","fields":[{"type":"long","name":"id"}]},{"type":"record","name":"com.gensler.models.common.GUID","fields":[{"type":"bytes","name":"bytes"}]},{"type":"record","name":"com.gensler.geometry.Point","fields":[{"type":"double","name":"x"},{"type":"double","name":"y"}]},{"type":"record","name":"com.gensler.geometry.Vertex","fields":[{"type":["com.gensler.geometry.Point","com.gensler.scalavro.Reference"],"name":"point"},{"type":"double","name":"includedAngle"}]},{"type":"record","name":"com.gensler.geometry.ClosedPolyline","fields":[{"type":{"type":"array","items":["com.gensler.geometry.Vertex","com.gensler.scalavro.Reference"]},"name":"vertices"}]},{"type":"record","name":"com.gensler.drawings.DrawingDefinition","fields":[{"type":{"type":"array","items":["com.gensler.geometry.ClosedPolyline","com.gensler.scalavro.Reference"]},"name":"regions"},{"type":{"type":"array","items":"string"},"name":"activeLayerNames"}]},{"type":"record","name":"com.gensler.cadrepository.ImportCad","fields":[{"type":["com.gensler.models.common.GUID","com.gensler.scalavro.Reference"],"name":"fileId"},{"type":{"type":"array","items":["com.gensler.drawings.DrawingDefinition","com.gensler.scalavro.Reference"]},"name":"definitions"}]},{"type":"record","name":"com.gensler.cadrepository.ImportSuccess","fields":[{"type":["com.gensler.models.common.GUID","com.gensler.scalavro.Reference"],"name":"id"},{"type":["com.gensler.models.common.GUID","com.gensler.scalavro.Reference"],"name":"fingerprintGuid"},{"type":["com.gensler.models.common.GUID","com.gensler.scalavro.Reference"],"name":"versionGuid"},{"type":{"type":"array","items":["com.gensler.models.common.GUID","com.gensler.scalavro.Reference"]},"name":"drawingIds"}]},{"type":"record","name":"com.gensler.properties.GetProperty","fields":[{"type":["com.gensler.models.common.GUID","com.gensler.scalavro.Reference"],"name":"id"}]},{"type":"record","name":"com.gensler.models.locations.GpsLocation","fields":[{"type":"double","name":"latitude"},{"type":"double","name":"longitude"}]},{"type":"record","name":"com.gensler.models.locations.Address","fields":[{"type":{"type":"array","items":"string"},"name":"street"},{"type":"string","name":"city"},{"type":"string","name":"province"},{"type":"string","name":"country"},{"type":"string","name":"postalCode"}]},{"type":"enum","name":"com.gensler.models.space.UnitOfDistance","symbols":["UNITLESS","INCHES","FEET","MILES","MILLIMETERS","CENTIMETERS","METERS","KILOMETERS","MICROINCHES","MILS","YARDS","ANGSTROMS","NANOMETERS","MICRONS","DECIMETERS","DECAMETERS","HECTOMETERS","GIGAMETERS","ASTRONOMICAL_UNITS","LIGHT_YEARS","PARSECS"]},{"type":"record","name":"com.gensler.models.space.Measurement","fields":[{"type":"string","name":"name"},{"type":"com.gensler.models.space.UnitOfDistance","name":"baseUnit"},{"type":"double","name":"value"}]},{"type":"record","name":"com.gensler.models.properties.Property","fields":[{"type":["com.gensler.models.common.GUID","com.gensler.scalavro.Reference"],"name":"id"},{"type":"string","name":"name"},{"type":{"type":"array","items":"string"},"name":"aliases"},{"type":["com.gensler.models.locations.GpsLocation","com.gensler.scalavro.Reference"],"name":"gps"},{"type":{"type":"array","items":["com.gensler.models.locations.Address","com.gensler.scalavro.Reference"]},"name":"addresses"},{"type":{"type":"array","items":["com.gensler.models.space.Measurement","com.gensler.scalavro.Reference"]},"name":"measurements"}]}]';

describe("Avro Schema ", function(){
  it("should be able to construct a schema object", function(){
    var schema = new addon.AvroSchema('{ "type": "string"}');
    assert.equal(schema.toJson(), '"string"');

  });
  it("should be able to construct a complex schema", function(){
    var schema = new addon.AvroSchema(complexSchema);
    assert.equal(schema.toJson(), complexSchema);
  });

  it("should be able to construct the types union", function(){
    var protocolSchema = new addon.AvroSchema(protocolTypes);
    var avro = new addon.Avro();
    avro.addSchema(protocolSchema);
    //console.log(protocolSchema.toJson());
  });
});

