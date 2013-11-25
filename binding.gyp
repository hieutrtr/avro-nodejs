{
  "targets": [
    {
      "target_name": "addon",
      "type": "loadable_module",
      "product_extension": "node",
      'include_dirs': ['/usr/local/include/avrocpp/include', "./"],
      'cflags_cc!': ['-fno-rtti', '-fno-exceptions'],
      'cflags_cc+': ['-frtti', '-fexceptions'],
      'link_settings': {
        'ldflags': ['-L/usr/local/include/avrocpp/lib'],
        'libraries': ['/usr/local/include/avrocpp/lib/libavrocpp.so']
      },
      "sources": ["./src/addon.cc", "./src/node_avro.cc", "./src/AvroFileReader.cc", "./src/AvroFileWriter.cc", "./src/DynamicBuffer.cc", "./src/BufferedInputStream.cc", "./src/translate.cc", "./src/helpers.cc"]
    }
  ]
}
