<?php

/**
 * @file
 * Handles the form elements of valghalla_mail 'admin/valhalla/sendmail'
 * @author Yani Xu <yani@bellcom.dk>
 */

/**
 * @file
 * @see email_node_form_alter() add an extra submit function
 * @see valghalla_mail_sendmail_form
 */
function valghalla_mail_sendmail_form_step(&$form, &$form_state) {
  $nid = $form_state['node']->nid;
  drupal_goto('admin/valhalla/sendto/'.$nid);
}
/**
 * Step 2 : get_recipients
 * Step 3 : send_confirm
 * 
 * @see valghalla_mail_sendmail_form_validate().
 * @see valghalla_mail_sendmail_form_submit().
 * 
 * @param type $form
 * @param string $form_state
 * @return type 
 */

function valghalla_mail_sendmail_form($form, &$form_state,$nid) {
  drupal_add_css(drupal_get_path('module', 'valghalla_mail') . '/css/valghalla_mail.css');
  if (!isset($form_state['stage']))
    $form_state['stage'] = 'get_recipients';
  module_load_include('inc', 'node', 'node.pages');

  $form = array();
  $form = valghalla_mail_sendmail_get_header($form, $form_state);

  switch ($form_state['stage']) {

    case 'get_recipients': 
      return valghalla_mail_sendmail_get_recipients_form($form, $form_state,$nid);  
     break;

    case 'send_confirm': 
      return valghalla_mail_sendmail_send_confirm_form($form, $form_state);  
     break;

    default:
      return valghalla_mail_sendmail_get_recipients_form($form, $form_state,$nid);
     break; 
  }

  return $form;
}

/**
 * Form for Step 1 create_email_node step.
 *
 * @see valghalla_mail_form_email_node_form_alter
 *
 * @param type $form
 * @param type $form_state
 * @return type 
 */
function valghalla_mail_sendmail_create_email_node() {
  module_load_include('inc', 'node', 'node.pages');
  return node_add('email');
}

/**
 * Form for Step 2 get_recipients step.
 * 
 * @see valghalla_mail_sendmail_form().
 *  
 * @param type $form
 * @param type $form_state
 * @return type 
 */
