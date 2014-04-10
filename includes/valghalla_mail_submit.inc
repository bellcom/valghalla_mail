<?php
/**
 * @file
 * Handles the form submission of the valghalla_mail_sendmail form
 * @author Yani Xu <yx@bellcom.dk>
 */

/**
 * Handles what to do with the submitted form depending on what stage has been
 * completed.
 *
 * @see valghalla_mail_sendmail_form()
 */
function valghalla_mail_sendmail_form_submit($form, &$form_state) {
  switch ($form_state['stage']) {
    case 'send_confirm':
      $form_state['multistep_values'][$form_state['stage']] = $form_state['values'];
      if ($form_state['triggering_element']['#value'] != 'Tilbage') {
        valghalla_mail_sendmail_send_confirm_submit($form, $form_state);
        $form_state['complete'] = TRUE;
      }
      break;

    default:
      $form_state['multistep_values'][$form_state['stage']] = $form_state['values'];
      $form_state['new_stage'] = valghalla_mail_sendmail_move_to_next_stage($form, $form_state);
      break;
  }

  if (isset($form_state['complete'])) {
    drupal_set_message("Mails sendt");
    drupal_goto('/admin/valhalla/sendmail');
  }

  if ($form_state['triggering_element']['#value'] == 'Tilbage') {
    $form_state['new_stage'] = valghalla_mail_sendmail_move_to_previous_stage($form, $form_state);
  }

  if (isset($form_state['multistep_values']['form_build_id'])) {
    $form_state['values']['form_build_id'] = $form_state['multistep_values']['form_build_id'];
  }
  $form_state['multistep_values']['form_build_id'] = $form_state['values']['form_build_id'];
  $form_state['stage'] = $form_state['new_stage'];
  $form_state['rebuild'] = TRUE;
}

 /**
  * Handles the submission of the final stage
  * Step 3 : Send mails .(TODO: save in pool table, queued and wait for cron job to send email.)
  *
  * @param type $form
  * @param type $form_state
  * @author yani
  */
function valghalla_mail_sendmail_send_confirm_submit($form, &$form_state) {
  $multstep_values = $form_state['multistep_values'];
  $nid = $multstep_values['get_recipients']['valghalla_mail_email_node_ID'];
  $sendMailList_count = $form_state['valghalla_sendlist']['valghalla_sendlist_acount'];
  $mails = $form_state['valghalla_sendlist']['valghalla_sendlist_array'];

  // TODO : Set email queue size to 500 email. TODO: admin config bar.
  // $queue = $sendMailList_count/5;
  // Save email node and volunteer email addresses to mail pool table.
  //valghalla_mail_save_mpool($mpool);
  $mail_sender = $multstep_values['get_recipients']['email_sender'];
  if ($mail_sender) {
    $from = $multstep_values['get_recipients']['user_email'];
  }
  else {
    $from = variable_get('site_mail', 'admin@example.com');
  }
  $language = language_default();

  foreach ($mails as $key => $to) {
    $volunteer_node = node_load($key);
    $params = valghalla_mail_get_mail(FALSE, FALSE, $nid);
    $tokens = _valhalla_helper_get_volunteer_info_params($volunteer_node);
    $params = array_merge(array('tokens' => $tokens), $params);
    $result = drupal_mail('valghalla_mail', 'mail', $to, $language, $params, $from, TRUE);
    if ($result['result'] == TRUE) {
      $msg = t('Mail nid = %nid is sent to email=%email');
      watchdog(
        'Valghalla mail', $msg, $variables = array(
          '%nid' => $nid,
          '%email' => $to
          )
      );
    }
    else {
      $msg = t('Error : Mail nid = %nid is NOT sent to email=%email');
      watchdog(
        'Valghalla mail', $msg, $variables = array(
          '%nid' => $nid,
          '%email' => $to
          )
      );
    }
  }
}
