<?php

/*
 * Controller layer of the BotchaRecipebook objects.
 */
class BotchaRecipebook {

  /**
   * Identifier of the recipe book.
   */
  public $id;

  /**
   * A title of the book.
   */
  public $title;

  /**
   * Description of the recipe book.
   */
  public $description;

  /**
   * List of recipes.
   * @var BotchaRecipe
   */
  protected $recipes = array();
  /**
   * List of forms.
   * @var BotchaForm
   */
  protected $forms = array();

  public static function getRecipebook($id, $create = TRUE) {
    $recipebook = NULL;
    $rb = BotchaRecipebookModel::getRecipebook($id);
    if (!$rb && !$create) {
      return new BotchaRecipebookNone($id);
    }
    else { //($rb || $create)
      $recipebook = new BotchaRecipebook($id);
      if ($rb) {
        $recipebook->setTitle($rb->title)
          ->setDescription($rb->description);
      }
    }
    return $recipebook;
  }

  protected function __construct($id) {
    $this->id = $id;
  }

  public function isApplicable(&$form, $form_state) {
    $form_id = $form['form_id']['#value'];
    // @todo BotchaRevipebook isApplicable Think about a better place for admin links. That is the problem because it is the only reason why the form is passed by reference.
    // Admin links functionality.
    if (variable_get('botcha_administration_mode', FALSE)
      && user_access('administer BOTCHA settings')
      && (arg(0) != 'admin'
        || variable_get('botcha_allow_on_admin_pages', FALSE)
        || ($form_id == 'user_register'))) {
      // Add BOTCHA administration tools.
      $recipebook = Botcha::getForm($form_id, FALSE)->getRecipebook();
      // !!- $botcha_point = botcha_get_form_id_setting($form_id);
      // For administrators: show BOTCHA info and offer link to configure it
      $botcha_element = array(
        '#type' => 'fieldset',
        '#title' => t('BOTCHA'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#attributes' => array('class' => array('botcha-admin-links')),
      );
      if (!($recipebook instanceof BotchaRecipebookNone)) {
      // !!- if ($botcha_point !== NULL && $botcha_point->botcha_type  && $botcha_point->botcha_type != 'none') {
        $botcha_element['#title'] = t('BOTCHA: protection enabled (@recipebook recipe book)', array('@recipebook' => $recipebook->id));
        // !!- $botcha_element['#title'] = t('BOTCHA: protection enabled (@type cookbook)', array('@type' => $botcha_point->botcha_type));
        // !!- $botcha_element['#description'] = t('Untrusted users will have form %form_id protected by BOTCHA (!settings).',
        $botcha_element['#description'] = t('Untrusted users will have form %form_id protected by BOTCHA (!recipebook_settings, !general_settings).',
          // !!- array('%form_id' => $form_id, '!settings' => l(t('general BOTCHA settings'), 'admin/config/people/botcha'))
          array(
            '%form_id' => $form_id,
            '!recipebook_settings' => l(t('Recipe book settings'), "admin/config/people/botcha/recipebook/{$recipebook->id}"),
            '!general_settings' => l(t('General BOTCHA settings'), 'admin/config/people/botcha'),
          )
        );
        $botcha_element['protection'] = array(
          '#type' => 'item',
          '#title' => t('Enabled protection'),
          '#markup' => t('Form is protected by "@recipebook" recipe book (!edit, !delete)', array(
          // !!- '#markup' => t('"@type" cookbook (!change, !disable)', array(
            '@recipebook' => $recipebook->id,
            // !!- '@type' => $botcha_point->botcha_type,
            '!edit' => l(t('edit'), "admin/config/people/botcha/form/$form_id", array('query' => drupal_get_destination(), 'html' => TRUE)),
            // !!- '!change' => l(t('change'), "admin/config/people/botcha/botcha_point/$form_id", array('query' => drupal_get_destination(), 'html' => TRUE)),
            '!delete' => l(t('delete'), "admin/config/people/botcha/form/$form_id/delete", array('query' => drupal_get_destination(), 'html' => TRUE)),
            // !!- '!disable' => l(t('disable'), "admin/config/people/botcha/botcha_point/$form_id/disable", array('query' => drupal_get_destination(), 'html' => TRUE)),
          )),
        );
      }
      else {
        $botcha_element['#title'] = t('BOTCHA: no protection enabled');
        $botcha_element['add_botcha'] = array(
          '#markup' => l(t('Add BOTCHA protection on form'), "admin/config/people/botcha/form/add", array('query' => array_merge(drupal_get_destination(), array('botcha_form_id' => $form_id)), 'html' => TRUE)),
          // !!- '#markup' => l(t('Add BOTCHA protection on form %form_id for untrusted users.', array('%form_id' => $form_id)), "admin/config/people/botcha/botcha_point/$form_id", array('query' => drupal_get_destination(), 'html' => TRUE)),
        );
      }
      // Get placement in form and insert in form.
      // @todo BotchaRecipebook isApplicable Make away with a dependency from botcha.inc.
      $botcha_placement = _botcha_get_botcha_placement($form_id, $form);
      _botcha_insert_botcha_element($form, $botcha_placement, $botcha_element);
    }
    $isApplicable = FALSE;
    if (!user_access('skip BOTCHA')) {
      $isApplicable = TRUE;
    }
    switch ($form_id) {
      case 'user_register':
        // Only change the registration form. There is also 'user_register' form
        // at /admin/config/people/user/create path, but we leave it alone.
        if (FALSE === strpos($form['#action'], 'user/register')) {
          if (!variable_get('botcha_allow_on_admin_pages', FALSE)) {
            $isApplicable = FALSE;
          }
        }
        break;
    }
    return $isApplicable;
  }

  public function save() {
    if ($this instanceof BotchaRecipebookNone) {
      // @todo Refactor this with delete() : unify it.
      Botcha::unsetRecipebook($this);
    }
    else {
      // Save our recipe book to cache.
      Botcha::setRecipebook($this);
    }
    // Save recipe book to DB.
    BotchaRecipebookModel::save($this);
  }

  public function delete() {
    Botcha::unsetRecipebook($this);
    // Delete recipe book from DB.
    BotchaRecipebookModel::delete($this);
  }

  function setTitle($title) {
    $this->title = $title;
    return $this;
  }

  function getTitle() {
    return $this->title;
  }

  function setDescription($description) {
    $this->description = $description;
    return $this;
  }

  function getDescription() {
    return $this->description;
  }

  function setRecipe($recipe_id) {
    $this->recipes[$recipe_id] = $recipe_id;
    return $this;
  }

  function unsetRecipe($recipe_id) {
    unset($this->recipes[$recipe_id]);
    return $this;
  }

  /**
   * @todo BotchaRecipebook getRecipes Description.
   * @return array
   */
  function getRecipes() {
    $recipes = array();
    // @todo BotchaRecipebook getRecipes Move it to model layer.
    if (!isset($this->recipes)) {
      $this->recipes = db_select('botcha_recipebook_recipe', 'brr')
        ->fields('brr', array('recipe_id'))
        ->condition('rbid', $this->id)
        ->execute()
        ->fetchCol();
    }
    foreach ($this->recipes as $recipe_id) {
      $recipes[$recipe_id] = Botcha::getRecipe($recipe_id);
    }
    return $recipes;
  }

  function setForm($form_id) {
    $this->forms[$form_id] = $form_id;
    return $this;
  }

  /**
   * @todo BotchaRecipebook getForms Description.
   * @return BotchaForm
   */
  function getForms() {
    $forms = array();
    // @todo BotchaRecipebook getForms Move it to model layer.
    if (!isset($this->forms)) {
      $this->forms = db_select('botcha_recipebook_form', 'brf')
        ->fields('brf', array('form_id'))
        ->condition('rbid', $this->id)
        ->execute()
        ->fetchCol();
    }
    foreach ($this->forms as $form_id) {
      $forms[$form_id] = Botcha::getForm($form_id);
    }
    return $forms;
  }

  /**
   * Get the list of recipes by status of spam checking.
   *
   * @param string $status
   * @return array
   */
  function getRecipesByStatus($status) {
    $recipes_list = array();
    foreach ($this->getRecipes() as $recipe) {
      if ($recipe->status == $status) {
        $recipes_list[$recipe->id] = $recipe;
      }
    }
    return $recipes_list;
  }

  /**
   * Handle form depending on the result of spam check.
   *
   * @param string $result
   *   This parameter is string and not boolean to have a chance to easily implement
   *   new results of spam check (such as 'postponed', 'suspected' or other).
   * @param array $form
   * @param array $form_state
   */
  function handle($result, $form, $form_state) {
    $recipes_success = $this->getRecipesByStatus('success');
    $recipes_success_count = count($recipes_success);
    $recipes_spam = $this->getRecipesByStatus('spam');
    $recipes_spam_count = count($recipes_spam);
    // !!~ @todo Recipebook handle Reduce code duplication.
    switch ($result) {
      case 'success':
        // !!~ @todo Recipebook handle Throw away these variables, use object properties.
        variable_set('botcha_form_passed_counter', $recipes_success_count);
        // Show good submissions in log.
        if (BOTCHA_LOGLEVEL >= 3) {
          watchdog(BOTCHA_LOG,
            '%form_id post approved by BOTCHA.!more',
            array(
              '%form_id' => $form['#id'],
              '!more' => ''
                . ((BOTCHA_LOGLEVEL >= 3) ? ' Checked ' . count($this->recipes) . ' botchas (' . join(', ', array_keys($this->recipes)) . ').' : '')
                . ((BOTCHA_LOGLEVEL >= 5) ? '<br /><br />' . 'POST=<pre>' . print_r(_botcha_filter_form_values_log($_POST), 1) . '</pre>' : '')
                . ((BOTCHA_LOGLEVEL >= 5) ? '<br /><br />' . 'GET=<pre>' . print_r(_botcha_filter_form_values_log($_GET), 1) . '</pre>' : '')
                . ((BOTCHA_LOGLEVEL >= 5) ? '<br /><br />' . 'SERVER=<pre>' . print_r($_SERVER, 1) . '</pre>' : '')
                . ((BOTCHA_LOGLEVEL >= 5) ? '<br /><br />' . ' form=<pre>' . print_r(_botcha_filter_form_log($form), 1) . '</pre>' : '')
                . ((BOTCHA_LOGLEVEL >= 5) ? '<br /><br />' . ' values=<pre>' . print_r(_botcha_filter_form_values_log($form_state['values']), 1) . '</pre>' : ''),
            ),
            WATCHDOG_INFO
          );
        }
        $rules_event_name = 'botcha_form_approved';
        break;
      case 'spam':
      default:
        // !!~ @todo Recipebook handle Throw away these variables, use object properties.
        variable_set('botcha_form_blocked_counter', $recipes_spam_count);
        // Prepare a list of failed recipes.
        foreach ($recipes_spam as $recipe_spam) {
          $recipe_spam_ids[$recipe_spam->id] = $recipe_spam->id;
        }
        // Just using the first failed recipe to reject form submission.
        $recipe_spam = reset($recipes_spam);
        form_set_error($recipe_spam->error_field, $recipe_spam->error_text);
        // Show blocked submissions in log.
        if (BOTCHA_LOGLEVEL >= 1) {
          watchdog(BOTCHA_LOG,
          '%form_id post blocked by BOTCHA: submission looks like from a spambot.!more',
          array(
            '%form_id' => $form['#id'],
            '!more' => ''
              . ((BOTCHA_LOGLEVEL >= 2) ? '<br /><br />' . ' Failed  ' . $recipes_spam_count . ' of ' . count($this->recipes) . ' recipes [' . implode(', ', $recipe_spam_ids) . '] from "' . $this->id . '" recipe book.' : '')
              . ((BOTCHA_LOGLEVEL >= 5) ? '<br /><br />' . 'POST=<pre>' . print_r(_botcha_filter_form_values_log($_POST), 1) . '</pre>' : '')
              . ((BOTCHA_LOGLEVEL >= 5) ? '<br /><br />' . 'GET=<pre>' . print_r(_botcha_filter_form_values_log($_GET), 1) . '</pre>' : '')
              . ((BOTCHA_LOGLEVEL >= 5) ? '<br /><br />' . 'SERVER=<pre>' . print_r($_SERVER, 1) . '</pre>' : '')
              . ((BOTCHA_LOGLEVEL >= 5) ? '<br /><br />' . ' values=<pre>' . print_r(_botcha_filter_form_values_log($form_state['values']), 1) . '</pre>' : ''),
          ),
          WATCHDOG_WARNING
          );
        }
        $rules_event_name = 'botcha_form_rejected';
        break;
    }
    // Invoke rules event.
    if (module_exists('rules')) {
      $arguments = array(
  //      'form' => &$form,
  //      'form_state' => &$form_state,
        'form_id' => $form['#id'],
        'total_recipes' => count($this->recipes),
        'passed_recipes' => $recipes_success_count,
        'passed_recipes_names' => join(', ', array_keys($recipes_success)),
        // !!~ @todo handle Add last recipe name.
        //'last_recipe_name' => $recipe->name,
        // !!~ @todo Recipebook handle Add a reason of fail to rules event invokation.
        //'fail' => $fail,
        'fail' => 'FAIL',
        'failed_field' => 'mail',
        // !!~ @todo Provide an error_text from Recipebook level.
        //'failed_error' => $recipe->error_text,
      );
      // !!? Do we need per recipe rules event invoking?
      rules_invoke_event($rules_event_name, $arguments);
    }
  }

  protected function getRecipeSecret($value) {
    return md5($value . BOTCHA_SECRET);
  }

  /**
   * Spam check.
   * Currently the logic is as follows: if we could find a recipe that failed
   * spam check - then we consider this form submission as spam and decline it.
   *
   * @param array $form
   * @param array $form_state
   * @return boolean
   */
  function isSpam($form, $form_state) {
    $isSpam = FALSE;
    foreach ($this->getRecipes() as $recipe) {
      if ($recipe->isSpam($form, $form_state)) {
        // It is used during getting the list of failed recipes.
        $recipe->status = 'spam';
        // Do per recipe handling right here. Global handling will be done later.
        $recipe->handle('spam', $form, $form_state);
        $isSpam = TRUE;
      }
      else {
        $recipe->status = 'success';
        $recipe->handle('success', $form, $form_state);
      }
    }
    return $isSpam;
  }

  protected function getCsss() {
    $csss = array();
    foreach ($this->getRecipes() as $recipe) {
      if ($css = $recipe->getCss()) {
        $csss[] = $css;
      }
    }
    return $csss;
  }

  protected function getJss() {
    $jss = array();
    foreach ($this->getRecipes() as $recipe) {
      if ($recipe instanceof BotchaRecipeUsingJsAbstract) {
          $jss[] = $recipe->getJsValue();
      }
    }
    return $jss;
  }

  function apply(&$form, &$form_state) {
    $form_state['no_cache'] = TRUE;
    $recipe_way = array();
    // !!~ @todo Reduce code duplication.
    // @see BotchaRecipe:applyRecipe
    $build_id_submit = isset($_POST['form_build_id']) ? $_POST['form_build_id'] : FALSE;
    $build_id = $form['#build_id'];
    if ($build_id_submit) {
      $recipe_way['build_id_submit'] = $build_id_submit;
    }
    $recipe_way['build_id'] = $build_id;
    foreach ($recipe_way as $key => $value) {
      $this->applyForBuildId($form, $form_state, $key, $value);
    }
    // User_login forms open session in validate hooks instead of submit
    // we should be the first to validate - add our hook to the beginning
    if (is_array($form['#validate'])) {
      // Workaround since array_unshift'ing by reference was deprecated.
      // @see http://www.php.net/manual/ru/function.array-unshift.php#40270
      array_unshift($form['#validate'], '');
      $form['#validate'][0] = '_botcha_form_validate';
    }
    else {
      $form['#validate'] = array('_botcha_form_validate');
    }
    $form_state['#botcha'] = $this->id;
    // Logging.
    $csss = $this->getCsss();
    $jss = $this->getJss();
    if (BOTCHA_LOGLEVEL >= 4) {
      watchdog(BOTCHA_LOG,
      '%form_id form prepared by BOTCHA: added recipes - !botchas!more',
      array(
        '%form_id' => $this->settings['form_id'],
        '!botchas' => join(', ', $this->recipes),
        '!more' => ''
          . ((BOTCHA_LOGLEVEL >= 5) ? '<br /><br />' . 'POST=<pre>' . print_r(_botcha_filter_form_values_log($_POST), 1) . '</pre>' : '')
          . ((BOTCHA_LOGLEVEL >= 5) ? '<br /><br />' . 'GET=<pre>' . print_r(_botcha_filter_form_values_log($_GET), 1) . '</pre>' : '')
          . ((BOTCHA_LOGLEVEL >= 5) ? '<br /><br />' . 'SERVER=<pre>' . print_r($_SERVER, 1) . '</pre>' : '')
          . ((BOTCHA_LOGLEVEL >= 5) ? '<br /><br />' . 'form=<pre>' . print_r(_botcha_filter_form_log($form), 1) . '</pre>' : '')
          . ((BOTCHA_LOGLEVEL >= 5 && count($jss)) ? '<br /><br />' . 'JS=<pre>' . join("\n", $jss) . '</pre>' : '')
          . ((BOTCHA_LOGLEVEL >= 5 && count($csss)) ? '<br /><br />' . 'CSS=<pre>' . join("\n", $csss) . '</pre>' : ''),
      ),
      WATCHDOG_NOTICE
    );
    }
  }

  protected function applyForBuildId(&$form, &$form_state, $key, $value) {
    $secret = $this->getRecipeSecret($value);
    $method = $key;
    foreach ($this->getRecipes() as $recipe) {
      // Set necessary parameters.
      $recipe->setSecret($secret)
        ->setMethod($method);
      // Apply recipe to the concrete form.
      $recipe->applyRecipe($form, $form_state);
    }
  }
}


/**
 * Dummy class, created for data consistency and for interface unifying.
 * When there is no recipe book binded to form, this class is used as a handler.
 * It has no logic at all - by design.
 */
class BotchaRecipebookNone extends BotchaRecipebook {
  public function __construct($id = NULL) {
    $this->id = 'none';
    $this->title = 'None';
  }

  public function isApplicable(&$form, &$form_state) {
    // It is for admin links to be placed.
    parent::isApplicable($form, $form_state);
    return FALSE;
  }
}
