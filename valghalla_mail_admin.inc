<?php
/**
 * Valghalla mail module
 *
 * @copyright 2012 OS2 - Offentligt Digitaliseringsfællesskab.
 * See README.md at https://github.com/os2web/valghalla_mail
 */

/**
 * Page callback for valghalla mails admin page
 *
 * @author Thomas Thune Hansen <tth@bellcom.dk>
 */
function valghalla_mail_admin_valghalla_mails(){
  // Light styling for the form.
  $css = "
    #edit-mails,
    #edit-options {
      width: 49%;
      float: left;
      margin-right: 0.5%;
    }
    #edit-valghalla-role-mails {
      width: 100%;
    }
    #edit-valghalla-mail-cancel {
      width: 49%;
    }
    #edit-valghalla-role-mails fieldset {
      width: 19%;
      float: left;
      margin-right: 0.4%;
    }
    .form-item-valghalla-mail-cancellation-from-participants-others {
      float: left;
      padding-right: 10px !important;
    }
    .form-item-valghalla-mail-cancellation-from-participants-to-mails{
      padding: 0 0 0 5px !important;
    }";

  drupal_add_css($css, $option['type'] = 'inline');

  // This array holds the mail types. Used for generating
  // forms. Array key corresponds field on volunteer role
  // node. Ex. field_{key}. And system variables for general
  // mails. Ex. valghalla_mail_{key}.
  $mail_types = array(
    'invitation' => t('Invitation'),
    'reminder' => t('Påmindelse'),
    'rsvp_yes' => t('RSVP Ja'),
    'rsvp_no' => t('RSVP Nej'),
    'rsvp_never' => t('RSVP Aldrig'),
    'remove' => t('Slet fra pladsen'),
  );

  // Load all volunteer and mail role nodes.
  $mail_nodes = node_load_multiple(array(), array('type' => 'email'));
  $role_nodes = node_load_multiple(array(), array('type' => 'roles'));

  $form['valghalla_volunteers_mail']['mails'] = array(
    '#type' => 'fieldset',
    '#title' => t('E-Mails'),
  );

  // Populate array with mail options for general mails.
  $mail_options[0] = t('None');
  foreach($mail_nodes as $mail_node){
    $mail_options[$mail_node->nid] = $mail_node->title;
  }

  // Generate form for selecting general mails.
  foreach($mail_types as $mail_type => $mail_type_title){
    $description = t('Vælg standard mail for: ') . $mail_type_title;
    $mail_nid = variable_get('valghalla_mail_' . $mail_type, FALSE);

    if($mail_nid){
      $description .= ' <a class="form-submit" href="/node/' . $mail_nid . '/edit?destination=/admin/valhalla/mails">'.t('edit').'</a>';
    }
    elseif (node_type_load('email')) {
      $description .= ' <a class="form-submit" href="/node/add/email?destination=/admin/valghalla/mails">'.t('add').'</a>';
    }
    $form['valghalla_volunteers_mail']['mails']['valghalla_mail_' . $mail_type] = array(
      '#title' => $mail_type_title,
      '#type' => 'select',
      '#options' => $mail_options,
      '#default_value' => $mail_nid,
      '#description' => $description,
    );
  }

  // Fieldset for mail options.
  $form['valghalla_volunteers_mail']['options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Options'),
  );

  // Reminder settings.
  $form['valghalla_volunteers_mail']['options']['valghalla_rsvp_reminder_days'] = array(
      '#type' => 'textfield',
      '#title' => t('Påmindelses dage'),
      '#description' => 'Liste af dage før valg der skal udsendes påmindelser, adskildt af komma ( , ). f.eks. 14, 7, 5, 3',
      '#default_value' => variable_get('valghalla_rsvp_reminder_days', '14, 7, 5, 3'),
  );

  // Cancellation from participants mail settings
  $roles = db_query("SELECT name, rid FROM {role} WHERE rid > 2");
  $form['valghalla_volunteers_mail']['valghalla_mail_cancel'] = array(
    '#type' => 'fieldset',
    '#title' => t('Besked ved afbud fra deltagere'),
    '#collapsible' => TRUE,
    '#description' => t('Vælg en eller flere rolle(r) at sende mail til, eller skriv mail adresser i tekst feltet.'),
    //'#collapsed' => TRUE,
  );
  foreach ($roles as $role) {
    if ($role->rid == 5 || $role->name == "afbud" || $role->name == "afbud mail") {
      $form['valghalla_volunteers_mail']['valghalla_mail_cancel']["valghalla_mail_cancellation_from_participants_".$role->rid]= array(
        '#type' => 'checkbox',
        '#title' => $role->name,
        '#default_value' => variable_get("valghalla_mail_cancellation_from_participants_".$role->rid),
      );
    }
  }
  $other_mails = variable_get("valghalla_mail_cancellation_from_participants_to_mails");
  $other_check = variable_get("valghalla_mail_cancellation_from_participants_others");
  if (!empty($other_mails) && empty($other_check)) {
    variable_set("valghalla_mail_cancellation_from_participants_others", 0);
    variable_set("valghalla_mail_cancellation_from_participants_to_mails","");
  }
  elseif (empty($other_mails) && $other_check) {
    variable_set("valghalla_mail_cancellation_from_participants_others", 0);
  }

  $form['valghalla_volunteers_mail']['valghalla_mail_cancel']['valghalla_mail_cancellation_from_participants_others'] = array(
      '#type' => 'checkbox',
      '#title' => t("Andre"),
      '#default_value' => variable_get("valghalla_mail_cancellation_from_participants_others"),
    );
  $form['valghalla_volunteers_mail']['valghalla_mail_cancel']['valghalla_mail_cancellation_from_participants_to_mails'] = array(
      '#type' => 'textfield',
      '#description' => t("Separer modtagere med kommaer"),
      '#default_value' => variable_get("valghalla_mail_cancellation_from_participants_to_mails"),
    );

  // Only choose the rsvp_no and rsvp_never types.
  foreach($mail_types as $mail_type => $mail_type_title){
    if ($mail_type == "rsvp_no" || $mail_type == "rsvp_never") {
      $description_2 = t('Vælg standard mail for: ') .$mail_type_title;
      $mailnid = variable_get("valghalla_mail_cancellation_from_participants_mail_$mail_type", FALSE);

      if($mailnid){
        $description_2 .= ' <a class="form-submit" href="/node/' . $mailnid . '/edit?destination=/admin/valghalla/mails">'.t('edit').'</a>';
      }
      elseif (node_type_load('email')) {
        $description_2 .= ' <a class="form-submit" href="/node/add/email?destination=/admin/valghalla/mails">'.t('add').'</a>';
      }
      $form['valghalla_volunteers_mail']['valghalla_mail_cancel']['valghalla_mail_cancellation_from_participants_mail_' . $mail_type] = array(
        '#type' => 'select',
        '#options' => $mail_options,
        '#default_value' => $mailnid,
        '#title' => $mail_type_title . t(' til rolle(r)'),
        '#description' => $description_2,
      );
    }
  }

  $form['valghalla_role_mails'] = array(
    '#title' => t('Rollespecifikke mails'),
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  // Generate fieldset for each role
  foreach($role_nodes as $role_node){
    $form['valghalla_role_mails'][$role_node->nid] = array(
      '#type' => 'fieldset',
      '#title' => $role_node->title . ': <a href="/node/' . $role_node->nid . '/edit?destination=/admin/valhalla/mails">' . t('edit') . '</a>',
    );

    // Show referenced mails for each role
    foreach($mail_types as $mail_type => $mail_type_title){
      $mail_nid = '';
      $mail_title = '';

      // If the node (role) has the field set for the mail type.
      // Get the node id and load the node, for additinal info.
      if(!empty($role_node->{'field_'.$mail_type})){
        $field_mail_type = field_get_items('node', $role_node, 'field_' . $mail_type);
        $mail_nid = $field_mail_type[0]['target_id'];
        $mail_node = node_load($mail_nid);
        // Check if mail_node is valid
        if(is_object($mail_node)){
          $mail_title = $mail_node->title;
        }
      }

      // Show mail title, and edit link if a mail is chosen.
      $description = t('standard');
      if(is_numeric($mail_nid)){
        $description = $mail_title .': <a href="/node/' . $mail_nid . '/edit?destination=/admin/valhalla/mails">'.t('edit').'</a>';
      }

      $form['valghalla_role_mails'][$role_node->nid][$mail_type] = array(
        '#type' => 'item',
        '#title' => $mail_type_title,
        '#description' => $description,
      );
    }
  }

  return system_settings_form($form);
}

/**
 * Extra validation needed to verify reminder days can be safely exloded.
 */
function valghalla_mail_admin_valghalla_mails_validate(&$elements, &$form_state, $form_id = NULL) {
  if (preg_match('/[^\d|,| ]/', $form_state['values']['valghalla_rsvp_reminder_days'])) {
    // Found illigal chars in the form
    form_set_error('valghalla_rsvp_reminder_days', 'Dette felt skal være en komma-separeret liste af hele tal.');
  }
}
