# FOC48

48 Well Analysis Program

Installieren der MySQL Biblithek


Konfigurieren von Visual Studio
F�gen Sie in Visual Studio unter �Projekt� > �Eigenschaften� > �Linker� > �Allgemein� > �Zus�tzliche Bibliotheksverzeichnisse� 

das Verzeichnis �\lib\opt� (z. B. �C:\Programme (x86)\MySQL\MySQL Connector C++ 1.1.9\lib\opt�) des C++-Connectors hinzu.

Gehen Sie in Visual Studio unter �Projekt� > �Eigenschaften� > �C/C++� > �Allgemein� > �Zus�tzliche Includeverzeichnisse� wie folgt vor:
F�gen Sie das Verzeichnis �\include� des C++-Connectors (z. B. �C:\Programme (x86)\MySQL\MySQL Connector C++ 1.1.9\include�) hinzu.

F�gen Sie das Stammverzeichnis der Boost-Bibliothek hinzu (z. B. �C:\boost_1_64_0�).
F�gen Sie in Visual Studio unter �Projekt� > �Eigenschaften� > �Linker� > �Eingabe� > �Zus�tzliche Abh�ngigkeiten� im Textfeld die Bibliothek > hinzu.

Kopieren Sie die Datei mysqlcppconn.dll aus dem Bibliotheksordner des C++-Connectors in Schritt 3 in dasselbe Verzeichnis wie die ausf�hrbare Datei der Anwendung, oder f�gen Sie sie der Umgebungsvariablen hinzu, damit sie von Ihrer Anwendung gefunden werden kann.


https://dev.mysql.com/doc/connector-cpp/8.0/en/connector-cpp-apps-windows-notes.html