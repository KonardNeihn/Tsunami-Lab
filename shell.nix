{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
    packages = [
        (pkgs.python3.withPackages (python-pkgs: [
            python-pkgs.matplotlib
            python-pkgs.sphinx
            python-pkgs.sphinx-rtd-theme
            python-pkgs.pandas
            python-pkgs.imageio
        ]))
        pkgs.doxygen
        pkgs.gmt
        pkgs.scons
        pkgs.paraview
        pkgs.pugixml
        pkgs.pkg-config
    ];

    shellHook = ''
    export PUGIXML_INCLUDE=${pkgs.pugixml}/include
    export PUGIXML_LIB=${pkgs.pugixml}/lib
    '';
}