<?php
  /**
  * Implementation of hook_views_data
  */
  function views_database_connector_views_data() {
    $dbs = array();
    global $databases;
    foreach (array_keys($databases) as $key) {
      if ($key != 'default') {
        db_set_active($key);
        $tables = db_query("SHOW TABLES;");
        $tablelist = array();
        while ($row = $tables->fetchAssoc()) {
          foreach ($row as $v) {
            $cols = db_query("SHOW COLUMNS FROM ".$v.";"); // Apparently no sanitization is safe since it came from the database
            $collist = array();
            while ($r = $cols->fetchAssoc()) {
              $collist[] = $r['Field'];
            }
            $tablelist[] = array($v, $collist);
          }
        }
        $dbs[$key] = $tablelist;
      }
    }
    db_set_active('default');
    
    $data = array();
    foreach ($dbs as $dname => $db) {
      foreach ($db as $table) {
        $data[$table[0]]['table']['group'] = t($table[0]);
        $data[$table[0]]['table']['base'] = array(
          'field' => 'id',
          'title' => t("[VDC] ".$dname.":  ".$table[0]),
          'database' => $dname,
          'weight' => -9001,
        );
        
        foreach ($table[1] as $col) {
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
    
    return $data;
  }