function valghalla_mail_sendmail_get_recipients_form($form, &$form_state,$nid) {
  if (!is_numeric($nid) || !isset($nid)) {
    form_set_error('no_email', t("Ingen mail indhold.")); 
    return;
  }
  $values = isset($form_state['multistep_values']['get_recipients']) ? $form_state['multistep_values']['get_recipients'] : array();

  module_load_include('inc', 'valghalla_volunteers');
  module_load_include('inc', 'valghalla_mail');

  // Get parties from Term objects.
  $taxonomy=taxonomy_vocabulary_machine_name_load('partier');

  foreach (taxonomy_get_tree($taxonomy->vid) as $tid => $term) {
    $parties[$term->tid] = $term->name;
  }

  // Elections
  $elections = db_select('node', 'n')
          ->fields('n', array('nid', 'title'))
          ->condition('type', 'election')
          ->execute()->fetchAllKeyed();


  // Polling stations
  $ps = db_select('node', 'n')
          ->fields('n', array('nid', 'title'))
          ->condition('type', 'polling_station')
          ->execute()->fetchAllKeyed();

  // Roles
  $roles= db_select('node', 'n')
          ->distinct()
          ->fields('n', array('title', 'nid'))
          ->condition('n.type', 'roles')
          ->execute()->fetchAllKeyed();

  if ($nid) {
    if ($node = node_load($nid))
      $description = ' <a class="form-submit" href="/node/' . $nid . '/edit?destination=/admin/valhalla/sendto/' . $nid . '">'.t('edit').'</a>';
    else $description = ' <a class="form-submit" href="/admin/valhalla/sendmail">Add</a>';
  }
  else $description = ' <a class="form-submit" href="/admin/valhalla/sendmail">Add</a>';
  $form['mail_content']['valghalla_mail_email_node_ID'] = array(
      '#title' => "Mail indhold",
      '#type' => 'select',
      '#options' => array($nid => isset($node->title) ? $node->title : ''),
      '#default_value' => $nid,
      '#description' => $description,
    );
  $form['recipents'] = array(
      '#type' => 'fieldset',
      '#title' => 'Modtagere',
      '#description' => "Hold CTRL nede for at v�lge flere. Ingen valgte vil sende til alle.",
  );
  $form['recipents']['election'] = array(
      '#type' => 'select',
      '#title' => 'Valg',
      '#multiple' => true,
      '#options' => $elections,
      '#size' => 5,
      '#prefix' => '<div style="float:left;margin-right:2em">',
      '#suffix' => '</div>',
      '#default_value' => isset($values['election']) ? $values['election'] : NULL,

  );
  $form['recipents']['rsvp_status'] = array(
      '#type' => 'select',
      '#title' => 'RSVP Status',
      //'#multiple' => true,
      '#options' => array(
          '1' => t('Ja'),
          '2' => t('Nej'),
          '3' => t('Aldrig'),
          '0' => t('Ubesvaret'),
      ),
      '#size' => 5,
      '#prefix' => '<div style="float:left;margin-right:2em">',
      '#suffix' => '</div>',
      '#default_value' => isset($values['rsvp_status']) ? $values['rsvp_status'] : 1,
  );
  $form['recipents']['roles'] = array(
      '#type' => 'select',
      '#title' => 'Frivillig type',
      '#multiple' => true,
      '#options' => $roles,
      '#size' => 5,
      '#prefix' => '<div style="float:left;margin-right:2em">',
      '#suffix' => '</div>',
      '#default_value' => isset($values['roles']) ? $values['roles'] : NULL,
  );
  $form['recipents']['party'] = array(
      '#type' => 'select',
      '#title' => 'Parti',
      '#multiple' => true,
      '#options' => $parties,
      '#size' => 5,
      '#prefix' => '<div style="float:left;margin-right:2em">',
      '#suffix' => '</div>',
      '#default_value' => isset($values['party']) ? $values['party'] : NULL,
  );
  $form['recipents']['polling_station'] = array(
      '#type' => 'select',
      '#title' => 'Valgsted',
      '#multiple' => true,
      '#options' => $ps,
      '#size' => 5,
      '#prefix' => '<div style="float:left;margin-right:2em">',
      '#suffix' => '</div>',
      '#default_value' => isset($values['polling_station']) ? $values['polling_station'] : NULL,
  );
  $form['sender'] = array(
      '#type' => 'fieldset',
      '#title' => 'Afsender',
  );
  $form['sender']['email_sender'] = array(
      '#type' => 'checkbox',
      '#title' => "brug min egen mail",
      '#description' => "Denne vil ogs� bruges som svar-til email, s� svar fra modtageren sendes til den p�g�ldende e-mailadresse. ",
      '#default_value' => isset($values['email_sender']) ? $values['email_sender'] : 0,
  );
  global $user;
  $form['sender']['user_email'] = array(
      '#type' => 'hidden',
      '#default_value' => $user->mail,
  );

  $form['next'] = array(
    '#type' => 'submit',
    '#value' => t('Next')
  );

  return $form;
}


/**
 * Form for Step 3 send_confirm step.
 * 
 * @see valghalla_mail_sendmail_form().
 *  
 * @param type $form
 * @param type $form_state
 * @return type 
 */
