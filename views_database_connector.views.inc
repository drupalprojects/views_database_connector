<?php

/**
 * @file
 * Responsible for hooking views to add each database and its tables.
 */

/**
 * Implements hook_views_data().
 */
function views_database_connector_views_data() {
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
  foreach (array_keys($databases) as $key) {
    // Excluding the default database.
    if ($key != 'default') {
      // Switch to database in question.
      db_set_active($key);
      // Get a list of the tables in this database.
      $tables = db_query("SHOW TABLES;");
      $tablelist = array();
      // Fetch a row, each with a table name.
      while ($row = $tables->fetchAssoc()) {
        foreach ($row as $v) {
          // Get a list of columns from said table.
          $q = "SELECT table_name FROM information_schema.tables WHERE table_name ";
          $q .= "= :table;";
          $check = db_query($q, array(":table" => $v));
          if ($check->fetchAssoc() != FALSE) {
            // This sort of information gathering is safer than the below
            // method, because it allows substitution of table name with PDO,
            // rather than just string concatenation.  We will only sometimes
            // be able to use this because not all database platforms have
            // information_schema.
            $q = "SELECT column_name, data_type FROM information_schema.columns WHERE ";
            $q .= "table_name = :table;";
            $cols = db_query($q, array(":table" => $v));
          }
          // Removing potentially unsafe code, but commenting it out to
          // allow people to use it if they need it.
          //
          // // else {
          // //   // Apparently no sanitization is safe since it came from the
          // //   // database, but just to be safe, we use this as a fallback.  We
          // //   // can't use the above in the first place because PDO doesn't
          // //   // allow you to substitute table names in your prepared
					// //   // statements.
          // //   $cols = db_query("SHOW COLUMNS FROM " . $v . ";");
          // // }
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
            // Removing potentially unsafe code, but commenting it out to
            // allow people to use it if they need it.
            //
            // // if (isset($r['Type'])) {
            // //   foreach ($types as $type => $matches) {
            // //     foreach ($matches as $match) {
            // //       if (stristr($r['Type'], $match)) {
            // //         $t = $type;
            // //       }
            // //     }
            // //   }
            // //   $collist[] = array($t, $r['Field']);
            // // }
          }
          // Add table and its columns to the table list.
          $tablelist[] = array($v, $collist);
        }
      }
      // Add table list to the database list.
      $dbs[$key] = $tablelist;
    }
  }
  // Switch back to the default database for main Drupal operation.
  db_set_active('default');

  $data = array();
  // Iterate through each database.
  foreach ($dbs as $dname => $db) {
    // Iterate through each table.
    foreach ($db as $table) {
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

  // Return the finished result, allowing Views to be able to see everything
  // that it needs.
  return $data;
}
