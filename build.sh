#build otel and dyn_otel

cp /njt_telemetry/lib/* /opt/rh/devtoolset-7/root/usr/lib/


cd /njt_telemetry/njet_otel_module

if [ -d build ]; then
   rm -rf build
fi

mkdir build
cd build 
cmake3 .. 
make

cp *.so /njt_telemetry/lib


#build otel_webserver and dyn_otel_webserver
cd /njt_telemetry/njet_otel_webserver_module

if [ -d build ]; then
   rm -rf build
fi

mkdir build
cd build 
cmake3 .. 
make

cp *.so /njt_telemetry/lib

