<?php

/**
 * Created by IntelliJ IDEA.
 * User: dustise
 * Date: 14-8-12
 * Time: 下午3:03
 */
class date_serial {
  public static function new_id($module_name, $prefix) {
    $schema_list = cache_get('date_serial_schema');
    if ($schema_list === FALSE) {
      $schema_list = date_serial::get_all_schema_id();
      cache_set('date_serial_schema', $schema_list);
    }
    else {
      $schema_list = $schema_list->data;
    }

    $new_value = FALSE;
    db_query('LOCK TABLES flow_id_content as c READ, flow_id_content WRITE');
    try {
      foreach ($schema_list as $schema_id => $schema) {
        if ($module_name == $schema->{'module_name'} && $prefix == $schema->{'key_prefix'}) {
          $date_str = date($schema->{'date_format'});
          $key = "{$prefix}{$date_str}";
          //Search for the specified key
          $result = db_select('flow_id_content', 'c')
            ->fields('c', array())
            ->condition('id_key', $key, '=');
          $result = $result->{'execute'}();
          if ($result->{'rowCount'}() > 0) {
            $seed = $result->{'fetchAssoc'}()['id_seed'];
            $new_seed = $seed + 1;
            $new_value = sprintf($schema->{'seed_format'}, $new_seed);
            $new_value = "{$key}{$new_value}";
            db_update('flow_id_content')
              ->fields(array(
                'id_seed' => $new_seed,
                'id_value' => $new_value,
              ))
              ->condition('module_name', $module_name, '=')
              ->condition('key_prefix', $prefix, '=')
              ->condition('id_key', $key, '=')
              ->execute();
          }
          else {
            $new_seed = 1;
            $new_value = sprintf($schema->{'seed_format'}, $new_seed);
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
    } catch (Exception $e) {
      echo $e->getMessage();
      echo $e->getTraceAsString();
    }
    db_query('UNLOCK TABLES');
    return $new_value;
  }

  public static function get_all_schema_id() {
    $result = db_select('flow_id_schema', 'n')
      ->fields('n', array())
      ->execute()
      ->fetchAllAssoc('sid');
    return $result;
  }

  /**
   * @param int $sid
   * @param string $module_name
   * @param string $prefix
   * @return bool
   */
  public static function get_schema_info($sid = 0, $module_name = '', $prefix = '') {
    $query = db_select('flow_id_schema', 's')
      ->fields('s', array());
    if ($sid > 0) {
      $query = $query->condition('sid', $sid, '=');
    }
    else {
      if ((strlen($module_name) > 0) AND (strlen($prefix) > 0)) {
        $query->condition('module_name', $module_name)
          ->condition('key_prefix', $prefix);
      }
      else {
        return FALSE;
      }
    }
    $result = $query->{'execute'}();
    if ($result->{'rowCount'}() == 0) {
      return FALSE;
    }


    $schema = $result->{'fetchAssoc'}();

    $module_name = $schema['module_name'];
    $prefix = $schema['key_prefix'];

    $query = db_select('flow_id_content', 'content')
      ->fields('content', array())
      ->condition('module_name', $module_name, '=')
      ->condition('key_prefix', $prefix, '=')
      ->orderBy('fid', 'DESC')
      ->{'range'}(0, 1);

    $result = $query->{'execute'}();
    if ($result->{'rowCount'}() > 0) {
      $info = $result->{'fetchAssoc'}();
    }
    else {
      $info = array();
    }
    $ret = array_merge($schema, $info);
    return $ret;

  }

  public static function remove_schema($sid) {
    $schema = date_serial::get_schema_info($sid);
    if (!$schema) {
      return FALSE;
    }
    db_delete('flow_id_content')
      ->condition('module_name', $schema['module_name'], '=')
      ->condition('key_prefix', $schema['key_prefix'], '=')
      ->execute();
    db_delete('flow_id_schema')
      ->condition('sid', $sid, '=')
      ->execute();

    return TRUE;
  }
}