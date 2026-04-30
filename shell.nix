{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
    packages = [
        (pkgs.python3.withPackages (python-pkgs: [
            python-pkgs.matplotlib
            python-pkgs.sphinx
            python-pkgs.sphinx_rtd_theme
            python-pkgs.pandas
            python-pkgs.imageio
        ]))
        pkgs.doxygen
        pkgs.gmt
        pkgs.scons
        pkgs.paraview
    ];
}