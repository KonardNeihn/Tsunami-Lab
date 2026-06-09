Tsunami Lab Week 8+
=======================

Draco-Cluster 
-----------------
Im cmd oder in powershell:

LOGIN:  ssh *username*@login1.draco.uni-jena.de  - alternativ login2 für anderen login-knoten
        password: uni-passwort zum jeweiligen account

Noch auf dem Login-Knoten (nur einmalig):
    - git clone https://github.com/KonardNeihn/Tsunami-Lab.git
        - klont das Github auf das draco-cluster, Login-Knoten und Arbeitsknoten teilen sich den Speicher
    - wget https://cloud.uni-jena.de/s/CqrDBqiMyKComPc/download/data_in.tar.xz -O tsunami_lab_data_in.tar.xz
        - holt Inputdaten ins Cluster
    - tar -xf tsunami_lab_data_in.tar.xz -C Tsunami-Lab/src/bathymetry
        - entpackt Daten auf dem Cluster und verschiebt die Daten in den Ordner in dem sie erwartet werden (dauert etwas)
    - module load tools/python/3.8
        - lädt python (vorbereitung für scons)
    - pip install --user scons
        - installiert scons im eigenen home-Verzeichnis
    
    Dann kommt:
    Collecting scons
    Downloading scons-4.10.1-py3-none-any.whl (4.1 MB)
     |████████████████████████████████| 4.1 MB 7.2 MB/s
    Installing collected packages: scons
    Successfully installed scons-4.10.1
    WARNING: You are using pip version 21.3.1; however, version 25.0.1 is available.
    You should consider upgrading via the '/cluster/spack/opt/spack/linux-almalinux8-zen/gcc-8.3.1/python-3.8.6-3rggdkkl5u7llkowgbooapfr4ft7vvln/bin/python3 -m pip install --upgrade pip' command.

    vorerst ignoriert?

    - nano setup_env.sh
        - dann folgenden Text einfügen:

#!/bin/bash

# 1. Benötigte Module des Draco-Clusters laden
module load compiler/gcc/12.2.0
module load tools/python/3.8
module load libs/hdf5/1.10.7-gcc-10.2.1

# 2. Umgebungsvariablen für SCons setzen (da wir keine nix-shell nutzen)
# Auf Almalinux liegen globale Entwickler-Bibliotheken standardmäßig in /usr
export PUGIXML_INCLUDE="/home/xe46wam/Tsunami-Lab/thirdparty/pugixml"
export PUGIXML_LIB="/home/xe46wam/Tsunami-Lab/thirdparty/pugixml"
export NETCDF_INCLUDE="/usr/include"
export NETCDF_LIB="/usr/lib64"

# 3. Compiler zwingen, C++17 zu nutzen und die Filesystem-Bibliothek zu linken, funktioniert aber anscheinend nicht
export CXXFLAGS="-std=c++17"
export LINKFLAGS="-std=c++17 -lstdc++fs"
export SHLINKFLAGS="-std=c++17 -lstdc++fs"

echo "===================================================="
echo "  Tsunami-Lab Umgebung erfolgreich geladen!"
echo "  Compiler, Python und NetCDF Module sind aktiv."
echo "===================================================="

        - dann mit strg+o schreiben, namen der Datei mit Enter bestätigen und mit strg+x Texteditor wieder verlassen (^ bei den Befehlen steht für die Taste strg)

    Da das immernoch alles viel zu wenig aufwand war, ist PugiXML ist anscheinend nicht vorinstalliert und muss nachträglich zugefüggt werden:
    - mkdir -p /home/xe46wam/Tsunami-Lab/thirdparty/pugixml
        - erstellt Ordner für PugiXML Dateien
    - cd /home/xe46wam/Tsunami-Lab/thirdparty/pugixml
        - geht in den erstellten Ordner
    - wget https://raw.githubusercontent.com/zeux/pugixml/master/src/pugixml.hpp
    - wget https://raw.githubusercontent.com/zeux/pugixml/master/src/pugiconfig.hpp
    - wget https://raw.githubusercontent.com/zeux/pugixml/master/src/pugixml.cpp
        - downloadet direkt unsere benötigten Dateien für das setup environment, diese muss jetzt aber noch gebaut werden
    - cd /home/xe46wam/Tsunami-Lab/thirdparty/pugixml
        - wieder im richtigen Ordner
    - g++ -O3 -c pugixml.cpp -o pugixml.o
        - kompiliert pugixml.cpp
    - ar rcs libpugixml.a pugixml.o
        - packt die kompilierteDatei in ein Archiv, das der Linker finden sollte

Kompilieren sollte man auch auf dem Login-Knoten:
    - cd Tsunani-Lab
        - geht in den Projektordner
    - source setup_env.sh
        - erstellt den Ersatz für die nix.shell
    - scons 
        - kompiliert, NICHT AUF LOGIN-KNOTEN AUSFÜHREN


Weiterleitung zu Arbeitsknoten (interaktives testen):
    - salloc --partition=short  
        - reserviert Resourcen im Cluster, dann warten bis man sie auch bekommt (habe am Montag keine bekommen)
        - mit strg+c kann man pending job allocation sauber abbrechen (man muss vorher evtl. irgendwas schreiben um im Fenster zu sein)
    - danach kann auf der partition tatsächlich ausgeführt werden

Alternativ zum interaktiven Testen; Batch Jobs:
    - nano job_tsunami.sh
        - legt Textdatei an, die folgenden Text enthalten soll (einfach dann einfügen):

#!/bin/bash
#SBATCH --partition=short
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=96
#SBATCH --time=02:00:00
#SBATCH --output=tsunami_sim_%j.out
#SBATCH --error=tsunami_sim_%j.err

# %j sollte dabei die einzigartige job-ID sein und damit die output-Dateien eindeutig machen

# 1. In den richtigen Projektordner wechseln (wichtig!)
cd /home/*username*/Tsunami-Lab

# 2. Die Software-Umgebung laden
source setup_env.sh

# 3. Die eigentliche Simulation mit srun starten
srun ./*Setup + Parameter*
    
Danach
    - sbatch job_tsunami.sh
        - führt das angelegte skript aus sobald die Ressourcen im cluster frei sind und schreibt den output bzw. errors
        - die NetCdf-output-Datei sollte genau dort erstellt werden wo sie immer erstellt wird (in solutions)



    


.. toctree::
   :maxdepth: 2
   :caption: Contents: