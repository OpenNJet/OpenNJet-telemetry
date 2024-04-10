1. 简介
telemetry 采用外部模块编译
包含四个模块功能
    1. otel模块: 提供service间 trace跟踪
    2. otel_webserver模块：提供service内http请求不同阶段的trace跟踪
    3. dync_otel模块：提供otel模块的动态配置开关功能
    4. dync_otel_webserver模块: 提供otel_webserver模块的动态配置开关功能

NOTE：
    本仓库提供centos7以及ubuntu18.04下编译底层基础库以及telemetry模块的方法
    通过第2部分介绍编译出来以后，将基础库so文件替换仓库baselibs目录下的文件即可

2. telemetry模块需要依赖如下底层基础telemetry相关的so
```
libopentelemetry_common.so
libopentelemetry_exporter_ostream_span.so
libopentelemetry_exporter_otlp_grpc.so
libopentelemetry_otlp_recordable.so
libopentelemetry_resources.so
libopentelemetry_trace.so
libopentelemetry_webserver_sdk.so
liblog4cxx.so.10
libaprutil-1.so.0
libapr-1.so.0
```
底层基础telemetry相关so库的编译方法如下(编译出的so文件在lib目录下)：
```
tar xvf base_telemetry_lib.tar.gz

ubuntu:
   cd base_telemetry_lib
   cp Dockerfile_ubuntu Dockerfile
   docker build -t ubuntu18.04:telemetry .
   mkdir lib
   docker run -it -v `pwd`/lib:/baselib ubuntu18.04:telemetry /bin/bash -c "cp /dependencies/baselib/* /baselib"
   ls -al lib

centos:
   cd base_telemetry_lib
   cp Dockerfile_centos Dockerfile
   docker build -t centos7:telemetry .
   mkdir lib
   docker run -it -v `pwd`/lib:/baselib centos7:telemetry /bin/bash -c "cp /dependencies/baselib/* /baselib"
   ls -al lib
```


3. telemetry编译方式
```
    git clone --recursive git@gitee.com:njet-rd/njet-telemetry.git
    
    ubuntu:
        cp njet-telemetry/Dockerfile_ubuntu Dockerfile
        docker build -t ubuntu:telemetry_module .
        mkdir lib
        docker run -it -v `pwd`/lib:/baselib ubuntu:telemetry_module /bin/bash -c "cd njet-telemetry; cp njet_otel_module/build/*.so /baselib; cp njet_otel_webserver_module/build/*.so /baselib"
    
    centos:
        cp njet-telemetry/Dockerfile_centos Dockerfile
        docker build -t centos7:telemetry_module .
        mkdir lib
        docker run -it -v `pwd`/lib:/baselib centos7:telemetry_module /bin/bash -c "cd njet-telemetry; cp njet_otel_module/build/*.so /baselib; cp njet_otel_webserver_module/build/*.so /baselib"
```
    lib目录下可以看到编译出的相关模块的so
        njt_agent_dyn_otel_module.so
        njt_agent_dyn_otel_webserver_module.so
        njt_otel_module.so
        njt_otel_webserver_module.so
    上述4个so即为目标so， 该dependlibs目录下的其他动态库为依赖库文件，需要将他们放到目标系统的lib相关目录下 

4. 模块使用及配置
   请参考njet手册关于telemetry 配置部分

