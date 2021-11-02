{ pkgs ? import <nixpkgs> {} }:
let
  libnfc = pkgs.stdenv.mkDerivation {
    pname = "libnfc";
    version = "1.7.2";

    src = pkgs.fetchFromGitHub {
        owner = "nfc-tools";
        repo = "libnfc";
        rev = "4525cd1c32871e287500a384de283fb89d256738";
        sha256 = "sha256-bUHHECjtzM0f3si6av4vZon4ibRtw4In5yOPsc8gDoU=";
    };

    nativeBuildInputs = [ pkgs.pkg-config pkgs.autoreconfHook ];
    buildInputs = [ pkgs.readline ];
    propagatedBuildInputs = [ pkgs.libusb-compat-0_1 ];
  };
  libfreefare = pkgs.stdenv.mkDerivation {
    pname = "libfreefare";
    version = "0.4.0-master";

    src = pkgs.fetchFromGitHub {
        owner = "nfc-tools";
        repo = "libfreefare";
        rev = "c2b0cfa4b9fb0e4be88604f00b7a2405618d5abc";
        sha256 = "sha256-BHC0vmFhcjbcrI55ib4qaomtnSplfyFASN+zWI1PuA8=";
    };

    nativeBuildInputs = [ pkgs.autoreconfHook pkgs.pkg-config ];
    buildInputs = [ pkgs.openssl ] ++ pkgs.lib.optional pkgs.stdenv.isDarwin pkgs.libobjc;
    propagatedBuildInputs = [ libnfc ];
  };
in
pkgs.stdenv.mkDerivation rec {
  pname = "tuerd-manage";
  version = "0.1.0";

  src = builtins.path { name = "src"; path = ./.; };

  buildInputs = [
    libfreefare
    pkgs.libgcrypt
  ];

  nativeBuildInputs = [
  ];

  propagatedBuildInputs = [ ];

  configurePhase = ":";

  buildPhase = ''
    gcc -o deploy deploy.c -lnfc -lfreefare -lgcrypt
  '';

  installPhase = ''
    mkdir -p /bin
    cp deploy /bin
  '';
}
