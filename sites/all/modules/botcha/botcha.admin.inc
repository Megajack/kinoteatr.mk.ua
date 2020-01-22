<?php

// @todo Migrate all necessary functions not to have this including.
module_load_include('inc', 'botcha', 'botcha');

/**
 * @file
 * Implementation of botcha administration forms.
 */

/**
 * Module settings form.
 */
function _botcha_admin_settings(&$form_state) {
  $form = array();
  $form['botcha_secret'] = array(
    '#type' => 'textfield',
    '#title' => t('Secret key'),
    '#default_value' => variable_get('botcha_secret', botcha_generate_secret_key() ),
    '#description' => t('It is recommended to enter some random text into the secret key. This setting makes your site\'s BOTCHA challenges unique and harder to break.')
      . '<br />' . t('If you leave this field empty and save configuration, a new random key will be generated for you.')
    ,
  );
  // BOTCHA Statistics & Logging
  $form['botcha_statistics'] = array(
    '#type' => 'fieldset',
    '#title' => t('Statistics & logging'),
    '#description' => t('BOTCHA collects statistics of form submissions and it can report different events into the system log.'),
  );
  $dblog_link = l(t('log'), 'admin/reports/dblog');
  $form['botcha_statistics']['botcha_loglevel'] = array(
    '#type' => 'select',
    '#title' => t('Log level'),
    '#default_value' => variable_get('botcha_loglevel', 1),
    '#options' => array(
      0 => t('0: no log'),
      1 => t('1: blocked/bad submissions only'),
      2 => t('2: ... and why blocked'),
      3 => t('3: ... and good submissions'),
      4 => t('4: ... and protected forms'),
      5 => t('5: ... and extra submission details'),
      6 => t('6: ... and misc development items'),
    ),
    '#description' => t('Select what information to report into the !log.', array('!log' => $dblog_link)),
  );
  // Button for resetting the BOTCHA statistics.
  $form['botcha_statistics']['botcha_statistics_group'] = array(
    '#type' => 'item',
    '#title' => t('BOTCHA statistics'),
    '#description' => t('Reset all accumulated statistics of form submissions.'),
  );
  // Show statistic counters.
  $block_cnt = variable_get('botcha_form_blocked_counter', 0);
  $build_cnt = variable_get('botcha_form_passed_counter', 0) + $block_cnt;
  $form['botcha_statistics']['botcha_statistics_group']['botcha_statistics'] = array(
    '#type' => 'item',
    '#value' => format_plural(
      $block_cnt,
      'Already 1 blocked form submission',
      'Already @count blocked form submissions'
    )
    . ($build_cnt > 0 ? ' ' . t('(!percent% of total !build_cnt processed)',
      array('!percent' => sprintf("%0.3f", 100 * $block_cnt / $build_cnt), '!build_cnt' => $build_cnt)) : ''),
  );
  $form['botcha_statistics']['botcha_statistics_group']['botcha_statistics_reset'] = array(
    '#type' => 'button',
    '#value' => t('Reset BOTCHA statistics'),
    '#submit' => array('botcha_statistics_reset'),
  );
  // Handle the button for resetting the BOTCHA statistics.
  // This is done here instead of in a submit handler because the button is
  // not a submitting button.
  if (isset($form_state['input']['op']) && $form_state['input']['op']
    == $form['botcha_statistics']['botcha_statistics_group']['botcha_statistics_reset']['#value']) {
    variable_set('botcha_form_passed_counter', 0);
    variable_set('botcha_form_blocked_counter', 0);
    drupal_set_message(t('BOTCHA statistics have been reset.'));
  }
  return $form;
}

/**
 * Custom theme function for a table of (form_id -> BOTCHA type) settings
 */
function theme_botcha_forms_form_botcha_forms($variables) {
  $form = $variables['form'];
  $header = array('form_id', t('Operations'));
  $rows = array();
  // Existing BOTCHA points.
  foreach (element_children($form['botcha_forms']) as $key) {
    $row = array();
    $row[] = drupal_render($form['botcha_forms'][$key]['form_id']);
    $row[] = drupal_render($form['botcha_forms'][$key]['operations']);
    $rows[] = $row;
  }
  $output = theme('table', array('header' => $header, 'rows' => $rows));
  return $output;
}

