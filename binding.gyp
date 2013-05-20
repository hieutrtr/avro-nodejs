{
  "targets": [
    {
      "target_name": "avro",
      "type": "loadable_module",
      "product_extension": "node",
      'include_dirs': ['/home/benjamin/Workspace/AvroNode/avrocpp/include'],
      'cflags_cc!': ['-fno-rtti', '-fno-exceptions'],
      'cflags_cc+': ['-frtti', '-fexceptions'],
      'link_settings': {
        'ldflags': ['-L/home/benjamin/Workspace/AvroNode/avrocpp/lib'],
        'libraries': ['/home/benjamin/Workspace/AvroNode/avrocpp/lib/libavrocpp.so']
      },
      "sources": [ "node_avro.cc", "BufferedInputStream.cc"]
    }
  ]
}