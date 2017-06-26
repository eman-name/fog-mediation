FROM alpine:3.5

WORKDIR /mnt

ADD grpc.patch /mnt/grpc.patch

RUN set -ex \
    && apk add --no-cache sudo make g++ openblas openssl protobuf-dev jpeg libpng zlib \
    && apk add --no-cache --virtual .build-deps cmake bzip2 openssl-dev perl linux-headers coreutils yasm openblas-dev jpeg-dev libpng-dev zlib-dev \
    \
    && wget -q https://github.com/grpc/grpc/archive/v1.2.5.tar.gz -O - | tar zxf - \
    && patch -p0 <grpc.patch \
    && cd grpc-* \
    && make -j`nproc` CFLAGS=-w \
    && make install PREFIX=/usr/local \
    && cd .. \
    && rm -rf grpc-* \
    && ln -s /usr/local/lib/libgrpc++.so.1.2.5 /usr/local/lib/libgrpc++.so.1 \
    && rm /usr/local/lib/*libgrpc*.a \
    \
    && wget -q https://github.com/davisking/dlib/archive/v19.4.tar.gz -O - | tar zxf - \
    && cd dlib-* \
    && cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-w \
    && make -j`nproc`\
    && mkdir -p /usr/local/include \
    && cp -a dlib /usr/local/include/dlib \
    && make install \
    && cd .. \
    && rm -rf dlib-* \
    \
    && wget -q https://github.com/FFmpeg/FFmpeg/archive/n3.3.tar.gz -O - | tar zxf - \
    && cd FFmpeg-* \
    && ./configure --prefix=/usr/local \
                   --disable-static \
                   --enable-shared \
                   --disable-programs \
                   --disable-doc \
                   --disable-encoders \
                   --disable-muxers \
    && make -j`nproc` install \
    && cd .. \
    && rm -rf FFmpeg-* \
    \
    && wget -q https://github.com/opencv/opencv/archive/3.2.0.tar.gz -O - | tar zxf - \
    && cd opencv-* \
    && cmake -DCMAKE_INSTALL_PREFIX=/usr/local \
             -DCMAKE_BUILD_TYPE=Release \
             -DBUILD_PERF_TESTS=OFF \
             -DBUILD_TESTS=OFF \
             -DBUILD_DOCS=OFF \
             -DWITH_GSTREAMER=OFF \
             -DWITH_MATLAB=OFF \
             -DWITH_IPP=OFF \
             -DWITH_VTK=OFF \
             -DWITH_CUDA=OFF \
             -DWITH_OPENCL=OFF \
             -DWITH_GTK=OFF \
             -DWITH_GTK_2_X=OFF \
             -DWITH_1394=OFF \
             -DWITH_GPHOTO2=OFF \
             -DWITH_WEBP=OFF \
             -DWITH_OPENEXR=OFF \
             -DWITH_JASPER=OFF \
             -DWITH_TIFF=OFF \
             -DBUILD_opencv_flann=OFF \
             -DBUILD_opencv_ml=OFF \
             -DBUILD_opencv_photo=OFF \
             -DBUILD_opencv_video=OFF \
             -DBUILD_opencv_shape=OFF \
             -DBUILD_opencv_highgui=OFF \
             -DBUILD_opencv_objdetect=OFF \
             -DBUILD_opencv_superres=OFF \
             -DBUILD_opencv_features2d=OFF \
             -DBUILD_opencv_calib3d=OFF \
             -DBUILD_opencv_stitching=OFF \
             -DBUILD_opencv_videostab=OFF \
    && make -j`nproc` install \
    && cd .. \
    && rm -rf opencv-* \
    \
    && wget -q https://github.com/torch/torch7/archive/8e7898166d1844620b1bde58ca05a7ca9291fa39.tar.gz -O - | tar zxf - \
    && cd torch7-* \
    && cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release lib/TH \
    && make -j`nproc` install \
    && cd .. \
    && rm -rf torch7-* \
    \
    && wget -q https://github.com/torch/nn/archive/22ffc4fcbe9e5f92cd812c7231c833da72165a49.tar.gz -O - | tar zxf - \
    && cd nn-* \
    && wget -q -O lib/THNN/CMakeLists.txt https://raw.githubusercontent.com/tuotuoxp/nn/a36d6a69c2ebb6b5891b41fbcd7011e88bbace56/lib/THNN/CMakeLists.txt \
    && cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/usr/local lib/THNN \
    && make -j`nproc` install \
    && cd .. \
    && rm -rf nn-* \
    \
    && wget -q https://github.com/tuotuoxp/cpp-torch/archive/master.tar.gz -O - | tar zxf - \
    && cd cpp-torch-* \
    && cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release \
    && make -j`nproc` install \
    && cd .. \
    && rm -rf cpp-torch-* \
    \
    && wget -q https://github.com/cjlin1/libsvm/archive/v322.tar.gz -O - | tar -zxf - \
    && cd libsvm-* \
    && g++ -shared -fpic -fno-plt -ffast-math -O2 -s svm.cpp -o /usr/local/lib/libsvm.so \
    && mkdir -p /usr/local/include \
    && cp svm.h /usr/local/include/ \
    && cd .. \
    && rm -rf libsvm-* \
    \
    && rm /usr/local/lib/libdlib.a \
    && strip --strip-unneeded /usr/local/lib/*.so \
    \
    && wget -q https://github.com/davisking/dlib-models/blob/master/shape_predictor_68_face_landmarks.dat.bz2?raw=true -O - \
       | bunzip2 - > /usr/local/share/shape_predictor_68_face_landmarks.dat \
    && wget -q https://storage.cmusatyalab.org/openface-models/nn4.small2.v1.t7 -O /usr/local/share/nn4.small2.v1.t7 \
    \
    && apk del --purge --no-cache .build-deps \
    && ln -s /usr/lib/libopenblas.so.3 /usr/lib/libopenblas.so \
    && rm /mnt/grpc.patch