function botcha_admin_settings($form, &$form_state) {
//  $form = system_settings_form(_botcha_admin_settings($form_state));
  // We can't use system_settings_form() here because it will put all extra stuff into variables, that we want to avoid.
  $form = _botcha_admin_settings($form_state);

  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );
  $form['buttons']['reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset to defaults'),
  );

//  if (!empty($_POST) && form_get_errors()) {
//    drupal_set_message(t('The settings have not been saved because of the errors.'), 'error');
//  }
//  $form['#submit'][] = 'system_settings_form_submit';
  $form['#theme'] = 'system_settings_form';

//  $form['#validate'][] = 'botcha_admin_settings_validate';
//  $form['#submit'][] = 'botcha_admin_settings_submit';
  return $form;
}

/**
 * Submission function for botcha_admin_settings form.
 */
function botcha_admin_settings_submit($form, &$form_state) {
  // Generate the secret key.
  // @todo botcha_admin_settings_submit Move secret key generation to validate phase.
  if (empty($form_state['values']['botcha_secret'])) {
    // Generate unique secret for this site
    $secret = botcha_generate_secret_key();
    $form_state['values']['botcha_secret'] = $secret;
    drupal_set_message(t('New BOTCHA secret key have been generated.'));
  }
  // Do what system_settings_form() would do with regular variable fields
  variable_set('botcha_secret', $form_state['values']['botcha_secret']);
  variable_set('botcha_loglevel', $form_state['values']['botcha_loglevel']);
  drupal_set_message(t('The BOTCHA settings were saved.'), 'status');
}

/**
 * Edit existent or add a new recipe book.
 * @param array $form
 *  Form API form array.
 * @param array $form_state
 *  Form API form state array.
 * @param BotchaRecipebook $recipebook
 *  Recipe book object.
 */
function botcha_recipebook_form($form, &$form_state, $recipebook = NULL) {
  // Determine default values depending on whether we add or edit recipe book.
  // Form a list of recipes.
  $options_recipes = array();
  foreach (Botcha::getRecipes() as $recipe) {
    $options_recipes[$recipe->id] = $recipe->title;
  }
  if (empty($recipebook)) {
    $disabled_id = FALSE;
    $default_id = '';
    $default_title = '';
    $default_description = '';
    $default_recipes = array();
    $button = t('Add');
  }
  else {
    $disabled_id = TRUE;
    $default_id = $recipebook->id;
    $default_title = $recipebook->title;
    $default_description = $recipebook->description;
    $default_recipes = array_keys($recipebook->getRecipes());
    $button = t('Save');
  }
  $form['id'] = array(
    '#type' => 'machine_name',
    '#default_value' => $default_id,
    '#disabled' => $disabled_id,
    '#maxlength' => 128,
    '#machine_name' => array(
      'exists' => 'botcha_recipebook_exists',
    ),
  );
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#description' => t('A title for this recipe book. You can always change this name later.'),
    '#default_value' => $default_title,
    '#required' => TRUE,
    '#maxlength' => 128,
  );
  $form['description'] = array(
    '#type' => 'textarea',
    '#rows' => 5,
    '#title' => t('Description'),
    '#description' => t('A description of the recipe book.'),
    '#default_value' => $default_description,
  );
  $form['recipes'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Recipes'),
    '#description' => t('Choose what recipes are included in recipe book.'),
    '#options' => $options_recipes,
    '#default_value' => $default_recipes,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => $button,
  );
  return $form;
}

/**
 * Submit handler for botcha_recipebook_form.
 * @param $form
 *  Form API form array.
 * @param $form_state
 *  Form API form state array.
 */
