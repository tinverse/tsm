with import <nixpkgs> {};
stdenv.mkDerivation {
  name = "tsm";
  src = ./.;
  enableParallelBuilding = true;

  cmakeFlags = ["-GNinja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DBUILD_COVERAGE=ON -DBUILD_DEPENDENCIES=OFF"];

  nativeBuildInputs = [cmake ninja graphviz doxygen] ++
    (if stdenv.isDarwin then [llvm]
        else if stdenv.isLinux then [lcov gcc]
        else throw "unsupported platform");

  buildInputs = [catch2] ;

  buildPhase = ''
    cmake --build .
  '';

  meta = with lib; {
    description = "tsm, a c++ state machine framework";
    platforms = with platforms; darwin ++ linux;
    license = licenses.mit;
  };
}