function valghalla_mail_sendmail_send_confirm_form($form, &$form_state) {
  global $language;

  $values = isset($form_state['multistep_values']['get_recipients']) ? $form_state['multistep_values']['get_recipients'] : array();

  $volunteers_nids = valghalla_mail_sendmail_get_mailList($values);

  //
  $count = 0;
  $mail_list = "";
  $to_mails = array();
  foreach (node_load_multiple($volunteers_nids) as $volunteer_node) {
    $to = $volunteer_node->field_email[$language->language][0]['email'];
    $recipients = $volunteer_node->title . " (".$to .")";
    $fc_id = $values['valghalla_mail_email_node_ID'] . "/" . $volunteer_node->nid;
    // Add link to preview
    $prefix = " <a class='js-show-volunteer-mail' id='" . $fc_id . "' href='#'>";
    $recipients = $prefix . $recipients ."</a>";
    if ($mail_list == "") {
      $mail_list .= $recipients;
      $to_mails[$volunteer_node->nid]= $to;
    }
    else {
      $mail_list .= ", ";
      $mail_list .= $recipients;
      // TODO duplicate mail addresses.
      $to_mails[$volunteer_node->nid]= $to;
    }
    $count += 1;
  }
  $form['confirm_email_node'] = array(
    '#title' => t('Mail indhold'),
    '#type' => 'fieldset',
    '#description' => t('Click name or email address to see email'),
  );
  $form['confirm_email_node']['js_mail_body'] = array(
    '#type' => 'item',
    '#prefix' => '<div class="js-volunteer-mail well">',
    '#markup' => '',
    "#suffix" => "</div>",
  );
  $form['confirm_info'] = array(
    '#title' => t('Er du sikker p� du vil udsende mail til @count modtagere ?', array('@count' => $count)),
    '#type' => 'fieldset',
    '#description' => $mail_list,
  );

  $form['back'] = array(
    '#type' => 'submit',
    '#value' => t('Tilbage')
  );
  
  $form['next'] = array(
    '#type' => 'submit',
    '#label' => 'Send!',
    '#value' => t('Submit')
  );
  $form_state['valghalla_sendlist'] = array(
    'valghalla_sendlist_array' => $to_mails,
    'valghalla_sendlist_acount' => $count,
  );
  //error_log("valghalla_sendlist =>".print_r($form_state['valghalla_sendlist'], TRUE),3, "/Applications/acquia-drupal/apache/logs/error_log");

  return $form;
}
/**
 * Get all the mail eddresses
 * @param $values Step 2 form_state
 * @return valghalla_mail_get_mail_byId
 *
 */
function valghalla_mail_sendmail_get_mailList($values) {
  global $language;

  module_load_include('inc', 'valhalla_volunteers');

  $rsvp_status = count($values['rsvp_status']) > 0 ? $values['rsvp_status'] : false;
  $role = count($values['roles']) > 0 ? $values['roles'] : false;
  $party = count($values['party']) > 0 ? $values['party'] : false;
  $polling_station = count($values['polling_station']) > 0 ? $values['polling_station'] : false;
  $election = count($values['election']) > 0 ? $values['election'] : false;

  // Prepare statement.
  $query = db_select('node', 'n');
  $query->fields('n', array('nid'));
  $query->condition('type', 'volunteers');

  if ($rsvp_status) {
    $query->leftjoin('field_data_field_rsvp', 'r', 'n.nid = r.entity_id');
    $sql = '{field_rsvp_value} IN (' . $rsvp_status . ')';
    if (in_array('0', $rsvp_status)) {
      $sql .= ' OR {field_rsvp_value} IS NULL';
    }
    $query->where($sql);
  }
  if ($role) {
    $query->join('field_data_field_label', 'v', 'n.nid = v.entity_id');
    $query->condition('field_label_value', $role);
  }
  if ($party) {
    $query->join('field_data_field_party', 'p', 'n.nid = p.entity_id');
    $query->condition('field_party_tid', $party);
  }

  if ($polling_station) {
    $query->join('field_data_field_polling_station', 'ps', 'n.nid = ps.entity_id');
    $query->condition('field_polling_station_nid', $polling_station);
  }

  // Get the results
  $volunteers_nids = $query->execute()->fetchAll(PDO::FETCH_COLUMN);

  // load election node
  $election = node_load($election);

  return $volunteers_nids;
}
/**
 * Implements hook_form_FORM_ID_alter()
 *
 * Insert info block with tokens, below body field.
 *
 * @author Thomas Thune Hansen <tth@bellcom.dk>
 */
function valghalla_mail_form_email_node_form_alter(&$form, &$form_state, $form_id){
  $form['body']['#suffix'] = "";
  $form['body']['#suffix'] = _valghalla_helper_get_volunteer_info_params('info') . $form['body']['#suffix'];
  $url = current_path();
  if ($url == "admin/valhalla/sendmail") {
    //dpm($form);
    //module_load_include('inc', 'valghalla_mail', 'includes/valghalla_mail_form');
    $form['field_valghalla_mail_type'][LANGUAGE_NONE]['#default_value'][0] = 2;
    $form['options']['status']['#default_value'] = 1;
    $form['additional_settings']['#prefix'] = '<div style="display:none;">';
    $form['additional_settings']['#suffix'] = '</div>';
    $form['actions']['submit']['#submit'][] = 'valghalla_mail_sendmail_form_step';
    $form['actions']['submit']['#value'] = t('Next');
    hide($form['actions']['preview']);
  }
}