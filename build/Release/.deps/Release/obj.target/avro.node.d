cmd_Release/obj.target/avro.node := flock ./Release/linker.lock g++ -shared -pthread -rdynamic -m64 -L/home/benjamin/Workspace/AvroNode/avrocpp/lib  -Wl,-soname=avro.node -o Release/obj.target/avro.node -Wl,--start-group Release/obj.target/avro/node_avro.o Release/obj.target/avro/BufferedInputStream.o -Wl,--end-group /home/benjamin/Workspace/AvroNode/avrocpp/lib/libavrocpp.so
