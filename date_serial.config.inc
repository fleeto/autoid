<?php
/**
 * @file
 * Configuration forms.
 *
 * Form to add a new schema.
 * List, view and delete existing schemas.
 */

/**
 * Implements hook_form().
 */
function date_serial_create_schema_form($form, &$form_state) {
  $form['module_name'] = array(
    '#type' => 'select',
    '#title' => t('Module Name'),
    '#options' => module_list(FALSE, FALSE, TRUE),
    '#required' => TRUE,
    '#description' => t('Which module will use this id generator.'),
  );
  $form['key_prefix'] = array(
    '#type' => 'textfield',
    '#required' => TRUE,
    '#title' => t('Prefix'),
    '#size' => 35,
    '#maxlength' => 32,
    '#description' => t('The prefix for the new IDs'),
  );
  $form['date_format'] = array(
    '#type' => 'textfield',
    '#required' => TRUE,
    '#title' => t('Date format'),
    '#size' => 35,
    '#default_value' => 'Ymd',
    '#maxlength' => 32,
    '#description' => t('Format to process the date. go to http://php.net/manual/en/function.date.php for details.'),
  );
  $form['seed_format'] = array(
    '#type' => 'textfield',
    '#required' => TRUE,
    '#title' => t('Seed format'),
    '#size' => 35,
    '#default_value' => '%04d',
    '#maxlength' => 32,
    '#description' => t('Format to process the new IDs. go to http://php.net/SprintF for details.'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add'),
  );

  return $form;
}

/**
 * Submit handler for date_serial_create_schema_form.
 */
function date_serial_create_schema_form_submit($form, &$form_state) {
  cache_clear_all('date_serial_schema', 'cache');
  drupal_write_record('flow_id_schema', $form_state['values']);
  drupal_set_message(t('New ID Schema had been created.'));
}

/**
 * Implements hook_form().
 */
function date_serial_edit_schema_form($form, &$form_state) {
  $header = array(
    'sid' => t('Schema ID'),
    'module_name' => t('Module name'),
    'key_prefix' => t('ID prefix'),
    'date_format' => t('Date format'),
    'seed_format' => t('Seed format'),
    'id_value' => t('Current ID'),
    'id_seed' => t('Current seed'),
  );

  $rows = array();
  $schema_list = DateSerial::getAllSchema();

  foreach ($schema_list as $schema_id => $schema) {
    $row = array_fill(0, 7, 'N/A');
    $info = DateSerial::getSchemaInfo($schema_id);
    $row['sid'] = $schema_id;
    $row['module_name'] = $info['module_name'];
    $row['key_prefix'] = $info['key_prefix'];
    $row['date_format'] = $info['date_format'];
    $row['seed_format'] = $info['seed_format'];
    $row['id_value'] = isset($info['id_value']) ? $info['id_value'] : "N/A";
    $row['id_seed'] = isset($info['id_seed']) ? $info['id_seed'] : "N/A";
    $rows[$schema_id] = $row;
  }
  $form['schema_select'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $rows,
    '#empty' => t('No Schema.'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
  );

  return $form;

}

/**
 * Submit handler for date_serial_edit_schema_form.
 */
function date_serial_edit_schema_form_submit($form, &$form_state) {

  cache_clear_all('date_serial_schema', 'cache');
  $select = $form_state['values']['schema_select'];
  foreach ($select as $id => $value) {
    if ($value === 0) {
      continue;
    }
    if (DateSerial::removeSchema($value)) {
      drupal_set_message(t("Schema @id : @value had been deleted.",
        array(
          '@id' => $id,
          '@value' => $value,
        )));
    }
  }
}
