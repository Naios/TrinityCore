#!/bin/sh

Build(){
  cd bin

  make -j 8 && make install
  rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

  cd ..
}

InitialDBSetup() {
  mysql -uroot < sql/create/create_mysql.sql
  rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

  mysql -utrinity -ptrinity auth < sql/base/auth_database.sql
  rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

  mysql -utrinity -ptrinity characters < sql/base/characters_database.sql
  rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
}

DropDB() {
  mysql -uroot < sql/create/drop_mysql.sql
  rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
}

# Strategies
# Build the servers and update with the DBUpdater.
StrategyBuildDBUpdaterUpdate() {
  echo "Using DBUpdater update..."

  InitialDBSetup
  Build

  cd bin/check_install
  mv etc/worldserver.conf.dist etc/worldserver.conf
  mv etc/authserver.conf.dist etc/authserver.conf
  cd bin

  wget -q $DATABASE_DOWNLOAD_LINK
  7z x *.7z
  ./authserver --version
  rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

  ./worldserver --update
  rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

  cd ../../..

  DropDB
}

# Build the servers and test updates through the cat function.
StrategyCatUpdate() {
  echo "Using CatUpdate..."

  InitialDBSetup
  mysql -utrinity -ptrinity world < sql/base/dev/world_database.sql
  rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

  cat sql/updates/world/*.sql | mysql -utrinity -ptrinity world
  Build

  cd bin/check_install/bin

  ./authserver --version
  rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

  ./worldserver --version
  rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

  cd ../../..

  DropDB
}

if [ $USE_DBUPDATER = 1 ]
    then StrategyBuildDBUpdaterUpdate
  else
    StrategyCatUpdate
fi