function botcha_recipebook_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  $recipebook = Botcha::getRecipebook($values['id'])
    ->setTitle($values['title'])
    ->setDescription($values['description']);
  /*
  foreach (array_filter($values['recipes']) as $recipe_id) {
    $recipebook->setRecipe($recipe_id);
  }
   *
   */
  foreach ($values['recipes'] as $recipe_id => $value) {
    if ($value) {
      $recipebook->setRecipe($recipe_id);
    }
    else {
      $recipebook->unsetRecipe($recipe_id);
    }
  }
  $recipebook->save();
  $form_state['redirect'] = 'admin/config/people/botcha/recipebook';
  drupal_set_message(t('Settings for recipe book "%recipebook" are successfully saved.', array('%recipebook' => $recipebook->id)), 'status');
}

/**
 * Delete configuration form.
 */
function botcha_recipebook_delete_form($form, &$form_state, $recipebook) {
  $form['#recipebook'] = $recipebook;
  return confirm_form(
    $form,
    t('Would you really like to delete the recipe book @recipebook?', array('@recipebook' => $recipebook->title)),
    'admin/config/people/botcha/recipebook',
    t('This action cannot be undone.'),
    t('Delete')
  );
}

/**
 * Submit handler for botcha_recipebook_delete_form().
 */
function botcha_recipebook_delete_form_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/config/people/botcha/recipebook';
  // Remove recipe book.
  $form['#recipebook']->delete();
}

function botcha_recipebook_exists($value) {
  return !(Botcha::getRecipebook($value, FALSE) instanceof BotchaRecipebookNone);
}

/**
 * Edit existent or add BOTCHA protection to another form.
 * @param array $form
 *  Form API form array.
 * @param array $form_state
 *  Form API form state array.
 * @param BotchaForm $botcha_form
 *  Botcha form object.
 */
function botcha_form_form($form, $form_state, $botcha_form = NULL) {
  $form = array();
  // Determine default values depending on whether we add or edit form.
  $edit = !empty($botcha_form);
  if ($edit) {
    // @todo Refactor the protection from editing unexisting forms.
    if ($botcha_form instanceof BotchaFormNone) {
      drupal_goto('admin/config/people/botcha/form/add', array('query' => array('botcha_form_id' => $botcha_form->id)));
    }
    $operation = 'edit';
    $recipebook = $botcha_form->getRecipebook();
    $botcha_form_id_default = check_plain($botcha_form->id);
    $button = t('Save');
  }
  else {
    $operation = 'add';
    // 'default' will be selected.
    $recipebook = Botcha::getRecipebook();
    $query_params = drupal_get_query_parameters();
    $botcha_form_id_default = !empty($query_params['botcha_form_id']) ? $query_params['botcha_form_id'] : NULL;
    $button = t('Add');
  }
  // Insert a field that allows to understand whether it is creation or edition form submission.
  $form['botcha_operation'] = array(
    '#type' => 'value',
    '#value' => $operation,
  );
  // Use passed as a parameter form id.
  $form['botcha_form_id'] = array(
    '#type' => 'machine_name',
    '#title' => t('Form ID'),
    '#description' => t('The Drupal form_id of the form to add the BOTCHA protection to.'),
    '#default_value' => $botcha_form_id_default,
    '#disabled' => $edit,
    '#maxlength' => 128,
    '#machine_name' => array(
      'exists' => 'botcha_form_exists',
    ),
  );
  $options = array();
  $recipebooks = Botcha::getRecipebooks(TRUE);
  foreach ($recipebooks as $rb) {
    $options[$rb->id] = $rb->title;
  }
  $form['botcha_form_recipebook'] = array(
    '#type' => 'select',
    '#title' => t('Recipe book'),
    '#description' => t('Recipe book to apply to the form.'),
    '#default_value' => $recipebook->id,
    '#options' => $options,
  );
  // @todo Why not just general submit field?
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => $button,
  );
  return $form;
}

function botcha_form_exists($value) {
  return !(Botcha::getForm($value, FALSE) instanceof BotchaFormNone);
}

/**
 * Submit handler for botcha_form_form.
 */
