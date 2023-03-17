
build_liburing() {
  echo "building liburing"
  pushd src/third_party/liburing
  make
  popd
  echo "finished building liburing"
}

build() {
  build_liburing
  cmake --build .
}

build