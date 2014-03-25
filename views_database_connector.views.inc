<?php
/**
 * Implements hook_views_data().
 */
function views_database_connector_views_data() {
  $dbs = array();
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
          // Get a list of columns from said table.  (Apparently no
          // sanitization is safe since it came from the database).
          $cols = db_query("SHOW COLUMNS FROM " . $v . ";");
          $collist = array();
          // Fetch a row, each with a column name.
          while ($r = $cols->fetchAssoc()) {
            // Add column to column list.
            $collist[] = $r['Field'];
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
        $data[$table[0]][$col] = array(
          'title' => $col,
          'help' => $col,
          'field' => array(
            'handler' => 'views_handler_field',
            ' click sortable' => TRUE,
          ),
          'sort' => array(
            'handler' => 'views_handler_sort',
          ),
          'filter' => array(
            'handler' => 'views_handler_filter_string',
          ),
          'argument' => array(
            'handler' => 'views_handler_argument_string',
          ),
        );
      }
    }
  }

  // Return the finished result, allowing Views to be able to see everything
  // that it needs.
  return $data;
}
