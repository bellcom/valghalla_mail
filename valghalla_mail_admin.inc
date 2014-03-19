<?php
/**
 * Valghalla mail module
 *
 * @file valghalla_mail_admin.module
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

  drupal_add_css('#edit-mails, #edit-options { width: 49%; float: left; margin-right: 0.5%; } #edit-valghalla-role-mails {width: 100%;} #edit-valghalla-role-mails fieldset{width: 19%; float: left; margin-right: 0.4%;}',$option['type'] = 'inline');

  $mail_types = array(
    'invitation' => t('Invitation'),
    'reminder' => t('Påmindelse'),
    'rsvp_yes' => t('RSVP Ja'),
    'rsvp_no' => t('RSVP Nej'),
    'rsvp_never' => t('RSVP Aldrig'),
  );

  // Load all volunteer and mail role nodes
  $mail_nodes = node_load_multiple(array(), array('type' => 'email'));
  $role_nodes = node_load_multiple(array(), array('type' => 'roles'));

  $form['valghalla_volunteers_mail']['mails'] = array(
    '#type' => 'fieldset',
    '#title' => t('E-Mails'),
  );

  // Generate form for selecting general mails.
  $mail_options['-none-'] = t('None');
  foreach($mail_nodes as $mail_node){
    $mail_options[$mail_node->nid] = $mail_node->title;
  }

  foreach($mail_types as $mail_type => $mail_type_title){
    $description = t('Vælg standard mail for: ') . $mail_type_title;
    $mail_nid = variable_get('valghalla_mail_' . $mail_type, FALSE);

    if($mail_nid){
      $description .= ' <a class="form-submit" href="/node/' . $mail_nid . '/edit">'.t('edit').'</a>';
    }

    $form['valghalla_volunteers_mail']['mails']['valghalla_mail_' . $mail_type] = array(
      '#title' => $mail_type_title,
      '#type' => 'select',
      '#options' => $mail_options,
      '#default_value' => $mail_nid,
      '#description' => $description,
    );
  }

  $form['valghalla_volunteers_mail']['options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Options'),
  );

  $form['valghalla_volunteers_mail']['options']['valghalla_rsvp_reminder_days'] = array(
      '#type' => 'textfield',
      '#title' => t('Påmindelses dage'),
      '#description' => 'Liste af dage før valg der skal udsendes påmindelser, adskildt af komma ( , ). f.eks. 14, 7, 5, 3',
      '#default_value' => variable_get('valghalla_rsvp_reminder_days', '14, 7, 5, 3'),
  );

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
      '#title' => $role_node->title . ': <a href="/node/' . $role_node->nid . '/edit">' . t('edit') . '</a>',
    );

    // Show referenced mails for each role
    foreach($mail_types as $mail_type => $mail_type_title){
      $mail_nid = '';
      $mail_title = '';

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
      $description = '*';
      if(is_numeric($mail_nid)){
        $description = $mail_title .': <a href="/node/' . $mail_nid . '/edit">'.t('edit').'</a>';
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

function valghalla_volunteers_admin_mail_form($form, &$form_state) {
  module_load_include('inc', 'valghalla_volunteers');
  $form['valghalla_volunteers_mail'] = array(
      '#type' => 'fieldset',
      '#title' => 'Mail til automatisk udsendelse',
      '#suffix' =>  _valghalla_helper_get_volunteer_info_params('info'),
  );

  $form['valghalla_volunteers_mail']['valghalla_rsvp_email'] = array(
      '#type' => 'fieldset',
      '#title' => 'Mail til indkaldelser',
      '#collapsible' => true,
      '#collapsed' => true,
  );
  $form['valghalla_volunteers_mail']['valghalla_rsvp_email']['valghalla_rsvp_email_subject'] = array(
      '#type' => 'textfield',
      '#title' => t('Emne linie til indkalselde'),
      '#default_value' => variable_get('valghalla_rsvp_email_subject', _valghalla_volunteers_default_rsvp_email('subject')),
  );
  $form['valghalla_volunteers_mail']['valghalla_rsvp_email']['valghalla_rsvp_email_body'] = array(
      '#type' => 'textarea',
      '#title' => t('Indkaldelse mail indhold'),
      '#rows' => 15,
      '#default_value' => variable_get('valghalla_rsvp_email_body', _valghalla_volunteers_default_rsvp_email('body')),
  );

  $form['valghalla_volunteers_mail']['valghalla_rsvp_reminder'] = array(
      '#type' => 'fieldset',
      '#title' => 'Mail til påmindelser',
      '#collapsible' => true,
      '#collapsed' => true,
  );
  $form['valghalla_volunteers_mail']['valghalla_rsvp_reminder']['valghalla_rsvp_reminder_subject'] = array(
      '#type' => 'textfield',
      '#title' => t('Emne linie til påmindelses mail'),
      '#default_value' => variable_get('valghalla_rsvp_reminder_subject', _valghalla_volunteers_default_rsvp_reminder('subject')),
  );
  $form['valghalla_volunteers_mail']['valghalla_rsvp_reminder']['valghalla_rsvp_reminder_body'] = array(
      '#type' => 'textarea',
      '#title' => t('Påmindelses mail indhold'),
      '#rows' => 15,
      '#default_value' => variable_get('valghalla_rsvp_reminder_body', _valghalla_volunteers_default_rsvp_reminder('body')),
  );
  $form['valghalla_volunteers_mail']['valghalla_rsvp_reminder']['valghalla_rsvp_reminder_days'] = array(
      '#type' => 'textfield',
      '#title' => t('Påmindelses dage'),
      '#description' => 'Liste af dage før valg der skal udsendes påmindelser, adskildt af komma ( , ). f.eks. 14, 7, 5, 3',
      '#default_value' => variable_get('valghalla_rsvp_reminder_days', '14, 7, 5, 3'),
  );
  $form['valghalla_volunteers_mail']['valghalla_rsvp_status'] = array(
      '#type' => 'fieldset',
      '#title' => 'Bekræftelses mails til tilforordnede',
      '#collapsible' => false,
      '#collapsed' => false,
  );
  // Yes mail
  $form['valghalla_volunteers_mail']['valghalla_rsvp_status']['valghalla_rsvp_yes'] = array(
      '#type' => 'fieldset',
      '#title' => 'Mail til "Ja" svar.',
      '#collapsible' => true,
      '#collapsed' => true,
  );
  $form['valghalla_volunteers_mail']['valghalla_rsvp_status']['valghalla_rsvp_yes']['valghalla_rsvp_status_subject_yes'] = array(
      '#type' => 'textfield',
      '#title' => t('Emne linie'),
      '#default_value' => variable_get('valghalla_rsvp_status_subject_yes', _valghalla_volunteers_default_rsvp_status('subject_yes')),
  );
  $form['valghalla_volunteers_mail']['valghalla_rsvp_status']['valghalla_rsvp_yes']['valghalla_rsvp_status_body_yes'] = array(
      '#type' => 'textarea',
      '#title' => t('Mail indhold'),
      '#rows' => 15,
      '#default_value' => variable_get('valghalla_rsvp_status_body_yes', _valghalla_volunteers_default_rsvp_status('body_yes')),
  );
  // No mail
  $form['valghalla_volunteers_mail']['valghalla_rsvp_status']['valghalla_rsvp_no'] = array(
      '#type' => 'fieldset',
      '#title' => 'Mail til "Nej" svar',
      '#collapsible' => true,
      '#collapsed' => true,
  );
  $form['valghalla_volunteers_mail']['valghalla_rsvp_status']['valghalla_rsvp_no']['valghalla_rsvp_status_subject_no'] = array(
      '#type' => 'textfield',
      '#title' => t('Emne linie'),
      '#default_value' => variable_get('valghalla_rsvp_status_subject_no', _valghalla_volunteers_default_rsvp_status('subject_no')),
  );
  $form['valghalla_volunteers_mail']['valghalla_rsvp_status']['valghalla_rsvp_no']['valghalla_rsvp_status_body_no'] = array(
      '#type' => 'textarea',
      '#title' => t('Mail indhold'),
      '#rows' => 15,
      '#default_value' => variable_get('valghalla_rsvp_status_body_no', _valghalla_volunteers_default_rsvp_status('body_no')),
  );
  // Never mail
  $form['valghalla_volunteers_mail']['valghalla_rsvp_status']['valghalla_rsvp_never'] = array(
      '#type' => 'fieldset',
      '#title' => 'Mail til "Aldrig" svar',
      '#collapsible' => true,
      '#collapsed' => true,
  );
  $form['valghalla_volunteers_mail']['valghalla_rsvp_status']['valghalla_rsvp_never']['valghalla_rsvp_status_subject_never'] = array(
      '#type' => 'textfield',
      '#title' => t('Emne linie'),
      '#default_value' => variable_get('valghalla_rsvp_status_subject_never', _valghalla_volunteers_default_rsvp_status('subject_never')),
  );
  $form['valghalla_volunteers_mail']['valghalla_rsvp_status']['valghalla_rsvp_never']['valghalla_rsvp_status_body_never'] = array(
      '#type' => 'textarea',
      '#title' => t('Mail indhold'),
      '#rows' => 15,
      '#default_value' => variable_get('valghalla_rsvp_status_body_never', _valghalla_volunteers_default_rsvp_status('body_never')),
  );
  return system_settings_form($form);
}

/**
 * Extra validation needed to verify reminder days can be safely exloded.
 */