function botcha_form_form_submit($form, &$form_state) {
  $form_id = $form_state['values']['botcha_form_id'];
  $rbid = $form_state['values']['botcha_form_recipebook'];
  $operation = $form_state['values']['botcha_operation'];
  $add = ($operation == 'add');
  Botcha::getForm($form_id, $add)
    ->setRecipebook($rbid)
    ->save();
  switch ($operation) {
    case 'edit':
      drupal_set_message(t('Saved BOTCHA form settings for %form_id.', array('%form_id' => $form_id)), 'status');
      break;
    case 'add':
    default:
      drupal_set_message(t('Added BOTCHA form %form_id.', array('%form_id' => $form_id)), 'status');
      break;
  }
  $form_state['redirect'] = 'admin/config/people/botcha/form';
}

/**
 * Confirm dialog for deleting a BOTCHA form.
 */
function botcha_form_delete_form($form, $form_state, $botcha_form = NULL) {
//function botcha_point_disable_confirm($form, &$form_state, $botcha_point_form_id, $delete) {
  $form = array();
  $form['botcha_form_id'] = array(
    '#type' => 'value',
    '#value' => $botcha_form->id,
  );
  $message = t('Are you sure you want to delete the BOTCHA protection for form_id %form_id?', array('%form_id' => $botcha_form->id));
  return confirm_form($form, $message, 'admin/config/people/botcha', NULL, t('Delete'));
}

/**
 * Submission handler of BOTCHA form deleting.
 */
function botcha_form_delete_form_submit($form, &$form_state) {
  $form_id = $form_state['values']['botcha_form_id'];
  Botcha::getForm($form_id, FALSE)
    ->delete();
  drupal_set_message(t('Deleted BOTCHA protection for form %form_id.', array('%form_id' => $form_id)));
  $form_state['redirect'] = 'admin/config/people/botcha/form';
}

/**
 * Callback for "Forms" admin page.
 * Configuration of which forms to protect, with what recipe.
 */
function botcha_forms_form() {
  $form = array();
  if (module_exists('captcha')) {
    // Checkbox to put BOTCHA on same forms as CAPTCHA.
    $form['botcha_form_protection']['botcha_on_captcha_forms'] = array(
      '#type' => 'checkbox',
      '#title' => t('Add BOTCHA to forms selected for CAPTCHA'),
      '#default_value' => variable_get('botcha_on_captcha_forms', TRUE),
      '#description' => t('This option makes it easy to manage BOTCHA settings on forms. When enabled, all forms that are configured to have CAPTCHA on them (<a href="@captcha">see configuration</a>) will also be selected for BOTCHA protection.!more',
        array(
          '@captcha' => url('admin/config/people/captcha'),
          '!more' => module_exists('captcha') ? '' : '<br />' . t('<b>Note:</b> <a href="@captcha_home">CAPTCHA module</a> is not installed. This setting will have no effect.',
            array('@captcha_home' => 'http://drupal.org/project/captcha')
          ),
        )),
    );
  }
  // List known form_ids.
  $form['botcha_form_protection']['botcha_form_id_overview'] = array(
    '#theme' => 'botcha_forms_form_botcha_forms',
    '#tree' => TRUE,
  );
  $form['botcha_form_protection']['botcha_form_id_overview']['botcha_forms'] = array();
  $forms = Botcha::getForms(TRUE);
  foreach ($forms as $botcha_form) {
    $form['botcha_form_protection']['botcha_form_id_overview']['botcha_forms'][$botcha_form->id] = array();
    $form['botcha_form_protection']['botcha_form_id_overview']['botcha_forms'][$botcha_form->id]['form_id'] = array(
      '#markup' => $botcha_form->id,
    );
    // Additional operations.
    $form['botcha_form_protection']['botcha_form_id_overview']['botcha_forms'][$botcha_form->id]['operations'] = array(
      '#markup' => implode(" | ", array(
        l(t('Edit'), "admin/config/people/botcha/form/{$botcha_form->id}"),
        l(t('Delete'), "admin/config/people/botcha/form/{$botcha_form->id}/delete"),
      )),
    );
  }
  // Field for the BOTCHA administration mode.
  $form['botcha_form_protection']['botcha_administration_mode'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add BOTCHA administration links to forms'),
    '#default_value' => variable_get('botcha_administration_mode', FALSE),
    '#description' => t('This option makes it easy to manage BOTCHA settings on forms. When enabled, users with the "%adminbotcha" permission will see a fieldset with BOTCHA administration links on all forms, except on administrative pages.', array('%adminbotcha' => t('administer BOTCHA settings'))),
  );
  // Field for the BOTCHAs on admin pages.
  $form['botcha_form_protection']['botcha_allow_on_admin_pages'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow BOTCHAs and BOTCHA administration links on administrative pages'),
    '#default_value' => variable_get('botcha_allow_on_admin_pages', FALSE),
    '#description' => t('This option makes it possible to add BOTCHAs to forms on administrative pages. BOTCHAs are disabled by default on administrative pages (which shouldn\'t be accessible to untrusted users normally) to avoid the related overhead. In some situations, e.g. in the case of demo sites, it can be usefull to allow BOTCHAs on administrative pages.'),
  );
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );
  $form['#theme'] = 'system_settings_form';
  return $form;
}

