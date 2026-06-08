Tsunami Lab Week 8+
=======================

Draco-Cluster 
-----------------
Im cmd oder in powershell:

LOGIN:  ssh *username*@login1.draco.uni-jena.de  - alternativ login2 für anderen login-knoten
        password: uni-passwort zum jeweiligen account

Noch auf dem Login-Knoten:
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

Kompilieren sollte man auch auf dem Login-Knoten:
    - cd Tsunani-Lab
        - geht in den Projektordner
    - nix-shell
        - erstellt die nix.shell wie gewöhnlich
    - scons 
        - kompiliert, NICHT AUF LOGIN-KNOTEN AUSFÜHREN


Weiterleitung zu Arbeitsknoten (interaktives testen):
    - salloc --partition=short  
        - reserviert Resourcen im Cluster, dann warten bis man sie auch bekommt (habe am Montag keine bekommen)
        - mit strg+c kann man pending job allocation sauber abbrechen (man muss vorher evtl. irgendwas schreiben um im Fenster zu sein)
    - danach kann auf der partition tatsächlich ausgeführt werden

Alternativ zum interaktiven Testen; Batch Jobs:
    - nano job_tsunami.sh
        - legt Textdatei an, die folgenden Text enthalten soll:

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

# 2. Die Software-Umgebung laden (evtl- unnötig)
module load tools/python/3.8

# 3. Die eigentliche Simulation mit srun starten
srun ./*Setup + Parameter*
    
Danach
    - sbatch job_tsunami.sh
        - führt das angelegte skript aus sobald die Ressourcen im cluster frei sind und schreibt den output bzw. errors



    


.. toctree::
   :maxdepth: 2
   :caption: Contents: