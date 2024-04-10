docker build -t my_telemetry:v1.0 .

docker run -v `pwd`:/njt_telemetry my_telemetry:v1.0 /bin/bash -c "cd /njt_telemetry && ./build.sh" 
