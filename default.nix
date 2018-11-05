with import <nixpkgs> {};
stdenv.mkDerivation {
  name = "tsm";
  src = ./.;
  enableParallelBuilding = true;

  cmakeFlags = ["-DGTEST_INCLUDE_DIR=${gtest}/include"];

  buildInputs = [gcc cmake gtest glog];
}
