<?php
  /**
  * Implementation of hook_views_data
  */
  function views_database_connector_views_data() {
    $dbs = array();
    global $databases;
    foreach (array_keys($databases) as $key) { // Iterate through each of the database configurations
      if ($key != 'default') { // Excluding the default database
        db_set_active($key); // Switch to database in question
        $tables = db_query("SHOW TABLES;"); // Get a list of the tables in this database
        $tablelist = array();
        while ($row = $tables->fetchAssoc()) { // Fetch a row, each with a table name
          foreach ($row as $v) {
            $cols = db_query("SHOW COLUMNS FROM ".$v.";"); // Get a list of columns from said table.  (Apparently no sanitization is safe since it came from the database)
            $collist = array();
            while ($r = $cols->fetchAssoc()) { // Fetch a row, each with a column name
              $collist[] = $r['Field']; // Add column to column list
            }
            $tablelist[] = array($v, $collist); // Add table and its columns to the table list
          }
        }
        $dbs[$key] = $tablelist; // Add table list to the database list
      }
    }
    db_set_active('default'); // Switch back to the default database for main Drupal operation
    
    $data = array();
    foreach ($dbs as $dname => $db) { // Iterate through each database
      foreach ($db as $table) { // Iterate through each table
        $data[$table[0]]['table']['group'] = t($table[0]); // Setup the table for Views to be able to see it
        $data[$table[0]]['table']['base'] = array(
          'field' => 'id',
          'title' => t("[VDC] ".$dname.":  ".$table[0]),
          'database' => $dname,
          'weight' => -9001,
        );
        
        foreach ($table[1] as $col) { // Add each column to its respective table
          $data[$table[0]][$col] = array(
            'title' => t($col),
            'help' => t($col),
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
    
    return $data; // Return the finished result, allowing Views to be able to see everything that it needs
  }
