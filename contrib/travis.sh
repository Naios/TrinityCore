#!/bin/sh

Build(){
  cd bin
  make -j 8 && make install
  cd ..
}

InitialDBSetup() {
  mysql -uroot < sql/create/create_mysql.sql
  mysql -utrinity -ptrinity auth < sql/base/auth_database.sql
  mysql -utrinity -ptrinity characters < sql/base/characters_database.sql
}

DropDB() {
  mysql -uroot < sql/create/drop_mysql.sql
}

# Strategies
# Build the servers and update with the DBUpdater.
StrategyBuildDBUpdaterUpdate() {
  echo "Using DBUpdater update..."

  InitialDBSetup
  Build

  cd bin/check_install
  mv etc/worldserver.conf.dist etc/worldserver.conf
  mv etc/bnetserver.conf.dist etc/bnetserver.conf
  cd bin

  wget -q $DATABASE_DOWNLOAD_LINK
  7z x *.7z
  ./bnetserver --version
  ./worldserver --update
  cd ../../..

  DropDB
}

# Build the servers and test updates through the cat function.
StrategyCatUpdate() {
  echo "Using CatUpdate..."

  InitialDBSetup
  mysql -utrinity -ptrinity world < sql/base/dev/world_database.sql
  mysql -utrinity -ptrinity hotfixes < sql/base/dev/hotfixes_database.sql
  cat sql/updates/world/*.sql | mysql -utrinity -ptrinity world
  cat sql/updates/hotfixes/*.sql | mysql -utrinity -ptrinity hotfixes

  Build

  cd bin/check_install/bin
  ./bnetserver --version
  ./worldserver --version
  cd ../../..

  DropDB
}

if [ $USE_DBUPDATER = 1 ]
    then StrategyBuildDBUpdaterUpdate
  else
    StrategyCatUpdate
fi
