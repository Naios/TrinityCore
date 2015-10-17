#!/bin/sh
name=$1
database=$2

echo "Database Updater check script:"
echo "  Checking database '${name}' for missing filenames in tables..."
echo

# Select all entries which are in the updates table
entries=$(mysql -uroot ${database} -e "SELECT name FROM updates" | grep ".sql")

cd sql/updates/${name}

error=0
updates=0
reapplyable=0

for file in *.sql
do
  # Check if the given update is in the `updates` table.
  if echo "${entries}" | grep -q "^${file}";
    then
      # File is ok
      updates=$((updates+1))
    else
      # The update isn't listed in the updates table of the given database.
      # Test if the update is reapplyable
      if cat ${file} | mysql -uroot ${database} &> /dev/null;
        then
          if grep -q -i "alter" ${file}
            then
              echo "- [ERROR] Update: \"sql/updates/${name}/${file}\""
              echo "  -> Is reapplyable but contains an \"ALTER\" keyword."
              echo "     It's recommended to add an update entry."
              echo
              error=1
            else
              updates=$((updates+1))
              reapplyable=$((reapplyable+1))
          fi
        else
          echo "- [ERROR] Update: \"sql/updates/${name}/${file}\""
          echo "  -> Is not reapplyable and missing in the '${name}'.'updates' table!"
          echo
          error=1
      fi
  fi
done

if [ ${error} -ne 0 ]
  then
    echo "Fatal error:"
    echo "  The Database Updater is broken for the '${name}' database,"
    echo "  because the applied update is missing in the '${name}'.'updates' table."
    echo
    echo "How to fix:"
    echo "  Insert the missing names of the already applied sql updates"
    echo "  to the 'updates' table of the '${name}' base dump ('sql/base/${name}_database.sql')."
    echo
    echo "  It's also possible to split your update into several files:"
    echo "    - Add the updates which change the table structure to the \"updates\" table"
    echo "    - Don't add the updates that only change data."
    exit 1
  else
    echo "  Everything is OK, finished checking ${updates} updates (${reapplyable} reapplyable)."
    exit 0
fi
