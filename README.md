# SUR WINDOWS 
### 1-Installer gcc dans votre machine grace a cette commande dans votre terminal windowws powershell: 
* `Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))`.
* Une fois chocolatey installe, tu tapes **choco install mingw** pour pouvoir utiliser gcc sans probleme.
### 2-Ensuite taper la commande **gcc -o main main.c** pour compiler le projet.
### 3-Enfin lancer l'executable comme ceci **.\main.exe**
