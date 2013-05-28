{
  "targets": [
    {
      "target_name": "avro",
      "type": "loadable_module",
      "product_extension": "node",
      'include_dirs': ['avrocpp/include'],
      'cflags_cc!': ['-fno-rtti', '-fno-exceptions'],
      'cflags_cc+': ['-frtti', '-fexceptions'],
      'link_settings': {
        'ldflags': ['-Lavrocpp/lib'],
        'libraries': ['avrocpp/lib/libavrocpp.so']
      },
      "sources": [ "node_avro.cc", "BufferedInputStream.cc"]
    }
  ]
}