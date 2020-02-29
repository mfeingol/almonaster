# Introduction

Almonaster is a web-based multi-player interstellar war game

# Getting Started

To install your own server:

- Download the binary and site packages from https://github.com/mfeingol/almonaster/releases. The pdb package isn't needed.
- Unzip both packages into the same directory
- Open .\site\Config\Default.conf in a text editor
- Set the DatabaseConnectionString variable to a valid connection string
- Open a command prompt. Run Alajar.exe
- Open http://localhost in your web browser

# Building the server yourself

- Clone the repo
- Open .\Source\Almonaster.sln in Visual Studio 2019
- Edit the Alajar.conf file as appropriate in the Alajar solution
- Edit the Default.conf file as appropriate in the Almonaster solution
- Select Alajar as the startup project
- Build the solution
- Run or debug the Alajar project

Binaries will be placed into .\Source\Drop\<platform>\<configuration>

# Managing SQL Server LocalDB instances

- Install SQL Server LocalDB
- Use the `sqllocaldb` tool

# Copying a SQL Server backup to a LocalDB instance

Create a new LocalDB instance. Run the following SQL, using the appropriate file names:

```
RESTORE DATABASE Almonaster
FROM DISK = 'Almonaster-2016-12-25.bak'

WITH MOVE 'Almonaster' TO 'Almonaster-2016-12-25.mdf',
MOVE 'Almonaster_log' TO 'Almonaster-2016-12-25.ldf',
REPLACE;
```
