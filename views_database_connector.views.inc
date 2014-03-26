<?php

/**
 * @file
 * Responsible for hooking views to add each database and its tables.
 */

/**
 * Implements hook_views_data().
 */
function views_database_connector_views_data() {
  $dbs = views_database_connector_get_database_schemas();
  $data = array();
  // Iterate through each database.
  foreach ($dbs as $dname => $db) {
    // Iterate through each table.
    foreach ($db as $table) {
      // Make sure the table name hasn't been used before.
      if (!isset($data[$table[0]])) {
        // Setup the table for Views to be able to see it.
        $data[$table[0]]['table']['group'] = $table[0];
        $data[$table[0]]['table']['base'] = array(
          'field' => 'id',
          'title' => "[VDC] " . $dname . ":  " . $table[0],
          'database' => $dname,
          'weight' => -9001,
        );

        // Add each column to its respective table.
        foreach ($table[1] as $col) {
          $data[$table[0]][$col[1]] = array(
            'title' => $col[1],
            'help' => $col[1],
            'field' => array(
              'handler' => 'views_handler_field',
              ' click sortable' => TRUE,
            ),
            'sort' => array(
              'handler' => 'views_handler_sort',
            ),
            'filter' => array(
              'handler' => 'views_handler_filter_' . $col[0],
            ),
            'argument' => array(
              'handler' => 'views_handler_argument_' . $col[0],
            ),
          );
        }
      }
    }
  }

  // Return the finished result, allowing Views to be able to see everything
  // that it needs.
  return $data;
}

/**
 * Gathers appropriate information from each potential database driver type.
 */
function views_database_connector_get_database_schemas() {
  $dbs = array();
  $types = array(
    "numeric" => array(
      "int",
      "decimal",
      "numeric",
      "float",
      "double",
      "bit",
    ),
    "date" => array(
      "date",
      "time",
      "year",
    ),
    "string" => array(
      "char",
      "binary",
      "blob",
      "text",
      "enum",
      "set",
    ),
  );

  global $databases;
  // Iterate through each of the database configurations.
  foreach ($databases as $key => $datab) {
    // Excluding the default database.
    if ($key != 'default') {
      // Switch to database in question.
      db_set_active($key);

      if (strtolower($datab['default']['driver']) == 'mysql') {
        // Get a list of the tables in this database.
        $tables = db_query("SHOW TABLES;");
        $tablelist = array();
        // Fetch a row, each with a table name.
        while ($row = $tables->fetchAssoc()) {
          // This is the one of two database formats that can have whacky table
          // names due to using information_schema.  We have the ability to
          // check on columns without the PDO table substitution problem.
          foreach ($row as $v) {
            if (!in_array($v, array_keys(drupal_get_complete_schema()))) {
              // Fetch column names and their data type from said table.
              $q = "SELECT column_name, data_type FROM ";
              $q .= "information_schema.columns WHERE table_name = :table;";
              $cols = db_query($q, array(":table" => $v));
              $collist = array();
              // Fetch a row, each with a column name.
              while ($r = $cols->fetchAssoc()) {
                $t = 'broken';
                // Add column to column list.
                if (isset($r['column_name'])) {
                  foreach ($types as $type => $matches) {
                    foreach ($matches as $match) {
                      if (stristr($r['data_type'], $match)) {
                        $t = $type;
                      }
                    }
                  }
                  $collist[] = array($t, $r['column_name']);
                }
              }
              // Add table and its columns to the table list.
              $tablelist[] = array($v, $collist);
            }
          }
        }
        // Add table list to the database list.
        $dbs[$key] = $tablelist;
      }
      if (strtolower($datab['default']['driver']) == 'sqlite') {
        // Get a list of the tables in this database.
        $q = "SELECT name FROM sqlite_master WHERE type='table';";
        $tables = db_query($q);
        $tablelist = array();
        // Fetch a row, each with a table name.
        while ($row = $tables->fetchAssoc()) {
          foreach ($row as $v) {
            // Check that the table name is safe to substitute in the query.
            if ($v == db_escape_table($v)) {
              // Fetch column names and their data type from said table.
              $q = "PRAGMA table_info(:table);";
              $cols = db_query(str_ireplace(":table", $v, $q));
              $collist = array();
              // Fetch a row, each with a column name.
              while ($r = $cols->fetchAssoc()) {
                $t = 'broken';
                // Add column to column list.
                if (isset($r['name'])) {
                  foreach ($types as $type => $matches) {
                    foreach ($matches as $match) {
                      if (stristr($r['type'], $match)) {
                        $t = $type;
                      }
                    }
                  }
                  $collist[] = array($t, $r['name']);
                }
              }
              // Add table and its columns to the table list.
              $tablelist[] = array($v, $collist);
            }
          }
        }
        // Add table list to the database list.
        $dbs[$key] = $tablelist;
      }
      if (strtolower($datab['default']['driver']) == 'pgsql') {
        // Get a list of the tables in this database.
        $q = "SELECT table_name FROM information_schema.tables WHERE table_type =";
        $q .= " 'BASE TABLE' AND table_schema NOT IN ('pg_catalog', ";
        $q .= "'information_schema');";
        $tables = db_query($q);
        $tablelist = array();
        // Fetch a row, each with a table name.
        while ($row = $tables->fetchAssoc()) {
          foreach ($row as $v) {
            // Fetch column names and their data type from said table.
            $q = "SELECT column_name, data_type FROM ";
            $q .= "information_schema.columns WHERE table_name = :table;";
            $cols = db_query($q, array(":table" => $v));
            $collist = array();
            // Fetch a row, each with a column name.
            while ($r = $cols->fetchAssoc()) {
              $t = 'broken';
              // Add column to column list.
              if (isset($r['column_name'])) {
                foreach ($types as $type => $matches) {
                  foreach ($matches as $match) {
                    if (stristr($r['data_type'], $match)) {
                      $t = $type;
                    }
                  }
                }
                $collist[] = array($t, $r['column_name']);
              }
            }
            // Add table and its columns to the table list.
            $tablelist[] = array($v, $collist);
          }
        }
        // Add table list to the database list.
        $dbs[$key] = $tablelist;
      }
    }
  }
  // Switch back to the default database for main Drupal operation.
  db_set_active('default');
  return $dbs;
}