/**
 * Submission handler for botcha_forms_form form.
 */
function botcha_forms_form_submit($form, &$form_state) {
  // Do what system_settings_form() would do with regular variable fields
  variable_set('botcha_on_captcha_forms', !empty($form_state['values']['botcha_on_captcha_forms']) ? $form_state['values']['botcha_on_captcha_forms'] : FALSE);
  variable_set('botcha_administration_mode', $form_state['values']['botcha_administration_mode']);
  variable_set('botcha_allow_on_admin_pages', $form_state['values']['botcha_allow_on_admin_pages']);
  drupal_set_message(t('The BOTCHA settings were saved.'), 'status');
}

/**
 * Callback for "Recipe books" admin page.
 * @todo ?Is it form really? Perhaps table?
 */
function botcha_recipebooks_form() {
  $form['#header'] = array(
    t('Title'),
    t('Description'),
    t('Operations'),
  );
  // Get all recipe books from database.
  $recipebooks = Botcha::getRecipebooks();
  // Protect default recipebook from being deleted.
  foreach ($recipebooks as $recipebook) {
    $form['recipebooks'][$recipebook->id]['title']['#markup'] = $recipebook->title;
    $form['recipebooks'][$recipebook->id]['description']['#markup'] = $recipebook->description;
    $form['recipebooks'][$recipebook->id]['operations']['#markup'] = l(t('Edit'), 'admin/config/people/botcha/recipebook/' . $recipebook->id)
      . (($recipebook->id == 'default') ? '' : ' | ' . l(t('Delete'), 'admin/config/people/botcha/recipebook/' . $recipebook->id . '/delete'));
  }
  return $form;
}

/**
 * Theme botcha_recipebooks_form().
 */
function theme_botcha_recipebooks_form($variables) {
  $form = $variables['form'];
  // Iterate through all recipebooks and build a table.
  $rows = array();
  //foreach (array('enabled', 'disabled') as $type) {
  //  if (isset($form[$type])) {
      foreach (element_children($form['recipebooks']) as $id) {
        $row = array();
        foreach (element_children($form['recipebooks'][$id]) as $col) {
          $row[$col] = array(
            'data' => drupal_render($form['recipebooks'][$id][$col]),
          );
        }
        $rows[] = array(
          'data' => $row,
        );
      }
  //  }
  //}

  $output = theme('table', array(
    'header' => $form['#header'],
    'rows' => $rows,
    'empty' => t('No recipebooks available.'),
  ));

  if (!empty($rows)) {
    $output .= drupal_render_children($form);
  }

  return $output;
}

/**
 * Generate a random secret key.
 */
function botcha_generate_secret_key() {
  return md5(uniqid(mt_rand(), TRUE));
}

//END