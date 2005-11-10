#!/bin/bash

/usr/local/jdk1.5.0_04/bin/jar xvf /usr/local/netbeans-4.1/ide5/modules/ext/AbsoluteLayout.jar;
/usr/local/jdk1.5.0_04/bin/jar xvf ~/postgresql.jar;
cp -R ~/ptolemy.plot2.0/ptolemy .;
/usr/local/jdk1.5.0_04/bin/jar uf BB_GUI.jar org;
/usr/local/jdk1.5.0_04/bin/jar uf BB_GUI.jar ptolemy;

