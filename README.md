# Introduction

Almonaster is a web-based multi-player interstellar war game

# Getting Started

# Build and Test

# Managing SQL Server LocalDB instances

Use the `sqllocaldb` tool.

# Loading a database from deployment

Create a new LocalDB instance. Run the following query, using the appropriate file names:

```
RESTORE DATABASE Almonaster
FROM DISK = 'Almonaster-2016-12-25.bak'

WITH MOVE 'Almonaster' TO 'Almonaster-2016-12-25.mdf',
MOVE 'Almonaster_log' TO 'Almonaster-2016-12-25.ldf',
REPLACE;
```

# Contribute