function valghalla_volunteers_admin_mail_form_validate(&$elements, &$form_state, $form_id = NULL) {
  if (preg_match('/[^\d|,| ]/', $form_state['values']['valghalla_rsvp_reminder_days'])) {
    // Found illigal chars in the form
    form_set_error('valghalla_rsvp_reminder_days', 'Dette felt skal være en komma-separeret liste af hele tal.');
  }
}

/**
 * Default texting for rsvp_email type emails.
 *
 * @param string $type either 'body' or 'subject'
 * @return string
 */
function _valghalla_volunteers_default_rsvp_email($type) {
  switch ($type) {
    case 'body':
      return <<< EOM
Kære !name

Århus Byråds "Udvalg for besættelse af udvalg m.m." har den !date udpeget dig som mulig valgtilforordnet ved Folketingsvalget 2011 den 18. oktober 2011 i Aarhus kommune.

Du er tilmeldt som !position. (VAF: valgstyrerformand, VA: valgstyrer, TI: tilforordnet)

På følgende afstemningssted:

!polling_station
Adresse: !polling_station_address

Du bedes møde på afstemningsstedet på valgdagen kl. 8.00 for at bistå valgbestyrelsen med afstemningen indtil kl. 20.00, og derefter så længe optællingen varer.

Der ydes forplejning i løbet af dagen. Efter beslutning i Valgbestyrelsen serveres der ikke alkohol. Når afstemningen er afsluttet vil der dog blive serveret natmad og en øl eller vand. Diæter udbetales efter gældende lovgivning via Nemkonto.

For at sikre os, at du kan komme, bedes du gå ind på følgende link og svare på denne invitation. Vi vil meget gerne have dit svar hurtigt, så vi kan reservere din plads eller tilbyde den til en anden, så vi kan få bemandet valget på forsvarlig demokratisk vis.

!link

Da der er brug for dig på afstemningsstedet hele dagen, bør du afgive stemme pr. brev forud for valg dagen, medmindre du skal afgive stemme på det afstemningssted, hvor du medvirker som valgstyrer. Sidste dag for brevafstemning er den 2. oktober 201. Du kan læse mere om brevafstemning på Aarhus Kommunes hjemmeside.

Med venlig hilsen
Valgsekretariatet
Aarhus Kommune
EOM;
    case 'subject':
      return 'Invitation til at være valgtilforordnet ved Folketingsvalget 2011';
    default:
  }
}

