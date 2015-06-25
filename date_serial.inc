<?php

/**
 * @file
 * Core class.
 */

/**
 * Class DateSerial: Core class for this module.
 */
class DateSerial {

  /**
   * Generate new serial code.
   *
   * @param string $module_name
   *   Which module will use this serial number.
   * @param string $prefix
   *   The prefix for the serial number.
   *
   * @return bool|string
   *   return FALSE if we can't find the schema.
   */
  public static function newSerial($module_name, $prefix) {
    $schema_list = cache_get('date_serial_schema');

    if ($schema_list == FALSE) {
      $schema_list = DateSerial::getAllSchema();
      cache_set('date_serial_schema', $schema_list);
    }
    else {
      $schema_list = $schema_list->data;
    }

    $new_value = FALSE;
    db_query('LOCK TABLES flow_id_content as c READ, flow_id_content WRITE');
    try {
      foreach ($schema_list as $schema) {
        if ($module_name == $schema->module_name && $prefix == $schema->key_prefix) {
          $date_str = date($schema->date_format);
          $key = "{$prefix}{$date_str}";
          // Search for the specified key.
          $result = db_select('flow_id_content', 'c')
            ->fields('c')
            ->condition('id_key', $key);
          $result = $result->execute();
          if ($result->rowCount() > 0) {
            $assoc = $result->fetchAssoc();
            $seed = $assoc['id_seed'];
            $new_seed = $seed + 1;
            $new_value = sprintf($schema->seed_format, $new_seed);
            $new_value = "{$key}{$new_value}";
            db_update('flow_id_content')
              ->fields(array(
                'id_seed' => $new_seed,
                'id_value' => $new_value,
              ))
              ->condition('module_name', $module_name)
              ->condition('key_prefix', $prefix)
              ->condition('id_key', $key)
              ->execute();
          }
          else {
            $new_seed = 1;
            $new_value = sprintf($schema->seed_format, $new_seed);
            $new_value = $key . $new_value;
            db_insert('flow_id_content')->fields(array(
              'module_name' => $module_name,
              'key_prefix' => $prefix,
              'id_key' => $key,
              'id_value' => $new_value,
              'id_seed' => $new_seed,
            ))->execute();
          }

        }
      }
    }
    catch (Exception $e) {
      $new_value = FALSE;
    }

    db_query('UNLOCK TABLES');
    return $new_value;
  }

  /**
   * Get all schema from db.
   *
   * @return mixed
   *   The content of the schema table.
   */
  public static function getAllSchema() {
    $result = db_select('flow_id_schema', 'n')
      ->fields('n')
      ->execute()
      ->fetchAllAssoc('sid');
    return $result;
  }

  /**
   * Find schema by sid, or by module name and prefix.
   *
   * @param int $sid
   *   Primary key.
   * @param string $module_name
   *   Module name.
   * @param string $prefix
   *   Prefix of the serial.
   *
   * @return bool|array
   *   Schema info in an array, or FALSE.
   */
  public static function getSchemaInfo($sid = 0, $module_name = '', $prefix = '') {
    $query = db_select('flow_id_schema', 's')
      ->fields('s');
    if ($sid > 0) {
      $query = $query->condition('sid', $sid);
    }
    else {
      if ((strlen($module_name) > 0) && (strlen($prefix) > 0)) {
        $query->condition('module_name', $module_name)
          ->condition('key_prefix', $prefix);
      }
      else {
        return FALSE;
      }
    }
    $result = $query->execute();
    if ($result->rowCount() == 0) {
      return FALSE;
    }

    $schema = $result->fetchAssoc();
    $module_name = $schema['module_name'];
    $prefix = $schema['key_prefix'];

    $query = db_select('flow_id_content', 'content')
      ->fields('content')
      ->condition('module_name', $module_name)
      ->condition('key_prefix', $prefix)
      ->orderBy('fid', 'DESC')
      ->range(0, 1);

    $result = $query->execute();
    if ($result->rowCount() > 0) {
      $info = $result->fetchAssoc();
    }
    else {
      $info = array();
    }
    $ret = array_merge($schema, $info);
    return $ret;

  }

  /**
   * Remove specified schema and all its recordes.
   *
   * @param int $sid
   *   Primary key.
   *
   * @return bool
   *   Result of deleting.
   */
  public static function removeSchema($sid) {
    $schema = DateSerial::getSchemaInfo($sid);
    if (!$schema) {
      return FALSE;
    }
    db_delete('flow_id_content')
      ->condition('module_name', $schema['module_name'])
      ->condition('key_prefix', $schema['key_prefix'])
      ->execute();
    db_delete('flow_id_schema')
      ->condition('sid', $sid)
      ->execute();

    return TRUE;
  }

}