/**
 * Default texting for reminder mails.
 *
 * @param string $type
 * @return string
 */
function _valghalla_volunteers_default_rsvp_reminder($type) {
  switch ($type) {
    case 'body':
      return <<< EOM
 Hej !name,

Vi gør opmærksom på at der nu kun er !days til valget og du har endnu ikke meldt tilbage om du ønsker at være hjælper på dagen. Vi vil meget gerne have dit svar hurtigt, så vi kan reservere din plads eller tilbyde den til en anden, så vi kan få bemandet valget på forsvarlig demokratisk vis. Tak for din forståelse.

Brug følgende link til at svare:
!link

Med venlig hilsen
Jesper Eltved
Aarhus Kommune
EOM;
    case 'subject':
      return 'Påmindelse om tilbagemelding';
    default:
  }
}


/**
 * Default texting for confirmation mails.
 *
 * @param string $type
 * @return string
 */
function _valghalla_volunteers_default_rsvp_status($type) {
  switch ($type) {
    case 'body_yes':
      return <<< EOM
 Hej !name,

Din status er hermed bekræftet som værende accepteret.

Du skal derfor møde på:

!polling_station
Adresse: !polling_station_address

Med venlig hilsen
Jesper Eltved
Aarhus Kommune
EOM;
    case 'body_no':
      return <<< EOM
 Hej !name,

Din status er hermed bekræftet som værende ikke-accepteret.

Med venlig hilsen
Jesper Eltved
Aarhus Kommune
EOM;
    case 'body_never':
      return <<< EOM
 Hej !name,

Din status er hermed bekræft som værende afmeldt yderliger valgdeltagelse.

Med venlig hilsen
Jesper Eltved
Aarhus Kommune
EOM;
    case 'subject':
    case 'subject_yes':
    case 'subject_no':
    case 'subject_never':
      return 'Bekræftelse på valgdeltagelse';
    default:
  }
}
